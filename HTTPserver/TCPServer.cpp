#include <iostream>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

/*
 * Simple TCP web server using winsocket (based on youtube tutorial: https://www.youtube.com/watch?v=WDn-htpBlnU&t=2s)
 *
 * Initializing socket -> Create socket -> Bind socket -> Start listening -> Waiting for a connection -> Accept connection
 * -> Close listen socket -> Communicate using client socket -> close socket -> clean up
 * 
 * Note:
 *  -network is big-endian, pc is usually little-endian   (htons(), ntohs() are used for handle endian problem
 *  -AF_INET: standard IPV4 address family
 *  -AF_INET6: IPV6 address family
 *  -Included file info(WS2tcpip.h vs Winsock2.h): https://msdn.microsoft.com/en-us/library/windows/desktop/ms738562(v=vs.85).aspx
 */
int main() {

	//Initializing winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int wsOK = WSAStartup(ver, &wsData);
	
	if (wsOK != 0) {
		cerr << "Error initializing winsock!" << endl;
		return -1;
	}

	//Create a socket
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET) {
		cerr << "Error creating socket!" << endl;
		return -1;
	}

	//Bind
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;						//Alternative: inet_pton
	
	bind(listenSocket, (sockaddr *)&hint, sizeof(hint));

	//Tell winsock to listening
	listen(listenSocket, SOMAXCONN);

	//Waiting for a connection
	sockaddr_in client;
	int clientSize = sizeof(client);
	
	SOCKET clientSocket = accept(listenSocket, (sockaddr*)&client, &clientSize);
	if (clientSocket == INVALID_SOCKET) {
		cerr << "Error connection to client!" << endl;
	}

	char host[NI_MAXHOST];				//Client's remote name
	char service[NI_MAXHOST];			//Servie the client is connect on

	ZeroMemory(host, NI_MAXHOST);		//same as memset(host, 0, NI_MAXHOST)
	ZeroMemory(service, NI_MAXHOST); 

	//check if we can get client name information, otherwise display the ip address
	if (getnameinfo((sockaddr*)&client, clientSize, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		cout << host << " connected on port " << service << endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " << ntohs(client.sin_port) << endl;
	}

	//Close listening socket
	closesocket(listenSocket);

	//Accept and echo message back to client
	char buf[4096];
	
	while (true) {
		ZeroMemory(buf, 4096);

		int bytesReceived = recv(clientSocket, buf, 4096, 0);
		if (bytesReceived == SOCKET_ERROR) {
			cerr << "Error receiving bytes from client!" << endl;
			break;
		}

		if (bytesReceived == 0) {
			cout << "Client disconnected " << endl;
			break;
		}
		cout << "Client: " << buf;
		send(clientSocket, buf, bytesReceived + 1, 0);   // + 1 for nullterminating character
	}

	//close client socket
	closesocket(clientSocket);

	//clear up
	WSACleanup();
	return 0;
}