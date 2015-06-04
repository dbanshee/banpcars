#ifndef __SERIAL_H__
#define __SERIAL_H__

// Takes the string name of the serial port (e.g. "/dev/tty.usbserial","COM1")
// and a baud rate (bps) and connects to that port at that speed and 8N1.
// opens the port in fully raw mode so you can send binary data.
// returns valid fd, or -1 on error
int serial_init(const char* serialport, int baud);

// Reads a line from the serial port
int serial_readline(int fd, char* buf, int blen);

#endif
