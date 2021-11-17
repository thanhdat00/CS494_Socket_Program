#include<iostream>
#include<winsock.h>
#include <fstream>
#include<vector>
#include <string>
using namespace std;
#define PORT 9909
typedef char TinyInt;

#define MaxStringSize 255

struct sockaddr_in srv;
fd_set fr, fw, fe;
int nMaxFd;
int nSocket;
int nArrClient[5];

struct Question {
	string keyword;
	string description;

	Question(string key, string des) {
		keyword = key;
		description = des;
	}
};

enum MessageTypes { Register = 'REG', Response = 'RES' };

struct Message {
	string messageType;
	string content;

	Message(string mt, string mess) {
		messageType = mt;
		content = mess;
	}
	Message() {
		content = "";
	}
};
vector <Question> questions;
int questionCount;
void convertToMessage(char*buff, Message& message) {
	for (int i = 0; i < 

}
void ProcessNewMessage(int nClientSocket)
{
	cout << endl << "Processing the new message for the client socket:" << nClientSocket;
	char buff[256 + 5];
	int nRet = recv(nClientSocket, buff, 256 + 5, 0);
	if (nRet < 0)
	{
		cout << endl << "Something wrong happend..Closing the connection for the client";
		closesocket(nClientSocket);
		for (int i = 0; i < 5; i++)
		{
			if (nArrClient[i] == nClientSocket)
			{
				nArrClient[i] = 0;
				break;
			}
		}
	}
	else
	{
		Message message;
		convertToMessage(buff, message);
		message.messageType = buff.substr(0, 3);
		message.content = 
		cout << endl << "The message recieved from client is:" << buff;
		//Send the response to client
		send(nClientSocket, "Processed your request", 23, 0);
		cout << endl << "*******************************";
	}
}

void ProcessTheNewRequest()
{
	if (FD_ISSET(nSocket, &fr))
	{
		int nLen = sizeof(struct sockaddr);
		int nClientSocket = accept(nSocket, NULL, &nLen);
		if (nClientSocket > 0)
		{
			//Put it into the client fd_set.
			int i;
			for (i = 0; i < 5; i++)
			{
				if (nArrClient[i] == 0)
				{
					nArrClient[i] = nClientSocket;
					send(nClientSocket, "Got the connection done successfully", 255, 0);
					break;
				}
			}
			if (i == 5)
			{
				cout << endl << "No space for new connection";
			}
		}
	}
	else
	{
		for (int i = 0; i < 5; i++)
		{
			if (FD_ISSET(nArrClient[i], &fr))
			{
				//Got the new message fromt the client
				//Recv new message
				//Just queue that for new workers of server to fulfill the request
				ProcessNewMessage(nArrClient[i]);
			}
		}
	}
}

void GetData() {
	ifstream fin("database.txt");
	fin >> questionCount;
	string n;
	getline(fin, n);
	while (!fin.eof())
	{
		string keyword, description;
		getline(fin, keyword);
		getline(fin, description);
		if (keyword.length() > 30) continue;
		Question q = Question(keyword, description);
		questions.push_back(q);
	}
	questionCount = questions.size();
}
int main()
{
	int nRet = 0;
	// Init the WSA variables
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) < 0)
	{
		cout << endl << "WSA Failed to initialize";
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << endl << "WSA initialized";
	}

	// Init socket
	nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nSocket < 0)
	{
		cout << endl << "The socket not opened";
	}
	else
	{
		cout << endl << "The socket opened successfully" << nSocket;
	}
	// Init env for sockaddr structure
	srv.sin_family = AF_INET;
	srv.sin_port = htons(PORT);
	srv.sin_addr.s_addr = INADDR_ANY;
	memset(&(srv.sin_zero), 0, 8);

	//About the blocking and non blocking sockets
	//optval = 0 means blocking and !=0 means non blocking 
	u_long optval = 1;
	nRet = ioctlsocket(nSocket, FIONBIO, &optval);
	if (nRet != 0)
	{
		cout << endl << "iocltsocket call failed";
	}
	else
	{
		cout << endl << "ioctlsocket call passed";
	} 

	//setsockopt 
	int nOptVal = 0;
	int nOptLen = sizeof(nOptVal);
	nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, nOptLen);
	if (!nRet)
	{
		cout << endl << "The setsockopt call successfully";
	}
	else
	{
		cout << endl << "The setsockopt call failed";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	// Bind the socket to the local port
	nRet = bind(nSocket, (sockaddr*)&srv, sizeof(sockaddr));
	if (nRet < 0)
	{
		cout << endl << "Fail to bind to local port";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << endl << "Successfull to bind to local port";
	}

	// Listen the request from client (queues the requests)
	nRet = listen(nSocket, 5);
	if (nRet < 0)
	{
		cout << endl << "Fail to start listen to local port";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << endl << "Started listening to local port";
	}

	nMaxFd = nSocket;
	struct timeval tv;
	tv.tv_sec = 4;
	tv.tv_usec = 0;

	//read data
	GetData();

	for (int i = 0; i < questions.size(); i++) {
		cout << questions[i].keyword << endl;
	}

	while (1)
	{
		FD_ZERO(&fr);
		FD_ZERO(&fw);
		FD_ZERO(&fe);

		FD_SET(nSocket, &fr);
		FD_SET(nSocket, &fe);

		for (int i = 0; i < 5; i++)
		{
			if (nArrClient[i] != 0)
			{
				FD_SET(nArrClient[i], &fr);
				FD_SET(nArrClient[i], &fe);
			}
		}

		// Keep waiting for new requests and proceed as per the request
		nRet = select(nMaxFd + 1, &fr, &fw, &fe, &tv);
		if (nRet > 0)
		{
			//When someone connects or communicates with a message over
			//a dedicated connection
			cout << endl << "Data on port ... Processing now...";
			ProcessTheNewRequest();
		}
		else if (nRet == 0)
		{
			//No connection or any communication request made or you can
			//say that none of the socket descriptors are ready
			/*cout << endl << "Nothing on port:" << PORT;*/
			//Process the request
		}
		else
		{
			cout << endl << "I failed...";
			WSACleanup();
			exit(EXIT_FAILURE);
			//It failed and your application should show some useful message
		}
		/*cout << endl << "After the select call:" << fr.fd_count;*/
	}

	return 0;
}