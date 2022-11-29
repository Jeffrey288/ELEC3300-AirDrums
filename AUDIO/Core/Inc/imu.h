#include "MPU9250_Config.h"
#include "MPU9250.h"
#include "madgwickFilter.h"
#include "kalman.h"

#define IMU_REFRESH_RATE 2000

typedef enum {
	IMU_IDLE,
	IMU_NEG,
	IMU_POS,
	IMU_HIT_NP,
	IMU_HIT_PN,
	IMU_IDLE_NP,
	IMU_IDLE_PN,
} imuState;

extern imuState states[6];

typedef struct {
// pin of imu
	GPIO_TypeDef *port;
	int16_t pin;

// raw gyroscope data
	int16_t raw[9];
	float gyro_offset[3];
	float acc[3];
	float gyro[3];
	float mag[3];
	float prev_gyro[7];
	float prev_ticks[7];

// calculation-related stuff
	struct quaternion q;
	uint32_t last_tick;

// calculated angles and data processing
	union {
		float angles[3]; // roll, pitch, yaw
		struct {
			float roll;
			float pitch;
			float yaw;
		};
	};
	float prev_pitch[7];
	float pitch_acc;
	int pitch_multiplier;

// some flags for buttons
	int accelFlag;
	uint32_t hit_tick;
	imuState state;
	uint32_t upCount;
	uint32_t downCount;

	Kalman gyro_filters[3];
	Kalman acc_filters[3];
} imuStruct;

extern imuStruct *imuStructs[2];
extern imuStruct imuLeft;
extern imuStruct imuRight;

static void inline imu_setActive(imuStruct* p) {
	MPU9250_CS_GPIO = p->port;
	MPU9250_CS_PIN = p->pin;
}

#define numCalPoints 10000
static void imu_calibrateGyro(imuStruct* p) {
    // Init
	imu_setActive(p);
	float sum[3] = {0, 0, 0};

    // Save specified number of points
    for (uint16_t i = 0; i < numCalPoints; i++) {
    	HAL_Delay(3);
    	MPU9250_GetData(p->raw, p->raw + 6, p->raw + 3);
    	for (int j = 0; j < 3; j++) sum[j] += p->raw[j + 3];
    }

    // Average the saved data points to find the gyroscope offset
    for (int j = 0; j < 3; j++) p->gyro_offset[j] = (float) sum[j] / numCalPoints;
}

extern void inline updateIMUs();
extern void inline updateIMUs_T();
extern void inline updateIMU(imuStruct*);
extern void inline updateIMU_T(imuStruct*);
void initIMUStructs();
void initIMUStruct(imuStruct*);
