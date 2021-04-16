#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <vector>
#include <fstream>

#pragma comment (lib, "Ws2_32.lib")

#define IP_ADDRESS "192.168.1.66"
#define DEFAULT_PORT "3504"
#define DEFAULT_BUFLEN 512

using namespace std;

struct server_config
{
	string gameType;
	string gameMap;
	string gameDifficulty;
	int maximumPlayers = 0;
	int minimumPlayers = 0;
}gameServer;

struct client_type
{
	int id;
	SOCKET socket;
	string playertype;
};

const char OPTION_VALUE = 1;
const int MAX_CLIENTS = 5;
int num_clients = 0;
int players = 0;
int spectators = 0;
int numdisconnects = 0;
int numspectators = 0;
bool gameStarted = false;

ofstream log_file;
ofstream client_disconnects;
ofstream client_spectators;
ofstream total_clients;

//Function Prototypes
void configureServer();
bool validateServer();
void printServer();
int sendMessageToAllClients(string msg, vector<client_type>& client_array);
void disconnectClient(client_type& client, vector<client_type>& client_array);
int makeSpectatorAPlayer(vector<client_type>& client_array);
int process_client(client_type& new_client, vector<client_type>& client_array, thread& thread);
int main();

void configureServer() 
{
	int choice = 0, subchoice = 0;
	bool check = false;

	do
	{
		cout << "--------------------Server Settings--------------------" << endl;
		cout << "-                                                     -" << endl;
		cout << "-    1.) Set Game Type                                -" << endl;
		cout << "-    2.) Set Game Map                                 -" << endl;
		cout << "-    3.) Set Game Difficulty                          -" << endl;
		cout << "-    4.) Set Minimum Players                          -" << endl;
		cout << "-    5.) Set Maximum Players                          -" << endl;
		cout << "-    6.) Start Game                                   -" << endl;
		cout << "-                                                     -" << endl;
		cout << "-------------------------------------------------------" << endl;
		cout << "Select an option: " << endl;

		cin >> choice;

		switch (choice) 
		{
		    case 1: 
			        cout << "----------Game Types----------" << endl;
			        cout << "-                            -" << endl;
			        cout << "-  1.) Deathmatch            -" << endl;
			        cout << "-  2.) Capture the flag      -" << endl;
			        cout << "-  3.) Blood Diamond         -" << endl;
			        cout << "-                            -" << endl;
			        cout << "------------------------------" << endl;
			        cin >> subchoice;
			        if(subchoice == 1) 
			        {
				        gameServer.gameType = "Deathmatch";
						cout << "Deathmatch selected" << endl;
			        }
			        else if (subchoice == 2) 
			        {
				        gameServer.gameType = "Capture the flag";
						cout << "Capture the flag selected" << endl;
			        }
			        else if (subchoice == 3) 
			        {
				        gameServer.gameType = "Blood Diamond";
						cout << "Blood Diamond selected" << endl;
			        }
				break;
		
		    case 2:
					cout << "----------Game Maps----------" << endl;
					cout << "-                           -" << endl;
					cout << "-  1.) 2fort                -" << endl;
					cout << "-  2.) Dustbowl             -" << endl;
					cout << "-  3.) Badwater Basin       -" << endl;
					cout << "-                           -" << endl;
					cout << "-----------------------------" << endl;
					cin >> subchoice;
					if (subchoice == 1)
					{
						gameServer.gameMap = "2fort";
						cout << "2fort selected" << endl;
					}
					else if (subchoice == 2)
					{
						gameServer.gameMap = "Dustbowl";
						cout << "Dustbowl selected" << endl;
					}
					else if (subchoice == 3)
					{
						gameServer.gameMap = "Badwater Basin";
						cout << "Badwater Basin selected" << endl;
					}
				break;

			case 3:
					cout << "---------Difficulty---------" << endl;
					cout << "-                          -" << endl;
					cout << "-  1.) Easy                -" << endl;
					cout << "-  2.) Normal              -" << endl;
					cout << "-  3.) Hard                -" << endl;
					cout << "-                          -" << endl;
					cout << "----------------------------" << endl;
					cin >> subchoice;
					if (subchoice == 1)
					{
						gameServer.gameDifficulty = "Easy";
						cout << "Easy selected" << endl;
					}
					else if (subchoice == 2)
					{
						gameServer.gameDifficulty = "Normal";
						cout << "Normal selected" << endl;
					}
					else if (subchoice == 3)
					{
						gameServer.gameDifficulty = "Hard";
						cout << "Hard selected" << endl;
					}
				break;

			case 4: cout << "Enter minimum number of players" << endl;
				    cin >> gameServer.minimumPlayers;
					break;

			case 5: cout << "Enter maximum number of players" << endl;
				    cin >> gameServer.maximumPlayers;
					break;

			case 6: 
				check = validateServer();
				  break;	
		}
	} while (check == false);

}

