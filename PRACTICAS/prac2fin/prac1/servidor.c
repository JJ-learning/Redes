#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


main()
{
	system("clear");
	/*Iniciamos el socket*/
	int Servidor;
	struct sockaddr_in Servidor_S;
	Servidor = socket (AF_INET, SOCK_DGRAM, 0);
	if (Servidor == -1)
	{
		printf ("No se puede abrir socket servidor\n");
		exit (-1);	
	}

	/*Declaramos la información del cliente*/
	struct sockaddr_in Cliente;
	int longitudCliente = sizeof(Cliente); 
	
	//Datos para la funcion strftime
	time_t tiempo;
	struct tm * stTm;
	char cadena [80];
	tiempo = time(NULL);
	stTm = localtime(&tiempo);
			

	/*Declaramos la información del socker para la función bind()*/
	Servidor_S.sin_family = AF_INET;
	Servidor_S.sin_port = htons(2000);
	Servidor_S.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind ( Servidor, (struct sockaddr *)&Servidor_S, sizeof (Servidor_S))==-1)
	{
		close(Servidor);
		exit(-1);
	}

	/*Hacemos que el servidor espere todo el rato los mensajes del cliente*/
	while(1)
	{
		int recibido;
		recibido=recvfrom (Servidor, (char *)&cadena, sizeof(cadena), 0, (struct sockaddr *)&Cliente, &longitudCliente);
		
		if(recibido>0)
		{
			//printf("Recibo:\n");
			if(strcmp(cadena,"DAY")==0){
			strftime(cadena,80,"%A, %d de %B de %Y", stTm);
			printf("Envio: %s \n", cadena);
			}
			if(strcmp(cadena,"TIME")==0){
				strftime(cadena,80,"%H:%M:%S", stTm);
				printf("Envio: %s \n", cadena);
			}
			if(strcmp(cadena,"DAYTIME")==0){
				strftime(cadena,80,"%A, %d de %B de %Y; %H:%M:%S", stTm);
				printf("Envio: %s \n", cadena);
			}
			if(recibido>0){
				int enviado=sendto (Servidor, (char *)&cadena, sizeof(cadena), 0, (struct sockaddr *)&Cliente, longitudCliente);
				if (enviado < 0)
					printf("Error al solicitar el servicio\n");
			}

		  	/*Rellenamos el dato a enviar*/
			/*tiempo = time(NULL);
			stTm = localtime(&tiempo);
			strftime(cadena,80,"%A, %d de %B", stTm);

			printf("Envio: %s \n", cadena);
			int enviado=sendto (Servidor, (char *)&cadena, sizeof(cadena), 0, (struct sockaddr *)&Cliente, longitudCliente);
		  	if (enviado < 0)
			printf("Error al solicitar el servicio\n");*/
		}
	}
	close(Servidor);
}