#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Maths.h"



//---------- very fast (indeed) random number generator --------//
// requires 32 bit int representation
// Used by inline Rnd functions.

namespace RandQD1 {
	unsigned int idum;

	// Test whether the machine handles ints the right way. The value of variable 'idum'
	// should follow this pattern: 
	// 00000000,3c6ef35f,47502932,d1ccf6e9,aaf95334,6252e503
	// 9f2ec686,57fe6c2d,a3d95fa8,81fdbee7,94f0af1a,cbf633b1
	bool PlatformTest () {
		// Test with shorts (sign bit masked off)
		// For hi bit masked off:
		//unsigned int testInts[11] = { 0x3c6e, 0x4750, 0x51cc, 0x2af9, 0x6252, 0x1f2e, 0x57fe, 0x23d9, 0x01fd, 0x14f0, 0x4bf6};
		// For extra shift:
		unsigned int testInts[11] = { 0x1e37, 0x23a8, 0x68e6, 0x557c, 0x3129, 0x4f97, 0x2bff, 0x51ec, 0x40fe, 0x4a78, 0x65fb};
		unsigned int a=0;
		int i=0;
		seed( 0 );
		bool Passed = true;
		while(i<11){
			a = RandQD1::rand();
			if (testInts[i] != a)
				Passed = false;
			i++;
		}

		// Bin values from 0 to 255 and display results
		i = 0;
		int EachNum[256];
		while(i<256) EachNum[i++]=0;
		i=0;
		while(i<5000) {
			a = RandQD1::rand();
			a *= (256);
			a = a >> RandQD1::MAX_RAND_SHIFT;
			if ( (a<0) || (a>255) ) {
				Passed = false;
				continue;
			}
			EachNum[a]++;
			i++;
		}
		i = 0;
		int mini=256;
		int maxi=-1;
		unsigned tot=0;
		while(i<256) {
			int n = EachNum[i];
//			TRACE("%3.3d ",n);
			if (n>maxi) maxi = n;
			if (n<mini) mini = n;
			tot += n;
			if(n==0) Passed=false;
			i++;
		}
//		TRACE("\n");
		return Passed;
	}
}


/*
// Used to confirm propoer operation of the new random number generator
bool RndPlatformTest () {
	int a=0;
	int i=0;
	bool Passed=TRUE;
	int min=0;
	int max=0;
	int tot=0;

#define TNUMS 25

	// Bin values from 0 to 24 and display results
	i = 0;
	int EachNum[TNUMS];
	while(i<TNUMS) EachNum[i++]=0;
	i=0;
	while(i<5000) {
		a = Rnd(TNUMS-1);
		if ( (a<0) || (a>TNUMS-1) ) {
			Passed = FALSE;
			continue;
		}
		EachNum[a]++;
		i++;
	}
	i = 0;
	min=TNUMS;
	max=-1;
	tot=0;
	while(i<TNUMS) {
		int n = EachNum[i];
		TRACE("%3.3d ",n);
		if (n>max) max = n;
		if (n<min) min = n;
		tot += n;
		if(n==0) Passed=FALSE;
		i++;
	}
	TRACE("\n");

	// Bin values from 10 to 34 and display results
	i = 0;
	while(i<TNUMS) EachNum[i++]=0;
	i=0;
	while(i<5000) {
		a = Rnd(10,34);
		if ( (a<10) || (a>34) ) {
			Passed = FALSE;
			continue;
		}
		EachNum[a-10]++;
		i++;
	}
	i = 0;
	min=999;
	max=-1;
	tot=0;
	while(i<TNUMS) {
		int n = EachNum[i];
		TRACE("%3.3d ",n);
		if (n>max) max = n;
		if (n<min) min = n;
		tot += n;
		if(n==0) Passed=FALSE;
		i++;
	}
	TRACE("\n");

		// Bin values from -7 to 17 and display results
	i = 0;
	while(i<TNUMS) EachNum[i++]=0;
	i=0;
	while(i<5000) {
		a = Rnd(-7,17);
		if ( (a<-7) || (a>17) ) {
			Passed = FALSE;
			continue;
		}
		EachNum[a+7]++;
		i++;
	}
	i = 0;
	min=999;
	max=-1;
	tot=0;
	while(i<TNUMS) {
		int n = EachNum[i];
		TRACE("%3.3d ",n);
		if (n>max) max = n;
		if (n<min) min = n;
		tot += n;
		if(n==0) Passed=FALSE;
		i++;
	}
	TRACE("\n");

	// Bin values from 0 to 2 and display results
	TRACE("waS\n");
	i = 0;
	while(i<TNUMS) EachNum[i++]=0;
	i=0;
	while(i<640) {
		//a = 2 - abs(Rnd(2) + Rnd(2) - 2);
		a = Rnd(2);
		Rnd(2);
		a = 2 - abs(a+Rnd(2)-2);
		if ( (a<0) || (a>3) ) {
			Passed = FALSE;
			continue;
		}
		EachNum[a]++;
		i++;
	}
	i = 0;
	min=999;
	max=-1;
	tot=0;
	while(i<3) {
		int n = EachNum[i];
		TRACE("%3.3d ",n);
		if (n>max) max = n;
		if (n<min) min = n;
		tot += n;
		if(n==0) Passed=FALSE;
		i++;
	}
	TRACE("\n");

	TRACE("Saw\n");
	// Bin values from 0 to 2 and display results
	i = 0;
	while(i<TNUMS) EachNum[i++]=0;
	i=0;
	while(i<640) {
		//a = abs(Rnd(2) + Rnd(2) - 2);
		a = Rnd(2);
		Rnd(2);
		a = abs(a+Rnd(2)-2);
		if ( (a<0) || (a>3) ) {
			Passed = FALSE;
			continue;
		}
		EachNum[a]++;
		i++;
	}
	i = 0;
	min=999;
	max=-1;
	tot=0;
	while(i<3) {
		int n = EachNum[i];
		TRACE("%3.3d ",n);
		if (n>max) max = n;
		if (n<min) min = n;
		tot += n;
		if(n==0) Passed=FALSE;
		i++;
	}
	TRACE("\n");
	
	TRACE("Flat\n");
		// Bin values from 0 to 2 and display results
	i = 0;
	while(i<TNUMS) EachNum[i++]=0;
	i=0;
	while(i<640) {
		a = Rnd(2);
		if ( (a<0) || (a>3) ) {
			Passed = FALSE;
			continue;
		}
		EachNum[a]++;
		i++;
	}
	i = 0;
	min=999;
	max=-1;
	tot=0;
	while(i<3) {
		int n = EachNum[i];
		TRACE("%3.3d ",n);
		if (n>max) max = n;
		if (n<min) min = n;
		tot += n;
		if(n==0) Passed=FALSE;
		i++;
	}
	TRACE("\n");

	TRACE("Normal\n");
		// Bin values from 0 to 2 and display results
	i = 0;
	while(i<TNUMS) EachNum[i++]=0;
	i=0;
	while(i<640) {
		//a = (Rnd(2) + Rnd(2)) / 2;
		a = Rnd(2);
		Rnd(2);
		a = (a+Rnd(2))/2;
		if ( (a<0) || (a>3) ) {
			Passed = FALSE;
			continue;
		}
		EachNum[a]++;
		i++;
	}
	i = 0;
	min=999;
	max=-1;
	tot=0;
	while(i<3) {
		int n = EachNum[i];
		TRACE("%3.3d ",n);
		if (n>max) max = n;
		if (n<min) min = n;
		tot += n;
		if(n==0) Passed=FALSE;
		i++;
	}
	TRACE("\n");
return Passed;
}
*/

