#pragma warning(disable : 4996)
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

enum MessageTypes { Register = 'r', Response = 'R' };

struct Message {
	char messageType;
	int strLength;
	string content;

	Message(char mt, string mess, int len) {
		messageType = mt;
		content = mess;
		strLength = len;
	}
	Message() {
		content = "";
		strLength = 0;
	}
};
vector <Question> questionList;
int questionCount;

struct User {
	string name;
	int socket;

	User(string nameReg, int socketReg) {
		name = nameReg;
		socket = socketReg;
	}
};

vector <User> userList;
int userCount;

void convertToMessage(char*buff, Message& message) {
	message.messageType = buff[0];
	string len = "";
	for (int i = 1; i < 3; i++)
		len += buff[i];
	int n = stoi(len);
	cout << n << endl;
	string s = "";
	for (int i = 3; i < n + 3; i++)
		s += buff[i];
	message.content = s;
	message.strLength = n;
	cout << s << endl;
}

bool IsExistedUserName(string userName) {
	auto it = find_if(userList.begin(), userList.end(), [&userName](const User& obj) {return obj.name == userName; });
	if (it != userList.end()) return true;
	return false;
}
bool isUnderScores(char c)
{
	if (c == '_') return true;
	return false;
}
string IsValidUserName(string name) {
	for (const char c : name) {
		if (!isalpha(c) && !isalnum(c) && !isUnderScores(c))
			return "Name can only have letter number or '_'";
	}
	if (name.length() > 10) return "Name length can't be more than 10";
	return "";
}
string ValidateUserName(string userName) {
	if (IsExistedUserName(userName)) return "User name is existed";
	return IsValidUserName(userName);
}

void SendResponseSuccess(User user) {
	//Send the response to client
	cout << user.socket << endl;
	send(user.socket, "SUCCESS", MaxStringSize, 0);
	cout << "*******************************" << endl;
}

void SendResponseFailed(int socket, string message) {
	char buff[MaxStringSize];
	strcpy(buff, message.c_str());
	send(socket, buff, MaxStringSize, 0);
}
void HandleRegisterRequest(int nClientSocket, string userName) {
	string res = ValidateUserName(userName);
	if (res.length()) {
		SendResponseFailed(nClientSocket, res);
	}
	else {
		User user = User(userName, nClientSocket);
		userList.push_back(user);
		SendResponseSuccess(user);
	}
}
void HandleMessageRecv(int nClientSocket, Message message) {
	switch (message.messageType) {
	case 'R':
		HandleRegisterRequest(nClientSocket, message.content);
		break;
	}
}

void ProcessNewMessage(int nClientSocket)
{
	cout << "Processing the new message for the client socket:" << nClientSocket <<endl;
	char buff[256 + 5];
	int nRet = recv(nClientSocket, buff, 256 + 5, 0);
	if (nRet < 0)
	{
		cout << "Something wrong happend..Closing the connection for the client" <<endl;
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
		cout << "The message recieved from client is:" << buff << endl;
		cout << message.messageType << " " << message.content << " " << message.strLength << endl;

		HandleMessageRecv(nClientSocket,message);
		
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
				cout << "No space for new connection" << endl;
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
		questionList.push_back(q);
	}
	questionCount = questionList.size();
}
int main()
{
	int nRet = 0;
	// Init the WSA variables
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) < 0)
	{
		cout << "WSA Failed to initialize" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << "WSA initialized" << endl;
	}

	// Init socket
	nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nSocket < 0)
	{
		cout << "The socket not opened" << endl;
	}
	else
	{
		cout << "The socket opened successfully" << nSocket << endl;
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
		cout << "iocltsocket call failed" << endl ;
	}
	else
	{
		cout << "ioctlsocket call passed" << endl;
	} 

	//setsockopt 
	int nOptVal = 0;
	int nOptLen = sizeof(nOptVal);
	nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, nOptLen);
	if (!nRet)
	{
		cout << "The setsockopt call successfully" << endl;
	}
	else
	{
		cout << "The setsockopt call failed" << endl;
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	// Bind the socket to the local port
	nRet = bind(nSocket, (sockaddr*)&srv, sizeof(sockaddr));
	if (nRet < 0)
	{
		cout << "Fail to bind to local port" << endl;
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << "Successfull to bind to local port" << endl;
	}

	// Listen the request from client (queues the requests)
	nRet = listen(nSocket, 5);
	if (nRet < 0)
	{
		cout << "Fail to start listen to local port" << endl;
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << "Started listening to local port" << endl;
	}

	nMaxFd = nSocket;
	struct timeval tv;
	tv.tv_sec = 4;
	tv.tv_usec = 0;

	//read data
	GetData();

	for (int i = 0; i < questionList.size(); i++) {
		cout << questionList[i].keyword << endl;
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
			cout << "Data on port ... Processing now..." << endl;
			ProcessTheNewRequest();
		}
		else if (nRet == 0)
		{
			//No connection or any communication request made or you can
			//say that none of the socket descriptors are ready
			/*cout << "Nothing on port:" << PORT;*/
			//Process the request
		}
		else
		{
			cout << "I failed..." << endl;
			WSACleanup();
			exit(EXIT_FAILURE);
			//It failed and your application should show some useful message
		}
		/*cout << "After the select call:" << fr.fd_count;*/
	}

	return 0;
}