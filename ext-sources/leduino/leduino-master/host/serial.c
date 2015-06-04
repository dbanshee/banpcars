#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "serial.h"
#include "log.h"

// takes the string name of the serial port (e.g. "/dev/tty.usbserial","COM1")
// and a baud rate (bps) and connects to that port at that speed and 8N1.
// opens the port in fully raw mode so you can send binary data.
// returns valid fd, or -1 on error
int serial_init(const char* serialport, int baud)
{
    struct termios toptions;
    int fd;
    
    fd = open(serialport, O_RDWR | O_NOCTTY);
    if (fd == -1)  {
        LOG_ERROR("Open %s",serialport);
        return -1;
    }
    
    if (tcgetattr(fd, &toptions) < 0) {
        LOG_ERROR("Can't get term attributes");
        return -1;
    }
    speed_t brate = baud; // let you override switch below if needed
    switch(baud) {
    case 4800:   brate=B4800;   break;
    case 9600:   brate=B9600;   break;
#ifdef B14400
    case 14400:  brate=B14400;  break;
#endif
    case 19200:  brate=B19200;  break;
#ifdef B28800
    case 28800:  brate=B28800;  break;
#endif
    case 38400:  brate=B38400;  break;
    case 57600:  brate=B57600;  break;
    case 115200: brate=B115200; break;
    }
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw
    toptions.c_cflag &= ~HUPCL; //arduino auto-reset OFF

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 20;
    
    if( tcsetattr(fd, TCSANOW, &toptions) < 0) {
        LOG_ERROR("Couldn't set term attributes");
        return -1;
    }

    // Empty command
    write(fd,"\r",1);

    // Clear buffer
    char kk;
    int n;
    while((n = read(fd,&kk,1))>0);

    return fd;
}

// Reads a line from the serial port
int serial_readline(int fd, char* buf, int blen) {
  int i=0;
  int r = read(fd,buf+i,1);
  while(r>0 && (buf[i] == '\r' || buf[i] == '\n')) {
    r = read(fd,buf+i,1);
  }

  while(r>0 && buf[i] != '\r' && buf[i] != '\n') {
    i++;
    r = read(fd,buf+i,1);
  }

  buf[i] = '\0';

  return 0;
}

