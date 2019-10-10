/* UDP server */
#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <WinSock2.h>
#include <iostream>
#include <mutex>
#include <thread>
#pragma comment(lib, "ws2_32.lib")

#define MAX 512

using namespace cv;

void send_data(int);
void recv_data(int);

VideoCapture cap("TLOUPII-SOP-EP3-FINAL_FR.mp4");
std::mutex mtx_lock;
bool running = false;
int QUIT = 0;

void udpServer(const char *portNumber)
{
	WSADATA wsaData;
	SOCKET hServSock;
	struct sockaddr_in hServAddr, hClntAddr;
	char buf[MAX] = "\0";
	int clen = sizeof(hClntAddr);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.0 DLL */
	{
		fprintf(stderr, "WSAStartup() failed");
		exit(1);
	}

	if ((hServSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "socket() failed");
		WSACleanup();
		exit(1);
	}

	memset(&hServAddr, 0, sizeof(hServAddr));
	hServAddr.sin_family = AF_INET;
	hServAddr.sin_addr.s_addr = INADDR_ANY;
	hServAddr.sin_port = htons(atoi(portNumber));

	if (bind(hServSock, (struct sockaddr *)&hServAddr, sizeof(hServAddr)) < 0)
	{
		printf("bind() error!\n");
		WSACleanup();
		closesocket(hServSock);
		exit(1);
	}
	printf("bind() ok..\n");

	listen(hServSock, 0);

	std::cout << "Waiting for connections...\n"
		<< "Server Port:" << "1234" << std::endl;

	int addrLen = sizeof(hServAddr);
	int remoteSocket;

	std::thread send_thread;
	std::thread recv_thread;

	while (1) {
		remoteSocket = accept(hServSock, (struct sockaddr *)&hServAddr, &addrLen);
		if (remoteSocket < 0) {
			perror("accept failed!");
			exit(1);
		}
		std::cout << "Connection accepted" << std::endl;

		send_thread = std::thread(send_data, remoteSocket);
		recv_thread = std::thread(recv_data, remoteSocket);
	}
	
	send_thread.join();
	recv_thread.join();

	// close the socket
	closesocket(remoteSocket);
}

void send_data(int ptr) {
	int socket = ptr;
	//OpenCV Code
	//----------------------------------------------------------

	Mat img;

	int height = cap.get(CAP_PROP_FRAME_HEIGHT);
	int width = cap.get(CAP_PROP_FRAME_WIDTH);

	std::cout << "height: " << height << std::endl;
	std::cout << "width: " << width << std::endl;

	img = Mat::zeros(height, width, CV_8UC3);

	int imgSize = img.total() * img.elemSize();
	int bytes = 0;

	// make img continuos
	if (!img.isContinuous()) {
		img = img.clone();
	}
	std::cout << "Image Size:" << imgSize << std::endl;

	bool status = true;

	while (status) {
		// get a frame from the camera
		cap >> img;
		cv::Mat out;		
		cv::resize(img, out, cv::Size(480, 480));
		imgSize = out.total() * out.elemSize();

		// send the flipped frame over the network 
		if ((bytes = send(socket, reinterpret_cast<const char*>(out.data), imgSize, 0)) < 0) {
			std::cerr << "bytes = " << bytes << std::endl;
			break;
		}

		mtx_lock.lock();
		if (running) status = false;
		mtx_lock.unlock();
		cv::waitKey(1);
	}

	std::cerr << "send quitting..." << std::endl;
	closesocket(socket);
}

void recv_data(int ptr) {
	int socket = ptr;
	char *buffer = nullptr;
	int bytes;

	while (1) {
		bytes = recv(socket, buffer, sizeof(int), MSG_WAITALL);

		if (bytes < 0) {
			std::cerr << "error receiving from client" << std::endl;
		}
		else if (bytes > 0) {
			int msg = atoi(buffer);

			if (msg == QUIT) {
				mtx_lock.lock();
				running = true;
				mtx_lock.unlock();
				break;
			}
		}
	}

	std::cerr << "recv quitting..." << std::endl;
	closesocket(socket);
}

void main()
{
	udpServer("1234");
}