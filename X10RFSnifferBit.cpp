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
#define MAX_CHANGE_TO_KEEP 67
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
  static int lockChange;

  long time = micros();
  duration = time - lastTime;

  //wait for lock
  if (lockChange == 0 && duration > 26000 && duration < 28000 && lockPassed == 0) { 
    changeCount = 0;
    log("Lock started"); cout << endl;
    log(measure2String(duration));cout << endl;
    //store duration
    durations[changeCount++] = duration;
    lockChange = 1;
  } else if (lockChange == 1 && duration > 7000 && duration < 9000 && lockPassed == 0) {
    log("Lock started 2"); cout << endl;
    log(measure2String(duration));cout << endl;
    durations[changeCount++] = duration;
    lockChange = 2;
  } else if (lockChange == 2 && duration > 3000 && duration < 5000 && lockPassed == 0) {
    log("Lock started 3"); cout << endl;
    log(measure2String(duration));cout << endl;
    durations[changeCount++] = duration;
    lockChange = 3;
    lockPassed = 1; //lock is finished
  }


  else if (lockPassed != 0) { // REST OF DATA
    //log(measure2String(duration));

    if (changeCount > MAX_CHANGE_TO_KEEP) { // store 100 change Max
      //ended
      lockPassed = 0;
      changeCount = 0;
      lockChange = 0;
      log("===============================");
      //start after the lock
      for (int i=3; i < MAX_CHANGE_TO_KEEP; i=i+2) {


        if ((i-3)%16==0) {
          //space each 8bits
          cout << " ";
        }

        if (durations[i] > 350 && durations[i] < 550 &&
            durations[i+1] > 450 && durations[i+1] < 650  ) {
            //a 0
            cout << "0";
        } else if (durations[i] > 350 && durations[i] < 550 &&
            durations[i+1] > 1400 && durations[i+1] < 1600  ) {
            //a 0
            cout << "1";
        } else {
          //errror reset
          log ("EROOR");cout << endl;
          log(measure2String(durations[i]));cout << " ";
          log(measure2String(durations[i+1]));cout << endl;
          changeCount = 0;
          lockChange = 0;
          lockPassed = 0;
          break;
        }
      }
      cout << endl;
    } else {
      //store duration
      durations[changeCount++] = duration;
    }
  } else {
    //wait for another 
    changeCount = 0;
    lockChange = 0;
    lockPassed = 0;
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
