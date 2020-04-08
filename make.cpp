#include <iostream>
#include <fstream>
#include <windows.h>

int main() {
	system("g++ ./main.cpp -o ./cdriver.exe");
	system("g++ ./uartmon.cpp -o ./uartmon.exe");
	system("csc ./bootloader.cs -out:./flash.exe -nologo");
	
	std::ofstream ini("uartmon.ini");
	ini << "[UARTMON]\nMAX_DATA_LENGTH=18\nBAUD_RATE=115200\nBYTE_SIZE=8\nSTOP_BITS=0\nPARITY=0\n\n[REMOTE]\nMOUSE_SENSITIVITY_X=1.5\nMOUSE_SENSITIVITY_Y=2.5\nSMA_FACTOR=20\nCLICK_DURATION=20";
	return 0;
}