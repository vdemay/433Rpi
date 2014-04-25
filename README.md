433Rpi
======

Code to play with 433Mhz on RaspberryPi.

Visit my blog : http://homautomation.org to get more details on how to use the code and a post about this code: http://www.homautomation.org/2014/04/25/how-to-decode-x10-rf-protocol/

### Dependencies
* wiringPi : https://projects.drogon.net/raspberry-pi/wiringpi/

### Building WiringPi
```bash
pi@raspberrypi ~ $ git clone git://git.drogon.net/wiringPi
...
pi@raspberrypi ~ $ cd wiringPi/wiringPi
pi@raspberrypi ~/wiringPi/wiringPi $ sudo su
...
root@raspberrypi:/home/pi/wiringPi/wiringPi# ./build
```

### Features
* SigalReceiver Folder : final code to decode 
 * X10 Protocol
 * More to come

### Tools
See: http://www.homautomation.org/2014/04/25/how-to-decode-x10-rf-protocol/
* receive.cpp and send.cpp : send and receive command from chacon DI.O devices
* searchLock.cpp : simple code to try to find a lock on 433 emission
* RFSniffer.cpp : simple code to try to sniff 433 code
* simpleSniff : sniff code after a X10 lock
