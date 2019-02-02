#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define MSG_SIZE 250
#define MAX_CLIENTS 50
#define NUM_MESAS 10


struct Cliente
{
    char usuario[20];
    char password[50];
    char id;//Es el socker
    int conectado;
    int validado;
    int jugando;
    int esperandoJugar;
   	int puntos;
    int intentos;
   	int mesa;
    int partida;//1-Partida individual, 2-Partida grupal
};

struct Mesa
{
	int njugadores;
	struct Cliente jugadores[3];
	int partida;//Partida en esa mesa
    int turno;//1-Turno 0-No turno
	int puntos;
	int fin;
};

int buscarUsuario(char cadena[]){       
        struct Cliente c;
        int find=0;
        FILE * f;
        if(f=fopen("usuarios.txt","r")){
                while(fscanf(f,"%s %s\n", c.usuario, c.password)==2){
                        if(strcmp(cadena, c.usuario)==0)
                        {
                            find=1;
                        }
                }
                fclose(f);
        }else{
                printf("ERROR al abrir el fichero.\n");
        }

        return find;    
}

int buscarPassword(char user[], char pass[]){       
        struct Cliente c;
        int find=0;
        FILE * f;
        if(f=fopen("usuarios.txt","r")){
                while(fscanf(f,"%s %s\n",c.usuario, c.password)==2){
                        if(strcmp(user, c.usuario)==0 && strcmp(pass, c.password)==0)
                                find=1;
                }
                fclose(f);
        }else{
                printf("ERROR al abrir el fichero.\n");
        }

        return find;    
}

int registrarUsuario(char user[], char pass[]){
	FILE *f;
	struct Cliente c;
	if(f=fopen("usuarios.txt","a")){
		fprintf(f,"%s %s\n", user, pass);
		fclose(f);
	return 1;
	}
	return 0;
}

int leerRegistro(char aux[MSG_SIZE], int i, int jugador, char consonante)
{
    int j, enc=0, lineaAct;
    char buffer[MSG_SIZE];
    char buffer2[MSG_SIZE]; 
    int term=0;
    FILE *f2;
    FILE *f3;

   if(f3=fopen("registro.txt", "r"))
    {    
        fgets(buffer2, MSG_SIZE, f3); 
    
        if (f2=fopen("registro.txt", "w+"))
        {   
            
            for(j=0;j<i;j++)
            {

                if(aux[j]==consonante)
                {
                    buffer[j]=consonante;
                    fprintf(f2, "%c", buffer[j]);
                    enc++;
                }
                else if(aux[j]==buffer2[j])//VEMOS SI LA HEMOS ACERTADO ANTES
                {
                    buffer[j]=buffer2[j];
                    fprintf(f2, "%c", buffer[j]);
                } 
                else if(aux[j]!=' ' && aux[j]!= '\n' && aux[j]!=',')
                {
                    buffer[j]='-';
                    fprintf(f2, "-");                    
                }
                else if(aux[j]==' ')
                {
                    buffer[j]=' ';
                    fprintf(f2, " ");
                }
                else if(aux[j]==',')
                {
                    buffer[j]=',';
                    fprintf(f2, ",");
                }
                else if(aux[j]=='\0')
                {
                    buffer[j]='\0';
                    fprintf(f2, "\0");   
                }
                else if(aux[j]=='\n')
                {
                    buffer[j]='\n';
                    fprintf(f2, "\n");
                }
            }
            fprintf(f2, "\n");
        }
        for(j=0;j<i;j++)
        {
            if(buffer2[j]==consonante)//YA LA HEMOS ACERTADO ANTERIORMENTE
            {
                enc=-2;
            }
            if(aux[j]==buffer2[j])
            {
                term++;//vEMOS SI HEMOS TERMINADO LA FRASE
            }
        }

    }
    if(term==i-1)
    {
        enc=-1;
    }
    send(jugador, buffer,strlen(buffer), 0);
    fclose(f2);
    fclose(f3);

    return enc;
}

int isConsonante(char consonante, int jugador, int refran)
{
    int enc;
    int i,x=0;
    char aux[MSG_SIZE];
    FILE *f;

    if(f=fopen("refranes.txt", "r"))
    { 
        while(fgets(aux, MSG_SIZE, f))
        {
            if(x==refran)
            {
                i=strlen(aux);
                enc=leerRegistro(aux, i, jugador, consonante);
            }
            x++;
        }

    }
    fclose(f); 
    return enc;
}

int isVocal(char vocal, int jugador, int refran)
{
    int enc;
    int i, x=0;
    char aux[MSG_SIZE];
    FILE *f;

    if(f=fopen("refranes.txt", "r"))
    {       
        while(fgets(aux, MSG_SIZE, f))
        {
            if(x==refran)
            {
                i=strlen(aux);
                enc=leerRegistro(aux, i, jugador, vocal);
            }
            x++;
        }
    }
    fclose(f); 
    return enc;
}

int resolver(char buffer[], int jugador, int refran)
{
    int enc;
    int i,z=0, x=0;
    char aux[MSG_SIZE];
    char correcto[MSG_SIZE];
    FILE *f;

    if(f=fopen("refranes.txt", "r"))
    {
        while(fgets(aux, MSG_SIZE, f))
        {
            if(x==refran)
            {
                 i=strlen(aux);
                for(i=0;i<strlen(buffer); i++)
                {
                    if(buffer[i]== ' ')
                    {
                        for(i = i+1; i<strlen(buffer); i++)
                        {
                            if(aux[z]==buffer[i])
                            {
                                correcto[z]= buffer[i];
                                z++;
                            }
                            correcto[i]= '\0';
                        }
                    }                                            
                }
                if (strcmp(correcto, aux)==0)
                {
                    enc=1;            
                }                 
            }
            x++;
        }
        
    }
    fclose(f); 
    return enc;   
}

int mostrarPanel(int jugador)
{
    char aux[MSG_SIZE];
    char buffer[MSG_SIZE];
    char buffer2[MSG_SIZE];
    int i,j;
    int opc, x=0;
    FILE *f;
    system("clear");

    srand(time(NULL));
    opc=rand()%15;
    

    if(f=fopen("refranes.txt", "r"))
    {
        while(fgets(aux, MSG_SIZE, f))
        {
            if(x==opc)
            {
                i=strlen(aux);
                for(j=0;j<i-1;j++)
                {
                    if(aux[j]!=' ' && aux[j]!= '\n' && aux[j]!=',')
                    {
                        aux[j]='-';
                        
                    }
                    else aux[j]=' ';
                }
                bzero(buffer2, sizeof(buffer2));
                sprintf(buffer2, "REFRÁN DE HOY\n");
                send(jugador, buffer2,strlen(buffer2), 0);

                bzero(buffer, sizeof(buffer));
                strcpy(buffer, aux);
                send(jugador, buffer,strlen(buffer), 0);
            }
            x++;
        }
        
    }
    fclose(f);
    return opc;  
}

#endif

