#include <stdio.h>
#include <WinSock2.h>
#include <iostream>
#include "opencv2/opencv.hpp"
using namespace cv;
#pragma comment(lib, "ws2_32.lib")

void udpClient(const char *IPaddress)
{
	WSADATA wsaData;
	SOCKET socket_fd;
	struct  sockaddr_in serverAddr;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		fprintf(stderr, "WSAStartup() failed");
		exit(1);
	}

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "socket() failed");
		WSACleanup();
		exit(1);
	}
	
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(IPaddress);
	serverAddr.sin_port = htons(5252);

	if (connect(socket_fd, (sockaddr*)&serverAddr, sizeof(struct sockaddr_in)) < 0) {
		std::cerr << "connect() failed!" << std::endl;
		closesocket(socket_fd);
		WSACleanup();
		exit(1);
	}

	//----------------------------------------------------------
	//OpenCV Code
	//----------------------------------------------------------

	Mat img;
	img = Mat::zeros(480, 480, CV_8UC3);
	int imgSize = img.total() * img.elemSize();
	
	int bytes = 0;
	int key = 0;

	//make img continuous
	if (!img.isContinuous()) {
		img = img.clone();
	}

	std::cout << "Image Size:" << imgSize << std::endl;

	namedWindow("CV Video Client", 1);

	int idx = 0;
	while (key != 'q') {
		if ((bytes = recv(socket_fd, reinterpret_cast<char*>(img.data), imgSize, MSG_WAITALL)) == -1) {
			std::cerr << "recv failed, received bytes = " << bytes << std::endl;
		}		
		else
		{
			std::cout << "received bytes = " << bytes << std::endl;
		}
		cv::imshow("CV Video Client", img);		
		cv::waitKey(100);		
	}
	

	closesocket(socket_fd);
	WSACleanup();
}

void main()
{
	udpClient("127.0.0.1");
}