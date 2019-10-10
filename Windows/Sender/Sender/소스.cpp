#pragma comment(lib, "ws2_32.lib")

#include "opencv2/opencv.hpp"
#include <iostream>
#include <WinSock2.h>
#include <mutex>
#include <thread>

using namespace cv;

void send_data(int);
void recv_data(int);

std::mutex mtx_lock;
bool running = false;
int QUIT = 0;
int capDev = 0;
// open the default camera
VideoCapture cap("TLOUPII-SOP-EP3-FINAL_FR.mp4");

int main(int argc, char** argv)
{
	//--------------------------------------------------------
	//networking stuff: socket, bind, listen
	//--------------------------------------------------------
	int localSocket,
		remoteSocket,
		port = 4097;
	struct  sockaddr_in localAddr,
		remoteAddr;
	int addrLen = sizeof(struct sockaddr_in);

	std::thread send_thread;
	std::thread recv_thread;
	

	localSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (localSocket == -1) {
		perror("socket() call failed!!");
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = INADDR_ANY;
	localAddr.sin_port = htons(port);

	if (bind(localSocket, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0) {
		perror("Can't bind() socket");
		exit(1);
	}

	// Listening
	listen(localSocket, 1);

	std::cout << "Waiting for connections...\n"
		<< "Server Port:" << port << std::endl;

	//accept connection from an incoming client
	while (1) {
		remoteSocket = accept(localSocket, (struct sockaddr *)&remoteAddr, &addrLen);

		if (remoteSocket < 0) {
			perror("accept failed!");
			exit(1);
		}

		std::cout << "Connection accepted" << std::endl;
		send_thread = std::thread(send_data, remoteSocket);
		recv_thread = std::thread(recv_data, remoteSocket);

		// pthread_join(thread_id, NULL);
	}

	send_thread.join();
	recv_thread.join();

	// close the socket
	closesocket(remoteSocket);

	return 0;
}

void send_data(int ptr) {
	int socket = ptr;
	//OpenCV Code
	//----------------------------------------------------------

	Mat img, flippedFrame;

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
		cv::resize(img, out, cv::Size(480,480));
		cv::imshow("test", img);

		// send the flipped frame over the network
		if ((bytes = send(socket, reinterpret_cast<const char*>(flippedFrame.data), imgSize, 0)) < 0) {
			std::cerr << "bytes = " << bytes << std::endl;
			break;
		}

		mtx_lock.lock();
		if (running) status = false;
		mtx_lock.unlock();
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