bool validateServer() 
{
	bool valid;

	if(gameServer.gameType == "Deathmatch" || gameServer.gameType == "Capture the flag" || gameServer.gameType == "Blood Diamond")
	{
		valid = true;
	}
	else 
	{
		valid = false;
		cout << "Game type has not been selected" << endl;
	}
	
	if (gameServer.gameMap == "2fort" || gameServer.gameMap == "Dustbowl" || gameServer.gameMap == "Badwater Basin") 
	{
		valid = true;
	}
	else 
	{
		valid = false;
		cout << "Game map has not been selected" << endl;
	}

	if (gameServer.gameDifficulty == "Easy" || gameServer.gameDifficulty == "Normal" || gameServer.gameDifficulty == "Hard") 
	{
		valid = true;
	}
	else 
	{
		valid = false;
		cout << "Game difficulty has not been selected" << endl;
	}

	if (gameServer.minimumPlayers <= 0) 
	{
		valid = false;
		cout << "Minimum players must be greater than 0" << endl;
	}
	
	if (gameServer.maximumPlayers <= 0) 
	{
		valid = false;
		cout << "Maximum players must be greater than 0" << endl;
	}

	if (gameServer.minimumPlayers > gameServer.maximumPlayers) 
	{
		valid = false;
		cout << "Minimum players cannot be greater than the maximum number of players" << endl;
	}

	if (gameServer.maximumPlayers > MAX_CLIENTS) 
	{
		valid = false;
		cout << "Maximum number of players cannot be greater than the maximum number of clients - " << MAX_CLIENTS << endl;
	}

	return valid;
}

void printServer() 
{
	const char* path = "/Users/lukas/Desktop/SPC++ Assignment/Project/Client/Client/serverConfig.txt";
	ofstream server_info(path);

	cout << "--------------------Server Settings--------------------" << endl;
	cout << "-                                                     " << endl;
	cout << "-    GAME TYPE - " << gameServer.gameType << endl;
	cout << "-    GAME MAP - " << gameServer.gameMap << endl;
	cout << "-    GAME DIFFICULTY - " << gameServer.gameDifficulty << endl;
	cout << "-    MINIMUM PLAYERS - " << gameServer.minimumPlayers << endl;
	cout << "-    MAXIMUM PLAYERS - " << gameServer.maximumPlayers << endl;
	cout << "-                                                     -" << endl;
	cout << "-------------------------------------------------------" << endl;

	server_info << IP_ADDRESS << endl;
	server_info << DEFAULT_PORT << endl;

	server_info.close();
}

int sendMessageToAllClients(string msg, vector<client_type>& client_array)
{
	int iResult = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (client_array[i].socket != INVALID_SOCKET)
		{
			iResult = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
		}
	}

	return iResult;
}

