#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(){

	int socket_servidor;
	struct sockaddr_in servidor;
	char cadena [80];
	time_t tiempo;
	struct tm * stTm;	

	tiempo=time(NULL);
	stTm=localtime(&tiempo);

	struct sockaddr_in cliente;
	socklen_t longitud_cliente;

	socket_servidor=socket(AF_INET,SOCK_DGRAM,0);
	if(socket_servidor==-1){
		printf("No se puede abrir el socket servidor\n");
		exit(-1);	
	}

	servidor.sin_family=AF_INET;
	servidor.sin_port=htons(2000);
	servidor.sin_addr.s_addr=htonl(INADDR_ANY);

	if(bind(socket_servidor,(struct sockaddr *) &servidor,sizeof(servidor))==-1){
		close (socket_servidor);
		exit(-1);
	}

	longitud_cliente=sizeof(cliente);

	while(1){

		int recibido=recvfrom(socket_servidor,&cadena,sizeof(cadena),0,(struct sockaddr *) &cliente,&longitud_cliente);

		if(strcmp(cadena,"DAY")==0){
			strftime(cadena,80,"%A, %d de %B de %Y", stTm);
		}

		if(strcmp(cadena,"TIME")==0){
			strftime(cadena,80,"%H:%M:%S", stTm);
		}

		if(strcmp(cadena,"DAYTIME")==0){
			strftime(cadena,80,"%A, %d de %B de %Y; %H:%M:%S", stTm);
		}

		if(recibido>0){
			int enviado=sendto(socket_servidor,&cadena,sizeof(cadena),0,(struct sockaddr *) &cliente,longitud_cliente);
		}

	}

	close(socket_servidor);

return 0;
}
