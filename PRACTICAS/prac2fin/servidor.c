#include "funciones.h"

void salirCliente(int socket, fd_set * readfds, int * numClientes, struct Cliente arrayCliente[]){
  
    char buffer[250];
    int j;
    
    close(socket);
    FD_CLR(socket,readfds);
    
    //Re-estructurar el array de clientes
    for (j = 0; j < (*numClientes) - 1; j++)
        if (arrayCliente[j].id == socket)
            break;
    for (; j < (*numClientes) - 1; j++)
        (arrayCliente[j] = arrayCliente[j+1]);
    
    (*numClientes)--;
    
    bzero(buffer,sizeof(buffer));
    sprintf(buffer,"Desconexión del cliente: %d\n",socket);
    
    for(j=0; j<(*numClientes); j++)
        if(arrayCliente[j].id != socket)
            send(arrayCliente[j].id,buffer,strlen(buffer),0);
}

void manejador (int signum){
    printf("\nSe ha recibido la señal sigint\n");
    signal(SIGINT,manejador);
    
    //Implementar lo que se desee realizar cuando ocurra la excepción de ctrl+c en el servidor
}


int main()
{
    system("clear");
	/*Descriptor del socket y buffer de datos*/
	int sd, new_sd, cliente, todos=0;
	struct sockaddr_in sockname, from;
	char buffer[MSG_SIZE];
	socklen_t from_len;
	fd_set readfds, auxfds;
	int salida;
	int numClientes = 0;

    int recibidos, connect, nJugadores, nmesa, gana=100, fin=0, turno;
    char usuario[20], password[50], id, aux[30],consonante, vocal;
    char *ganador;

	struct Cliente arrayCliente[MAX_CLIENTS];

	int on, ret;
	//Contadores
	int i, j, c, t, m, p;
    int numL;//Número de letras de la frase
    int numAciertos;
    int numIntentos=0;
    //int x=0;Variable para saber qué cliente tiene el turno
    FILE *f;
    int refran;

    char sol[MSG_SIZE], correcto[MSG_SIZE];//Variable auxiliar para recoger la frase y comprobar si es correcta o no
    char *aux2;
    
    //Inicializamos las mesas
    struct Mesa mesa[NUM_MESAS];
    for (m=0;m<NUM_MESAS;m++){
        mesa[m].partida=0;
        mesa[m].puntos=0;
        mesa[m].njugadores=0;
    }

	/*Abrimos el socket*/
	sd=socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1)
	{
		perror("No se puede abrir el socket cliente");
		exit(1);
	}

	// Activaremos una propiedad del socket que permitir· que otros
    // sockets puedan reutilizar cualquier puerto al que nos enlacemos.
    // Esto permitir· en protocolos como el TCP, poder ejecutar un
    // mismo programa varias veces seguidas y enlazarlo siempre al
    // mismo puerto. De lo contrario habrÌa que esperar a que el puerto
    // quedase disponible (TIME_WAIT en el caso de TCP)
    on=1;
    ret = setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /*Inicializamos la información del socket para la función bind()*/
    sockname.sin_family=AF_INET;
    sockname.sin_port= htons(2050);
    sockname.sin_addr.s_addr = INADDR_ANY;

    if(bind(sd, (struct sockaddr *) &sockname, sizeof(sockname)) == -1)
    {
    	perror("Error en la operación bind");
		exit(1);
    }

    /*	De las peticiones que vamos a aceptar sólo necesitamos el 
		tamaño de su estructura, el resto de información (familia, puerto, 
		ip), nos la proporcionará el método que recibe las peticiones.
	*/
	from_len = sizeof(from);

	/*Escuchamos a algún cliente*/
	if(listen(sd,1) == -1)
	{
		perror("Error en la operación de listen");
		exit(1);
	}

	//Inicializar los conjuntos fd_set
    FD_ZERO(&readfds);
    FD_ZERO(&auxfds);
    FD_SET(sd,&readfds);
    FD_SET(0,&readfds);

    //Capturamos la señal SIGINT (Ctrl+c)
    signal(SIGINT,manejador);

    /*El servidor acepta la petición*/

    while(1)
    {
    	//Esperamos a recibir mensajes de los clientes
    	auxfds= readfds;
    	salida= select(FD_SETSIZE, &auxfds, NULL,NULL,NULL);

    	if(salida > 0)
    	{
    		for (i = 0; i < FD_SETSIZE; ++i)
    		{
    			//Buscamos el socket por el que se ha establecido la comunicación
    			if(FD_ISSET(i, &auxfds))
    			{
    				if(i == sd)
    				{
    					if((new_sd = accept(sd, (struct sockaddr *) &from, &from_len)) == -1)
    					{
    						perror("Error aceptando peticiones");
    					}
    					else
    					{
    						if(numClientes < MAX_CLIENTS)
    						{
    							arrayCliente[numClientes].id = new_sd;
                                arrayCliente[numClientes].conectado = 1;
    							numClientes++;

    							FD_SET(new_sd, &readfds);
                             
    							strcpy(buffer, "+Ok. Usuario conectado.\nBienvenido a la RULETA DE LA SUERTE\n");

    							//Enviamos la información al cliente
    							send(new_sd, buffer, strlen(buffer), 0);
                                strcpy(buffer, "Introduce el comando USUARIO usuario para entrar\n");
                                send(new_sd, buffer, strlen(buffer), 0);

    							for(j=0; j<(numClientes-1);j++)
    							{
    								bzero(buffer, sizeof(buffer));//Inicializamos el buffer
    								sprintf(buffer, "Nuevo cliente conectado: %d\n", new_sd);
    								//Enviamos la información de la ID al cliente
    								send(arrayCliente[j].id, buffer, strlen(buffer), 0);
    							}
    						}
    						else
    						{
    							bzero(buffer, sizeof(buffer));
    							strcpy(buffer, "Demasiados clientes conectados. SORRY\n");
    							//Enviamos la información al cliente
    							send(new_sd, buffer, strlen(buffer), 0);
    							//Cerramos el nuevo socket
    							close(new_sd);
    						}
    					}
    				}
    				else if(i == 0)
    				{
    					//Se ha introducido información desde el teclado
    					bzero(buffer, sizeof(buffer));
    					fgets(buffer, sizeof(buffer), stdin);

    					//Conprobamos si se ha introducido SALIR, y cerramos todos los socket
    					if(strcmp(buffer, "SALIR\n") == 0)
    					{
    						for(j=0; j<numClientes; j++)
    						{
    							send(arrayCliente[j].id, "+ Ok. Desconexion del servidor\n", strlen("+Ok. Desconexion del servidor\n"), 0);
                                close(arrayCliente[j].id);
                                FD_CLR(arrayCliente[j].id, &readfds);
    						}
                            printf("+Ok. Desconectando servidor\n");
                            close(sd);//Cerramos el socket
                            exit(-1);
    					}
                        //Mensajes que se quieren enviar al cliente
    				}
                    else
                    {
                        bzero(buffer, sizeof(buffer));
                        recibidos = recv(i, buffer, sizeof(buffer),0);

                        if(recibidos > 0)
                        {
                            for(c=0; c<numClientes; c++)
                            {
                                if(arrayCliente[c].id == i)
                                    cliente=c;
                            }
                            if(strcmp(buffer, "SALIR\n")==0)
                            {
                                salirCliente(i, &readfds, &numClientes, arrayCliente);
                            }
                            
                            else if( strncmp(buffer, "USUARIO", 7)==0)
                            {
                                todos=0;
                                if(arrayCliente[cliente].conectado==1)//El usuario quiere conectado
                                {
                                    arrayCliente[cliente].conectado=0;
                                    sscanf(buffer, "%s %s",aux, arrayCliente[cliente].usuario);


                                    for(t=0; t<numClientes;t++)
                                    {
                                        if(strcmp(arrayCliente[t].usuario, usuario))
                                            connect=1;
                                    }
                                    if(arrayCliente[cliente].conectado==0)
                                    {
                                        sscanf(buffer, "%s %s", aux, usuario);
                            
                                        if(buscarUsuario(usuario))//Encontramos el usuario
                                        {
                                            strcpy(arrayCliente[cliente].usuario, usuario);
                                            arrayCliente[cliente].conectado=2;//El ususario está conectado
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "+Ok.Usuario Correcto.\n Bienvenido.\n Por favor, introduce tu contraseña con el comando PASSWORD password");

                                        }         
                                        else if(buscarUsuario(usuario)!= 1)//No encontramos el usuario
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "El nombre de usuario no se ha encontrado en la base de datos\n");
                                        }
                                    }
                                    else if(arrayCliente[cliente].conectado==2)
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "Ya tienes nombre usuario. Introduce el comando 'PASSWORD password\n");
                                        arrayCliente[cliente].validado==1;
                                    }
                                    else if(arrayCliente[cliente]. validado==1)
                                    {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "Ya estás validado, introduce INICIAR-PARTIDA \n");
                                    }
                                    else if(arrayCliente[cliente].jugando==1)
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "Ya estás jugando, introduce un comando de partida. \n");
                                    }
                                    else
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "-Err. Usuario Incorrecto.\n No estás conectado\n");
                                        sprintf(buffer, "%d", connect);
                                    }
                                    //Enviamos esta información al cliente
                                    send(arrayCliente[cliente].id, buffer, strlen(buffer), 0);
                                }
                            }
                            else if(strncmp(buffer, "PASSWORD", 8)==0)
                            {
                                todos=0;
                                if(arrayCliente[cliente].conectado==2)
                                {
                                    sscanf(buffer, "%s %s", aux, password);

                                    if(buscarPassword(arrayCliente[cliente].usuario, password))
                                    {
                                        arrayCliente[cliente].validado=1;
                                        arrayCliente[cliente].conectado=3;
                                        strcpy(arrayCliente[cliente].password, password);

                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "+Ok.Contraseña correcta. \nBienvenido!.\nSeleccione:\n  PARTIDA-INDIVIDUAL:Para una partida tú solo.\n  PARTIDA-GRUPO: Para una partida grupal.\n");
                                    }
                                    else
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "-Err. Contraseña incorrecta\n. Pruebe otra vez.\n");
                                    }
                                }
                                else if(arrayCliente[cliente].validado == 1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya estás validado. Introduce el comando necesario. \n");
                                }
                                else if(arrayCliente[cliente].conectado == 1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "No estás conectado. Introduce tu usuario con USUARIO usuario\n");
                                }
                                else if(arrayCliente[cliente].esperandoJugar == 1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Espera a que empiece la partida\n");
                                }
                                else if(arrayCliente[cliente].jugando == 1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya estás jugando, prueba con comandos de la partida\n");
                                }

                                send(i, buffer, strlen(buffer), 0);
                            }
                            else if(strncmp(buffer,"REGISTER", 8)==0)
                            {
                                    
                                if(arrayCliente[cliente].conectado==1)
                                {
                                
                                    sscanf(buffer, "%s -U %s -P %s ", aux, usuario, password);
                                    if (buscarUsuario(usuario))
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "-Err. Usuario encontrado.\nYa hay un usuario con ese nick, introduce otro. \n");
                                    }
                                    else
                                    {
                                        registrarUsuario(usuario, password);
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "+Ok. Usuario registrado.\nRegistrado, %s - %s\n", usuario, password);
                                    }

                                }
                                else if(arrayCliente[cliente].validado==1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya estás validado. Introduce INICIAR-PARTIDA \n");
                                }
                                else if(arrayCliente[cliente].conectado==2)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya has introducido un USUARIO. Introduce el comando 'PASSWORD password' \n");  
                                }
                                else if(arrayCliente[cliente].esperandoJugar==1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya estás logeado, espera a que empiece la partida.' \n");  
                                }
                                else if(arrayCliente[cliente].jugando==1)
                                {
                                    
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya estás jugando, usa comandos de la partida.' \n");   
                                }
                                send(i, buffer, strlen(buffer),0);
                            }
                            else if(strncmp(buffer, "PARTIDA-INDIVIDUAL", 18)==0)
                            {
                                todos=0;
                                if(arrayCliente[cliente].validado==1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Esperando a encontrar partida...\n");
                                    send(i, buffer, strlen(buffer), 0);
                                    for(m=0;m<NUM_MESAS;m++)
                                    {
                                        if(mesa[m].partida == 0)
                                        {
                                            nJugadores=mesa[m].njugadores;
                                            if(mesa[m].njugadores<1)
                                            {
                                                arrayCliente[cliente].esperandoJugar=1;
                                                arrayCliente[cliente].validado=2;
                                                arrayCliente[cliente].mesa=m;
                                                mesa[m].jugadores[nJugadores]=arrayCliente[cliente];
                                                mesa[m].njugadores++;
                                            }
                                        }
                                    }
                                    if(f=fopen("registro.txt", "w"))//ABRIMOS EL FICHERO PARA TENERLO VACÍO EN CADA PARTIDA
                                    {

                                    }
                                    fclose(f);

                                    if(arrayCliente[cliente].esperandoJugar==1)
                                    {
                                        nmesa=arrayCliente[cliente].mesa;

                                        if(mesa[nmesa].njugadores==1)
                                        {
                                            todos=1;
                                            mesa[nmesa].partida=1;
                                            for(t=0;t<1;t++)
                                            {
                                                //mesa[nmesa].jugadores[t].esperandoJugar=2;
                                                arrayCliente[cliente].jugando=1;
                                                arrayCliente[cliente].partida=1;
                                                //system("clear");
                                                bzero(buffer, sizeof(buffer));
                                                sprintf(buffer, "+Ok. EMPIEZA LA PARTIDA\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ',')\nSALIR: Desconexión\n\n");
                                                send(mesa[nmesa].jugadores[t].id,buffer,strlen(buffer),0);
                                                refran=mostrarPanel(mesa[nmesa].jugadores[t].id);
                                            }
                                            printf("Empieza una partida en la mesa %d\n", nmesa);
                                        }
                                    }
                                }
                                else if(arrayCliente[cliente].conectado==1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "No has iniciado sesión. Usa USUARIO o REGISTER. \n");
                                }
                                else if(arrayCliente[cliente].conectado==2)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya has introducido un USUARIO. Introduce el comando 'PASSWORD password' \n");  
                                }
                                else if(arrayCliente[cliente].esperandoJugar=1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya estás logeado, espera a que empiece la partida.' \n"); 
                                }
                                else if(arrayCliente[cliente].jugando==1)
                                {                                        
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "Ya estás jugando, usa comandos de la partida.' \n");   
                                }
                            }
                            else if(strncmp(buffer, "PARTIDA-GRUPO", 13)==0)
                            {
                                //¿Y si utilizas el toten Ring de SSOO?
                                todos=0;
                                if(arrayCliente[cliente].validado==1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Esperando a encontrar partida...\n");
                                    send(i, buffer, strlen(buffer), 0);
                                    for(m=0;m<NUM_MESAS;m++)
                                    {
                                        if(mesa[m].partida == 0)
                                        {
                                            nJugadores=mesa[m].njugadores;
                                            if(mesa[m].njugadores<3)
                                            {
                                                arrayCliente[cliente].esperandoJugar=1;
                                                arrayCliente[cliente].validado=2;
                                                
                                                arrayCliente[cliente].mesa=m;
                                                mesa[m].jugadores[nJugadores]=arrayCliente[cliente];
                                                mesa[m].njugadores++;
                                            }
                                        }
                                    }
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Esperando a encontrar jugadores...\n");

                                    if(f=fopen("registro.txt", "w"))//ABRIMOS EL FICHERO PARA TENERLO VACÍO EN CADA PARTIDA
                                    {

                                    }
                                    fclose(f);

                                    if(arrayCliente[cliente].esperandoJugar==1)
                                    {
                                        nmesa=arrayCliente[cliente].mesa;

                                        if(mesa[nmesa].njugadores==3)
                                        {
                                            todos=1;
                                            mesa[nmesa].partida=1;
                                            for(t=0;t<3;t++)
                                            {
                                                mesa[nmesa].jugadores[t].esperandoJugar=2;
                                                arrayCliente[cliente].esperandoJugar=2;
                                                arrayCliente[cliente].partida=2;
                                                arrayCliente[cliente].jugando=1;                                                
                                                bzero(buffer, sizeof(buffer));
                                                sprintf(buffer, "+ Ok. EMPIEZA LA PARTIDA\n-------COMANDOS-------\nPASO-TURNO: Pasas el turno al siguiente.\nCONSONANTE: Introduces una consonante.\nSALIR: Desconexión\n\n");
                                                send(mesa[nmesa].jugadores[t].id,buffer,strlen(buffer),0);
                                                mostrarPanel(mesa[nmesa].jugadores[t].id);
                                            }
                                            printf("Empieza una partida en la mesa %d\n", nmesa);
                                        }
                                    }
                                }
                                else if(arrayCliente[cliente].conectado==1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "No has iniciado sesión. Usa USUARIO o REGISTER. \n");
                                }
                                else if(arrayCliente[cliente].conectado==2)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya has introducido un USUARIO. Introduce el comando 'PASSWORD password' \n");  
                                }
                                else if(arrayCliente[cliente].esperandoJugar=1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya estás logeado, espera a que empiece la partida.' \n"); 
                                }
                                else if(arrayCliente[cliente].jugando==1)
                                {                                        
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "Ya estás jugando, usa comandos de la partida.' \n");   
                                }
                            }
                            else if(strncmp(buffer, "CONSONANTE",10)==0)
                            {
                                if(arrayCliente[cliente].partida==1)//PARTIDA INDIVIDUAL
                                {
                                    sscanf(buffer, "%s %c", aux, &consonante);
                                    if(consonante=='A' || consonante=='E'|| consonante=='I'|| consonante=='O'|| consonante=='U')
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "-Err. Eso es una vocal. Por favor, introduzca otra\n");
                                        send(id, buffer,strlen(buffer), 0);
                                    }
                                    else
                                    {                   
                                        nmesa=arrayCliente[cliente].mesa;
                                        id=arrayCliente[cliente].id;
                                        numAciertos=isConsonante(consonante, id, refran);
                                        numIntentos++;

                                        if(numAciertos>0)//Vemos si la consonante es correcta
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "+Ok. HAS ACERTADO %d CONSONANTE(S)!\n",numAciertos );
                                            send(id, buffer,strlen(buffer), 0);
                                            arrayCliente[cliente].puntos++;
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "TU NUMERO DE INTENTOS ES: %d \n", arrayCliente[cliente].puntos);
                                            send(id, buffer,strlen(buffer), 0);
                                            sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ',')\nSALIR: Desconexión\n\n");
                                            send(id,buffer,strlen(buffer),0);
                                        }
                                        else if(numAciertos==-2)
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "YA LA HAS ACERTADO\n");
                                            send(id, buffer,strlen(buffer), 0);
                                            sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ',')\nSALIR: Desconexión\n\n");
                                            send(id,buffer,strlen(buffer),0);    
                                        }
                                        else if(numAciertos==-1)
                                        {
                                            ganador=arrayCliente[cliente].usuario;
                                            sprintf(buffer, "EL JUGADOR %s HA ADIVINADO LA FRASE!\n", ganador);
                                            send(id,buffer,strlen(buffer),0); 
                                        }
                                        else 
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "-Err. HAS FALLADO");   
                                            send(id, buffer,strlen(buffer), 0);
                                            sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ',')\nSALIR: Desconexión\n\n");
                                            send(id,buffer,strlen(buffer),0);
                                        }
                                    }                                 
                                }
                                else if(arrayCliente[cliente].partida==2)//PARTIDA GRUPAL
                                {                                   
                                    sscanf(buffer, "%s %c", aux, &consonante);
                                    if(consonante=='A' || consonante=='E'|| consonante=='I'|| consonante=='O'|| consonante=='U')
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "-Err. Eso es una vocal. Por favor, introduzca otra\n");
                                        send(id, buffer,strlen(buffer), 0);
                                    }
                                    else
                                    {                   
                                        nmesa=arrayCliente[cliente].mesa;
                                        id=arrayCliente[cliente].id;
                                        numAciertos=isConsonante(consonante, id, refran);

                                        if(numAciertos>0)//Vemos si la consonante es correcta
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "+Ok. HAS ACERTADO %d CONSONANTE(S)!\n",numAciertos );
                                            send(id, buffer,strlen(buffer), 0);
                                            arrayCliente[cliente].puntos+=(50*numAciertos);
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "TU PUNTUACIÓN ACTUAL ES: %d \n", arrayCliente[cliente].puntos);
                                            send(id, buffer,strlen(buffer), 0);
                                            sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ,'z')\nSALIR: Desconexión\n\n");
                                            send(id,buffer,strlen(buffer),0);
                                        }
                                        else if(numAciertos==-2)
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "YA LA HAS ACERTADO\n");
                                            send(id, buffer,strlen(buffer), 0);
                                            sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ,'z')\nSALIR: Desconexión\n\n");
                                            send(id,buffer,strlen(buffer),0);    
                                        }
                                        else if(numAciertos==-1)
                                        {
                                            sprintf(buffer, "HAS ADIVINADO LA FRASEEEE!\n");
                                            send(id,buffer,strlen(buffer),0); 
                                        }
                                        else 
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "-Err. HAS FALLADO");   
                                            send(id, buffer,strlen(buffer), 0);
                                            sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ,'z')\nSALIR: Desconexión\n\n");
                                            send(id,buffer,strlen(buffer),0);
                                            mesa[nmesa].turno++;
                                       }
                                    }                       
                                }
                                else if(arrayCliente[cliente].conectado==1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "No has iniciado sesión. Usa USUARIO o REGISTER. \n");

                                }
                                else if(arrayCliente[cliente].conectado==2)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya has introducido un USUARIO. Introduce el comando 'PASSWORD password' \n");  
                                    send(i, buffer, strlen(buffer),0); 
                                }
                                else if(arrayCliente[cliente].esperandoJugar=1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya estás logeado, espera a que empiece la partida.' \n"); 
                                    send(i, buffer, strlen(buffer),0); 
                                }
                                else if(arrayCliente[cliente].jugando==1)
                                {                                        
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "Ya estás jugando, usa comandos de la partida.' \n");  
                                        send(i, buffer, strlen(buffer),0);  
                                }  
                                else
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "NO HAS INTRODUCIDO EL COMANDO PARA JUGAR LA PARTIDA\n");
                                    send(id, buffer, strlen(buffer), 0);
                                }
                            }
                            else if(strncmp(buffer, "VOCAL",5)==0)
                            {
                                if(arrayCliente[cliente].partida==1)//PARTIDA INDIVIDUAL
                                {                                     
                                    sscanf(buffer, "%s %c", aux, &vocal);
                                    if(vocal=='A' || vocal=='E'|| vocal=='I'|| vocal=='O'|| vocal=='U')
                                    {
                                        nmesa=arrayCliente[cliente].mesa;
                                        id=arrayCliente[cliente].id;
                                        numAciertos=isVocal(vocal, id, refran);
                                        numIntentos++;

                                        if(numAciertos>0)//Vemos si la vocal es correcta
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "+OK. HAS ACERTADO UNA VOCAL!\n");
                                            send(id, buffer,strlen(buffer),0);
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "TUS INTENTOS SON:%d", numIntentos); 
                                            send(id, buffer,strlen(buffer), 0);
                                            sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ',')\nSALIR: Desconexión\n\n");
                                            send(id,buffer,strlen(buffer),0);
                                        }
                                        else if(numAciertos==-2)
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "-Err. YA LA HAS ACERTADO\n");
                                            send(id, buffer,strlen(buffer), 0);
                                            sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ',')\nSALIR: Desconexión\n\n");
                                            send(id,buffer,strlen(buffer),0);    
                                        }
                                        else if(numAciertos==-1)
                                        {
                                            ganador=arrayCliente[cliente].usuario;
                                            sprintf(buffer, "+Ok. EL JUGADOR %s HA ADIVINADO LA FRASE!\n", ganador);
                                            send(id,buffer,strlen(buffer),0); 
                                        }
                                        else 
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "- Err. HAS FALLADO\n");   
                                            send(id, buffer,strlen(buffer), 0);
                                            sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ',')\nSALIR: Desconexión\n\n");
                                            send(id,buffer,strlen(buffer),0);
                                        }
                                    }
                                    else
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "-Err. Eso no es una vocal. Por favor, introduzca otra\n");
                                        send(id, buffer,strlen(buffer), 0);
                                    }
                                }
                                else if(arrayCliente[cliente].partida==2)//PARTIDA GRUPAL
                                {
                                    if (arrayCliente[cliente].puntos>=50)
                                    {
                                        sscanf(buffer, "%s %c", aux, &vocal);
                                        if(vocal=='A' || vocal=='E'|| vocal=='I'|| vocal=='O'|| vocal=='U')
                                        {
                                            nmesa=arrayCliente[cliente].mesa;
                                            id=arrayCliente[cliente].id;
                                            numAciertos=isVocal(vocal, id, refran);

                                            if(numAciertos>0)//Vemos si la vocal es correcta
                                            {
                                                bzero(buffer, sizeof(buffer));
                                                sprintf(buffer, "+OK. HAS ACERTADO UNA VOCAL!\n");
                                                send(id, buffer,strlen(buffer), 0);
                                                arrayCliente[cliente].puntos= arrayCliente[cliente].puntos-50;
                                                bzero(buffer, sizeof(buffer));
                                                sprintf(buffer, "TU PUNTUACIÓN ACTUAL ES: %d \n", arrayCliente[cliente].puntos);
                                                send(id, buffer,strlen(buffer), 0);
                                                sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ',')\nSALIR: Desconexión\n\n");
                                                send(id,buffer,strlen(buffer),0);
                                            }
                                            else if(numAciertos==-2)
                                            {
                                                bzero(buffer, sizeof(buffer));
                                                sprintf(buffer, "YA LA HAS ACERTADO\n");
                                                send(id, buffer,strlen(buffer), 0);
                                                sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ',')\nSALIR: Desconexión\n\n");
                                                send(id,buffer,strlen(buffer),0);    
                                            }
                                            else 
                                            {
                                                bzero(buffer, sizeof(buffer));
                                                sprintf(buffer, "- Err. HAS FALLADO\n");   
                                                send(id, buffer,strlen(buffer), 0);
                                                sprintf(buffer, "\n-------COMANDOS-------\nCONSONANTE: Introduces una consonante.\nVOCAL: Introduces una vocal.\nRESOLVER: Resolver la frase (No olvidar introducir las ',')\nSALIR: Desconexión\n\n");
                                                send(id,buffer,strlen(buffer),0);
                                            }
                                        }
                                        else
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            sprintf(buffer, "-Err. Eso no es una vocal. Por favor, introduzca otra\n");
                                            send(id, buffer,strlen(buffer), 0);
                                        }
                                    }
                                    else
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "-Err. No tienes los puntos necesarios como para comprar una vocal.\n");
                                        send(id, buffer,strlen(buffer), 0); 
                                    }
                                    
                                }
                                else if(arrayCliente[cliente].conectado==1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "No has iniciado sesión. Usa USUARIO o REGISTER. \n");

                                }
                                else if(arrayCliente[cliente].conectado==2)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya has introducido un USUARIO. Introduce el comando 'PASSWORD password' \n");  
                                    send(i, buffer, strlen(buffer),0); 
                                }
                                else if(arrayCliente[cliente].esperandoJugar=1)
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Ya estás logeado, espera a que empiece la partida.' \n"); 
                                    send(i, buffer, strlen(buffer),0); 
                                }
                                else if(arrayCliente[cliente].jugando==1)
                                {                                        
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "Ya estás jugando, usa comandos de la partida.' \n");  
                                        send(i, buffer, strlen(buffer),0);  
                                }  
                                else
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "NO HAS INTRODUCIDO EL COMANDO PARA JUGAR LA PARTIDA\n");
                                    send(id, buffer, strlen(buffer), 0);
                                }
                            }
                            else if(strncmp(buffer, "RESOLVER", 8)==0)
                            {
                                if(arrayCliente[cliente].partida=1)
                                {
                                    sscanf(buffer, "%s", sol);
                                    nmesa=arrayCliente[cliente].mesa;
                                    id=arrayCliente[cliente].id;
                                    numAciertos=resolver(buffer, id, refran);

                                    if(numAciertos==1)
                                    {
                                        ganador=arrayCliente[cliente].usuario;
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "+Ok. EL JUGADOR %s HA ADIVINADO LA FRASE!\n", ganador);
                                        send(arrayCliente[cliente].id,buffer,strlen(buffer),0); 
                                    }
                                    else
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "HAS FALLADO. VUELVE A INTENTARLO\n");
                                        send(arrayCliente[cliente].id, buffer, strlen(buffer), 0);   
                                    }
                                }
                                else if(arrayCliente[cliente].partida==2)
                                {

                                }
                            }
                            else 
                            {
                                for(i = 0; i < numClientes; ++i)
                                {
                                    send(arrayCliente[j].id, "- Err. COMANDO NO VÁLIDO. POR FAVOR, INTRODUCE OTRO\n", strlen("COMANDO NO VÁLIDO. POR FAVOR, INTRODUCE OTRO\n"), 0);
                                }
                            }

                        }
                        //Si el cliente introdujo ctrl+c
                        if(recibidos== 0)
                        {
                            printf("El socket %d, ha introducido ctrl+c\n", i);
                            //Eliminar ese socket
                            salirCliente(i,&readfds,&numClientes,arrayCliente);
                        }
                    }
    			}
    		}
    	}
    }
    close(sd);
}