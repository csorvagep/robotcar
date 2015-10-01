/*
 * MotorController.h
 *
 *  Created on: 2015 máj. 31
 *      Author: Gábor
 */

#ifndef MOTORCONTROLLER_H_
#define MOTORCONTROLLER_H_

#define INCR_PER_METER 		7.07343e4f
#define METER_PER_INCR	 	1.41374e-5f
#define TIME_STEP			0.01f
#define FREQ				100.0f
#define MAX_VELOCITY		3.5f

class MotorController {
public:
	MotorController(float zd, float kc, float limit);

	void Reset();
	float Control(float vMeasure);

	/* v-Ref */
	void SetVRef(float vref);
	float GetVRef() const;

	/* Kc */
	void SetKc(float kc);
	float GetKc() const;

	/* U */
	float GetU() const;

private:
	/* Control reference signal */
	float vRef;

	/* Controller parameters */
	float zd,kc;
	float limit;

	/* Controller states */
	float u,uI;

	float saturate(float u);
};

#endif /* MOTORCONTROLLER_H_ */
