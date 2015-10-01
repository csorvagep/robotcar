/*
 * LookUpTable.cpp
 *
 *  Created on: 2015 máj. 31
 *      Author: Gábor
 */

#include "LookUpTable.h"

LookUpTable::LookUpTable(const float *x, const float *y, uint16_t n) :
		x(x), y(y), n(n) {
}

float LookUpTable::Look(float x0) {
	uint8_t i = 0;
	int8_t sign = 1;

	if (x0 < 0) {
		sign = -1;
		x0 *= -1;
	}

	while ((i < (n - 1)) && (y[i] < x0)) {
		i++;
	}

	float k = (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
	return sign * (k*(x0-x[i]) + y[i]);
}
