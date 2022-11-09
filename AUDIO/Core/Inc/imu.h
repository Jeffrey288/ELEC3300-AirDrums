#include "MPU9250_Config.h"
#include "MPU9250.h"

typedef struct {
	int16_t raw[9];
	int16_t gyro_offset[3];
	float acc[3];
	float gyro[3];
	float mag[3];
	GPIO_TypeDef *port;
	int16_t pin;
} imuStruct;

static void imu_setActive(imuStruct* p) {
	MPU9250_CS_GPIO = p->port;
	MPU9250_CS_PIN = p->pin;
}

#define numCalPoints 100
static void imu_calibrateGyro(imuStruct* p) {
    // Init
	imu_setActive(p);
	int32_t sum[3] = {0, 0, 0};

    // Save specified number of points
    for (uint16_t i = 0; i < numCalPoints; i++) {
    	HAL_Delay(3);
    	MPU9250_GetData(p->raw, p->raw + 6, p->raw + 3);
    	for (int j = 0; j < 3; j++) sum[j] += p->raw[j + 3];
    }

    // Average the saved data points to find the gyroscope offset
    for (int j = 0; j < 3; j++) p->gyro_offset[j] = (float) sum[j] / numCalPoints;
}
