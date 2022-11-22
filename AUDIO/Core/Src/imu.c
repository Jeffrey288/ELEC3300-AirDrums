#include "imu.h"
#include "stm32f1xx_hal.h"

imuState states[6] = { 0 };
extern int hits;
// imuLeft = (imuStruct ) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {0, 0, 0} };
// imuLeft = (imuStruct ) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {957.78, 515.08, 14.72} }; // 250DPS
imuStruct imuLeft = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_5}; // 2000DPS
imuStruct imuRight = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_6 };
imuStruct *imuStructs[2] = {&imuLeft, &imuRight};

void initIMUStruct(imuStruct *imu) {
	for (int i = 0; i < 7; i++) imu->prev_pitch[i] = 0;
	imu->pitch_acc = 0;
	for (int i = 0; i < 3; i++) imu->angles[i] = 0;
	imu->q.q1 = 1; imu->q.q2 = 0; imu->q.q3 = 0; imu->q.q4 = 0;
	imu->accelFlag = 0;
	imu->state = IMU_IDLE;
	imu->hit_tick = HAL_GetTick();
	imu->upCount = 0;
	imu->downCount = 0;

	MPU9250_GetData(imu->raw, imu->raw + 6, imu->raw + 3);
	imu->acc[3] = imu->raw[3] / 2048.0;
	if (imu->acc[3] < 0) imu->pitch_multiplier = 1;
	else imu->pitch_multiplier = -1;



}

void initIMUStructs() {
	for (int i = 0; i < 2; i++) initIMUStruct(imuStructs[i]);
}

