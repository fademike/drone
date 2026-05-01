
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#include "cb.h"

/// thread
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "tty.h"
static int tty =0;

// for circular buffer
#define BUF_SIZE 1000
#define BUF_PACK_LEN 255
circular_buffer cb_ttyRead;

#define error_message printf

int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        if (tcgetattr (fd, &tty) != 0)
        {
                error_message ("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        // tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
		tty.c_cflag |= (CLOCAL);
		tty.c_cflag &= ~CREAD; 

                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        //tty.c_cflag &= ~CRTSCTS;

        tty.c_iflag &= ~(0x100);//INLCR;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                error_message ("error %d from tcsetattr\n\r", errno);
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                error_message ("error %d from tggetattr\n\r", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                error_message ("error %d setting term attributes\n\r", errno);
}


void* thread_for_read_from_tty(void *arg)
{
        while(1){
                uint8_t b [100];
                int n_rx = tty_read(b, sizeof(b)); // read up to 100 characters if ready to read
                if (n_rx>0) {
                        if (n_rx >= cb_ttyRead.p_size) n_rx = cb_ttyRead.p_size-1;
                        cb_write(&cb_ttyRead, b, n_rx);
                }
                //else usleep(100);
        };
        return NULL;
}


int tty_init(char * portname){
        
	// init circular buffer for tty
	if (cb_init(&cb_ttyRead, BUF_SIZE, BUF_PACK_LEN) < 0) {
		printf("cb init cb_ttyRead error\n\r");
		return 0;
	}

	tty = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (tty < 0)
	{
                error_message ("error %d opening %s: %s\n\r", errno, portname, strerror (errno));
                return -1;
	}

	set_interface_attribs (tty, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (tty, 0);                // set no blocking

        pthread_t tid;
        int err = pthread_create(&tid, NULL, &thread_for_read_from_tty, NULL);
	if (err != 0) {printf("\ncan't create thread :[%s]", strerror(err));return -1;}

    return 0;
}

void tty_write(char * buf, int len){
    write(tty, buf, len);
}


int tty_read_get(char * buf, int len){
if (!cb_isEmpty(&cb_ttyRead)){	// read tty!!!
        
        uint8_t * b = cb_getBuf(&cb_ttyRead, cb_RD);
        int n_rx = cb_getBufLen(&cb_ttyRead);

        if (n_rx>len)n_rx = len;
        memcpy(buf, b, n_rx);
        cb_pass(&cb_ttyRead, cb_RD);
        return n_rx;

}
return 0;
}
int tty_read(char * buf, int len){
int n_rx = read (tty, buf, len);
return n_rx;
}


