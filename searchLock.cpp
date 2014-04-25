#include <wiringPi.h>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <sched.h>
#include <sstream>


using namespace std;

//store data into array 
#define MAX_CHANGE_TO_KEEP 30
static int durations[MAX_CHANGE_TO_KEEP];
 
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
  static unsigned int changeCount;
  static unsigned long lastTime;

  static int lockPassed;

  long time = micros();
  duration = time - lastTime;

  //wait for lock
  if (duration > 11000 && lockPassed == 0) { 
    changeCount = 0;
    log("Lock started"); cout << endl;
    log(measure2String(duration));cout << endl;
    //store duration
    durations[changeCount++] = duration;
    lockPassed = 1;
  } 

  else if (lockPassed != 0) { // REST OF DATA
    //log(measure2String(duration, digitalRead(2)));

    if (changeCount > MAX_CHANGE_TO_KEEP) { // store 100 change Max
      //ended
      lockPassed = 0;
      changeCount = 0;
      log("===============================");
      for (int i=0; i < MAX_CHANGE_TO_KEEP; i=i++) {
        log(measure2String(durations[i]));
        log(" ");
      }
      cout << endl;
    } else {
      //store duration
      durations[changeCount++] = duration;
    }
  }

  lastTime = time;  
}



int main(void) {
  if(wiringPiSetup() == -1)
       return 0;

	//attach interrupt to changes on the pin
	wiringPiISR(2, INT_EDGE_BOTH, &handleInterrupt);
  //interrupt will handle changes
  while(true);
}
