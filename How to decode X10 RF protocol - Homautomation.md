** Article recovered from Wayback Machine as the site is no longer available **

In this article I will describe the method I used to decode the X10 RF protocol with a Raspberry Pi.

Before starting you have to setup correctly a Raspberry and a RF receiver @ 433Mhtz as I described in a [previous post](https://web.archive.org/web/20180913045549/http://www.homautomation.org/2013/09/21/433mhtz-rf-communication-between-arduino-and-raspberry-pi/).

You can find all the code attached to this post on github: [https://github.com/vdemay/433Rpi](https://web.archive.org/web/20180913045549/https://github.com/vdemay/433Rpi)

## Needed hardware

#### ![300px-RaspberryPi](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/300px-RaspberryPi.jpg)

####  A **Raspberry Pi**

 **Website**: [http://www.raspberrypi.org/](https://web.archive.org/web/20180913045549/http://www.raspberrypi.org/)

 **Price**: less than 50$ ([Amazon](https://web.archive.org/web/20180913045549/http://www.amazon.com/s/?_encoding=UTF8&camp=1789&creative=390957&field-keywords=raspberry%20pi&linkCode=ur2&rh=n%3A172282%2Ck%3Araspberry%20pi&tag=homauomatio03-20&url=search-alias%3Delectronics)![](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/ir.gif))

Mine is a B Model

[![rf433k](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/rf433k-300x225.jpg)](https://web.archive.org/web/20180913045549/http://www.homautomation.org/wp-content/uploads/2013/09/rf433k.jpg)

####  433Mhz RF link kit

 **Price**: less than 5$ ([Amazon](https://web.archive.org/web/20180913045549/http://www.amazon.com/s/?_encoding=UTF8&camp=1789&creative=390957&field-keywords=raspberry%20pi&linkCode=ur2&rh=n%3A172282%2Ck%3Araspberry%20pi&tag=homauomatio03-20&url=search-alias%3Delectronics)![](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/ir.gif))

## Build the hardware to snif RF

Now You get the right hardware you have to make a simple breakout:

[![RPi](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/RPi.jpg)](https://web.archive.org/web/20180913045549/http://www.homautomation.org/wp-content/uploads/2013/09/RPi.jpg)

The Antenna should be a 17cm long simple wire. Here on my exemple there are both emitter and receiver connected to the Raspberry. Obviously only the receiver is needed

[![photo](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/photo-1024x768.jpg)](https://web.archive.org/web/20180913045549/http://www.homautomation.org/wp-content/uploads/2014/04/photo.jpg)

## Install the software

Now it’s to prepare the software. My Raspberry is running a [Raspbian](https://web.archive.org/web/20180913045549/http://www.raspbian.org/). Once ssh’ed on it, install [wiringPi (a lib to help to manage GPIO)](https://web.archive.org/web/20180913045549/https://projects.drogon.net/raspberry-pi/wiringpi/)
```bash
pi@raspberrypi ~ $ git clone git://git.drogon.net/wiringPi
...
pi@raspberrypi ~ $ cd wiringPi/wiringPi
pi@raspberrypi ~/wiringPi/wiringPi $ sudo su
...
root@raspberrypi:/home/pi/wiringPi/wiringPi# ./build
```

Test the install

```bash
root@raspberrypi:/home/pi/wiringPi/wiringPi# gpio readall
+----------+-Rev1-+------+--------+------+-------+
| wiringPi | GPIO | Phys | Name   | Mode | Value |
+----------+------+------+--------+------+-------+
|      0   |  17  |  11  | GPIO 0 | IN   | Low   |
|      1   |  18  |  12  | GPIO 1 | IN   | Low   |
|      2   |  21  |  13  | GPIO 2 | IN   | Low   |
|      3   |  22  |  15  | GPIO 3 | IN   | Low   |
|      4   |  23  |  16  | GPIO 4 | IN   | Low   |
|      5   |  24  |  18  | GPIO 5 | IN   | Low   |
|      6   |  25  |  22  | GPIO 6 | IN   | Low   |
|      7   |   4  |   7  | GPIO 7 | IN   | Low   |
|      8   |   0  |   3  | SDA    | IN   | High  |
|      9   |   1  |   5  | SCL    | IN   | High  |
|     10   |   8  |  24  | CE0    | IN   | High  |
|     11   |   7  |  26  | CE1    | IN   | High  |
|     12   |  10  |  19  | MOSI   | IN   | High  |
|     13   |   9  |  21  | MISO   | IN   | High  |
|     14   |  11  |  23  | SCLK   | IN   | High  |
|     15   |  14  |   8  | TxD    | ALT0 | High  |
|     16   |  15  |  10  | RxD    | ALT0 | High  |
+----------+------+------+--------+------+-------+
```
Ok, we are ready to go

## Start to decode rf signal

First of all you have to know that signal received by the receiver is based on logical state changes. Here is a typical exemple  of signal received via RF:

[![signal](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/signal.png)](https://web.archive.org/web/20180913045549/http://www.homautomation.org/wp-content/uploads/2014/04/signal.png)

So the idea is to mesure time between each signal state changes (0 to 1 or 1 to 0) in order try to decode the binary message. It’s also necessary to start to measure signal change only after the signal started and not during the noise period.

Most of protocol are based on a lock to indicate the start of the signal. A Lock is a long period (ms) when the emitter send a 0 or a 1 (or a sequence of long 0 and 1) to indicate the signal will start:

[![lock](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/lock.png)](https://web.archive.org/web/20180913045549/http://www.homautomation.org/wp-content/uploads/2014/04/lock.png)

So first of all we’ll try to handle a lock programmatically with a simple C++ code: The goal of the soft is to attach interrupt each time the signal is falling or rising in order to mesure the time needed for a signal change. If the time to change is long, we can expect a lock is received.

## Write a soft to try to understand x10 rf protocol

The aim here is to decode signal send by this device :

  
[X10 3 Unit Slimline Wireless Wall Switch + Dimming![](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/ir.1.gif)  
![41WZKMXA2WL](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/41WZKMXA2WL-300x300.jpg)  
$15 on Amazon](https://web.archive.org/web/20180913045549/http://www.amazon.com/gp/product/B00022OCDW/ref=as_li_ss_tl?ie=UTF8&camp=1789&creative=390957&creativeASIN=B00022OCDW&linkCode=as2&tag=homauomatio03-20)

 

Here is a program waiting for a long period and then display duration of 0 and 1

```
searchLock.cpp
```
```C++
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
```
Here is the output when I press a button:

```
Lock started
11224
===============================11224 316 25 139 89 286 64 25 126 82 14 136 60 128 21 100 109 88 22 74 30 75 22 92 23 74 27 82 41 56 
Lock started
11293
===============================11293 327 15 141 82 86 98 79 15 94 86 16 83 17 88 15 79 17 78 16 82 15 95 16 78 15 57 24 256 99 
Lock started
26783
===============================26783 390 8083 4069 471 532 470 1537 466 1553 448 547 466 540 471 528 480 527 480 536 469 1535 467 547 461 558 457 1536 467 1534 
Lock started
27166
===============================27166 8096 4068 464 531 482 1524 469 1540 464 560 447 560 447 555 454 551 460 539 470 1532 469 552 453 561 450 1550 452 1549 458 
Lock started
27155
===============================27155 8107 4077 457 543 465 1528 475 1533 467 549 462 538 476 539 459 555 452 552 460 1534 473 535 471 540 469 1545 456 1550 451 
Lock started
27160
===============================27160 8110 4072 458 552 451 1543 470 1523 478 541 467 552 453 555 452 553 455 556 450 1546 466 545 464 545 461 1541 457 1545 459 
Lock started
26445
===============================26445 800 8007 4089 459 550 455 1552 450 1542 466 550 459 542 466 546 460 642 149 24 148 639 449 1573 405 561 440 552 479 1524 
Lock started
27230
===============================27230 8055 4054 448 594 408 1604 417 1539 447 612 418 620 396 561 440 567 440 513 484 1592 421 521 483 534 464 1537 465 1610 416 
Lock started
27152
===============================27152 8115 4066 468 534 472 1522 477 1541 462 548 464 542 465 548 455 558 451 550 461 1535 476 527 479 552 454 1531 472 1543 456 
Lock started
24229
===============================24229 2968 8043 4106 455 559 444 1571 448 1523 473 533 474 549 455 553 458 550 457 557 449 1544 463 556 457 551 458 1535 463 1550 
Lock started
15903
===============================15903 18727 347 251 182 15 293 14 125 14 238 132 13 132 210 13 216 13 342 13 149 169 14 167 6422 14 476 321 15 187 
Lock started
11785
===============================11785 333 75 15 81 95 16 67 24 126 103 96 16 59 23 109 91 17 62 23 98 17 89 92 15 70 23 88 16 70
```
The program detect noise or signal from elsewhere, but when I press a button lot of output with a first values around 2700 are displayed. So we will suppose the lock code started by a value between around 2700us. If we look at the second value and third value when the first one is around 2700 we get something around 800us for the second value and 400us for the third one. So the lock seems to be:

-   2700us
-   800us
-   400us

I stopped to consider the lock at this point because other values are much smaller. **Reminder** each value is a duration of a 0 state or of a 1 state. So If we imagine the first state is 0 we get the following lock:

[![lockVal](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/lockVal.jpg)](https://web.archive.org/web/20180913045549/http://www.homautomation.org/wp-content/uploads/2014/04/lockVal.jpg) so now we will try to understand the rest of the data taking as exemple the following

```
Lock started
27152
===============================27152 8115 4066 468 534 472 1522 477 1541 462 548 464 542 465 548 455 558 451 550 461 1535 476 527 479 552 454 1531 472 1543 456
```

As each value is duration of a 0 or a 1, the rest of the data should be consider  by pair (RF binary is often encoded with different duration between hight level and low level):

```
468 534
472 1522
477 1541
462 548
464 542
465 548
455 558
451 550
461 1535
476 527
479 552
552 1531
472 1543
456...
```

It seems we get a pattern : sometimes 450 and 550 and sometimes 4500 and 1500. Wahoo we get something : [![lockVal+value](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/lockVal+value.jpg)](https://web.archive.org/web/20180913045549/http://www.homautomation.org/wp-content/uploads/2014/04/lockVal+value.jpg)   Binary is usually encoded over RF on the following way  

-   0 encoded with hight and low level with the same duration
-   1 encoded with hight level longer than low level or hight level shorter than the low one.

 So here it seems we get 0110000010011… [![lockVal+value+decode](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/lockVal+value+decode.jpg)](https://web.archive.org/web/20180913045549/http://www.homautomation.org/wp-content/uploads/2014/04/lockVal+value+decode.jpg)   At this point we are not sure to get all needed changes to decode the signal. So I will update the soft to take into account our new knowledge:  

-   Handle the 3 changes making the lock (with a big tolerance)
-   Dump more changes to try to find out the right needed number (70)

```
simpleSniff.cpp
```
```C++
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
```

Now when I press a button I get

```
Lock started
26983
Lock started 2
8011
Lock started 3
4106
===============================26983 8011 4106 442 581 381 1513 492 1530 443 511 481 559 448 542 500 500 461 515 480 1537 467 505 482 515 486 1512 527 1505 446 1496 481 1546 460 1492 481 552 450 525 476 546 461 509 487 543 461 512 485 522 480 546 466 1507 473 1556 434 1509 475 1546 454 1508 483 1538 449 1500 481 1547 453 26967 7997 
Lock started
26948
Lock started 2
8065
Lock started 3
4063
===============================26948 8065 4063 467 520 452 1525 463 1504 480 522 480 524 535 473 477 521 472 537 465 1509 481 524 479 518 479 1514 477 1527 465 1512 470 1518 475 1511 480 520 481 527 476 520 481 526 480 518 476 522 479 525 475 526 475 1525 469 1509 477 1518 475 1514 471 1534 460 1512 475 1509 478 1524 471 26941 8029

```

If you look at the 2 last changes, it seems to be the beginning of a new lock. If we count the number of changes we get :

```
Number_of_dump - Lock_at_begging - beggining_of_lock_at_the_end = 70 - 3 - 2 = 65.
```

As wee need 2 changes to make a bit, I suppose that 421 before the beggining of the lock at the end is useless. Let update the program

-   We have to reduce the number of change to sniff to 67 (3 locks + 64 changes — bits)
-   And we are going to display bits instead of duration
```
X10RFSnifferBit.cpp
```
```C++
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
```

Now we get the following

```
Lock started
26937
Lock started 2
7995
Lock started 3
4096
=============================== 01100000 10011111 00000000 11111111
Lock started
26926
Lock started 2
8047
Lock started 3
4091
=============================== 01100000 10011111 00000000 11111111
Lock started
26929
Lock started 2
7862
Lock started
26899
Lock started 2
8065
Lock started 3
4081
=============================== 01100000 10011111 00100000 11011111
Lock started
26925
Lock started 2
8049
Lock started 3
4068
=============================== 01100000 10011111 00100000 11011111
```
So we get bytes. It seems that the second byte is the complment from the first one and fourth one is the complement from the third one.

## How to decode the binary signal

Decoding the binary signal is more tricky. So I asked my frind google to help me with this task. Here is the link needed to continue:

[http://www.printcapture.com/files/X10\_RF\_Receiver.pdf](https://web.archive.org/web/20180913045549/http://www.printcapture.com/files/X10_RF_Receiver.pdf)

First all, it seems our reversed engineering is ok: [![Capture d’écran 2014-04-21 à 11.30.31](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/Capture-d%E2%80%99%C3%A9cran-2014-04-21-%C3%A0-11.30.31.png)](https://web.archive.org/web/20180913045549/http://www.homautomation.org/wp-content/uploads/2014/04/Capture-d’écran-2014-04-21-à-11.30.31.png) And the interesting section at this point is the table of byte values and meaning: [![Capture d’écran 2014-04-21 à 11.32.54](How%20to%20decode%20X10%20RF%20protocol%20-%20Homautomation/Capture-d%E2%80%99%C3%A9cran-2014-04-21-%C3%A0-11.32.54.png)](https://web.archive.org/web/20180913045549/http://www.homautomation.org/wp-content/uploads/2014/04/Capture-d’écran-2014-04-21-à-11.32.54.png)   So if the next phase, we’ll try to transform our binary signal to a real value: I firstly did it only for Axx command. I’ll try to continue to maintain the code here: [https://github.com/vdemay/433Rpi](https://web.archive.org/web/20180913045549/https://github.com/vdemay/433Rpi) All you need to decode X10 Protocol is here: [https://github.com/vdemay/433Rpi/blob/master/SignalReciever](https://web.archive.org/web/20180913045549/http://www.homautomation.org/2014/04/25/how-to-decode-x10-rf-protocol//%20https://github.com/vdemay/433Rpi/blob/master/SignalReciever)

```
SignalReciever/X10Protocol.cpp
```
```C++
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
```
And here is the output now:

```
 01100000 10011111 00000000 11111111
60 9f 00 ff 
A1 ON
 01100000 10011111 00000000 11111111
60 9f 00 ff 
A1 ON
 01100000 10011111 00110000 11001111
60 9f 30 cf 
A2 OFF
 01100000 10011111 00110000 11001111
60 9f 30 cf 
A2 OFF
 01100000 10011111 00001000 11110111
60 9f 08 f7 
A3 ON
 01100000 10011111 00001000 11110111
60 9f 08 f7 
A3 ON
 01100000 10011111 00100000 11011111
60 9f 20 df 
A1 OFF
 01100000 10011111 10011000 01100111
60 9f 98 67 
A DIM 005
```
