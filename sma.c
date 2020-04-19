#include "sma.h"

void shift(int* colptr, int factor, int places) {
	for (int i = 0; i < factor - places; i++) {
		*(colptr + i) = *(colptr + i + places);
	}
	for (int i = factor - places; i < factor; i++) {
		*(colptr + i) = 0x00;
	}
}

int average(int* colptr, int factor) {
	int sum = 0;
	for (int i = 0; i < factor; i++) {
		sum += *(colptr + i);
	}
	return sum / factor;
}