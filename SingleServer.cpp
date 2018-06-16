#pragma comment (lib, "ws2_32.lib")
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <winsock.h>
#include <string>
#define PRODUCT 50 // Started product at 50 because it's a nice number

using namespace std;

HANDLE ghSemaphore; //Create a sempahore

DWORD WINAPI receive_cmds(void * param)
{
	printf("My thread ID is: %d\n", GetCurrentThreadId()); // Once a client connects, print the ThreadID so we know it made it.
	
	//Create new SOCKET and initiate information for each thread
	sockaddr_in newSA;
	SOCKET newSock;
	newSock = socket(AF_INET, SOCK_STREAM, 0);
	newSA.sin_family = AF_INET;
	newSA.sin_addr.s_addr = INADDR_ANY;
	newSA.sin_port = htons(123);
	static int product = PRODUCT;


	char buf[100];		// buffer to hold received data
	char sendData[100];	// buffer to hold sent data
	char productBuf[100]; //Buffer to hold product number

	while (1)
	{
		recv((SOCKET)param, buf, sizeof(buf), 0); // Cast param (which was client from main), back to a SOCKET type so it can receive commands

		if (strstr(buf, "hello"))
		{ // greet this user
			printf("\nrecived hello cmd");

			strcpy_s(sendData, "Hello, greeting from Server\n");
			Sleep(10);
			send((SOCKET)param, sendData, sizeof(sendData), 0);
		}
		else if (strstr(buf, "bye"))
		{ // dissconnected this user
			printf("\nreceived bye cmd\n");

			strcpy_s(sendData, "cya\n");
			Sleep(10);
			send((SOCKET)param, sendData, sizeof(sendData), 0);

			// close the socket associted with this client and end this thread
			closesocket((SOCKET)param);
			ExitThread(0);
		}
		else if (strstr(buf, "buy"))
		{ // increment product, send product value to user
		  // if product = 100, do not increment

			printf("\nreceived buy cmd\n");

			if (product == 100)
			{
				strcpy_s(sendData, "Product has reached its limit!\n");
				Sleep(10);
				send((SOCKET)param, sendData, sizeof(sendData), 0);
			}
			else
			{
				WaitForSingleObject(ghSemaphore, 0L); //Wait the semaphore, with 0 second time-out interval
				product++;
				itoa(product, productBuf, 10); // Convert integer to string (itoa is integer to array/buffer) for passing via send
				Sleep(10);
				send((SOCKET)param, productBuf, sizeof(productBuf), 0);
				ReleaseSemaphore(ghSemaphore, 1, NULL); // Signal the semaphore, increasing its count by 1

			}

		}
		else if (strstr(buf, "sell"))
		{ // decrement product, send product value to user
		  // if product = 0, do not decrement
			printf("\nreceived sell cmd\n");

			if (product == 0)
			{
				strcpy_s(sendData, "Product cannot be less than 0\n");
				Sleep(10);
				send((SOCKET)param, sendData, sizeof(sendData), 0);
			}
			else
			{
				WaitForSingleObject(ghSemaphore, 0L); //Wait the semaphore, with 0 second time-out interval
				product--;
				itoa(product, productBuf, 10); // Convert integer to string for passing via send function
				Sleep(10);
				send((SOCKET)param, productBuf, sizeof(productBuf), 0);
				ReleaseSemaphore(ghSemaphore, 1, NULL); // Signal the semaphore, increasing its count by 1
			}
		}
		else
		{
			strcpy_s(sendData, "Invalid cmd\n");
			Sleep(10);
			send((SOCKET)param, sendData, sizeof(sendData), 0);
		}

		// clear buffers
		strcpy_s(sendData, "");
		strcpy_s(buf, "");

	}



	

	return 0;
}

int main()
{
	printf("Starting up Trading Server\n");

	SOCKET sock;
	ghSemaphore = CreateSemaphore(NULL, 1, 1, NULL); // global semaphore lock, inital and max Sem counts are 1 because only 1 thread allowed inside
	WSADATA wsaData;
	sockaddr_in server;
	DWORD threadID;

	int ret = WSAStartup(0x101, &wsaData); // use highest version of winsock avaliable

	if (ret != 0)
	{
		return 0;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(123);

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == INVALID_SOCKET)
	{
		return 0;
	}

	if (bind(sock, (sockaddr*)&server, sizeof(server)) != 0)
	{
		return 0;
	}

	if (listen(sock, 5) != 0)
	{
		return 0;
	}

	SOCKET client;
	sockaddr_in from;
	int fromlen = sizeof(from);

	while (true)
	{

		if (client = accept(sock, (struct sockaddr*) &from, &fromlen)) //If we open a new client window and it gets accepted
		{
			//Make a thread, parameters are security options, stack size, start_up function (receive_cmds), Parameter (client cast to LPVOID from SOCKET), creation flags, threadID)
			CreateThread(NULL, 0, receive_cmds, (LPVOID)client, 0, &threadID); 
			Sleep(3);

		}


	}



	closesocket(sock);
	WSACleanup();

	return 0;
}
