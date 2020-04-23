#include <Windows.h>
#include <stdio.h>
#include "sma.h"

// Map accelerometer values to on-screen points
int map(int x, int in_min, int in_max, int out_min, int out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Get first index of non-padding char in chararray
int sub(char c[], char r, int l) {
	char* s = strchr(c, r);
	int pos = 0;
	for (int i = 0; s != NULL && i < l; i++) {
		s = s + 1;
		if (*s != r)
			s = NULL;
		pos++;
	}
	return pos;
}

// Null-terminate string
char* nts(char* str, int l) {
	char* buf = str;
	buf[l] = '\0';
	return buf;
}

int main(void) {
	// Definitions
	HANDLE hPort;
	DWORD bytesTransferred; 
	DWORD dwEventMask;
	char data[15];
	BOOL isLDown = FALSE, isRDown = FALSE;
	char port[] = "\\\\.\\COM4";

	// Opening the serial port
	hPort = CreateFile(
		L"\\\\.\\COM4",
		GENERIC_READ | GENERIC_WRITE, 
		0, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL
	);

	if (hPort == INVALID_HANDLE_VALUE)
		printf("E%i\n", 0x000A);

	DWORD MsgID = GetLastError();

	// Setting the parameters for the serial port
	DCB dcb = { 0 };
	dcb.DCBlength = sizeof(dcb);

	if (GetCommState(hPort, &dcb) == FALSE)
		printf("E%i\n", 0x0000);

	dcb.BaudRate = CBR_115200;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity	 = NOPARITY;

	if (SetCommState(hPort, &dcb) == FALSE)
		printf("E%i\n", 0x0001);

	// Setting serial port timeouts
	COMMTIMEOUTS cto = { 0 };
	cto.ReadIntervalTimeout = 13000 / dcb.BaudRate + 1;
	cto.ReadTotalTimeoutConstant = 13000 / dcb.BaudRate + 1;
	cto.ReadTotalTimeoutMultiplier = 10;
	
	if (SetCommTimeouts(hPort, &cto) == FALSE)
		printf("E%i\n", 0x0002);
	
	// Setting serial port receive mask
	if (SetCommMask(hPort, 0x0001) == FALSE)
		printf("E%i\n", 0x0003);
	else
		printf("Established %s\n", port);

	// Establish things needed for the loop
	int sma_x[20] = { 0 };
	int sma_y[20] = { 0 };

	int l[4] = { 6, 6, 1, 1 };					// a collection of lengths of the arguments assuming sizeof char = 1
	char
		* vx = malloc(l[0] + sizeof(char)),		// a few char pointers to act as buffers for parsing the raw data:
		* vy = malloc(l[1] + sizeof(char)),		// value x, value y, value lmb, value rmb
		* vl = malloc(l[2] + sizeof(char)),
		* vr = malloc(l[3] + sizeof(char));

	// Doing main loop thingy
	for(;;){
		WaitCommEvent(hPort, &dwEventMask, NULL);
		if (dwEventMask & 0x0001)
		{
			if (ReadFile(hPort, &data, 15, &bytesTransferred, NULL)) {
				if (data[0] == 'n')
				{
					// Copy parts of data into respective value buffer based on known offsets (0x01, 0x06, 0x0B and 0x0C)
					strncpy(vx, data + 0x01, l[0]);
					strncpy(vy, data + 0x07, l[1]);
					strncpy(vl, data + 0x0D, l[2]);
					strncpy(vr, data + 0x0E, l[3]);

					// Turn plain chairs into proper null-terminated strings to prevent overflow
					vx = nts(vx, l[0]);
					vy = nts(vy, l[1]);
					vl = nts(vl, l[2]);
					vr = nts(vr, l[3]);

					// Get index of first non-padding char in x and y
					int xf = sub(vx, '=', 6);					// x-first
					int yf = sub(vy, '=', 6);					// y-first

					// Strip padding
					vx += xf;									// x + size of padding 
					vy += yf;									// remember vx is a charptr so adding to it will just move the ptr address and never writes to the memory in which vx is stored

					// Convert string literals to integers
					int x = atoi(vx),
						y = atoi(vy),
						lmb = atoi(vl),
						rmb = atoi(vr);

					// Calculate Simple Moving Average
					shift(&sma_x, 20, 1);
					sma_x[0x13] = x;
					shift(&sma_y, 20, 1);
					sma_y[0x13] = y;

					int sx = average(&sma_x[0], 20),
						sy = average(&sma_y[0], 20);

					int rx = map(sx * 1.5, -0x500, 0x500, 0, 1920),
						ry = map(sy * 2.5, -0x500, 0x500, 0, 1080);

					// MOVE
					SetCursorPos(rx, ry);

					// Handle mouse presses
					if (lmb == 1 && rmb == 1) {
						memset(&sma_x, 0x00, 20);				// Reset shit
						memset(&sma_y, 0x00, 20);
					}
					else {
						if (lmb == 1) {
							if (!isLDown) {
								mouse_event(MOUSEEVENTF_LEFTDOWN, rx, ry, 0, 0);
								Sleep(20);
								isLDown = 1;
							}
						}
						else {
							if (isLDown) {
								mouse_event(MOUSEEVENTF_LEFTUP, rx, ry, 0, 0);
								isLDown = 0;
								lmb = 0;
							}
						}
						if (rmb == 1) {
							if (!isRDown) {
								mouse_event(MOUSEEVENTF_RIGHTDOWN, rx, ry, 0, 0);
								Sleep(20);
								isRDown = 1;
							}
						}
						else {
							if (isRDown) {
								mouse_event(MOUSEEVENTF_RIGHTUP, rx, ry, 0, 0);
								isRDown = 0;
								rmb = 0;
							}
						}
					}
				}
			}
		}
	}

	CloseHandle(hPort);
	return 0;
}
