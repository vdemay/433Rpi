all: SignalReceiver

SignalReceiver: X10Protocol.o SignalReceiver.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi

clean:
	$(RM) *.o send