#include <wiringPi.h>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <sched.h>
#include <sstream>

#include "X10Protocol.h"
static X10Protocol _x10protocol;


using namespace std;

 
void log(string a){
  cout << a ;
}

string longToString(long mylong){
    string mystring;
    stringstream mystream;
    mystream << mylong;
    return mystream.str();
}

string intToString(int myint){
    string mystring;
    stringstream mystream;
    mystream << myint;
    return mystream.str();
}

string measure2String(long duration){
    string mystring;
    stringstream mystream;
    mystream << duration;
    return mystream.str();
}

void handleInterrupt() {

  static unsigned int duration;
  static unsigned long lastTime;

  long time = micros();
  duration = time - lastTime;

  if (_x10protocol.lockPassed() == 0) {
    _x10protocol.handleLock(duration);
  } else {
    _x10protocol.readAndStoreChange(duration);
    if (_x10protocol.hasData()) {
        _x10protocol.displayHasBinary();
        _x10protocol.displayHasHexa();
        _x10protocol.displayHasHumanReadable();
    }
  }
  lastTime = time;  
}



int main(void) {
  if(wiringPiSetup() == -1)
       return 0;

  _x10protocol = X10Protocol();

	//attach interrupt to changes on the pin
	wiringPiISR(2, INT_EDGE_BOTH, &handleInterrupt);
  //interrupt will handle changes
  while(true);
}
