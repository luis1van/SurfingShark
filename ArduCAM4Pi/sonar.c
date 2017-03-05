#include <stdio.h>
#include <string.h>
#include <unistd.h>			//Used for sonar cam
#include <fcntl.h>			//Used for sonar cam
#include <termios.h>		//Used for sonar cam
#include <stdlib.h>
#include <string.h>
#include <arducam_ov2640_capture.c>

//-------------------------
//----- SETUP USART 0 -----
//-------------------------
//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively
int main(int argc, char const *argv[])
{
    
    capture(argc,argv);
// kquwtvfguiqwrfuytvwfuvyqtwfruibtqwrf
    int uart0_filestream = -1;

    //OPEN THE UART
    //The flags (defined in fcntl.h):
    //	Access modes (use 1 of these):
    //		O_RDONLY - Open for reading only.
    //		O_RDWR - Open for reading and writing.
    //		O_WRONLY - Open for writing only.
    //
    //	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
    //											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
    //											immediately with a failure status if the output can't be written immediately.
    //
    //	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
    if (uart0_filestream == -1)
    {
        //ERROR - CAN'T OPEN SERIAL PORT
        printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
    }

    //CONFIGURE THE UART
    //The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
    //	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
    //	CSIZE:- CS5, CS6, CS7, CS8
    //	CLOCAL - Ignore modem status lines
    //	CREAD - Enable receiver
    //	IGNPAR = Ignore characters with parity errors
    //	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
    //	PARENB - Parity enable
    //	PARODD - Odd parity (else even)
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);



    //----- CHECK FOR ANY RX BYTES -----
    if (uart0_filestream != -1)
    {
        int cm_buffer_length = 200, buffer_step=0;
        
        unsigned char* length_array[cm_buffer_length];
        //printf("Enter reading page code uart0_filestream: %d",uart0_filestream);
        // Read up to 255 characters from the port if they are there
        while(1){
            unsigned char rx_buffer[256];
            int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)

            // printf("rx_length: %d will occur if there are no bytes\n", rx_length);

            if (rx_length < 0)
            {
                //An error occured (will occur if there are no bytes)
            }

            else if (rx_length == 0)
            {
                //No data waiting
            }
            else
            {	 
//                length_array[buffer_step]=malloc(4);
//                memcpy( length_array[buffer_step], rx_buffer+1, 3 );
                
                length_array[buffer_step] = malloc(sizeof(int));
                printf("%d cm 1 #%d \n",( *(rx_buffer+1) - '0'),buffer_step);
                *length_array[buffer_step] = (*(rx_buffer+1) - '0') * 100 + (*(rx_buffer+2) - '0') * 10 + (*(rx_buffer+3) - '0'); 
               
                
                printf("%d cm 2 #%d \n", length_array[buffer_step],buffer_step);
                
                buffer_step = ++buffer_step%200;
                
                if(rx_buffer[0] == 'R')
                {rx_buffer[0] = ' ';
                 rx_buffer[rx_length-1] = 'c';
                 rx_buffer[rx_length] = 'm';
                 rx_buffer[rx_length + 1] = '\0';}

                printf("Distance: %s\n", rx_buffer);
                //printf("%i bytes read : %s\n", rx_length, rx_buffer);

            }
        }
    }
}