#include "X10Protocol.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
using namespace std;

X10Protocol::X10Protocol() {
	this->lockPosition = 0;
	this->globalPositon = 0;
	this->hasReceivedData = 0;
}

int X10Protocol::lockPassed() {
	if (this->lockPosition == 3) {
		return 1;
	} else {
		return 0;
	}
}

int X10Protocol::handleLock(long duration) {
	if (this->lockPosition == 0) {
		if (duration > 26000 && duration < 28000) {
			this->changes[0] = duration;
			this->lockPosition = 1;
			this->globalPositon = 1;
		} 
	} else if (this->lockPosition == 1) {
		if (duration > 7000 && duration < 9000) {
			this->changes[1] = duration;
			this->lockPosition = 2;
			this->globalPositon = 2;
		}
	} else if (this->lockPosition == 2) {
		if (duration > 3000 && duration < 5000) {
			this->changes[2] = duration;
			this->lockPosition = 3;
			this->globalPositon = 3;
			this->hasReceivedData = 0;
		}
	} else if (this->lockPassed() == 0) {
		this->reset();
	}

	return this->lockPosition;
}

int X10Protocol::readAndStoreChange(long duration) {
	if (this->lockPassed() == 0) {
		this->reset();
		return 0;
	} else {
		//Store
		if (this->globalPositon < X10PROTOCOL_MAX_CHANGES) {
			this->changes[this->globalPositon++] = duration;
			return 1;
		} else {
			this->hasReceivedData = 1;
			this->reset();
		}
	}
}
int X10Protocol::hasData() {
	return this->hasReceivedData;
}

unsigned char* X10Protocol::readAsByteArray() {
	unsigned char* bytes = new unsigned char[4];
	//reset 
	for (int i=0; i<4; i++) {
		bytes[i] = 0;
	}
	//read
	if (this->hasReceivedData == 1) {
		for (int j=0; j<4; j++) {
			int pos = 7;
		    for (int i=3 + (j*8); i < 3 + (j*8) + 8*2; i=i+2) {
		    	int val = this->changesToBit(this->changes[i + (j*8)], this->changes[i+(j*8)+1]);
		    	if (val == 1) {
		    		bytes[j] |= 1 << pos;
		    	}
		    	pos --;
		    }
		}
	}
	return bytes;
}

char* X10Protocol::readAsHumanReadable() {
	if (this->hasReceivedData == 1) {
		unsigned char *bytes = this->readAsByteArray();
		//check complement
		if ((bytes[0] | bytes[1] == 0xff) && (bytes[2] | bytes[3] == 0xff)) {
			//data are ok
			switch(bytes[0]) {
				case 96:
					switch(bytes[2]) {
						case 0 : return (char*)"A1 ON"; 
						case 32 : return (char*)"A1 OFF";
						case 16 : return (char*)"A2 ON"; 
						case 48 : return (char*)"A2 OFF";
						case 8 : return (char*)"A3 ON"; 
						case 40 : return (char*)"A3 OFF";
						case 24 : return (char*)"A4 ON"; 
						case 56 : return (char*)"A4 OFF";
						case 64 : return (char*)"A5 ON"; 
						case 96 : return (char*)"A5 OFF";
						case 80 : return (char*)"A6 ON"; 
						case 112 : return (char*)"A6 OFF"; 
						case 72 : return (char*)"A7 ON"; 
						case 104 : return (char*)"A7 OFF";
						case 88 : return (char*)"A8 ON"; 
						case 120 : return (char*)"A8 OFF";

						case 136 : return (char*)"A BRIGHT 005";
						case 152 : return (char*)"A DIM 005";
					}
				case 100:
					switch(bytes[2]) {
						case 0 : return (char*)"A9 ON"; 
						case 32 : return (char*)"A9 OFF";
						case 16 : return (char*)"A10 ON"; 
						case 48 : return (char*)"A10 OFF";
						case 8 : return (char*)"A11 ON"; 
						case 40 : return (char*)"A11 OFF";
						case 24 : return (char*)"A12 ON"; 
						case 56 : return (char*)"A12 OFF";
						case 64 : return (char*)"A13 ON"; 
						case 96 : return (char*)"A13 OFF";
						case 80 : return (char*)"A14 ON"; 
						case 112 : return (char*)"A14 OFF"; 
						case 72 : return (char*)"A15 ON"; 
						case 104 : return (char*)"A15 OFF";
						case 88 : return (char*)"A16 ON"; 
						case 120 : return (char*)"A16 OFF";
					} 
			}
			return (char*)"TODO";
		} else {
			return (char*)"INVALID DATA";
		}
	} else {
		return (char*)"NO DATA";
	} 
}

void X10Protocol::displayHasBinary() {
	if (this->hasReceivedData == 1) {
		for (int i=3; i < X10PROTOCOL_MAX_CHANGES; i=i+2) {
	        if ((i-3)%16==0) {
	          //space each 8bits
	          cout << " ";
	        }
	        cout << this->changesToBit(this->changes[i], this->changes[i+1]);
	    }
	    cout << endl;
	} else {
		cout << "NO DATA" << endl;
	}
}

void X10Protocol::displayHasHexa() {
	if (this->hasReceivedData == 1) {
		unsigned char *bytes = this->readAsByteArray();
		for (int i=0; i< 4; i++) {
			printf("%02x ", bytes[i] & 0xff);
		}
		cout << endl;
	}
}


void X10Protocol::displayHasHumanReadable() {
	if (this->hasReceivedData == 1) {
		cout << this->readAsHumanReadable();
		cout << endl;
	}
}

/** -------------------------- PRIVATE --------------------------- **/

int X10Protocol::changesToBit(long duration1, long duration2) {
	if (duration1 > 350 && duration1 < 650 &&
	    duration2 > 350 && duration2 < 650  ) {
		return 0;
	} else if (duration1 > 350 && duration1 < 650 &&
	    duration2 > 1350 && duration2 < 1650  ) {
		return 1;
	}
	//defautly says 1
	return 1;
}


void X10Protocol::reset() {
	this->lockPosition = 0;
	this->globalPositon = 0;
}