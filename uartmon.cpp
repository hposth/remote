#include <iostream>
#include <windows.h>
#include <stdio.h>

const char* INIPath = "./uartmon.ini";

TCHAR PORT_ADDR[64];
const int MAX_DATA_LENGTH = GetPrivateProfileInt("UARTMON", "MAX_DATA_LENGTH", 18, INIPath);
const int BAUD_RATE = GetPrivateProfileInt("UARTMON", "BAUD_RATE", 115200, INIPath);
const int BYTE_SIZE = GetPrivateProfileInt("UARTMON", "BYTE_SIZE", 8, INIPath);
const int STOP_BITS = GetPrivateProfileInt("UARTMON", "STOP_BITS", 0, INIPath);
const int PARITY = GetPrivateProfileInt("UARTMON", "PARITY", 0, INIPath);

const int MOUSE_SENSITIVITY_X = GetPrivateProfileInt("REMOTE", "MOUSE_SENSITIVITY_X", 1.5, INIPath);
const int MOUSE_SENSITIVITY_Y = GetPrivateProfileInt("REMOTE", "MOUSE_SENSITIVITY_Y", 2.5, INIPath);
const int SMA_FACTOR = GetPrivateProfileInt("REMOTE", "SMA_FACTOR", 20, INIPath);
const int CLICK_DURATION = GetPrivateProfileInt("REMOTE", "CLICK_DURATION", 20, INIPath);

int main() {
	GetPrivateProfileString(
		TEXT("UARTMON"),
		TEXT("PORT_ADDR"),
		TEXT("\\\\.\\COM3"),
		PORT_ADDR,
		64,
		INIPath);

	std::cout << "uartmon" << std::endl << std::endl;
	std::cout << "Attaching to " << PORT_ADDR << "..." << std::endl;

	DCB dcb;
	int retVal;
	//byte data[18] = { 0 };
	DWORD
		bytesTransferred,
		modemStatus;

	HANDLE hPort = CreateFile(
		(char*)PORT_ADDR,
		GENERIC_READ,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0
	);

	if (!GetCommState(hPort, &dcb))
		std::cout << "E" << 0x000A << std::endl;

	dcb.BaudRate = BAUD_RATE;
	dcb.ByteSize = BYTE_SIZE;
	dcb.StopBits = STOP_BITS;
	dcb.Parity = PARITY;

	if (!SetCommState(hPort, &dcb))
		std::cout << "E" << 0x0001 << std::endl;

	SetCommMask(hPort, 0x0001 | 0x0080);

	while (true)
	{
		byte data[18] = { 0 };
		WaitCommEvent(hPort, &modemStatus, 0);
		if (modemStatus & EV_RXCHAR) {
			if (ReadFile(hPort, &data, MAX_DATA_LENGTH, &bytesTransferred, 0)) {
				if (data[0] == '(')
				{
					//std::cout << " " << &data;
					//for (int i = 0; i < MAX_DATA_LENGTH; i++)
					//	printf(" %.2X", data[i]);
					std::cout << data << std::endl;
				}
				else
					PurgeComm(hPort, 0x0002 | 0x008 | 0x0001 | 0x0004);
			}
		}
	}
}