#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32")

/* CS423 Project 3
	Aldo Anaya
	This is the Client side of a TCP IM program.  The client will first sign on with the server
	and given the list of other active clients, and send messages directly to other clients
	It will need to encrypt outgoing, and decrypt incoming messages*/

#include<stdio.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<process.h>
#include<sstream>
#include<string>
#include<list>
#include<iostream>
#include"cipher.h"

using namespace std;

char *server = "192.168.10.200";
char *port = "34567";
int typeOfMessage = 0;
char server_reply[1000];
void listen(void *);		// listen to socket continuesly with second thread
void *get_in_addr(struct sockaddr *);	// used to see whether remote addr is IPv4 or IPv6
void sendMsg(int);
string encryptMessage(string);
struct active_user {
	string user;
	char *addr;
	char *port;
	active_user(string user1, char *addr1, char *port1) :
		user(user1), addr(addr1), port(port1) {}

};
list<active_user> users;
SOCKET s;
string user = "";
int main() {
	WSADATA wsa;

	struct active_user server1 = active_user("server", server, port);
	users.push_back(server1);
	//	new 4.26.16-00:18
	struct sockaddr_in server;
	//struct sockaddr_in server;	
	//char *message;
//	int recv_size;
	int return_value;
	cout << "Welcome to CS423 TCP IM program" << endl;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d", WSAGetLastError());		
		return 1;
	}
	struct addrinfo server_struct, *servinfo,*p;
//	struct sockaddr_storage their_addr;
	memset(&server_struct, 0, sizeof server_struct);
	server_struct.ai_family = AF_UNSPEC;
	server_struct.ai_socktype = SOCK_STREAM;
	//Create a socket	
	char *ser;
	ser = "192.168.10.200";

	if ((return_value = getaddrinfo(server1.addr, server1.port, &server_struct, &servinfo)) != 0) {
		printf("getaddrinfo: %s\n", gai_strerror(return_value));
		return 1;
	}
	s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);	
	//set SO_RESESADDR on a socket s to true	
	// new 4.26.16-00:16
	//if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
	//	printf("Could not create socket: %d", WSAGetLastError());
	//	return 1;
	//}
	//server.sin_addr.s_addr = inet_addr(SERVER);
	//server.sin_family = AF_INET;
	//server.sin_port = htons(34567);


	int optval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval);

	//Connect to remote/local TCP server	
	connect(s, servinfo->ai_addr, servinfo->ai_addrlen);
	printf("Connected to %s\n", server1.addr);
	//char *message;
	//message = "hello world";
	//if (send(s, message, strlen(message), 0) < 0) {
	//	cout << "Send Failed" << endl;
	//	return 1;
	//}
	//else {
	//	cout << "sent" << endl;
	//}

	//				NEW
	printf("Enter your IM name: \n");
	getline(cin, user);

	// use send msg function to send initial function.  not created yet 11:31:4/24/16
	sendMsg(1);
	_beginthread(listen, 0, (void *)&s);  // create listen function for second thread
	char action = 'c';

	while (action != 'q') {
		if (typeOfMessage == 0) {
			cout << "Enter q (for quit), s (send message), or c (check messages)" << endl;
			cin >> action;
		}
		else {
			action = 'c';
		}

		switch (action) {
			case 'S':
			case 's':
			{
				break;
			}
			case 'C':
			case 'c':
			{
				if (typeOfMessage == 0)
					printf("No new Messages\n");
				else {
					printf("New Message Arrived \n");
					cout << server_reply << endl;
					typeOfMessage == 0;
				}
				break;
			}
			case 'Q':
			case 'q':
			{
				action = 'q';
				break;
			}
			default:
				break;
		}

	}

/*
	//			original code  
	//Send some data									//
	message = "This is AMAZING";						//
	if (send(s, message, strlen(message), 0) < 0) {		//
		cout << "Send Failed" << endl;					//
		return 1;										//
	}									
	cout << "Data Sent" << endl;

	//Receive a reply from the server	
	if ((recv_size = recv(s, server_reply, 500, 0)) == SOCKET_ERROR) {
		cout << "recv failed" << endl;
		return 1;
	}
	cout << "reply Received" << endl;
	server_reply[recv_size] = '\0';
	cout << server_reply << endl;
*/

	WSACleanup();
	return 0;											

}

void listen(void * socket) {
	int recv_size;
	int last_char = 0;
	SOCKET s;
	string num;
	s = *(SOCKET *)socket;
	char new_reply[500];
	while (true) {
		if (typeOfMessage == 0){
			do {
				if ((recv_size = recv(s, new_reply, 500, 0)) != SOCKET_ERROR) {
					for (int i = last_char; i < (last_char + recv_size); i++) {
						server_reply[i] = new_reply[i];
					}
					last_char += recv_size;
				}
			} while (new_reply[recv_size] != '#');
			if (isdigit(server_reply[0])) {
				num = server_reply[0];
				typeOfMessage = server_reply[0] - '0';
			}
			else {
				typeOfMessage = 5; // for error
			}
		}
	}
	//printf("\n\nNew Message is %s\n", server_reply);
}

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sendMsg(int msgType) {
	struct addrinfo serv, *server_info;
	//int return_value;
	char *message;
	string msg = "";

	switch (msgType) {
		case 1:
		{
			/*if ((return_value = getaddrinfo(SERVER, PORT, &serv, &server_info)) != 0) {
				printf("getaddrinfo: %s\n", gai_strerror(return_value));
				exit(1);//return 1;
			}
			SOCKET s = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
			connect(s, server_info->ai_addr, server_info->ai_addrlen);*/
			msg = "1;" + user + ";34567#";
			message = new char[msg.length() + 1];
			strcpy(message, encryptMessage(msg).c_str()); // encrypt message and switch string to char* to send message
			if (send(s, message, strlen(message), 0) < 0) {		//
				cout << "Send Failed" << endl;					//
				exit(1);										//
			}
			//closesocket(s);
		}
		default:
			break;
	}
}

string encryptMessage(string buf) {
	string dMessage = "";
	int next = 0;
	int recv_size = buf.length();
	for (int i = 0; i < recv_size; i++){
		if (buf[i] == ';') {
			next = ++i;
			break;
		}
	}
	for (int j = next; j < recv_size;j++) {
		buf[j] = encrypt(buf[j]);
	}
	return buf;
}