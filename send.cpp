#include <wiringPi.h>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <sched.h>
#include <sstream>

using namespace std;


int pin;
bool bit2[26]={};              // 26 bit Identifiant emetteur
bool bit2Interruptor[4]={}; 
int interruptor;
int sender;
string onoff;

void log(string a){
	cout << a << endl;
}

void scheduler_realtime() {
	struct sched_param p;
	p.__sched_priority = sched_get_priority_max(SCHED_RR);
	if( sched_setscheduler( 0, SCHED_RR, &p ) == -1 ) {
		perror("Failed to switch to realtime scheduler.");
	}
}

void scheduler_standard() {
	struct sched_param p;
	p.__sched_priority = 0;
	if( sched_setscheduler( 0, SCHED_OTHER, &p ) == -1 ) {
		perror("Failed to switch to normal scheduler.");
	}
}

void sendBit(bool b) {
	if (b) {
		digitalWrite(pin, HIGH);
		delayMicroseconds(310);   //275 originally, but tweaked.
		digitalWrite(pin, LOW);
		delayMicroseconds(1340);  //1225 originally, but tweaked.
	} else {
		digitalWrite(pin, HIGH);
		delayMicroseconds(310);   //275 originally, but tweaked.
		digitalWrite(pin, LOW);
		delayMicroseconds(310);   //275 originally, but tweaked.
	}
}

unsigned long power2(int power){
	unsigned long integer=1;
	for (int i=0; i<power; i++){
		integer*=2;
	}
	return integer;
} 

// Integer to binary
void itob(unsigned long integer, int length) {
	for (int i = 0; i < length; i++) {
		if ((integer / power2(length - 1 - i)) == 1) {
			integer -= power2(length - 1 - i);
			bit2[i] = 1;
		} else
			bit2[i] = 0;
	}
}

void itobInterruptor(unsigned long integer, int length) {
	for (int i = 0; i < length; i++) {
		if ((integer / power2(length - 1 - i)) == 1) {
			integer -= power2(length - 1 - i);
			bit2Interruptor[i] = 1;
		} else
			bit2Interruptor[i] = 0;
	}
}

void sendPair(bool b) {
	if (b) {
		sendBit(true);
		sendBit(false);
	} else {
		sendBit(false);
		sendBit(true);
	}
}

void transmit(int blnOn) {
	int i;

	digitalWrite(pin, HIGH);
	delayMicroseconds(275);
	digitalWrite(pin, LOW);
	delayMicroseconds(9900);   // first lock
	digitalWrite(pin, HIGH);   // high again
	delayMicroseconds(275);    // wait
	digitalWrite(pin, LOW);    // second lock
	delayMicroseconds(2675);
	digitalWrite(pin, HIGH);

	//Code from the emitor -- emitor ID
	for (i = 0; i < 26; i++) {
		sendPair(bit2[i]);
	}

	// 26th bit -- grouped command
	sendPair(false);

	// 27th bit -- On or off
	sendPair(blnOn);

	// 4 last bits -- reactor code 0000 -> 0 -- 0001 -> 1
	for (i = 0; i < 4; i++) {
		if (bit2Interruptor[i] == 0) {
			sendPair(false);
		} else {
			sendPair(true);
		}
	}

	digitalWrite(pin, HIGH);   // lock - end of data
	delayMicroseconds(275);    // wait
	digitalWrite(pin, LOW);    // lock - end of signal

}

/**
 * Usage:
 * send 0 12325261 1 on
 * 0 		-> connected pin
 * 12325261 -> Emitor code
 * 1		-> Receiver id
 * on		-> On or Off command
 */
int main(int argc, char** argv) {
	if (setuid(0)) {
		perror("setuid");
		return 1;
	}

	scheduler_realtime();
	log("Let's start dude...");
	pin = atoi(argv[1]);
	sender = atoi(argv[2]);
	interruptor = atoi(argv[3]);
	onoff = argv[4];
	//check for wiringPI
	if (wiringPiSetup() == -1) {
		log("Wiring Pi not installed... END");
		log("Please insall wiring PI : INSTRUCTIONS");
		log("======================================");
		log(">$ git clone git://git.drogon.net/wiringPi");
		log(">$ cd wiringPi/wiringPi");
		log(">$ sudo su");
		log(">$ ./build");
		return -1;
	}
	pinMode(pin, OUTPUT);
	itob(sender, 26);            // convert emitor code
	itobInterruptor(interruptor, 4);

	if (onoff == "on") {
		log("ON Signal");
		for (int i = 0; i < 5; i++) {
			transmit(true);            // ON
			delay(10);
		}

	} else {
		log("OFF Signal");
		for (int i = 0; i < 5; i++) {
			transmit(false);           // OFF
			delay(10);
		}
	}
	scheduler_standard();
}