void disconnectClient(client_type& client, vector<client_type>& client_array)
{
	numdisconnects++;
	client_disconnects << "Number of disconnects: " << numdisconnects << endl;
	string msg = "Player# " + to_string(client.id) + " disconnected as a " + client.playertype;

	num_clients--;
	if (client.playertype == "PLAYER")
	{
		players--;
	}
	else if (client.playertype == "SPECTATOR")
	{
		spectators--;
	}

	cout << msg << endl;
	log_file << msg << endl;

	closesocket(client.socket);
	closesocket(client_array[client.id].socket);
	client_array[client.id].socket = INVALID_SOCKET;

	//Broadcast the disconnection message to the other clients
	int iResult = sendMessageToAllClients(msg, client_array);

	if (players < gameServer.minimumPlayers)
	{
		msg = "Waiting for minimum number of players to be met " + to_string(num_clients) + "/" + to_string(gameServer.minimumPlayers);
		sendMessageToAllClients(msg, client_array);
	}

	if (players < gameServer.maximumPlayers && spectators != 0)
	{
		int id = makeSpectatorAPlayer(client_array);
		msg = "Player# " + to_string(id) + " is no longer spectating ";
		sendMessageToAllClients(msg, client_array);
	}
}

int makeSpectatorAPlayer(vector<client_type>& client_array)
{
	string msg = "\nSpace available, no longer spectating now playing.";
	int playerid = 0;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (client_array[i].playertype == "SPECTATOR")
		{
			client_array[i].playertype = "PLAYER";
			spectators--;
			players++;
			playerid = client_array[i].id;
			send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
			break;
		}
	}

	return playerid;
}

int process_client(client_type& new_client, vector<client_type>& client_array, thread& thread)
{
	DWORD timeout = 15 * 1000;
	setsockopt(new_client.socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);

	string msg = "";
	char tempmsg[DEFAULT_BUFLEN] = "";
	int pingretry = 0;

	//Prepare message to send to client
	if (num_clients < gameServer.minimumPlayers)
	{
		cout << "Waiting for minimum number of players to be met " << to_string(num_clients) << "/" << to_string(gameServer.minimumPlayers) << endl;
		msg = "Waiting for minimum number of players to be met " + to_string(num_clients) + "/" + to_string(gameServer.minimumPlayers);
		//Send message to client
		send(new_client.socket, msg.c_str(), strlen(msg.c_str()), 0);
		while (gameStarted == false);
		cout << gameServer.gameType << " game" << " in Progress on " + gameServer.gameMap << endl;
		msg = gameServer.gameType + " game on " + gameServer.gameMap + " joining as " + new_client.playertype;
		send(new_client.socket, msg.c_str(), strlen(msg.c_str()), 0);
	}
	else if (num_clients >= gameServer.minimumPlayers)
	{
		gameStarted = true;
		msg = gameServer.gameType + " game on " + gameServer.gameMap + " joining as " + new_client.playertype;
		send(new_client.socket, msg.c_str(), strlen(msg.c_str()), 0);
	}

	//Session
	while (1)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);

		//Check if client has a valid socket
		if (new_client.socket != 0)
		{	
			int iResult = recv(new_client.socket, tempmsg, DEFAULT_BUFLEN, 0);

			if (iResult != SOCKET_ERROR)
			{
				if(strcmp("Disconnected", tempmsg) == 0) //When player sends disconnect message
				{
					disconnectClient(new_client, client_array);

					//Refresh player counts
					cout << "======= Players: " << players << "/" << gameServer.maximumPlayers << " | Spectators: " << spectators << "/" << MAX_CLIENTS - gameServer.maximumPlayers << "=======" << endl;

					break;
				}
			}
			else if(iResult == ETIMEDOUT)
			{
				pingretry++;
				if(pingretry == 3)
				{
					cout << "Client# " << new_client.id << " did not return ping after 3 attempts, disconnecting..." << endl;
					disconnectClient(new_client, client_array);
					break;
				}
			}
			else //When player closes window.
			{
				disconnectClient(new_client, client_array);

				//Refresh player counts
				cout << "======= Players: " << players << "/" << gameServer.maximumPlayers << " | Spectators: " << spectators << "/" << MAX_CLIENTS - gameServer.maximumPlayers << "=======" << endl;

				break;
			}
		}
	} //end while

	thread.detach();

	return 0;
}

