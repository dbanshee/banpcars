INCLUDES=  -I.
INCLUDES+= -I/opt/vc/include
INCLUDES+= -I/opt/vc/include/interface/vcos/pthreads
INCLUDES+= -I/opt/vc/include/interface/vmcs_host/linux
CFLAGS=-O3 -Wall $(INCLUDES)
LDFLAGS=-L/opt/vc/lib -lbcm_host -lpthread
BIN=leduino

$(BIN): leduino.o serial.o video_rpi.o log.o
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

run: $(BIN)
	./$(BIN) /dev/ttyUSB0

memcheck: $(BIN)
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --track-origins=yes ./$(BIN) /dev/ttyUSB0

clean:
	rm -f $(BIN) *.o
