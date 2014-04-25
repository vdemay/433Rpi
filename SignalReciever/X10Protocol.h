#ifndef _X10Protocol_h
#define _X10Protocol_h

#define X10PROTOCOL_MAX_CHANGES 67
#define X10PROTOCOL_LOCK_CHANGES 3


class X10Protocol {

  public:
    X10Protocol();

    /**
     * return 1 is lock already passed and 
     * 0 otherwise 
     */
    int lockPassed();

	/**
	 * Try to handle a lock 
	 * --------------------
	 * return Lock progression 
	 * or 0 is no lock found
	 */
    int handleLock(long duration);

    /**
     * Read a change after lock
     * -------------------------
     * Read a change and store it
     */
    int readAndStoreChange(long duration);

    /**
     * Return 1 if data available
     * Return 0 if no data
     */
    int hasData();

    /**
     * Return a char array of bytes
     */
    unsigned char* readAsByteArray();

    /**
     * Return a human readable value
     * ex AO on
     */
    char* readAsHumanReadable();


    /**
     * display received data as hexa;
     */
    void displayHasHexa();

	/**
     * display received data as binary
     */
    void displayHasBinary();

    /**
     * display has human readable
     */
    void displayHasHumanReadable();


  private:
  	int changes[X10PROTOCOL_MAX_CHANGES];
  	int lockPosition;
  	int globalPositon;
  	int hasReceivedData;

  	void reset();

  	int changesToBit(long duration1, long duration2);
};


#endif