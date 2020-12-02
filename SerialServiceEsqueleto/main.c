#include <stdio.h>
#include <stdlib.h>
#include "SerialManager.h"

#include <string.h>
#include <unistd.h>
#include <signal.h>

#define SEGUNDO	2
// El SERIAL_PORT 0--> implica que se conecta a /dev/ttyUSB0, se especifica en "serial.c"
#define SERIAL_PORT	1
#define BAUDRATE	115200

#define MSG_SERIAL_SND ">OUT:1,1\r\n"
#define MSG_SERIAL_RCV ">SW:X,Y\r\n"
#define SIZE_MSG_SND strlen(MSG_SERIAL_SND)
#define SIZE_MSG_RCV strlen(MSG_SERIAL_SND)


void signal_receive(int signal)
{
    write(1,"Capturo las señales...\n",23);


   serial_close();
   exit(0);
}










int main(void)
{
	char data_send[]=MSG_SERIAL_SND;
	char data_rcvd[]=MSG_SERIAL_RCV;
	printf("Inicio Serial Service\r\n");

   /* Configuro las señales */
    struct sigaction sa;
    sa.sa_handler = signal_receive;
    sa.sa_flags =0; 					//SA_RESTART;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGINT,&sa,NULL)==-1){
		perror("Error de sigaction: SIGINT");
        	exit(1);
        }
    if(sigaction(SIGTERM,&sa,NULL)==-1){
		perror("Error de sigaction: SIGTERM");
        	exit(1);
        }
	
	if(serial_open(SERIAL_PORT,BAUDRATE) != 0){
		printf("No se pudo abrir el puerto serie\n");
		return 0;
	}	

	while(1){
		if(serial_receive(data_rcvd,SIZE_MSG_RCV)!=0){
			data_send[5]=data_rcvd[4];
			data_send[7]=data_rcvd[6];
			serial_send(data_send,SIZE_MSG_SND);
			sleep(SEGUNDO);		
		}
	}
	

	// El programa no debería llegar aquí
	return 0;
}
