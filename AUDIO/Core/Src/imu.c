#include "imu.h"
#include "stm32f1xx_hal.h"

imuState states[6] = { 0 };
extern int hits;
// imuLeft = (imuStruct ) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {0, 0, 0} };
// imuLeft = (imuStruct ) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {957.78, 515.08, 14.72} }; // 250DPS
imuStruct imuLeft = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {118.28, 63.09, -0.52},
	.prev_pitch = {0, 0, 0, 0, 0, 0, 0}, .pitch_acc = 0, .angles = {0, 0, 0}, .q = {1, 0, 0, 0}, .accelFlag = 0,
	.state = IMU_IDLE, .hit_tick = 0, .upCount = 0, .downCount = 0}; // 2000DPS
imuStruct imuRight = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_6 };
imuStruct *imuStructs[2] = {&imuLeft, &imuRight};

// future improvements
// https://aircconline.com/csit/papers/vol10/csit100306.pdf
void updateIMUs() {
	imu_setActive(&imuLeft);
	MPU9250_GetData(imuLeft.raw, imuLeft.raw + 6, imuLeft.raw + 3); // raw is acc, gyro, mag

	// check here for the scale factors: https://github.com/MarkSherstan/STM32-MPU6050-MPU9250-I2C-SPI/blob/main/C/SPI/Core/Src/MPU9250.c
	for (int i = 0; i < 3; i++) {
		imuLeft.acc[i] = imuLeft.raw[i] / 2048.0;
//			imuLeft.acc[i] = kalmanUpdate(&accKalman[i], imuLeft.acc[i]) * 9.81;
		imuLeft.acc[i] = imuLeft.acc[i] * 9.81;
		imuLeft.gyro[i] = (imuLeft.raw[i + 3] - imuLeft.gyro_offset[i]) / 16.4;
//			imuLeft.gyro[i] = kalmanUpdate(&gyroKalman[i], imuLeft.gyro[i]);
		imuLeft.gyro[i] = imuLeft.gyro[i] / 180 * PI;
		imuLeft.mag[i] = imuLeft.raw[i + 6];
	}

	float deltaT = (HAL_GetTick() - imuLeft.last_tick) / 1000.0;
	imuLeft.last_tick = HAL_GetTick();
	imu_filter(&imuLeft.q,
			imuLeft.acc[0], imuLeft.acc[1], imuLeft.acc[2],
			imuLeft.gyro[0], imuLeft.gyro[1], imuLeft.gyro[2],
			deltaT);

	eulerAngles(&imuLeft.q, &imuLeft.roll, &imuLeft.pitch, &imuLeft.yaw);

	// https://en.wikipedia.org/wiki/Finite_difference_coefficient
//	imuLeft.pitch_acc = 469.0/90 * imuLeft.pitch - 223.0/10 * imuLeft.prev_pitch[0] + 879.0/20 * imuLeft.prev_pitch[1] - 949.0/18 * imuLeft.prev_pitch[2]
//					   + 41.0 * imuLeft.prev_pitch[3] - 201.0/10 * imuLeft.prev_pitch[4] + 1019.0/180 * imuLeft.prev_pitch[5] - 7.0/10 * imuLeft.prev_pitch[6];
	imuLeft.pitch_acc = (2 * imuLeft.pitch - 5 * imuLeft.prev_pitch[0] + 4 * imuLeft.prev_pitch[1] - imuLeft.prev_pitch[2]) / (deltaT * deltaT);
	imuLeft.pitch_acc /= 900;
//	imuLeft.pitch_acc = 15.0/4 * imuLeft.pitch - 77.0/6 * imuLeft.prev_pitch[0]
//						   + 107.0/6 * imuLeft.prev_pitch[1] - 13 * imuLeft.prev_pitch[2]
//						  + 61.0/12 * imuLeft.prev_pitch[3] - 5.0/6 * imuLeft.prev_pitch[4];
 	for (int i = 7 - 1; i >= 1; i--) imuLeft.prev_pitch[i] = imuLeft.prev_pitch[i - 1]; // this is wrong :D
	imuLeft.prev_pitch[0] = imuLeft.pitch;

	if (imuLeft.state != states[0]) {
		for (int i = 5; i >= 1; i--)
			states[i] = states[i - 1];
		states[0] = imuLeft.state;
	}

	// (0.7 + 0.3 * expf((imuLeft.upCount - imuLeft.downCount)/50.0))
//	switch (imuLeft.state) {
//	case IMU_IDLE:
//	case IMU_IDLE_PN:
//	case IMU_IDLE_NP:
//		if (imuLeft.pitch_acc < ((imuLeft.downCount - imuLeft.upCount > 10) ? -8 : -13)) {
//			if (imuLeft.state != IMU_IDLE_PN) {
//				imuLeft.state = IMU_NEG;
//				imuLeft.hit_tick = HAL_GetTick();
//			} else if (imuLeft.state == IMU_IDLE_PN && HAL_GetTick() - imuLeft.hit_tick < 70 && imuLeft.pitch_acc < -12) {
//				imuLeft.hit_tick = HAL_GetTick();
//			} else {
//				imuLeft.state = IMU_IDLE;
//			}
//		} else if (imuLeft.pitch_acc > ((imuLeft.upCount - imuLeft.downCount > 10) ? 8 : 13)) {
//			if (imuLeft.state != IMU_IDLE_NP) {
//				imuLeft.state = IMU_POS;
//				imuLeft.hit_tick = HAL_GetTick();
//			} else if (imuLeft.state == IMU_IDLE_NP && HAL_GetTick() - imuLeft.hit_tick < 70 && imuLeft.pitch_acc > 12) {
//				imuLeft.hit_tick = HAL_GetTick();
//			} else {
//				imuLeft.state = IMU_IDLE;
//			}
//		} else if (HAL_GetTick() - imuLeft.hit_tick > 70) { // effectively a cooldown to prevent a hit being detected in the opposite direciton
//			imuLeft.state = IMU_IDLE;
//		}
//		break;
//	case IMU_POS:
//		if (HAL_GetTick() - imuLeft.hit_tick < 90) {
//			if (imuLeft.pitch_acc < -6) {
//				hits++;
//				imuLeft.upCount++;
//				imuLeft.state = IMU_HIT_PN;
//				imuLeft.hit_tick = HAL_GetTick();
//			}
//		} else {
//			imuLeft.state = IMU_IDLE;
//		}
//		break;
//	case IMU_NEG:
//		if (HAL_GetTick() - imuLeft.hit_tick < 90) {
//			if (imuLeft.pitch_acc > 6) {
//				hits++;
//				imuLeft.downCount++;
//				imuLeft.state = IMU_HIT_NP;
//				imuLeft.hit_tick = HAL_GetTick();
//			}
//		} else {
//			imuLeft.state = IMU_IDLE;
//		}
//		break;
//	case IMU_HIT_NP:
//	case IMU_HIT_PN:
//		if (imuLeft.pitch_acc < -8 || imuLeft.pitch_acc > 8) {
//			imuLeft.hit_tick = HAL_GetTick();
//		} else {
//			if (HAL_GetTick() - imuLeft.hit_tick > 60) {
//				if (imuLeft.state == IMU_HIT_NP) imuLeft.state = IMU_IDLE_NP;
//				else /*imuLeft.state == IMU_HIT_PN*/ imuLeft.state = IMU_IDLE_PN;
//			}
//		}
//		break;
//	}

	if (imuLeft.pitch_acc < -7) imuLeft.accelFlag = 1;
//	if (imuLeft.pitch_acc > 7) imuLeft.accelFlag = 1;
	else imuLeft.accelFlag = 0;
//	if (imuLeft.acc[2] < -15) imuLeft.accelFlag = 1;
//	if (imuLeft.acc[2] > 30) imuLeft.accelFlag = 0;
}
