/*
 * KalmanFilter.h
 *
 *  Created on: 2015 máj. 31
 *      Author: Gábor
 */

#ifndef KALMANFILTER_H_
#define KALMANFILTER_H_

class KalmanFilter {
public:
	KalmanFilter(float a, float b, float c, float r, float q);

	float Filter(float u, float z);
	void Reset();

private:
	/* State */
	float x;
	/* State and measurement vectors */
	float A,B,C;
	/* Co-variances */
	float r,q,P;
};

#endif /* KALMANFILTER_H_ */
