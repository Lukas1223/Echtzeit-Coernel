//============================================================================
// Name        : test.cpp
// Author      : Leon Urbas
// Version     :
// Copyright   : (c) 2008
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
using namespace std;

int x = 0;

void* test(void * arg) {
	int y = *(int *) arg;
	printf("y= %d \n", y);
	y++;
	*(int*) arg = y;
	printf("y= %d \n", y);
	return NULL;
}

int main() {
	cout << "Simulation" << endl; // prints Simulation
	printf("x= %d\n", x);
	test((void*) &x);
	printf("x= %d\n", x);

	int x[1];
	x[0] = 1;
	x[1] = 2;
	x[2] = 3;
	x[3] = 4;
	x[4] = 5;
	printf("%d\n%d\n%d\n%d\n%d\n",x[0],x[1],x[2],x[3],x[4]);
	return 0;
}
