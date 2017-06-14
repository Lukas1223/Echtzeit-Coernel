//============================================================================
// Name        : test.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <math.h>

using namespace std;

const double gAlpha = 0.00390802;
const double gBeta = 0.000000580195;
const double gThetaNull = 273.1;
const double gRNull = 100;

bool Rechnen(double r, double* temp)
{
	bool erfolg = false;

	double zwischen = 0;
	double zwischen2 = 0;
	double zwischen3 = 0;
	zwischen = (r/gRNull) - 1;
	zwischen = zwischen / gBeta;
	zwischen2 = gAlpha/(2*gBeta);
	zwischen3 = zwischen2*zwischen2;
	zwischen += zwischen3;
	zwischen = sqrt(zwischen);
	zwischen -= zwischen2;
	*temp = zwischen + gThetaNull;
	erfolg = true;

	return erfolg;
}


int main() {
	double temperatur = 0;
	char Puffer[100];
	for (int i = 0; i < 100; i++) {
		Puffer[i] = (char) 32;
	}

	double r = 110;
	Rechnen(r, &temperatur);
	printf("Die Temperatur beträgt: %f\n", temperatur);


	DWORD dw;
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	HANDLE handle;
	HANDLE handle2;

	handle = CreateFile("hallo2.txt", GENERIC_READ | GENERIC_WRITE, 0, NULL,
	OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		printf("Fehler beim CreateFile\n");
	}

	if (!ReadFile(handle, Puffer, 3, &dw, NULL)) {
		printf("Fehler im ReadFile\n");
	}

	handle2 = CreateFile("hallo.txt", GENERIC_READ | GENERIC_WRITE, 0, NULL,
	OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle2 == INVALID_HANDLE_VALUE) {
		printf("Fehler beim CreateFile2\n");
	}

	if (!WriteFile(handle2, Puffer, 100, &dw, NULL)) {
		printf("Fehler beim WriteFile\n");
	}

	bool x = false;
	x = CloseHandle(handle);
	if (x == false) {
		printf("Fehler beim CloseHandle\n");
	}
	x = CloseHandle(handle2);
	if (x == false) {
		printf("Fehler beim CloseHandle2\n");
	}

	return 0;
}
