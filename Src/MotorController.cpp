/*
 * MotorController.cpp
 *
 *  Created on: 2015 máj. 31
 *      Author: Gábor
 */

#include "MotorController.h"

MotorController::MotorController(float zd, float kc, float limit) :
	zd(zd), kc(kc), limit(limit)
{
	Reset();
}

float MotorController::GetVRef() const {
	return vRef * METER_PER_INCR * FREQ;
}

void MotorController::SetVRef(float vref) {
	if(vref > MAX_VELOCITY) {
		vref = MAX_VELOCITY;
	} else if(vref < -MAX_VELOCITY) {
		vref = -MAX_VELOCITY;
	}
	vRef = vref * INCR_PER_METER * TIME_STEP;
}

float MotorController::GetKc() const {
	return kc;
}

void MotorController::SetKc(float kc) {
	this->kc = kc;
}

float MotorController::GetU() const {
	return u;
}

void MotorController::Reset() {
	u = 0;
	uI = 0;
	vRef = 0;
}

float MotorController::Control(float vMeasure) {
	uI = zd * uI + (1 - zd) * u;
	float uP = kc*(vRef - vMeasure);
	return u = saturate(uP + uI);
}

float MotorController::saturate(float u) {
	if(u > limit)
		return limit;
	if(u < -limit)
		return -limit;
	return u;
}
