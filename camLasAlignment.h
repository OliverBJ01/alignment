//
// Created by bernard on 10/06/17.
//

#ifndef CAMLASALIGNMENT_CAMLASALIGNMENT_H
#define CAMLASALIGNMENT_CAMLASALIGNMENT_H

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <unistd.h>             // for usleep

using namespace cv;
using namespace std;

// IP cam stream addresses
//#define IPCAM_STREAMADDRESS "http://root:Isabel00@10.1.1.5/mjpg/video.mjpg"	//Axis IP Camera @ Port 80
#define IPCAM_STREAMADDRESS "http://admin:Isabel00@10.1.1.5/video.cgi?.mjpg"  	// D-Link IP camera @ Port 80

int cameraCapture( VideoCapture &cap, string camType, int usbDeviceNo, string ipCamStreamAddress,
                   int &frameWidth, int &frameHeight);


// -------------  scanner defs  ------------------
// scanner frame size is theoretically 0 to 65535 both x and y set by scanner DAC range
// However, this is the practical frame size as scanner board trips outside these values
#define DAC_X_MIN 512
#define DAC_X_MAX 65024
#define DAC_Y_MIN 512
#define DAC_Y_MAX 65024

// used only to reverse direction in sendScannerMssg()
#define SCANNER_DAC_LIMIT 65535

#endif //CAMLASALIGNMENT_CAMLASALIGNMENT_H

// mcuScanner server
#define SCANNERPORT "1000"          // scanner port No.
#define SCANNERADDRESS "10.1.1.99"  // scanner IP address

//#define OPENCVPORT 9988             // port to receive UDP packets from openCV

// ---------message details copied from mcuServer  -----------
struct statusWord {
	ushort	serverFault		:	1;	// server has fault
	ushort 	laserFault		:	1;	// server fault
	ushort	scanXFault		:	1;	// server fault
	ushort	scanYFault		:	1;	// server fault
	ushort	unknownScannerFault	:	1;	// server fault
	ushort	hwStartFail		:	1;	// server hardware didn't start
	ushort 	laserPower		: 	1;	// client command
	ushort	dacReset		:	1;	// client command
	ushort	spiError		:	1;	// server sets on excessive spi errors
	ushort	verbose			:	1;	// client command
} ;

// 1. ----  messages received by client from mcuServer
struct serverMssg {		        // message sent to client
	ushort scanX;				//scanner X value
	ushort scanY;
	ushort info;
	struct statusWord status;
};

// 2. -----  message sent to mcuServer by client
struct clientMssg {		    // message recvd. from client
	ushort scanX;		    //scanner X value
	ushort scanY;
	ushort info;		    // send data client to server & vice versa
} ;

// (this) client's commands to server
enum cmdMssg {CMD_LASER_OFF, CMD_LASER_ON,
	CMD_SCANXPWR_OFF, CMD_SCANXPWR_ON,
	CMD_SCANYPWR_OFF, CMD_SCANYPWR_ON,
	CMD_SLEW, CMD_VERBOSE_ON, CMD_VERBOSE_OFF,
	CMD_NULL_MSSG, CMD_RESET } ;



//void sendMssg(uint x, uint y, bool laserOn) ;
int initQtSocket(void);
void showCalRectangle(int state, void*);
void disableEngagement(int state, void*);
int initLaserEnableTimer(void) ;
void Bresenham(long x1, long y1, long x2, long y2);
int connectToServer();
void error(const char *msg);
void sendMssgToScanner(uint x, uint y, uint laserOn);
void *drawLaserExtents(void *arg);
void *drawNbyNPoints(void *arg);