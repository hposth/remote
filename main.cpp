#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <math.h>
#include "SMAVal.h"
using namespace std;

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

int map(double x, int in_min, int in_max, int out_min, int out_max)
{
	return (int)(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int main(void) {
	GetPrivateProfileString(
		TEXT("UARTMON"),
		TEXT("PORT_ADDR"),
		TEXT("\\\\.\\COM3"),
		PORT_ADDR,
		64,
		INIPath);
	
	std::cout << "AxialRemote" << std::endl << std::endl;
	std::cout << "Attaching to " << PORT_ADDR << "..." << std::endl;
	
	DCB dcb;
	int retVal;
	byte data[MAX_DATA_LENGTH] = { 0 };
	DWORD 
		bytesTransferred,
		modemStatus;

	HANDLE hPort = CreateFile(
		PORT_ADDR, 
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

	std::vector<SMAVal> SMA(SMA_FACTOR);

	bool 
		isLMBCurrentlyDown = false,
		isRMBCurrentlyDown = false;
	while (true)
	{
		WaitCommEvent(hPort, &modemStatus, 0);
		if (modemStatus & EV_RXCHAR) {
			if (ReadFile(hPort, &data, MAX_DATA_LENGTH, &bytesTransferred, 0)) {
				try
				{
					int end = 0;
					for (int i = sizeof(data) - 1; i > 0; i--) //end
					{
						//Strip '=', '\r' and '\n'
						if (data[i] != 0x3d && data[i] != 0x0d && data[i] != 0x0a) {
							end = i + 1;
							break;
						}
					}
					string content;
					content.resize(MAX_DATA_LENGTH - (MAX_DATA_LENGTH - end) - 2);
					for (int i = 1; i < MAX_DATA_LENGTH - (MAX_DATA_LENGTH - end) - 1; i++) {
						content[i - 1] = (char)data[i];
					}

					string sub;
					int x = 0,
						y = 0,
						lmb = 0,
						rmb = 0;
					std::istringstream iss(content);

					sub = content.substr(0, content.find(','));
					if (sub != "")
						x = std::stoi(sub);
					content.erase(0, content.find(',') + 1);

					sub = content.substr(0, content.find(','));
					if (sub != "")
						y = std::stoi(sub);
					content.erase(0, content.find(',') + 1);
						
					sub = content.substr(0, content.find(','));
					if (sub != "")
						lmb = std::stoi(sub);
					content.erase(0, content.find(',') + 1);

					sub = content.substr(0, content.find(','));
					if (sub != "")
						rmb = std::stoi(sub);

					// Calculate Simple Moving Average
					SMAVal xy = SMAVal(x, y);
					SMA.push_back(xy);
					int
						x_sum = 0,
						y_sum = 0,
						x_SMA = 0,
						y_SMA = 0;
					for (int i = 0; i < SMA.size(); i++)
					{
						if (i >= SMA_FACTOR)
						{
							x_sum -= SMA[i - SMA_FACTOR].x;
							y_sum -= SMA[i - SMA_FACTOR].y;
						}
						x_sum += SMA[i].x;
						y_sum += SMA[i].y;
						x_SMA = (int)(x_sum / SMA_FACTOR);
						y_SMA = (int)(y_sum / SMA_FACTOR);
					}

					int rx = map(x_SMA * MOUSE_SENSITIVITY_X, -1024, 1024, 0, 1920),
						ry = map(y_SMA * MOUSE_SENSITIVITY_Y, -1024, 1024, 0, 1080);
					SetCursorPos(rx, ry);
					if (lmb == 1) {
						if (!isLMBCurrentlyDown) {
							mouse_event(MOUSEEVENTF_LEFTDOWN, rx, ry, 0, 0);
							Sleep(CLICK_DURATION);
							isLMBCurrentlyDown = true;
						}
					}
					else {
						if (isLMBCurrentlyDown) {
							mouse_event(MOUSEEVENTF_LEFTUP, rx, ry, 0, 0);
							isLMBCurrentlyDown = false;
							lmb = 0;
						}
					}
					if (rmb == 1) {
						if (!isRMBCurrentlyDown) {
							mouse_event(MOUSEEVENTF_RIGHTDOWN, rx, ry, 0, 0);
							Sleep(CLICK_DURATION);
							isRMBCurrentlyDown = true;
						}
					}
					else {
						if (isRMBCurrentlyDown) {
							mouse_event(MOUSEEVENTF_RIGHTUP, rx, ry, 0, 0);
							isRMBCurrentlyDown = false;
							rmb = 0;
						}
					}
				}
				catch (exception e) {
					std::cout << e.what() << ": purging serial connection." << std::endl;
					PurgeComm(hPort,
						0x0002 | 0x008 | 0x0001 | 0x0004);
				}
			}
		}
		else if (modemStatus & EV_ERR)
			std::cout << "E" << 0x000F << std::endl;
			
		Sleep(1);
	}

	CloseHandle(hPort);
	return 0;
}