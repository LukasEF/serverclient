#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <vector>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

ofstream log_file;
ofstream client_heartbeat;

ifstream serverConfig;
string ip;
string port;

struct client_type
{
	SOCKET socket;
	int id;
	char received_message[DEFAULT_BUFLEN];
};

void printClientMenu();
int process_client(client_type& new_client);
void pingServer(client_type& client, thread& thread);
int main();

void printClientMenu() 
{
	cout << "--------------------Multiplayer Game--------------------" << endl;
	cout << "-                                                      -" << endl;
	cout << "-    i.p. address: " << ip << "                        -" << endl;
	cout << "-    port: " << port << "                                        -" << endl;
	cout << "-    1.) Connect to server                             -" << endl;
	cout << "-    2.) Disconnect from server                        -" << endl;
	cout << "-    3.) Exit                                          -" << endl;
	cout << "-                                                      -" << endl;
	cout << "-------------------------------------------------------" << endl;
	cout << "Select an option: " << endl;
}

void pingServer(client_type& client, thread& thread)
{
	int iResult = 0;

	while(1)
	{
		Sleep(5);

		string message = "ping";
		client_heartbeat << "Player #" << client.id << " ping" << endl;
		iResult = send(client.socket, message.c_str(), strlen(message.c_str()), 0);
		if(iResult == ENOTCONN)
		{
			log_file << client.id << " lost connection" << endl;
			break;
		}
	}

	thread.detach();
}

int process_client(client_type& new_client)
{
    string msg = "";

	while (1)
	{
		memset(new_client.received_message, 0, DEFAULT_BUFLEN);

		if (new_client.socket != 0)
		{
			int iResult = recv(new_client.socket, new_client.received_message, DEFAULT_BUFLEN, 0);

			if (iResult != SOCKET_ERROR) 
			{	
				log_file << new_client.received_message << endl;
				cout << new_client.received_message << endl;
			}
			else
			{
				log_file << "Disconnected" << endl;
				cout << "Disconnected" << endl;
				break;
			}
		}
	}

	if (WSAGetLastError() == WSAECONNRESET) {
		log_file << "The server has disconnected" << endl;
		cout << "The server has disconnected" << endl;
	}
	return 0;
}

