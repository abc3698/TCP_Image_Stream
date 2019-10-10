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

class ImgSender
{	
	struct sockaddr_in server_addr, client_addr;
	int client_socket;
	int server_socket;
public:
	ImgSender(std::string portNumber)
	{
		server_socket = socket(PF_INET, SOCK_STREAM, 0);
		if (server_socket == -1) {
			perror("socket() call failed!!");
		}

		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(5252);

		if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
			perror("Can't bind() socket");
			exit(1);
		}		
	}

	~ImgSender()
	{
		close(client_socket);
	}

	void Connect()
	{		
		if (listen(server_socket, 5) == -1)
		{
			perror("Listen Faildes");
		}		
		int addrLen = sizeof(client_addr);		
		client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&addrLen);		
		if (client_socket < 0) {
			perror("accept failed!!!!!!!!!!!!!!!!!!");
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
		if ((bytes = send(client_socket, out.data, imgSize, 0)) < 0) {
			std::cerr << "bytes = " << bytes << std::endl;
		}
		else
		{
			std::cerr << "bytes = " << bytes << std::endl;
		}
	}
};

int main()
{
	ImgSender imgUDP("5252");
	cv::VideoCapture cap;
	if (!cap.open("TLOUPII-SOP-EP3-FINAL_FR.mp4"))
	{
		perror("Open failed!!!!!!!!!!!!!!!!!!");
		exit(1);
	}

	imgUDP.Connect();

	cv::Mat img;
	for (;;) {		
		cap >> img;				
		imgUDP.SendImg(img);
	}

	return 0;
}