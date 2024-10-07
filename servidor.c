/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. Ingeníería de Telecomunicación
 * Universidad de Jaén
 *
 *******************************************************
 * Práctica 2.
 * Fichero: servidor.c
 * Versión: 1.2
 * Curso: 2024/2025
 * Descripción:
 * 	Servidor de eco sencillo sobre UDP en IPv4
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************************************************
 * Alumno 1: Vitor Samuel Miranda de Souza
 * Alumno 2: Alicia Gianny Silva Caralho
 *
 ******************************************************/
#include <stdio.h>		// Biblioteca estándar de entrada y salida
#include <ws2tcpip.h>	// Necesaria para las funciones IPv6
#include <locale.h>		// Para establecer el idioma de la codificación de texto, números, etc.

#define UDP_CLIENT_PORT	60001//Puerto del que deben venir los mensajes
#define UDP_SERVER_PORT	60000//Puerto en el que el servidor recibirá peticiones

#pragma comment(lib, "Ws2_32.lib")//Enlaza la biblioteca Ws2_32.lib

int main(int *argc, char *argv[])
{
	// Variables de incialización de los Windows Sockets
	WORD wVersionRequested;
	WSADATA wsaData;
	// Fin Variables de incialización de los Windows Sockets

	SOCKET sockfd;
	struct sockaddr_in server_in,input_in;
	int input_l;
	char buffer_in[2048], buffer_out[2048];
	char command[10];
	char user_input[1024];
	int recibidos=0;
	int enviados=0;
	char iplocal[20]="127.0.0.1";
	int n_secuencia=0;
	int err=0;

	//Inicialización de idioma
	setlocale(LC_ALL, "es-ES");

	// Inicialización Windows Sockets
	wVersionRequested=MAKEWORD(2,2);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=2||HIBYTE(wsaData.wVersion)!=2){
		WSACleanup();
		return(0);
	}// Fin Inicialización Windows Sockets
	
	sockfd=socket(PF_INET,SOCK_DGRAM,0);
	if(sockfd==INVALID_SOCKET){
		printf("SERVIDOR UDP> Error \r\n");
	}else{
		printf("SERVIDOR UDP> Socket creado correctamente\r\n");

		//Dirección local del servidor
		server_in.sin_family=AF_INET;
		server_in.sin_port=htons(UDP_SERVER_PORT);
		//server_in.sin_addr.s_addr=inet_addr(iplocal);
		inet_pton(AF_INET,iplocal,&server_in.sin_addr.s_addr);
		
		if(bind(sockfd,(struct sockaddr *)&server_in,sizeof(server_in))==SOCKET_ERROR){
			printf("SERVIDOR UDP> Error %d:\r\n",GetLastError());
		}else{
			printf("SERVIDOR UDP> Bienvenido al Servidor de Eco Sencillo UDP\r\n");

			while(1){//Bucle infinito de servicio
				input_l=sizeof(input_in);
				recibidos=recvfrom(sockfd,buffer_in,2047,0,(struct sockaddr *)&input_in,&input_l);
				if(recibidos!=SOCKET_ERROR){
					char peer[32]="";
					buffer_in[recibidos]=0;
					inet_ntop(AF_INET, &input_in.sin_addr, peer, sizeof(peer));
					printf("SERVIDOR UDP> Recibido de %s %d: %s\r\n",peer,ntohs(input_in.sin_port),buffer_in);
					if(ntohs(input_in.sin_port)==UDP_CLIENT_PORT){// Se comprueba que el mensaje llegue desde el puerto típico para
																  // este servicio, el 6001. Si no es así no se lleva a cabo ninguna
																  // acción.
						sscanf_s(buffer_in,"%s %d %[^\r]s\r\n",command,sizeof(command),&n_secuencia,user_input,sizeof(user_input));

						if(strcmp(command,"ECHO")==0){// Si el mensaje no está bien formateado tampoco se responde para evitar
													  // un gasto de recursos innecesario
							sprintf_s(buffer_out,sizeof(buffer_out),"OK %d %s\r\n",n_secuencia,user_input);
							
						}else{
							printf("SERVIDOR UDP> Comando no reconocido\r\n");
						}
						enviados=sendto(sockfd,buffer_out,(int)strlen(buffer_out),0,(struct sockaddr *)&input_in,sizeof(input_in));
						if (enviados == SOCKET_ERROR) {
							printf("SERVIDOR UDP> Error al enviar la respuesta.");
						}
					}
				}//Si hay un error de recepción se silencia
			}//Fin bucle del servicio
		}	
		closesocket(sockfd);
	}//fin sockfd==INVALID_SOCKET
}//fin main