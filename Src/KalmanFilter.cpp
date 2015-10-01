/*
 * KalmanFilter.cpp
 *
 *  Created on: 2015 máj. 31
 *      Author: Gábor
 */

#include "KalmanFilter.h"

KalmanFilter::KalmanFilter(float a, float b, float c, float r, float q) :
	A(a), B(b), C(c), r(r), q(q)
{
	x = 0;
	P = B*q*B;
}

float KalmanFilter::Filter(float u, float z) {
	/* State update */
	x = A*x+B*u;
	P = A*P*A + B*q*B;

	/* Measurement update */
	float K = P*C/(C*P*C+r);
	x = x + K*(z-C*x);
	P = (1-K*C)*P;

	return C*x;
}

void KalmanFilter::Reset() {
	x = 0;
	P = B*q*B;
}
