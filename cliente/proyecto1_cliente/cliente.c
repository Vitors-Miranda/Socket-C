/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. de Ingeníería de Telecomunicación
 * Universidad de Jaén
 *
 *******************************************************
 * Práctica 1
 * Fichero: cliente.c
 * Versión: 3.1
 * Curso: 2024/2025
 * Descripción: Cliente sencillo TCP para IPv4 e IPv6
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************************************************
 * Alumno 1: Miranda de Souza Vitor Samuel
 * Alumno 2: Silva Carvalho Alicia Gianny
 *
 ******************************************************/
#include <stdio.h>		// Biblioteca estándar de entrada y salida
#include <ws2tcpip.h>	// Necesaria para las funciones IPv6
#include <conio.h>		// Biblioteca de entrada salida básica
#include <locale.h>		// Para establecer el idioma de la codificación de texto, números, etc.
#include "protocol.h"	// Declarar constantes y funciones de la práctica
#include <math.h>

#pragma comment(lib, "Ws2_32.lib")//Inserta en la vinculación (linking) la biblioteca Ws2_32.lib


int main(int* argc, char* argv[])
{
	SOCKET sockfd;
	struct sockaddr* server_in = NULL;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024];
	int recibidos = 0, enviados = 0;
	int estado;
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256];
	char default_ip4[16] = "127.0.0.1";
	char default_ip6[64] = "::1";

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	//Inicialización de idioma
	setlocale(LC_ALL, "es-ES");


	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
		return(0);
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	printf("**************\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n*************\r\n");

	do {
		printf("CLIENTE> ¿Qué versión de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			//Si se introduce 6 se empleará IPv6
			ipversion = AF_INET6;

		}
		else { //Distinto de 6 se elige la versión IPv4
			ipversion = AF_INET;
		}

		sockfd = socket(ipversion, SOCK_STREAM, 0);  // SOCKET With "sockfd" we are initing a Socket to establish a connection with the server side to recieve requests
		/*
		PARAMETERS
		1) The first argument receive a adress family that will be used in our connection (AFNAT for IPV4 and AFNAT6 for IPV6).
		2) The second one is the type the new socket. In this case we are using "SOCK_STREAM" based in TCP with OOB data, in other words, it is a independent transmission.
		3) The third argument receive the protocol of our connection, in this case we are using the Transmission Control Protocol (TCP).
		*/
		if (sockfd == INVALID_SOCKET) {
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else {
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets_s(ipdest, sizeof(ipdest));

			//Dirección por defecto según la familia
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET)
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);

			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET6)
				strcpy_s(ipdest, sizeof(ipdest), default_ip6);

			if (ipversion == AF_INET) {
				server_in4.sin_family = AF_INET;
				server_in4.sin_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*) & server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in6.sin6_addr);
				server_in = (struct sockaddr*) & server_in6;
				address_size = sizeof(server_in6);
			}

			

			//Cada nueva conexión establece el estado incial en
			estado = S_INIT;

			if (connect(sockfd, server_in, address_size) == 0) { // SOCKET establishes a connection to a socket.
				/*
					PARAMETERS 
					1) That socket that will be used to connect.
					2) The socket adress that will be connect.
					3) the size of the adress structure
				*/
				

				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);

				//Inicio de la máquina de estados
				do {
					switch (estado) {
					case S_INIT:
						// Se recibe el mensaje de bienvenida
						break;
					case S_USER:
						// establece la conexion de aplicacion 
						printf("CLIENTE> Introduzca el usuario (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}
						else {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", SC, input, CRLF);
						}
						break;
					case S_PASS:
						printf("CLIENTE> Introduzca la clave (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}
						else
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", PW, input, CRLF);
						break;
					case S_DATA:
						printf("1. Echo \n2. EQ\n3. Salir\n");
						option = _getche();

						// Integer values of A,B and C
						int a = -100, b = -100, c = -100;

						switch (option) {

						//ECHO
						case '1':
							printf("CLIENTE> Introduzca la cadena para envial al servidor: \n");
							gets_s(input, sizeof(input));
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", ECHO, input, CRLF);
							break;

						//EQUATION
						case '2':
							printf("CLIENTE> Va a calcular la ecuacion de 2º grado:\n");
							boolean condicion;
							
							//do {
								//recieving values from user

								printf("CLIENTE> Escriba los tres coeficientes (a b c) entre -99 y 99 separados por un espacio:\n");
								scanf_s("%d %d %d", &a, &b, &c);

								//Condition to verify the range of our values.
								//condicion = ((a > 99 || a < -99) || (b > 99 || b < -99) || (c > 99 || c < -99));
								
								//if(condicion){
									//printf("Coeficientes fuera de rango. Inténtelo de nuevo. \n");
								//}

							//} while (condicion);

							//Sending data
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %d %d %d%s", EQ2D, a, b, c, CRLF);
							break;
								
						//QUIT
						case '3':
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
							break;

						//INVALID OPTION
						default:
							printf("CLIENTE> Opcion invalida");
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
							break;
						}
					}
					if (estado != S_INIT) {
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);  //SOCKET it sends a data to a connected socket.
						/*
							PARAMETERS
							1) The connected socket.
							2) The data to be transmitted.
							3) The length of the data that will be transmitted.
						*/
						if (enviados == SOCKET_ERROR) {
							estado = S_QUIT;
							continue;// La sentencia continue hace que la ejecución dentro de un
									 // bucle salte hasta la comprobación del mismo.
						}
					}

					recibidos = recv(sockfd, buffer_in, 512, 0); //SOCKET it recieves data of the client.
					/*
					PARAMETERS
					1) The connected socket.
					2) THE Buffer that will save the data.
					3) The length of the data that will be saved.
					*/
					if (recibidos <= 0) {
						DWORD error = GetLastError();
						if (recibidos < 0) {
							printf("CLIENTE> Error %d en la recepción de datos\r\n", error);
							estado = S_QUIT;
						}
						else {
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							estado = S_QUIT;
						}
					}
					else {
						buffer_in[recibidos] = 0x00;
						printf(buffer_in);
						if (estado != S_DATA && strncmp(buffer_in, OK, 2) == 0){
							estado++;
						}
						//Si la autenticación no es correcta se vuelve al estado S_USER
						if (estado == S_PASS && strncmp(buffer_in, OK, 2) != 0) {
							estado = S_USER;
						}
					}

				} while (estado != S_QUIT);
			}
			else {
				int error_code = GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);
			}
			closesocket(sockfd);

		}
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option = _getche();

	} while (option != 'n' && option != 'N');

	return(0);
}
