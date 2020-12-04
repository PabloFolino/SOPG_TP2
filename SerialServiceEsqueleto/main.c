//======================================= Librerías ===============================================
#include <stdio.h>
#include <stdlib.h>
#include "SerialManager.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <pthread.h>

//=============================== Definición de constantes ========================================
#define ESPERA	100000		// Se lee cada 100mseg el puero serie
// El SERIAL_PORT 0--> implica que se conecta a /dev/ttyUSB0, se especifica en "serial.c"
#define SERIAL_PORT	1
#define BAUDRATE	115200

#define PUERTO_TCP	10000
#define ADDR_IP		"127.0.0.1"

#define MSG_SERIAL_SND ">OUT:1,1\r\n"
#define MSG_SERIAL_RCV ">SW:X,Y\r\n"
#define SIZE_MSG_SND strlen(MSG_SERIAL_SND)
#define SIZE_MSG_RCV strlen(MSG_SERIAL_SND)

//=================================== Variablos globales ==========================================
static 	pthread_t h_thread,h_thread1;
void* ret;
volatile sig_atomic_t flag_fin;

// Variables que se usan para la conexión tcp
socklen_t addr_len;
struct sockaddr_in clientaddr;
struct sockaddr_in serveraddr;
char buffer_tx[128],buffer_rx[128];
char data_send[]=MSG_SERIAL_SND;
int newfd;
int n;
int fd_s;

//=================================== Manejo de señales ===========================================
void signal_receive(int signal)
{
	write(1,"Capturo las señales...\n",23);

	// Termino el programa de forma controlada
	flag_fin=1;
}

void bloquearSign(void)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void desbloquearSign(void)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}




//=================================== Manejo de hilos ============================================
void* start_tcp(void* message)
{
	//printf ("%s\n", (const char *) message);

	while(1){
		// Ejecutamos accept() para recibir conexiones entrantes,el newfd es activo
		// Ojo: la función accept es bloqueante
		printf("Esperando un cliente\n");
		addr_len = sizeof(struct sockaddr_in);
    	if ( (newfd = accept(fd_s, (struct sockaddr *)&clientaddr, &addr_len)) == -1){
	 		perror("Error en accept");
	   		exit(1);
		}

		char ipClient[32];
		inet_ntop(AF_INET, &(clientaddr.sin_addr), ipClient, sizeof(ipClient));
		printf  ("Server, conexión desde:  %s\n",ipClient);

		if( (n = read(newfd,buffer_tx,128)) == -1 ){
			perror("Error leyendo mensaje en socket");
			exit(1);
		}
		buffer_tx[n]=0x00;
		if(n==SIZE_MSG_RCV){
			serial_send(buffer_tx,n);
			printf("Se recibió del socket %d bytes: %s\n",n,buffer_tx);
		}

		while(n!=-1 && n!=0){
			n = read(newfd,buffer_tx,128);
			//printf("Recividos del socket %d bytes: %s\n",n,buffer_tx);
			if(n==SIZE_MSG_RCV){
				strcpy(data_send,buffer_tx);
				serial_send(data_send,n);
				printf("Se recibió del socket %d bytes: %s\n",n,buffer_tx);
			}
		}
		printf("Hubo un problema en al conexión\n");
		close(newfd);
	}
}

//=================================== Programa Principal ==========================================
int main(void)
{
	char data_rcvd[]=MSG_SERIAL_RCV;
	flag_fin=0;
	int n_rcvd;

	printf("Inicio Serial Service\r\n");

   // Configuro las señales 
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
	
	// Abro el canal serie hacia la EDU-CIAA
	if(serial_open(SERIAL_PORT,BAUDRATE) != 0){
		printf("No se pudo abrir el puerto serie\n");
		return -1;
	}	

	// Se configura la conexión tcp como servidor
	// Creamos socket
	fd_s = socket(AF_INET,SOCK_STREAM, 0);
	if(fd_s==-1){
		printf("No se pudo crear el socket\n");
		return -1;
	}
	// Cargamos datos de IP:PORT del server
    bzero((char*) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PUERTO_TCP);
    //serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(inet_pton(AF_INET, ADDR_IP, &(serveraddr.sin_addr))<=0){
        	fprintf(stderr,"ERROR invalid server IP\r\n");
        	return 1;
    }
	// Abrimos puerto con bind(), asocia la dirección IP al puerto--> IP:PUERTO
	if (bind(fd_s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
		close(fd_s);
		perror("Error al asociar IP:PUERTO");
		return 1;
	}
	// Seteamos socket en modo Listening, para escuchar conexiones entrantes
	if (listen (fd_s, 10) == -1) // backlog=10, hasta 10 conexiones entrantes
  	{
    	perror("error en listen");
    	exit(1);
  	}


	// Creación de un hilo para la aceptación de la conexión del TCP
	bloquearSign();
	if (pthread_create (&h_thread, NULL, start_tcp,NULL)!=0) {
		perror("Error al crear el hilo de start_tcp");
		return -1;
	}
	// Asigno modo detached al hilo start_tcp
	pthread_detach(h_thread);
	desbloquearSign();

	while(1){
		if((n_rcvd=serial_receive(data_rcvd,SIZE_MSG_RCV))!=0){
			printf("Se recibió del puerto serie %d bytes: %s",n_rcvd,data_rcvd);
			strcpy(buffer_rx,data_rcvd);
			// Enviamos mensaje a cliente
    		if (write (newfd, buffer_rx, SIZE_MSG_RCV) == -1){
      			perror("Error escribiendo mensaje en socket");
      			exit (1);
    		}
		}
		usleep(ESPERA);
		if(flag_fin==1){
			printf("\n Termino el programa \n");
			// Cerramos conexion con cliente
    		close(newfd);
    		close(fd_s);			
			//pthread_join (h_thread, &ret);
			serial_close();
			return 0;
		}	
	}

	// El programa no debería llegar aquí
	return 0;
}
