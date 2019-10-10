#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <WinSock2.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <string>
#pragma comment(lib, "ws2_32.lib")

class ImgUDP
{
	WSADATA wsaData;
	SOCKET hServSock;
	struct sockaddr_in hServAddr, hClntAddr;
	int remoteSocket;

public:
	ImgUDP(std::string portNumber)
	{
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
		hServAddr.sin_port = htons(atoi(portNumber.c_str()));

		if (bind(hServSock, (struct sockaddr *)&hServAddr, sizeof(hServAddr)) < 0)
		{
			printf("bind() error!\n");
			WSACleanup();
			closesocket(hServSock);
			exit(1);
		}
	}

	~ImgUDP()
	{
		closesocket(remoteSocket);
	}

	void Connect()
	{
		listen(hServSock, 0);
		int addrLen = sizeof(hServAddr);
		remoteSocket = accept(hServSock, (struct sockaddr *)&hServAddr, &addrLen);
		if (remoteSocket < 0) {
			perror("accept failed!");
			exit(1);
		}
	}

	void SendImg(cv::Mat &img)
	{
		cv::Mat sendImg;
		img.copyTo(sendImg);
		int bytes = 0;

		if (!sendImg.isContinuous()) {
			sendImg = sendImg.clone();
		}

		cv::Mat out;
		cv::resize(img, out, cv::Size(480, 480));
		int imgSize = out.total() * out.elemSize();

		// send the flipped frame over the network 
		int socket = remoteSocket;
		if ((bytes = send(socket, reinterpret_cast<const char*>(out.data), imgSize, 0)) < 0) {
			std::cerr << "bytes = " << bytes << std::endl;
		}
	}
};

void main()
{
	ImgUDP imgUDP("1234");
	cv::VideoCapture cap("TLOUPII-SOP-EP3-FINAL_FR.mp4");

	imgUDP.Connect();

	cv::Mat img;
	for (;;) {
		cv::waitKey(1000);
		cap >> img;
		imgUDP.SendImg(img);
	}
}