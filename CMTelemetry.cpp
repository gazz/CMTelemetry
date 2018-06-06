// CMTelemetry.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN sizeof(UDPPacket) //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data


void update_sim_sli(hid_device *device, UDPPacket telemetry) {

	_SLI_MboardOUT out_data;
	memset(&out_data, 0, sizeof(out_data));
	out_data.ReportType = 1;
	if (telemetry.m_gear < 0)
		out_data.Gear = 82;
	else
		out_data.Gear = (int)telemetry.m_gear;

	// revs
	float max_revs = telemetry.m_max_rpm, start_revs = max_revs / 2;
	float revs_per_light = start_revs / 13;
	for (int i = 0; i < 13; ++i) {
		out_data.RPMLED[i] = telemetry.m_engineRate > start_revs + i * revs_per_light ? 1 : 0;
	}

	// shift lights
	int shift_lights = telemetry.m_engineRate > telemetry.m_max_rpm * .9 ? 1 : 0;
	out_data.LED14 = shift_lights;
	out_data.LED15 = shift_lights;
	out_data.LED16 = shift_lights;
	out_data.LED17 = shift_lights;
	out_data.LED18 = shift_lights;
	out_data.LED19 = shift_lights;
	
	
	int res;
	res = hid_write(device, (const unsigned char *)&out_data, sizeof(out_data));
	if (res < 0) {
		printf("Unable to write()\n");
		printf("Error: %ls\n", hid_error(device));
	}
}


void listen_udp(hid_device *device) {
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	UDPPacket telemetryPacket;
	WSADATA wsa;

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Bind done on port: %s:%d\n", server.sin_addr, PORT);

	//keep listening for data
	while (1)
	{
		//clear the buffer by filling null, it might have previously received data
		memset(&telemetryPacket, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, (char*)&telemetryPacket, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//print details of the client/peer and the data received
		//printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		//printf("Gear: %f, Revs: %f, Max Revs: %f, Idle Revs: %f\n", 
		//	telemetryPacket.m_gear, telemetryPacket.m_engineRate, telemetryPacket.m_max_rpm, telemetryPacket.m_idle_rpm);

		update_sim_sli(device, telemetryPacket);

		//now reply the client with the same data
		//if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
		//{
		//	printf("sendto() failed with error code : %d", WSAGetLastError());
		//	exit(EXIT_FAILURE);
		//}
	}

	closesocket(s);
	WSACleanup();

}


hid_device *find_sim_sli() {

	int res;
	unsigned char buf[256];
#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;

	struct hid_device_info *devs, *cur_dev;

	devs = hid_enumerate(0x1dd2, 0x0);
	cur_dev = devs;
	while (cur_dev) {
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n", cur_dev->interface_number);
		printf("\n");
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	// Set up the command buffer.
	memset(buf, 0x00, sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x81;


	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
	handle = hid_open(0x1dd2, 0x0102, NULL);
	if (!handle) {
		printf("unable to open device\n");
		return NULL;
	}

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read indexed string 1\n");
	printf("Indexed String 1: %ls\n", wstr);

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 1);

	// Try to read from the device. There shoud be no
	// data here, but execution should not block.
	res = hid_read(handle, buf, 17);

	// Read a Feature Report from the device
	buf[0] = 0x2;
	res = hid_get_feature_report(handle, buf, sizeof(buf));
	if (res < 0) {
		printf("Unable to get a feature report.\n");
		printf("%ls", hid_error(handle));
	}
	else {
		// Print out the returned buffer.
		printf("Feature Report\n   ");
		for (i = 0; i < res; i++)
			printf("%02hhx ", buf[i]);
		printf("\n");
	}

	memset(buf, 0, sizeof(buf));

	_SLI_MboardOUT out_data;
	memset(&out_data, 0, sizeof(out_data));
	out_data.ReportType = 1;

	out_data.Gear = 45;
	res = hid_write(handle, (const unsigned char *)&out_data, sizeof(out_data));
	if (res < 0) {
		printf("Unable to write()\n");
		printf("Error: %ls\n", hid_error(handle));
	}
	else {
		printf("Successfully written to device");
	}

	return handle;
}

void dummy_package(hid_device *device) {
	UDPPacket testPacket;
	memset(&testPacket, 0, sizeof(testPacket));
	testPacket.m_gear = 3.0;
	testPacket.m_engineRate = 580;
	testPacket.m_max_rpm = 600;
	update_sim_sli(device, testPacket);
}

int main()
{
	hid_device *device = find_sim_sli();

	//dummy_package(device);
	
	listen_udp(device);

	hid_close(device);
	hid_exit();

	return 0;
}
