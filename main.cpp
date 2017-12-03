
#include "opencv2/calib3d.hpp"
#include "camLasAlignment.h"

using namespace cv;
using namespace std;

int mcuScannersockfd;
int threadLive = false;

//globalS needed for trackbar callbacks
int camXMin, camXMax, camYMin, camYMax;
int lXMin, lXMax, lYMin, lYMax;

// laser extents in scanner units, arbitrary starting values
// globals used in thread which seems hard to pass params to
long lasXMin = 14000;
long lasXMax = 41000;
long lasYMin = 14000;
long lasYMax = 32000;


//-----------------------------------------------------------------
//
// trackbar callback
//
// trackbar value is int. i.e. 35k, and scanner max is 65k,
// so the trackbar displays 1/10 of scanner value, and when changed is
// scaled * 10 here
//
//------------------------------------------------------------------
void on_trackbar( int, void* )   {
	lasXMin = lXMin*10; lasXMax = lXMax*10;
	lasYMin = lYMin*10; lasYMax = lYMax*10;
}

//-----------------------------------------------------------------
//  int main()
//------------------------------------------------------------------

int main() {


	// Setup SimpleBlobDetector parameters.
	SimpleBlobDetector::Params params;

	// Change thresholds
	//params.minThreshold = 10;
	//params.maxThreshold = 200;

	params.filterByColor = 1;
	params.blobColor = 255;
	params.minThreshold = 10;
	params.maxThreshold = 250;

	params.maxArea = 20000;
	cout << params.minArea << endl;

	// Set up detector with params
	// SimpleBlobDetector::create creates a smart pointer, so access like: detector->detect( im, keypoints);
	Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);


	std::vector<KeyPoint> myBlobs;    //storage for blobs

	Mat frame;						// the live image  (3 channels)
	VideoCapture cap;			    // opencv video capture class

	Size imageSize;
	vector<Mat> rvecs, tvecs;
	vector<vector<Point2f> > imagePoints;
	vector<vector<Point3f> > objectPoints(1);   //??
	Mat cameraMatrix, distCoeffs;   // camera calibration matrices
	int frameWidth, frameHeight;	// camera frame size read during videocapture
	int flags = 0;
	uint x, y;
	int bdelay = 100000;

	pthread_t lt;
	int s;

	// connect to mcu scanner server
	// set up network and return global socket descriptor
	if (connectToServer())
		error("client: Network not established");

	// connect to QT5 scanner server
	// socket: UDP packets to scanner Console app
