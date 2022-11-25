#include "imu.h"
#include "stm32f1xx_hal.h"


imuState states[6] = { 0 };
extern int hits;
// imuLeft = (imuStruct ) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {0, 0, 0} };
// imuLeft = (imuStruct ) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {957.78, 515.08, 14.72} }; // 250DPS
//imuStruct imuLeft = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {118.28, 63.09, -0.52}}; // 2000DPS
//imuStruct imuLeft = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {479.56, 259.84, -22.92}}; // 2000DPS
//imuStruct imuLeft = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {118.44, 63.68, 6.68}}; // 2000DPS
imuStruct imuLeft = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {122.06, 63.60, 6.72}}; // 2000DPS
//imuStruct imuLeft = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {89.12, 90.26, -5.78}}; // 2000DPS
//imuStruct imuLeft = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_5, .gyro_offset = {89.12, 90.26, -5.78},
//	.prev_pitch = {0, 0, 0, 0, 0, 0, 0}, .pitch_acc = 0, .angles = {0, 0, 0}, .q = {1, 0, 0, 0}, .accelFlag = 0,
//	.state = IMU_IDLE, .hit_tick = 0, .upCount = 0, .downCount = 0}; // 2000DPS
imuStruct imuRight = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_6, .gyro_offset = {-64.29, -22.97, -0.61} };
//imuStruct imuRight = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_6, .gyro_offset = {-53.88, -23.05, -0.40} };
//imuStruct imuRight = (imuStruct) { .port = GPIOE, .pin = GPIO_PIN_6, .gyro_offset = {-569.91, -162.70, 5.67} };
imuStruct *imuStructs[2] = {&imuLeft, &imuRight};

void initIMUStruct(imuStruct *imu) {
	for (int i = 0; i < 7; i++) imu->prev_pitch[i] = 0;
	for (int i = 0; i < 7; i++) imu->prev_gyro[i] = 0;
	imu->pitch_acc = 0;
	for (int i = 0; i < 3; i++) imu->angles[i] = 0;
	imu->q.q1 = 1; imu->q.q2 = 0; imu->q.q3 = 0; imu->q.q4 = 0;
	imu->accelFlag = 0;
	imu->state = IMU_IDLE;
	imu->last_tick = HAL_GetTick();
	imu->hit_tick = HAL_GetTick();
	imu->upCount = 0;
	imu->downCount = 0;

	for (int i = 0; i < 3; i++) {
//		imu->gyro_filters[i] = kalman(0.06, 0.2);
//		imu->acc_filters[i] = kalman(0.3, 0.3);
	}

	MPU9250_GetData(imu->raw, imu->raw + 6, imu->raw + 3);
	imu->acc[2] = imu->raw[2] / 2048.0;
//	imu->pitch_multiplier = 1;
	if (imu->acc[2] < 0) imu->pitch_multiplier = 1;
	else imu->pitch_multiplier = -1;

}

void initIMUStructs() {
	for (int i = 0; i < 2; i++) initIMUStruct(imuStructs[i]);
}

