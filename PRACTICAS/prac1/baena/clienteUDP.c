#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc,char ** argv){

	int socket_cliente;
	char cadena [80];

	struct sockaddr_in servidor;  
   	socklen_t longitud_servidor;

	struct timeval timeout;
	fd_set lectura;
	int salida;

	timeout.tv_sec=5;
	timeout.tv_usec=0;
	
	FD_ZERO(&lectura);
	FD_SET(0,&lectura);

	socket_cliente=socket(AF_INET,SOCK_DGRAM,0);
	if (socket_cliente==-1){
		printf("No se puede abrir el socket cliente\n");
    		exit (-1);	
	}

	servidor.sin_family=AF_INET;
	servidor.sin_port=htons(2000);
	servidor.sin_addr.s_addr=inet_addr("127.0.0.1");//Ponemos 127.0.0.1 para que se comunique el PC consigo mismo.
  	longitud_servidor=sizeof(servidor);

	strcpy(cadena,argv[1]);
	int i=0;
	int enviado;
	
	FD_SET(socket_cliente,&lectura);

	do{
		enviado=sendto(socket_cliente,cadena,sizeof(cadena),0,(struct sockaddr *) &servidor,longitud_servidor);
		salida=select(socket_cliente+1,&lectura,NULL,NULL,&timeout);

		if(salida==-1){
			printf("Se ha producido un error en select\n");
		}
		else if(salida==0){
			printf("Se ha agotado el tiempo\n");

			timeout.tv_sec=5;
			timeout.tv_usec=0;
			FD_ZERO(&lectura);
			FD_SET(0,&lectura);
			FD_SET(socket_cliente,&lectura);

		}
		else{

			
			if (enviado<0){
				printf("Error al solicitar el servicio\n");
			}
			else{
				i=3;
			int recibido=recvfrom(socket_cliente,cadena,sizeof(cadena),0,(struct sockaddr *) &servidor,&longitud_servidor);
			
   				if (recibido>0){
      					printf("%s\n",cadena);
				}
   				else{
      					printf("Error al leer del servidor\n");
				}		
			}
		}
		i++;
		
	}while(i<3);

	/*if(salida==0 && i==3){
		printf("Se ha agotado el tiempo y el numero de intentos\n");
		exit(-1);
	}*/

	

	close(socket_cliente);

return 0;
}
