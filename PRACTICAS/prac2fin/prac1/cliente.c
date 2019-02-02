#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc,char ** argv){

	int sd;
	char cadena [80];
	char * ip=argv[2];

	if(argc!=4)
	{
		printf("ERROR: Argumentos--> .exe, comando time/day/daytime, ip, tiempo\n");
		exit(-1);

	}
	else
	{
		struct sockaddr_in servidor;  
	   	socklen_t longitudServidor;

		struct timeval timeout;
		fd_set lectura;
		int salida, recibido;

		timeout.tv_sec=atoi(argv[3]);
		timeout.tv_usec=0;
	
		FD_ZERO(&lectura);
		FD_SET(0,&lectura);

		sd=socket(AF_INET,SOCK_DGRAM,0);
		if (sd==-1)
		{
			printf("No se puede abrir el socket cliente\n");
	    		exit (-1);	
		}

		servidor.sin_family=AF_INET;
		servidor.sin_port=htons(2000);
		servidor.sin_addr.s_addr=inet_addr(ip);//Ponemos 127.0.0.1 para local.
	  	longitudServidor=sizeof(servidor);

		strcpy(cadena,argv[1]);
		int i=0;
		int enviado;
	
		FD_SET(sd,&lectura);

		do
		{
			enviado=sendto(sd,cadena,sizeof(cadena),0,(struct sockaddr *) &servidor,longitudServidor);
			salida=select(sd+1,&lectura,NULL,NULL,&timeout);

			if(salida==-1)
			{
				printf("Se ha producido un error en select\n");
			}
			else if(salida==0)
			{
				printf("Se ha agotado el tiempo\n");

				timeout.tv_sec=atoi(argv[3]);
				timeout.tv_usec=0;
				FD_ZERO(&lectura);
				FD_SET(0,&lectura);
				FD_SET(sd,&lectura);
				//Reiniciar el tiempo a 5 segundos y reestablecer la lectura del socket.
			}
			else
			{

			
				if (enviado<0)
				{
					printf("Error al solicitar el servicio\n");
				}
				else
				{
					i=3;
					 recibido=recvfrom(sd,cadena,sizeof(cadena),0,(struct sockaddr *) &servidor,&longitudServidor);
			
	   				if (recibido>0)
	   				{
	      					printf("%s\n",cadena);
					}
	   				else
	   				{
	      					printf("Error al leer del servidor\n");
					}		
				}
			}
			i++;
		
		}while(i<3);

		close(sd);
	}
return 0;
}