// future improvements
// https://aircconline.com/csit/papers/vol10/csit100306.pdf
void inline updateIMU(imuStruct *imu) {

	imu_setActive(imu);

	MPU9250_GetData(imu->raw, imu->raw + 6, imu->raw + 3); // raw is acc, gyro, mag

	// check here for the scale factors: https://github.com/MarkSherstan/STM32-MPU6050-MPU9250-I2C-SPI/blob/main/C/SPI/Core/Src/MPU9250.c
//	for (int i = 0; i < 3; i++) {
////		imu->acc[i] = imu->raw[i] / 2048.0;
////		imu->acc[i] = imu->acc[i] * 9.81;
//		imu->acc[i] = imu->raw[i] * 9.81 / 2048.0;
////		imu->acc[i] = imu->raw[i];
////		imu->acc[i] = kalmanUpdate(&accKalman[i], imu->acc[i]) * 9.81;
////		imu->gyro[i] = imu->raw[i + 3];
////		imu->gyro[i] = imu->pitch_multiplier * (imu->raw[i + 3] - imu->gyro_offset[i]) / 16.4;
////		imu->gyro[i] = imu->gyro[i] / 180 * PI;
//		imu->gyro[i] = (imu->raw[i + 3] - imu->gyro_offset[i]) / 16.4 / 180 * PI;
////		imu->gyro[i] = kalmanUpdate(&gyroKalman[i], imu->gyro[i]);
////		imu->gyro[i] = kalmanUpdate(&(imu->gyro_filters[i]), imu->gyro[i]);
////		imu->mag[i] = imu->raw[i + 6]; // will not be using mag
//	}
//	imu->acc[0] = imu->raw[0] * 0.00479003906f;
//	imu->acc[1] = imu->raw[1] * 0.00479003906f;
	imu->acc[2] = imu->raw[2] * 0.00479003906f;
//	imu->gyro[0] = (imu->raw[3] - imu->gyro_offset[0]) * 0.00106422515f;
//	imu->gyro[1] = (imu->raw[4] - imu->gyro_offset[1]) * 0.00106422515f;
	imu->gyro[2] = (imu->raw[5] - imu->gyro_offset[2]) * 0.00106422515f;


//	for (int i = 7 - 1; i >= 1; i--) {
//		imu->prev_gyro[i] = imu->prev_gyro[i - 1]; // this is wrong :D
//		imu->prev_ticks[i] = imu->prev_ticks[i - 1];
//	}
//	imu->prev_gyro[0] = imu->gyro[2];
//	imu->prev_ticks[0] = HAL_GetTick();
//
//	float deltaT;

//	deltaT = (HAL_GetTick() - imu->last_tick) / 1000.0;
//	imu->last_tick = HAL_GetTick();
	imu->yaw += imu->gyro[2] * (1.0 / IMU_REFRESH_RATE) * 180 / PI;

//	deltaT = (imu->prev_ticks[0] - imu->prev_ticks[2]) / 1000.0 / 2;
//	imu->pitch += deltaT / 2 * (imu->prev_gyro[0] + imu->prev_gyro[2]) * 180 / PI;

//	deltaT = (imu->prev_ticks[0] - imu->prev_ticks[2]) / 1000.0 / 2;
//	imu->yaw += deltaT / 6 * (imu->prev_gyro[0] + 4 * imu->prev_gyro[1] + imu->prev_gyro[2]) * 180 / PI;
	if (imu->yaw > 80) imu->yaw = 80;
	else if (imu->yaw < -80) imu->yaw = -80;

//#define IMU_ITERS 3
//	for (int i = 0; i < IMU_ITERS; i++)
//		imu_filter(&(imu->q),
//				imu->acc[0], imu->acc[1], imu->acc[2],
//				imu->gyro[0], imu->gyro[1], imu->gyro[2],
//				1/400.0/(IMU_ITERS));

//	eulerAngles(&(imu->q), &(imu->roll), &(imu->pitch), &(imu->yaw));

	// https://en.wikipedia.org/wiki/Finite_difference_coefficient
//	imu->pitch_acc = 469.0/90 * imu->pitch - 223.0/10 * imu->prev_pitch[0] + 879.0/20 * imu->prev_pitch[1] - 949.0/18 * imu->prev_pitch[2]
//					   + 41.0 * imu->prev_pitch[3] - 201.0/10 * imu->prev_pitch[4] + 1019.0/180 * imu->prev_pitch[5] - 7.0/10 * imu->prev_pitch[6];
//	imu->pitch_acc = (2 * imu->pitch - 5 * imu->prev_pitch[0] + 4 * imu->prev_pitch[1] - imu->prev_pitch[2]) / (deltaT * deltaT);
//	imu->pitch_acc /= 900;
//	imu->pitch_acc = 15.0/4 * imu->pitch - 77.0/6 * imu->prev_pitch[0]
//						   + 107.0/6 * imu->prev_pitch[1] - 13 * imu->prev_pitch[2]
//						  + 61.0/12 * imu->prev_pitch[3] - 5.0/6 * imu->prev_pitch[4];
// 	for (int i = 7 - 1; i >= 1; i--) imu->prev_pitch[i] = imu->prev_pitch[i - 1]; // this is wrong :D
//	imu->prev_pitch[0] = imu->pitch;
//
//	if (imu->state != states[0]) {
//		for (int i = 5; i >= 1; i--)
//			states[i] = states[i - 1];
//		states[0] = imu->state;
//	}

	// (0.7 + 0.3 * expf((imu->upCount - imu->downCount)/50.0))
	switch (imu->state) {
	case IMU_IDLE:
	case IMU_IDLE_NP:
		imu->accelFlag = 0;
		if (imu->acc[2] < -22) {
//			if (imu->state != IMU_IDLE_PN) {
				imu->state = IMU_NEG;
				imu->hit_tick = HAL_GetTick();
//			} else if (imu->state == IMU_IDLE_PN && HAL_GetTick() - imu->hit_tick < 70 && imu->pitch_acc < -12) {
//				imu->hit_tick = HAL_GetTick();
//			} else {
//				imu->state = IMU_IDLE;
//			}
		} else if (HAL_GetTick() - imu->hit_tick > 70) { // effectively a cooldown to prevent a hit being detected in the opposite direciton
			imu->state = IMU_IDLE;
		}
		break;

	case IMU_NEG:
		if (HAL_GetTick() - imu->hit_tick < 150) {
			if (imu->acc[2] > 10) {
				imu->accelFlag = 1;
				imu->state = IMU_HIT_NP;
				imu->hit_tick = HAL_GetTick();
			}
		} else {
			imu->state = IMU_IDLE;
		}
		break;
	case IMU_HIT_NP:
		if (imu->acc[2] > 10) {
			imu->hit_tick = HAL_GetTick();
		} else {
			if (HAL_GetTick() - imu->hit_tick > 40) {
				imu->state = IMU_IDLE_NP;
			}
		}
		break;
	}
//
//	if (imu->pitch_acc < -7) imu->accelFlag = 1;
//	if (imu->pitch_acc > 7) imu->accelFlag = 1;
//	if (imu->acc[2] < -22) imu->accelFlag = 1;
//	else imu->accelFlag = 0;
//	if (imu->acc[2] > 30) imu->accelFlag = 0;
}

void inline updateIMUs() {
	for (int i = 0; i < 2; i++) updateIMU(imuStructs[i]);

}
