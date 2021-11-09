#include<iostream>
#include<winsock.h>
#pragma warning(disable:4996) 
using namespace std;
#define PORT 9909

int nClientSocket;
struct sockaddr_in srv;

int main()
{
	int nRet = 0;
	// Init the WSA variables
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) < 0)
	{
		cout << endl << "WSA Failed to initialize";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	// Init socket
	nClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nClientSocket < 0)
	{
		cout << endl << "The socket call failed";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	// Init env for sockaddr structure
	srv.sin_family = AF_INET;
	srv.sin_port = htons(PORT);
	srv.sin_addr.s_addr = inet_addr("127.0.0.1");  //192.168.1.7
	memset(&(srv.sin_zero), 0, 8);

	nRet = connect(nClientSocket, (struct sockaddr*)&srv, sizeof(srv));
	if (nRet < 0)
	{
		cout << endl << "Connect failed";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << endl << "Connect to the server";	
		char buff[255] = { 0, };
		recv(nClientSocket, buff, 255, 0);
		cout << endl << "Just press any key to see the message from the server";
		//getchar();
		cout << endl << buff;
		//Talk to the server ...
		cout << endl << "Send your message to server";
		//send(nClientSocket, "Hello Server", 256, 0);
		//recv(nClientSocket, buff, 256, 0);
		//cout << endl << buff;
		while (1)
		{
			fgets(buff, 256, stdin);
			cout << endl << buff;
			int check = send(nClientSocket,buff, 256, 0);
			if (check > 0)
			{
				cout << endl << "Press any key to get the response from server..";
				getchar();
				recv(nClientSocket, buff, 256, 0);
				cout << endl << buff << endl << "Now send next message";
			}
			else
			{
				cout << endl << "send failed";
			}
		}
	}

	return 0;
}