int main()
{
	WSAData wsa_data;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	string sent_message = "";
	client_type client = { INVALID_SOCKET, -1, "" };
	int iResult = 0;
	char choice = { '0' };
	string message;
	string line;
	bool connected = false;
	thread my_thread;
	thread ping_thread;
	log_file.open("client_log.txt");
	client_heartbeat.open("client_heartbeats.txt");
	serverConfig.open("serverConfig.txt");

	if (serverConfig.is_open()) 
	{
		int i = 0;
		vector<string> details(2);
		while (getline(serverConfig, line))
		{
			details[i] = line;
			i++;
		}
		ip = details[0];
		port = details[1];
	}

	printClientMenu();
	while(1)
	{
		cin >> choice;
		if (choice == '1') 
		{
			if(connected == false)
			{
				log_file << "Starting Client...\n";
				cout << "Starting Client...\n";

				// Initialize Winsock
				iResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);
				if (iResult != 0) {
					log_file << "WSAStartup() failed with error: " << iResult << endl;
					cout << "WSAStartup() failed with error: " << iResult << endl;
					return 1;
				}

				ZeroMemory(&hints, sizeof(hints));
				hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;

				log_file << "Connecting...\n";
				cout << "Connecting...\n";

				// Resolve the server address and port
				iResult = getaddrinfo(ip.c_str(), port.c_str(), &hints, &result);
				if (iResult != 0) {
					log_file << "getaddrinfo() failed with error: " << iResult << endl;
					cout << "getaddrinfo() failed with error: " << iResult << endl;
					WSACleanup();
					system("pause");
					return 1;
				}

				// Attempt to connect to an address until one succeeds
				for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

					// Create a SOCKET for connecting to server
					client.socket = socket(ptr->ai_family, ptr->ai_socktype,
						ptr->ai_protocol);
					if (client.socket == INVALID_SOCKET) {
						log_file << "socket() failed with error: " << WSAGetLastError() << endl;
						cout << "socket() failed with error: " << WSAGetLastError() << endl;
						WSACleanup();
						system("pause");
						return 1;
					}

					// Connect to server.
					iResult = connect(client.socket, ptr->ai_addr, (int)ptr->ai_addrlen);
					if (iResult == SOCKET_ERROR) {
						cout << "socket() failed with error: " << ptr->ai_family << endl;
						closesocket(client.socket);
						client.socket = INVALID_SOCKET;
						continue;
					}
					break;
				}

				freeaddrinfo(result);

				if (client.socket == INVALID_SOCKET) {
					log_file << "Unable to connect to server!" << endl;
					cout << "Unable to connect to server!" << endl;
					WSACleanup();
					system("pause");
					return 1;
				}

				log_file << "Successfully Connected" << endl;
				cout << "Successfully Connected" << endl;

				//Obtain id from server for this client;
				recv(client.socket, client.received_message, DEFAULT_BUFLEN, 0);
				message = client.received_message;

				if (message != "Server is full")
				{
					connected = true;
					client.id = atoi(client.received_message);
					cout << "Player number #" << client.id << endl;

					my_thread = thread(process_client, ref(client));
					ping_thread = thread(pingServer, ref(client), ref(ping_thread));

					memset(client.received_message, 0, DEFAULT_BUFLEN);

					if (client.socket != 0)
					{
						int iResult = recv(client.socket, client.received_message, DEFAULT_BUFLEN, 0);

						if (iResult != SOCKET_ERROR) 
						{
						string msg = client.received_message;

							//If client recieves message stating that minimum number of players haven't been met
							if (msg.find("Waiting") != string::npos) 
							{
								//Log waiting message
								log_file << client.received_message << endl;
								cout << client.received_message << endl;
								memset(client.received_message, 0, DEFAULT_BUFLEN);
								//Wait for message saying game has begun
								iResult = recv(client.socket, client.received_message, DEFAULT_BUFLEN, 0);
							}

							//Log playing message
							log_file << client.received_message << endl;
							cout << client.received_message << endl;

						}
						else
						{
							log_file << "recv() failed: " << WSAGetLastError() << endl;
							cout << "recv() failed: " << WSAGetLastError() << endl;
							break;
						}

					}

					if (WSAGetLastError() == WSAECONNRESET) {
						log_file << "The server has disconnected" << endl;
						cout << "The server has disconnected" << endl;
					}
				}
				else {
					connected = false;
					cout << client.received_message << endl;
				}
			}
			else
            {
			    cout << "Already connected" << endl;
            }
		}
		else if (choice == '2')
		{
		    if(connected == true)
			{
				my_thread.detach();
				ping_thread.detach();
				message = "Disconnected";
				send(client.socket, message.c_str(), strlen(message.c_str()), 0);

				log_file << "Shutting down socket..." << endl;
				cout << "Shutting down socket..." << endl;
				cout << "Disconnected from game" << endl;

				iResult = shutdown(client.socket, SD_SEND);
				if (iResult == SOCKET_ERROR) {
					log_file << "shutdown() failed with error: " << WSAGetLastError() << endl;
					cout << "shutdown() failed with error: " << WSAGetLastError() << endl;
					closesocket(client.socket);
					WSACleanup();
					system("pause");
					return 1;
				}

				closesocket(client.socket);
				WSACleanup();
			}

			connected = false;
		}
		else if(choice == '3')
        {
		    if(connected == true)
			{
				cout << "Disconnect from the game before exiting..." << endl;
			}
			else if(connected == false)
			{
				//Shutdown the connection since no more data will be sent
				cout << "Exiting..." << endl;
				break;
			}
        }
		else if(choice < 61 || choice > 63)
        {
		    cout << "Invalid choice..." << endl;
        }
		else
        {
		    cout << "Invalid choice..." << endl;
        }

	}

	return 0;
}
