//
// Created by bernard on 11/06/17.
//
#include "camLasAlignment.h"

extern int lasXMin, lasXMax, lasYMin, lasYMax;
extern int threadLive;

//--------------------------------------------------------------------------------------
//
// drawLaserExtents

// draws a rectangle
//
// uses globals cause seems difficult to pass params into the thread
//
//--------------------------------------------------------------------------------------
void *drawLaserExtents(void *arg) {

	// set flag to tell main() to wait untill finsihed
	threadLive = true;

	for (;;) {
		Bresenham(lasXMin, lasYMin, lasXMax, lasYMin);
		Bresenham(lasXMax, lasYMin, lasXMax, lasYMax);
		Bresenham(lasXMax, lasYMax, lasXMin, lasYMax);
		Bresenham(lasXMin, lasYMax, lasXMin, lasYMin);
	}
	threadLive = false;
	pthread_exit (NULL);
}


//--------------------------------------------------------------------------------------
//
// drawNbyNPoints

// draws a rectangle
//
// uses globals because it seems to be difficult to pass params into the thread
//
//--------------------------------------------------------------------------------------
void *drawNbyNPoints(void *arg) {

	// set flag to tell main() to wait untill finsihed
	threadLive = true;
	int bdelay = 100000;

	for(;;) {

		for (long x = lasXMin; x <= lasXMax; x = x + (lasXMax - lasXMin) / 6) {
			for (long y = lasYMin; y <= lasYMax; y = y + (lasYMax - lasYMin) / 6) {
				sendMssgToScanner(x, y, CMD_SLEW);
				usleep(bdelay);
				sendMssgToScanner(0, 0, CMD_LASER_ON);
				usleep(bdelay);
				sendMssgToScanner(0, 0, CMD_LASER_OFF);
				usleep(bdelay);
			}
		}
	}
}



//--------------------------------------------------------------------------------------
//
// Bresenham's line algorithm
//
// use blocking routine to drive laser else app locks up
//
//--------------------------------------------------------------------------------------
void Bresenham(long x1, long y1, long x2, long y2) {

#define bScale 400    // 1 when pixels, step 50 when scanner units

	x1 = x1 / bScale;
	x2 = x2 / bScale;
	y1 = y1 / bScale;
	y2 = y2 / bScale;

	int delta_x(x2 - x1);
	// if x1 == x2, then it does not matter what we set here
	signed char const ix((delta_x > 0) - (delta_x < 0));
	//int ix((delta_x > 0) - (delta_x < 0));
	delta_x = std::abs(delta_x) << 1;

	int delta_y(y2 - y1);
	// if y1 == y2, then it does not matter what we set here
	signed char const iy((delta_y > 0) - (delta_y < 0));
	//int iy((delta_y > 0) - (delta_y < 0));
	delta_y = std::abs(delta_y) << 1;

	sendMssgToScanner(x1 * bScale, y1 * bScale, CMD_SLEW);

	if (delta_x >= delta_y) {
		// error may go below zero
		long error(delta_y - (delta_x >> 1));

		while (x1 != x2) {

			if ((error >= 0) && (error || (ix > 0))) {
				error -= delta_x;
				y1 += iy;
			}
			// else do nothing
			error += delta_y;
			x1 += ix;
			sendMssgToScanner(x1 * bScale, y1 * bScale, CMD_SLEW);
		}
	} else {
		// error may go below zero
		long error(delta_x - (delta_y >> 1));

		while (y1 != y2) {
			if ((error >= 0) && (error || (iy > 0))) {
				error -= delta_y;
				x1 += ix;
			}
			// else do nothing

			error += delta_x;
			y1 += iy;

			sendMssgToScanner(x1 * bScale, y1 * bScale, CMD_SLEW);
		}
	}
}
