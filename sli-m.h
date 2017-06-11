#pragma once

// OUTPUT
typedef struct _SLI_MboardOUT {
	BYTE ReportID;      //  always 0
	BYTE ReportType;      //  type of report  1
	BYTE Gear;                 //  gear (revert display not implemented)
	BYTE RPMLED[13];      //  RPM leds (value = 0 or 1)
	BYTE LED14;         //  RED (Damage/heat/Hard Warning/...)
	BYTE LED15;         //  BLUE (optimal shiftpoint feedback)
	BYTE LED16;         //  YELLOW (low fuel/Yellow flag/...)
	BYTE LED17;         //  BLUE (optimal shiftpoint feedback)
	BYTE LED18;         //  YELLOW (Speedlimiter/Yellow Flag/...)
	BYTE LED19;         //  RED (TC/...)
	BYTE LEDExternal[5];           //  External leds 5
	BYTE Spare[27];
} _SLI_MboardOUT, *P_SLI_MboardOUT;


// Global Brightness
typedef struct _SLI_MboardOUT2 {
	BYTE ReportID;      // always 0
	BYTE ReportType;   // type of report  2
	BYTE GlobalBrightness;   // Value from 0 to 255
	BYTE Spare[51];
} _SLI_MboardOUT2, *P_SLI_MboardOUT2;

// INPUT (Btns)
typedef struct _SLI_MboardIN {
	BYTE ID; // zero
	BYTE Btn[2];   // digital IN first byte from 1 to 8 and second from 9 to 16
} _SLI_MboardIN, *P_SLI_MboardIN;
