#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h> 
#include <string.h>

void udpClient(const char *IPaddress, const char *portNumber)
{
	int         socket_fd;
	char*       serverIP;
	int         serverPort;	

	struct  sockaddr_in serverAddr;
	socklen_t           addrLen = sizeof(struct sockaddr_in);

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << "socket() failed" << std::endl;
	}

	serverAddr.sin_family = PF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(IPaddress);
	serverAddr.sin_port = htons(5252);

	if (connect(socket_fd, (sockaddr*)&serverAddr, addrLen) < 0) {
		std::cerr << "connect() failed!" << std::endl;
	}

	//----------------------------------------------------------
	//OpenCV Code
	//----------------------------------------------------------

	cv::Mat img;
	img = cv::Mat::zeros(480, 480, CV_8UC3);
	int imgSize = img.total() * img.elemSize();
	uchar *iptr = img.data;
	int bytes = 0;
	int key;

	//make img continuos
	if (!img.isContinuous()) {
		img = img.clone();
	}

	std::cout << "Image Size:" << imgSize << std::endl;	

	while (key != 'q') {
		if ((bytes = recv(socket_fd, iptr, imgSize, MSG_WAITALL)) == -1) {
			std::cerr << "recv failed, received bytes = " << bytes << std::endl;
		}
		cv::imwrite("test.jpg", img);
	}	

	close(socket_fd);
}

int main()
{
	udpClient("127.0.0.1", "4000");
	return 0;
}