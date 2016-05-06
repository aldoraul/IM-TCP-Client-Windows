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
//#include<Windows.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<process.h>
#include<sstream>
#include<string>
#include<list>
#include<iostream>
#include"cipher.h"


using namespace std;
int end_flag = 1;	// set to 0 to end program
int flag1 = 0; // 0 pass, 1 stop to read
#define	BACKLOG 10
#define BUFSIZE 500
char *server = "192.168.10.200";
char *port = "34567";
int typeOfMessage = 0;
char server_reply[1000];
string serv_reply = "";
void listen(void *);		// listen to socket continuesly with second thread
int New_Process(SOCKET);
void sendMsg(int);
string decryptMessage(string);
string encryptMessage(string);
struct active_user {
	string user;
	char *addr;
	char *port;
	active_user(string user1, char *addr1, char *port1) :
		user(user1), addr(addr1), port(port1) {}

};
list<active_user> users;
SOCKET s_notUsed;
string user = "";
int main() {
	WSADATA wsa;

	struct active_user server1 = active_user("server", server, port);
	users.push_back(server1);
	//	new 4.26.16-00:18
	struct sockaddr_in server;

	int return_value;
	cout << "Welcome to CS423 TCP IM program" << endl;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d", WSAGetLastError());		
		return 1;
	}
	printf("Enter your IM name: \n");
	getline(cin, user);

	// use send msg function to send initial function.  not created yet 11:31:4/24/16
	_beginthread(listen, 0, (void *)&s_notUsed);  // create listen function for second thread
	sendMsg(1);
/*	struct addrinfo server_struct, *servinfo,*p;
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

	int optval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval);

	//Connect to remote/local TCP server	
	connect(s, servinfo->ai_addr, servinfo->ai_addrlen);
	printf("Connected to %s\n", server1.addr);
	*/
	//				NEW

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
					printf("\n\nNew Message Arrived \n");
					cout << serv_reply << endl;
					typeOfMessage = 0;
				}
				break;
			}
			case 'Q':
			case 'q':
			{
				end_flag = 0;
				action = 'q';
				break;
			}
			default:
				break;
		}

	}
	Sleep(500);
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

void listen(void * sock) {
	int recv_size, rv;
	SOCKET s1, newfd;
	char *listenPort = "34567";
	struct addrinfo hints, *servinfo1, *p1;
	int yes = 1;

	memset(&hints, 0, sizeof hints);
	printf("thread 1 yes\n");

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo("192.168.10.101", listenPort, &hints, &servinfo1)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	for (p1 = servinfo1; p1 != NULL; p1 = p1->ai_next) {
		if ((s1 = socket(p1->ai_family, p1->ai_socktype, p1->ai_protocol)) == -1) {
			perror("TCP_Server: socket");
			continue;
		}
		if (setsockopt(s1, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(s1, p1->ai_addr, p1->ai_addrlen) == -1) {
			closesocket(s1);
			perror("TCP_Server: bind");
			continue;
		}
		break;
	}
	if (servinfo1 == NULL) {
		fprintf(stderr, "TCP_Server: failed to bind socket\n");
		exit(1);
	}

	freeaddrinfo(p1);


	if (listen(s1, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	struct sockaddr_storage remote_addr;
	socklen_t addr_len;

	while (true) {
		addr_len = sizeof remote_addr;
		newfd = accept(s1, (struct sockaddr *)&remote_addr, &addr_len);
		if (newfd == INVALID_SOCKET) {
			printf("accept error %d\n", GetLastError());
		}
		if (_beginthread((void(*)(void *))New_Process, 0, (void *)newfd) < 0)
			printf("_beginthread: %s\n", strerror(errno));
		if (end_flag == 0) {
			break;
		}
	}

	closesocket(newfd);
}

int New_Process(SOCKET fd) {
	//char	buf[BUFSIZE];
	printf("thread 2 yes");
	int	cc;
	cc = recv(fd, server_reply, sizeof server_reply, 0);
	server_reply[cc] = '\0';
	serv_reply = decryptMessage(server_reply);
	typeOfMessage = 4;
	closesocket(fd);
	return 0;
}

void sendMsg(int msgType) {
	struct addrinfo serv, *server_info;
	int return_value;
	char *message;
	string msg = "";
	int optval = 1;

	memset(&serv, 0, sizeof serv);
	serv.ai_family = AF_UNSPEC;
	serv.ai_socktype = SOCK_STREAM;

	switch (msgType) {
		case 1:
		{
			if ((return_value = getaddrinfo("192.168.10.200", "34567", &serv, &server_info)) != 0) {
				printf("FFaddrinfo: %s\n", gai_strerror(return_value));
				exit(1);//return 1;
			}
			SOCKET s = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
			setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval);
			connect(s, server_info->ai_addr, server_info->ai_addrlen);
			msg = "1;" + user + ";34567#";
			message = new char[msg.length() + 1];
			strcpy(message, encryptMessage(msg).c_str()); // encrypt message and switch string to char* to send message
			if (send(s, message, strlen(message), 0) < 0) {		//
				cout << "Send Failed" << endl;					//
				exit(1);										//
			}
			closesocket(s);
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

string decryptMessage(string buf) {
	int recv_size = buf.length();
	//cout << recv_size << endl;
	int next = 0;
	for (int i = 0; i < recv_size; i++) {
		if (buf[i] == ';') {
			next = ++i;
			break;
		}

	}
	for (int j = next; j < recv_size; j++) {
		buf[j] = decrypt(buf[j]);
	}
	return buf;
}
