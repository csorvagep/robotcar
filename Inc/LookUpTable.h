/*
 * LookUpTable.h
 *
 *  Created on: 2015 m�j. 31
 *      Author: G�bor
 */

#ifndef LOOKUPTABLE_H_
#define LOOKUPTABLE_H_

#include <stm32f4xx.h>

class LookUpTable {
public:
	LookUpTable(const float *x, const float *y, uint16_t n);

	float Look(float x0);

private:
	const float *x;
	const float *y;
	uint16_t n;
};

#endif /* LOOKUPTABLE_H_ */