int main()
{
	WSADATA wsaData;

	struct addrinfo hints;
	struct addrinfo* server = NULL;

	SOCKET server_socket = INVALID_SOCKET;

	string msg = "";

	vector<client_type> client_array(MAX_CLIENTS);

	int temp_id = -1;

	thread my_thread[MAX_CLIENTS];

	log_file.open("server_log.txt");
	client_disconnects.open("client_disconnects.txt");
	client_spectators.open("client_spectators.txt");
	total_clients.open("total_clients.txt");

	configureServer();

	//Initialize Winsock
	cout << "Intializing Winsock..." << endl;
	log_file << "Intializing Winsock..." << endl;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//Setup hints
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;	

	//Setup Server
	cout << "Setting up server..." << endl;
	log_file << "Setting up server..." << endl;
	getaddrinfo(IP_ADDRESS, DEFAULT_PORT, &hints, &server);

	//Create a listening socket for connecting to server
	cout << "Creating server socket..." << endl;
	log_file << "Creating server socket..." << endl;
	server_socket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

	//Setup socket options
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &OPTION_VALUE, sizeof(int)); //Make it possible to re-bind to a port that was used within the last 2 minutes
	setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int)); //Used for interactive programs

	//Assign an address to the server socket.
	cout << "Binding socket..." << endl;
	log_file << "Binding socket..." << endl;
	int tmp = ::bind(server_socket, server->ai_addr, (int)server->ai_addrlen);
	cout << tmp << endl;

	//Listen for incoming connections.
	cout << "Listening..." << endl;
	log_file << "Listening..." << endl;
	listen(server_socket, SOMAXCONN);

	printServer();

	//Initialize the client list
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		client_array[i] = { -1, INVALID_SOCKET };
	}

	//Constantly listening
	while (1)
	{
		SOCKET incoming = INVALID_SOCKET;

		//Accept connection from client
		incoming = accept(server_socket, NULL, NULL);

		//Check if client socket is valid
		if (incoming == INVALID_SOCKET) continue;

		//Reset the number of clients
		num_clients = 0;

		//Create a temporary id for the next client
		temp_id = -1;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			//Look for available client socket
			if (client_array[i].socket == INVALID_SOCKET && temp_id == -1)
			{
				client_array[i].socket = incoming;
				client_array[i].id = i;
				temp_id = i;
				if (num_clients <= gameServer.maximumPlayers)
				{
					//Set new client as player
					client_array[i].playertype = "PLAYER";
					players++;
				}
			}

			//Get total number of clients
			if (client_array[i].socket != INVALID_SOCKET)
			{
				num_clients++;
				if(num_clients > gameServer.maximumPlayers)
				{
					numspectators++;
					client_spectators << "Number of total spectators: " << numspectators << endl;
					//Set new client as spectator
					client_array[i].playertype = "SPECTATOR";
					spectators++;
					players--;
				}
			}

		}

		total_clients << "Number of clients: " << num_clients << endl;

		//Ran when a new player successfully connects
		if (temp_id != -1)
		{
			//Server prints the player that joined
			cout << "Player #" << client_array[temp_id].id << " has joined the game\n" << endl;

			//Log player join
			log_file << "Player #" << client_array[temp_id].id << " has joined the game" << endl;

			//Refresh player counts
			cout << "======= Players: " << players << "/" << gameServer.maximumPlayers << " | Spectators: " << spectators << "/" << MAX_CLIENTS - gameServer.maximumPlayers << "=======" << endl;

			log_file << "======= Players: " << players << "/" << gameServer.maximumPlayers << " | Spectators: " << spectators << "/" << MAX_CLIENTS - gameServer.maximumPlayers << "=======" << endl;

			//Prepare message to send to client (CLIENT ID)
			msg = to_string(client_array[temp_id].id);

			//Send message to client
			send(client_array[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);

			

			//Create new thread to process client and add to array
			my_thread[temp_id] = thread(process_client, ref(client_array[temp_id]), ref(client_array), ref(my_thread[temp_id]));
		}
		else
		{
			msg = "Server is full";
			send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
			cout << msg << endl;
			log_file << msg << endl;
		}
	} //end while


	//Close listening socket
	closesocket(server_socket);

	//Close client sockets
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		my_thread[i].detach();
		closesocket(client_array[i].socket);
	}

	//Clean up Winsock
	WSACleanup();
	cout << "Program has ended successfully" << endl;
	log_file << "Program has ended successfully" << endl;

	system("pause");
	return 0;
}