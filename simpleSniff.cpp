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
//state
int states[67];
//length
int durations[67];

void log(string a){
  cout << a << endl;
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

string measure2String(long duration, int value){
    string mystring;
    stringstream mystream;
    mystream << value;
    mystream << " : ";
    mystream << duration;
    return mystream.str();
}

void handleInterrupt() {
 
  static unsigned int duration;
  static unsigned int changeCount;
  static unsigned long lastTime;
  static int lockPassed = -1;

  //1 X10
  static int lockType = -1;


  long time = micros();
  duration = time - lastTime;

  //wait for lock
  if (duration > 25000 && lockPassed == -1) { //X10 lock started
    changeCount = 0;
    log("X10 lock started");
    log(measure2String(duration, digitalRead(2)));
    durations[changeCount++] = duration;
    lockType = 1;
  } else if (changeCount == 1 && lockType == 1) { //X10 second bit
    if(duration > 8000 && duration < 10000) {
      log("X10 lock continue");
      log(measure2String(duration, digitalRead(2)));
      durations[changeCount++] = duration;
    } else {
      lockType = -1;
      lockPassed = -1;
    }
  } else if (changeCount == 2 && lockType == 1) { //x10 third bit
    if(duration > 4000 && duration < 5000) {
      lockPassed = 1; //ok passed
      log("X10 lock continue 2");
      log(measure2String(duration, digitalRead(2)));
      durations[changeCount++] = duration;
    } else {
      lockType = -1;
      lockPassed = -1;
    }
  }

  else if (lockPassed != -1) { // REST OF DATA
     
    //Store duration and state
    //
    //log(measure2String(duration, digitalRead(2)));

    if (changeCount >= 70) {
      //ended
      lockPassed = -1;
      changeCount = 0;
      log("===============================");
      //decode
      if (lockType == 1) { // X10
          //ignore first 3 changes
          for (int i=3; i<=69; i=i+2) {
            if (durations[i]*2 < durations[i+1]) {
              cout << "1";
            } else {
              cout << "0";
            }
          }
          cout << endl;
      }
    } else {
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

  while(true);
}
