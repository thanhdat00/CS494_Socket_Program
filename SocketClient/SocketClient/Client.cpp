#include<iostream>
#include<winsock.h>
#include<string>
#pragma warning(disable:4996) 
using namespace std;
#define PORT 9909
const char SUCCESS_RES[] = "SUCCESS";
const char FAILED_RES[] = "FAILED";


int nClientSocket;
struct sockaddr_in srv;
//enum MessageTypes { Register = 'r', Response = 'R' };
enum State { Register = 0, Response = 1, WaitToStartGame = 2, InitialGame = 3, InGame = 4 };
State state;
struct Message {
	string messageType;
	int strLength;
	string content;

	Message(string mt, string mess, int len) {
		messageType = mt;
		content = mess;
		strLength = len;
	}
	Message() {
		content = "";
		strLength = 0;
	}
};

string CreateRegisterRequest(string s) {
	string res = "";
	res += "R";
	string len = to_string(s.length());
	if (len.length() < 2) len = "0" + len;
	res += len;
	res += s;
	return res;
}
void HandleRegisterProcess(char* buff) {
	if (strcmp(buff, SUCCESS_RES) == 0) {
		cout << "Register Success" << endl;
		
	}
	/*else if (strcmp(buff, FAILED_RES) == 0) {
		cout << "Register Failed" << endl;
	}*/
	else {
		cout << buff << endl;
	}
}
void HandleServerResponse(char* buff) {
	switch (state) {
	case Register:
		HandleRegisterProcess(buff);
		state = WaitToStartGame;
		break;
	case WaitToStartGame:
		cout << "[S]" << buff << endl;
		state = InitialGame; 
		//HandleStartGameProcess(buff);
		break;
	case InitialGame:
		cout << "[S]" << buff << endl;
		state = InGame;
		break;
	}
	
}
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
		char buff[256] = { 0, };
		recv(nClientSocket, buff, 255, 0);
		cout << endl <<"From server: " << buff;
		cout << "=================================" << endl;
		state = Register;
		//Talk to the server ...
		

		//send(nClientSocket, "Hello Server", 256, 0);
		//recv(nClientSocket, buff, 256, 0);
		//cout << endl << buff;
		while (1)
		{
			string sIn;
			if (state == Register) {
				cout << "Register a name: " << endl;
				cin >> sIn;
				sIn = CreateRegisterRequest(sIn);
				//cout << sIn << endl;
			}

			if (state == WaitToStartGame) {
				cout << "Waiting to start game...." << endl;
			}

			strcpy(buff, sIn.c_str());
			cout << endl << buff;
			if (state != WaitToStartGame && state != InitialGame) {
				int check = 0;

				if (sIn.length() != 0)
					check = send(nClientSocket, buff, sizeof(buff), 0);

				if (check > 0)
				{
					/*cout << endl << "Press any key to get the response from server..";
					getchar();*/
					recv(nClientSocket, buff, 256, 0);
					HandleServerResponse(buff);
				}
				else
				{
					cout << endl << "send failed";
				}
			}
			else {
				recv(nClientSocket, buff, 256, 0);
				HandleServerResponse(buff);
			}
			
		}
	}

	return 0;
}