//	if ( initQtSocket())
//		return 1;

	// set up camera   IP / USB
	if (cameraCapture (cap, "USB", 0, IPCAM_STREAMADDRESS, frameWidth, frameHeight ))    // establish videocapture with USB or IP camera
		return 1;
	cap >> frame;

	// set camera extents for trackbar
	camXMin = 100;
	camXMax = frameWidth-160;
	camYMin = 100;
	camYMax = frameHeight-100;
	// set laser extents for trackbar
	lXMin = lasXMin/10;
	lXMax = lasXMax/10;
	lYMin = lasYMin/10;
	lYMax = lasYMax/10;

	namedWindow ("Main Window", WINDOW_NORMAL);
	namedWindow ("keypoints", WINDOW_NORMAL);
	namedWindow ("blobs", WINDOW_NORMAL);

	// init scanner
	sendMssgToScanner(0, 0, CMD_LASER_OFF);
	sendMssgToScanner(0, 0, CMD_SCANXPWR_OFF);
	sendMssgToScanner(0, 0, CMD_SCANYPWR_OFF);
	sendMssgToScanner(0, 0, CMD_SCANXPWR_ON);
	sendMssgToScanner(0, 0, CMD_SCANYPWR_ON);
	sendMssgToScanner(0, 0, CMD_LASER_ON);
	sendMssgToScanner(DAC_X_MAX / 2, DAC_Y_MAX / 2, CMD_SLEW);

	// create camera extent trackbars
	createTrackbar("camXMin", "Main Window", &camXMin, frameWidth / 2, nullptr, 0);
	createTrackbar("camXMax", "Main Window", &camXMax, frameWidth, nullptr, 0);
	createTrackbar("camYMin", "Main Window", &camYMin, frameHeight / 2, nullptr, 0);
	createTrackbar("camYMax", "Main Window", &camYMax, frameHeight, nullptr, 0);

	// create laser extent trackbars
	createTrackbar("lasXMin", "Main Window", &lXMin, DAC_X_MAX/20, on_trackbar, 0);
	createTrackbar("lasXMax", "Main Window", &lXMax, DAC_X_MAX/10, on_trackbar, 0);
	createTrackbar("lasYMin", "Main Window", &lYMin,  DAC_Y_MAX/20, on_trackbar, 0);
	createTrackbar("lasYMax", "Main Window", &lYMax,  DAC_Y_MAX/10, on_trackbar, 0);


	for(;;) {
		cap >> frame;
		if (frame.empty()) {
			cout << "frame is empty" << endl;
			break;
		}

		Mat fred = frame.clone();

		// Set up detector with params
		Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);

		for(;;) {
			for (long x = lasXMin; x <= lasXMax; x = x + (lasXMax - lasXMin) / 6) {
				for (long y = lasYMin; y <= lasYMax; y = y + (lasYMax - lasYMin) / 6) {

					sendMssgToScanner(x, y, CMD_SLEW);
					usleep(bdelay);
					sendMssgToScanner(0, 0, CMD_LASER_ON);
					usleep(bdelay);

					for(;;) {
						cap >> frame;

						// Detect blobs.
						detector->detect(frame, myBlobs);
						cout << myBlobs.size() << endl;
						if (myBlobs.size() >= 1) {
//							int xa = myBlobs[0].pt.x;
//							int ya = myBlobs[0].pt.y;
//							cout << xa << " " << ya << endl;
							//for(vector<KeyPoint>::iterator blobIterator = myBlobs.begin(); blobIterator != myBlobs.end(); blobIterator++){
							for(auto iter = myBlobs.begin(); iter != myBlobs.end(); iter++){
							cout << "size: " << iter->size << " at: " << iter->pt.x << " " << iter->pt.y << endl;
							}
						}

						// Draw detected blobs as red circles.
						// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
						Mat frame_with_keypoints;
						drawKeypoints( frame, myBlobs, frame_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

						// Show blobs
						imshow("keypoints", frame_with_keypoints );
						imshow("Main Window", frame);

						cv::Mat blobImg;
						cv::drawKeypoints(fred, myBlobs, blobImg);
						cv::imshow("blobs", blobImg);

						char k = (char)waitKey(1);     // param is mS delay wait for key event
						if(k == ' '  ) break;
						if( k == 27 ) exit(1);
						}


					sendMssgToScanner(0, 0, CMD_LASER_OFF);
					usleep(bdelay);



				}
			}
		}




		// draw the camera view extents, adjust with sliders
		line(frame, Point(camXMin, camYMin), Point(camXMax, camYMin), CV_RGB(0, 255, 0), 1, LINE_8, 0);
		line(frame, Point(camXMin, camYMax), Point(camXMax, camYMax), CV_RGB(0, 255, 0), 1, LINE_8, 0);
		line(frame, Point(camXMin, camYMin), Point(camXMin, camYMax), CV_RGB(0, 255, 0), 1, LINE_8, 0);
		line(frame, Point(camXMax, camYMin), Point(camXMax, camYMax), CV_RGB(0, 255, 0), 1, LINE_8, 0);

		//imshow("Main Window", frame);

//		sendMssgToScanner(lasXMin+1, lasYMin, CMD_SLEW);
//		sendMssgToScanner(lasXMax, lasYMin, CMD_SLEW);
//		sendMssgToScanner(lasXMax, lasYMax, CMD_SLEW);
//		sendMssgToScanner(lasXMin, lasYMax, CMD_SLEW);

//		//--------------------------------------------------------------------------------
//		// ---------- invokes thread to draw points or to draw a rectangle ---------------
//		//--------------------------------------------------------------------------------
//		// draw the laser extents; adjust with sliders
//		// don't spawn another thread if the old one is still live
//		if (!threadLive) {
//			s = pthread_create(&lt, NULL, drawLaserExtents, (void *) ",,");
//			//s = pthread_create(&lt, NULL, drawNbyNPoints, (void *) ",,");
//			if (s) {
//				cerr << "Cannnot create drawLaserExtents pthread" << strerror(errno) << endl;
//				return 1;
//			}
//		}







		// --------------------- User input ---------------------
		char k = (char)waitKey(1);     // param is mS delay wait for key event
		if( k == 27 ) break;
		if( k == ' ' )  {   printf("Space bar pressed\n");  break;  }
	}

//	double rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
//	                             distCoeffs, rvecs, tvecs, flags | CALIB_FIX_K4 | CALIB_FIX_K5);
//	///*|CALIB_FIX_K3*/|CALIB_FIX_K4|CALIB_FIX_K5);
//	printf("RMS error reported by calibrateCamera: %g\n", rms)

	return 0;
}

// ----------------------------------------------------------------------------
// void error(char *msg)
// ----------------------------------------------------------------------------
void error(const char *msg) {
	perror(msg);
	exit(1);
}