// future improvements
// https://aircconline.com/csit/papers/vol10/csit100306.pdf
void updateIMU(imuStruct *imu) {

	imu_setActive(imu);
	MPU9250_GetData(imu->raw, imu->raw + 6, imu->raw + 3); // raw is acc, gyro, mag

	// check here for the scale factors: https://github.com/MarkSherstan/STM32-MPU6050-MPU9250-I2C-SPI/blob/main/C/SPI/Core/Src/MPU9250.c
	for (int i = 0; i < 3; i++) {
		imu->acc[i] = imu->raw[i] / 2048.0;
//			imu->acc[i] = kalmanUpdate(&accKalman[i], imu->acc[i]) * 9.81;
		imu->acc[i] = imu->acc[i] * 9.81;
		imu->gyro[i] = (imu->raw[i + 3] - imu->gyro_offset[i]) / 16.4;
//			imu->gyro[i] = kalmanUpdate(&gyroKalman[i], imu->gyro[i]);
		imu->gyro[i] = imu->pitch_multiplier * imu->gyro[i] / 180 * PI;
		imu->mag[i] = imu->raw[i + 6];
	}

	float deltaT = (HAL_GetTick() - imu->last_tick) / 1000.0;
	imu->last_tick = HAL_GetTick();
	imu_filter(&imu->q,
			imu->acc[0], imu->acc[1], imu->acc[2],
			imu->gyro[0], imu->gyro[1], imu->gyro[2],
			deltaT);

	eulerAngles(&imu->q, &imu->roll, &imu->pitch, &imu->yaw);

	// https://en.wikipedia.org/wiki/Finite_difference_coefficient
//	imu->pitch_acc = 469.0/90 * imu->pitch - 223.0/10 * imu->prev_pitch[0] + 879.0/20 * imu->prev_pitch[1] - 949.0/18 * imu->prev_pitch[2]
//					   + 41.0 * imu->prev_pitch[3] - 201.0/10 * imu->prev_pitch[4] + 1019.0/180 * imu->prev_pitch[5] - 7.0/10 * imu->prev_pitch[6];
	imu->pitch_acc = (2 * imu->pitch - 5 * imu->prev_pitch[0] + 4 * imu->prev_pitch[1] - imu->prev_pitch[2]) / (deltaT * deltaT);
	imu->pitch_acc /= 900;
//	imu->pitch_acc = 15.0/4 * imu->pitch - 77.0/6 * imu->prev_pitch[0]
//						   + 107.0/6 * imu->prev_pitch[1] - 13 * imu->prev_pitch[2]
//						  + 61.0/12 * imu->prev_pitch[3] - 5.0/6 * imu->prev_pitch[4];
 	for (int i = 7 - 1; i >= 1; i--) imu->prev_pitch[i] = imu->prev_pitch[i - 1]; // this is wrong :D
	imu->prev_pitch[0] = imu->pitch;

	if (imu->state != states[0]) {
		for (int i = 5; i >= 1; i--)
			states[i] = states[i - 1];
		states[0] = imu->state;
	}

	// (0.7 + 0.3 * expf((imu->upCount - imu->downCount)/50.0))
//	switch (imu->state) {
//	case IMU_IDLE:
//	case IMU_IDLE_PN:
//	case IMU_IDLE_NP:
//		if (imu->pitch_acc < ((imu->downCount - imu->upCount > 10) ? -8 : -13)) {
//			if (imu->state != IMU_IDLE_PN) {
//				imu->state = IMU_NEG;
//				imu->hit_tick = HAL_GetTick();
//			} else if (imu->state == IMU_IDLE_PN && HAL_GetTick() - imu->hit_tick < 70 && imu->pitch_acc < -12) {
//				imu->hit_tick = HAL_GetTick();
//			} else {
//				imu->state = IMU_IDLE;
//			}
//		} else if (imu->pitch_acc > ((imu->upCount - imu->downCount > 10) ? 8 : 13)) {
//			if (imu->state != IMU_IDLE_NP) {
//				imu->state = IMU_POS;
//				imu->hit_tick = HAL_GetTick();
//			} else if (imu->state == IMU_IDLE_NP && HAL_GetTick() - imu->hit_tick < 70 && imu->pitch_acc > 12) {
//				imu->hit_tick = HAL_GetTick();
//			} else {
//				imu->state = IMU_IDLE;
//			}
//		} else if (HAL_GetTick() - imu->hit_tick > 70) { // effectively a cooldown to prevent a hit being detected in the opposite direciton
//			imu->state = IMU_IDLE;
//		}
//		break;
//	case IMU_POS:
//		if (HAL_GetTick() - imu->hit_tick < 90) {
//			if (imu->pitch_acc < -6) {
//				hits++;
//				imu->upCount++;
//				imu->state = IMU_HIT_PN;
//				imu->hit_tick = HAL_GetTick();
//			}
//		} else {
//			imu->state = IMU_IDLE;
//		}
//		break;
//	case IMU_NEG:
//		if (HAL_GetTick() - imu->hit_tick < 90) {
//			if (imu->pitch_acc > 6) {
//				hits++;
//				imu->downCount++;
//				imu->state = IMU_HIT_NP;
//				imu->hit_tick = HAL_GetTick();
//			}
//		} else {
//			imu->state = IMU_IDLE;
//		}
//		break;
//	case IMU_HIT_NP:
//	case IMU_HIT_PN:
//		if (imu->pitch_acc < -8 || imu->pitch_acc > 8) {
//			imu->hit_tick = HAL_GetTick();
//		} else {
//			if (HAL_GetTick() - imu->hit_tick > 60) {
//				if (imu->state == IMU_HIT_NP) imu->state = IMU_IDLE_NP;
//				else /*imu->state == IMU_HIT_PN*/ imu->state = IMU_IDLE_PN;
//			}
//		}
//		break;
//	}
//
//	if (imu->pitch_acc < -7) imu->accelFlag = 1;
//	if (imu->pitch_acc > 7) imu->accelFlag = 1;
	if (imu->acc[2] < -15) imu->accelFlag = 1;
	else imu->accelFlag = 0;
//	if (imu->acc[2] > 30) imu->accelFlag = 0;
}

void updateIMUs() {
	for (int i = 0; i < 2; i++) updateIMU(imuStructs[i]);
}
