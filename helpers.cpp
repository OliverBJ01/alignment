/*
 * helpers.cpp
 *
 *  Created on: 30 Dec 2015
 *      Author: bernard
 */


#include "camLasAlignment.h"

//extern bool gshowCalRegion;	// set by button callback onButton() to display cal region
extern bool gLaserEnable;
extern bool gdisableEngagement;
extern uint xx, yy;											//co-ords send to scanner


//=============================================
// int cameraCapture( VideoCapture &cap, string camType, int usbDeviceNo, string ipCamStreamAddress)
//
//  establishes camera capture from USB or IP cameras
// 	camType: "USB" or "IP"
// 	for USB cam: usbDeviceNo: generally 0, or 1 with 2 cameras
// 	for IP cam: 	ipCamStreamAddress: stream address
//			e.g.  "http://root:Isabel00@10.1.1.69/mjpg/video.mjpg"	//Axis IP Camera @ Port 80
//										root  PW: usual
//					 "http://admin:Isabel00@10.1.1.5/video.cgi?.mjpg"  	// D-Link IP camera @ Port 80
//=============================================
int cameraCapture( VideoCapture &cap, string camType, int usbDeviceNo, string ipCamStreamAddress,
                   int &frameWidth, int &frameHeight)   {

	// IP camera capture
	if (camType == "IP") {
		cap.open(ipCamStreamAddress);
		if (!cap.isOpened())       {
			cerr << "Cannnot initialise IP camera:" << strerror(errno) << endl;
			return -1;
		}
	}
		// USB camera capture
	else  if (camType == "USB")  {
		cap.open(usbDeviceNo);
		if(!cap.isOpened()) {
			cerr << "Cannnot initialise USB camera:" << usbDeviceNo << " " << strerror(errno) << endl;
			return 1;
		}
	}
		// bad camera type
	else  {
		cerr << "bad camera parameter; " << camType << "use USB or IP"  << endl;
		return 1;
	}

	// falls through if OK
	cout << "camera initialised" << endl;

	frameWidth =  (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
	frameHeight = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	cout << "frame width: " << frameWidth << " frame height: " << frameHeight <<endl;

	return 0;
}
