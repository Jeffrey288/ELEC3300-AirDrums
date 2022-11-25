//
//  madgwickFilter.c
//  madgwickFilter
//
//  Created by Blake Johnson on 4/28/20.
//  Stolen from https://github.com/bjohnsonfl/Madgwick_Filter
//

#include "stm32f1xx_hal.h"
#include "madgwickFilter.h"

struct quaternion quat_mult (struct quaternion L, struct quaternion R){

    struct quaternion product;
    product.q1 = (L.q1 * R.q1) - (L.q2 * R.q2) - (L.q3 * R.q3) - (L.q4 * R.q4);
    product.q2 = (L.q1 * R.q2) + (L.q2 * R.q1) + (L.q3 * R.q4) - (L.q4 * R.q3);
    product.q3 = (L.q1 * R.q3) - (L.q2 * R.q4) + (L.q3 * R.q1) + (L.q4 * R.q2);
    product.q4 = (L.q1 * R.q4) + (L.q2 * R.q3) - (L.q3 * R.q2) + (L.q4 * R.q1);

    return product;
}


// The resulting quaternion is a global variable (q_est), so it is not returned or passed by reference/pointer
// Gyroscope Angular Velocity components are in Radians per Second
// Accelerometer componets will be normalized

void imu_filter(struct quaternion *q, float ax, float ay, float az, float gx, float gy, float gz, float deltaT){

	if (gx * gx + gy * gy + gz * gz < 3 * 0.05 * 0.05 || (ax == 0.0f && ay == 0.0f && az == 0.0f)) return;

//	float q0 = q->q1, q1 = q->q2, q2 = q->q3, q3 = q->q4;
//	float recipNorm;
//	float halfvx, halfvy, halfvz;
//	float halfex, halfey, halfez;
//	float qa, qb, qc;
//
//	volatile float twoKp = (2.0f * 0.5f);											// 2 * proportional gain (Kp)
//	volatile float twoKi = (2.0f * 0.0f);											// 2 * integral gain (Ki)			// quaternion of sensor frame relative to auxiliary frame
//	static volatile float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;
//	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
//
//	// Normalise accelerometer measurement
//	recipNorm = invSqrt(ax * ax + ay * ay + az * az);
//	ax *= recipNorm;
//	ay *= recipNorm;
//	az *= recipNorm;
//
//	// Estimated direction of gravity and vector perpendicular to magnetic flux
//	halfvx = q1 * q3 - q0 * q2;
//	halfvy = q0 * q1 + q2 * q3;
//	halfvz = q0 * q0 - 0.5f + q3 * q3;
//
//	// Error is sum of cross product between estimated and measured direction of gravity
//	halfex = (ay * halfvz - az * halfvy);
//	halfey = (az * halfvx - ax * halfvz);
//	halfez = (ax * halfvy - ay * halfvx);
//
//	// Compute and apply integral feedback if enabled
//	if(twoKi > 0.0f) {
//		integralFBx += twoKi * halfex * (deltaT);	// integral error scaled by Ki
//		integralFBy += twoKi * halfey * (deltaT);
//		integralFBz += twoKi * halfez * (deltaT);
//		gx += integralFBx;	// apply integral feedback
//		gy += integralFBy;
//		gz += integralFBz;
//	}
//	else {
//		integralFBx = 0.0f;	// prevent integral windup
//		integralFBy = 0.0f;
//		integralFBz = 0.0f;
//	}
//
//	// Apply proportional feedback
//	gx += twoKp * halfex;
//	gy += twoKp * halfey;
//	gz += twoKp * halfez;
//
//
//	// Integrate rate of change of quaternion
//	gx *= (0.5f * (deltaT));		// pre-multiply common factors
//	gy *= (0.5f * (deltaT));
//	gz *= (0.5f * (deltaT));
//	qa = q0;
//	qb = q1;
//	qc = q2;
//	q0 += (-qb * gx - qc * gy - q3 * gz);
//	q1 += (qa * gx + qc * gz - q3 * gy);
//	q2 += (qa * gy - qb * gz + q3 * gx);
//	q3 += (qa * gz + qb * gy - qc * gx);
//
//	// Normalise quaternion
//	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
//	q0 *= recipNorm;
//	q1 *= recipNorm;
//	q2 *= recipNorm;
//	q3 *= recipNorm;
//
//	q->q1 = q0; q->q2 = q1; q->q3 = q2; q->q4 = q3;

//	float q0 = q->q1, q1 = q->q2, q2 = q->q3, q3 = q->q4;
//
//	float recipNorm;
//	float s0, s1, s2, s3;
//	float qDot1, qDot2, qDot3, qDot4;
//	float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;
//
//	// Rate of change of quaternion from gyroscope
//	qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
//	qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
//	qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
//	qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);
//
//	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
//	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
//
//	// Normalise accelerometer measurement
//	recipNorm = 1.0 / (float) sqrt(ax * ax + ay * ay + az * az);
//	ax *= recipNorm;
//	ay *= recipNorm;
//	az *= recipNorm;
//
//	// Auxiliary variables to avoid repeated arithmetic
//	_2q0 = 2.0f * q0;
//	_2q1 = 2.0f * q1;
//	_2q2 = 2.0f * q2;
//	_2q3 = 2.0f * q3;
//	_4q0 = 4.0f * q0;
//	_4q1 = 4.0f * q1;
//	_4q2 = 4.0f * q2;
//	_8q1 = 8.0f * q1;
//	_8q2 = 8.0f * q2;
//	q0q0 = q0 * q0;
//	q1q1 = q1 * q1;
//	q2q2 = q2 * q2;
//	q3q3 = q3 * q3;
//
//	// Gradient decent algorithm corrective step
//	s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
//	s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
//	s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
//	s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;
//	recipNorm = 1.0 / (float) sqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
//	s0 *= recipNorm;
//	s1 *= recipNorm;
//	s2 *= recipNorm;
//	s3 *= recipNorm;
//
//	// Apply feedback step
//	qDot1 -= BETA * s0;
//	qDot2 -= BETA * s1;
//	qDot3 -= BETA * s2;
//	qDot4 -= BETA * s3;
//	}
//
//	// Integrate rate of change of quaternion to yield quaternion
//	q0 += qDot1 * deltaT;
//	q1 += qDot2 * deltaT;
//	q2 += qDot3 * deltaT;
//	q3 += qDot4 * deltaT;
//
//	// Normalise quaternion
//	recipNorm = 1.0 / (float) sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
//	q0 *= recipNorm;
//	q1 *= recipNorm;
//	q2 *= recipNorm;
//	q3 *= recipNorm;
//
//	q->q1 = q0; q->q2 = q1; q->q3 = q2; q->q4 = q3;


	struct quaternion q_est = *q;

    //Variables and constants
    struct quaternion q_est_prev = q_est;
    struct quaternion q_est_dot = {0};            // used as a place holder in equations 42 and 43
    //const struct quaternion q_g_ref = {0, 0, 0, 1};// equation (23), reference to field of gravity for gradient descent optimization (not needed because I used eq 25 instead of eq 21
    struct quaternion q_a = {0, ax, ay, az};    // equation (24) raw acceleration values, needs to be normalized

    float F_g [3] = {0};                        // equation(15/21/25) objective function for gravity
    float J_g [3][4] = {0};                     // jacobian matrix for gravity

    struct quaternion gradient = {0};

    /* Integrate angluar velocity to obtain position in angles. */
    struct quaternion q_w;                   // equation (10), places gyroscope readings in a quaternion
    q_w.q1 = 0;                              // the real component is zero, which the Madgwick uses to simplfy quat. mult.
    q_w.q2 = gx;
    q_w.q3 = gy;
    q_w.q4 = gz;

    quat_scalar(&q_w, 0.5);                  // equation (12) dq/dt = (1/2)q*w
    q_w = quat_mult(q_est_prev, q_w);        // equation (12)

    /* NOTE
    * Page 10 states equation (40) substitutes equation (13) into it. This seems false, as he actually
    * substitutes equation (12), q_se_dot_w, not equation (13), q_se_w.
    *
    * // quat_scalar(&q_w, deltaT);               // equation (13) integrates the angles velocity to position
    * // quat_add(&q_w, q_w, q_est_prev);         // addition part of equation (13)
    */

    /* Compute the gradient by multiplying the jacobian matrix by the objective function. This is equation 20.
     The Jacobian matrix, J, is a 3x4 matrix of partial derivatives for each quaternion component in the x y z axes
     The objective function, F, is a 3x1 matrix for x y and z.
     To multiply these together, the inner dimensions must match, so use J'.
     I calculated "by hand" the transpose of J, so I will be using "hard coordinates" to get those values from J.
     The matrix multiplcation can also be done hard coded to reduce code.

     Note: it is possible to compute the objective function with quaternion multiplcation functions, but it does not take into account the many zeros that cancel terms out and is not optimized like the paper shows
     */

    quat_Normalization(&q_a);              // normalize the acceleration quaternion to be a unit quaternion
    //Compute the objective function for gravity, equation(15), simplified to equation (25) due to the 0's in the acceleration reference quaternion
    F_g[0] = 2*(q_est_prev.q2 * q_est_prev.q4 - q_est_prev.q1 * q_est_prev.q3) - q_a.q2;
    F_g[1] = 2*(q_est_prev.q1 * q_est_prev.q2 + q_est_prev.q3* q_est_prev.q4) - q_a.q3;
    F_g[2] = 2*(0.5 - q_est_prev.q2 * q_est_prev.q2 - q_est_prev.q3 * q_est_prev.q3) - q_a.q4;

    //Compute the Jacobian matrix, equation (26), for gravity
    J_g[0][0] = -2 * q_est_prev.q3;
    J_g[0][1] =  2 * q_est_prev.q4;
    J_g[0][2] = -2 * q_est_prev.q1;
    J_g[0][3] =  2 * q_est_prev.q2;

    J_g[1][0] = 2 * q_est_prev.q2;
    J_g[1][1] = 2 * q_est_prev.q1;
    J_g[1][2] = 2 * q_est_prev.q4;
    J_g[1][3] = 2 * q_est_prev.q3;

    J_g[2][0] = 0;
    J_g[2][1] = -4 * q_est_prev.q2;
    J_g[2][2] = -4 * q_est_prev.q3;
    J_g[2][3] = 0;

    // now computer the gradient, equation (20), gradient = J_g'*F_g
    gradient.q1 = J_g[0][0] * F_g[0] + J_g[1][0] * F_g[1] + J_g[2][0] * F_g[2];
    gradient.q2 = J_g[0][1] * F_g[0] + J_g[1][1] * F_g[1] + J_g[2][1] * F_g[2];
    gradient.q3 = J_g[0][2] * F_g[0] + J_g[1][2] * F_g[1] + J_g[2][2] * F_g[2];
    gradient.q4 = J_g[0][3] * F_g[0] + J_g[1][3] * F_g[1] + J_g[2][3] * F_g[2];

    // Normalize the gradient, equation (44)
    quat_Normalization(&gradient);

    /* This is the sensor fusion part of the algorithm.
     Combining Gyroscope position angles calculated in the beginning, with the quaternion orienting the accelerometer to gravity created above.
     Noticably this new quaternion has not be created yet, I have only calculated the gradient in equation (19).
     Madgwick however uses assumptions with the step size and filter gains to optimize the gradient descent,
        combining it with the sensor fusion in equations (42-44).
     He says the step size has a var alpha, which he assumes to be very large.
     This dominates the previous estimation in equation (19) to the point you can ignore it.
     Eq. 36 has the filter gain Gamma, which is related to the step size and thus alpha. With alpha being very large,
        you can make assumptions to simplify the fusion equatoin of eq.36.
     Combining the simplification of the gradient descent equation with the simplification of the fusion equation gets you eq.
     41 which can be subdivided into eqs 42-44.
    */
    quat_scalar(&gradient, BETA);             // multiply normalized gradient by beta
    quat_sub(&q_est_dot, q_w, gradient);        // subtract above from q_w, the integrated gyro quaternion
    quat_scalar(&q_est_dot, deltaT);
    quat_add(&q_est, q_est_prev, q_est_dot);     // Integrate orientation rate to find position
    quat_Normalization(&q_est);                 // normalize the orientation of the estimate
                                                //(shown in diagram, plus always use unit quaternions for orientation)
    *q = q_est;
}

/*
 returns as pointers, roll pitch and yaw from the quaternion generated in imu_filter
 Assume right hand system
 Roll is about the x axis, represented as phi
 Pitch is about the y axis, represented as theta
 Yaw is about the z axis, represented as psi (trident looking greek symbol)
 */
/**
 * We specifically define, in our program,
 * - roll as the angle of self-rotation of the drumstick
 * - pitch as the up and down movement of the drumstick, and
 * - yaw as the pointing direction of the drumstick
 */
void eulerAngles(struct quaternion *q, float *roll, float *pitch, float *yaw) {
// Code for this part is stolen from
// https://web.archive.org/web/20180419114257/http://bediyap.com/programming/convert-quaternion-to-euler-rotations/
	// threeaxisrot\(([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),res\);
	// *roll = atan2f(\1,\2);\n *pitch = asinf (\3);\n *yaw = atan2f(\4,\5);\n

////	 this is xyz, we don't want this (this is from the original code)
//    *roll  = atan2f((2*q->q3*q->q4 - 2*q->q1*q->q2), (2*q->q1*q->q1 + 2*q->q4*q->q4 -1));
//    *pitch = -asinf(2*q->q2*q->q4 + 2*q->q1*q->q3);                                  // equatino (8)
//	*yaw = atan2f((2*q->q2*q->q3 - 2*q->q1*q->q4), (2*q->q1*q->q1 + 2*q->q2*q->q2 -1));  // equation (7)

	// this is xyz, we don't want this either
//	 *roll = atan2f(-2*(q->q2*q->q3 - q->q1*q->q4),
//	                    q->q1*q->q1 + q->q2*q->q2 - q->q3*q->q3 - q->q4*q->q4);
//	 *pitch = -asinf(2*(q->q2*q->q4 + q->q1*q->q3));
//	 *yaw = atan2f(-2*(q->q3*q->q4 - q->q1*q->q2),
//			 q->q1*q->q1 - q->q2*q->q2 - q->q3*q->q3 + q->q4*q->q4);

////	 this is zyx, not what we want either
//	 *yaw = atan2f(2*(q->q2*q->q3 + q->q1*q->q4), q->q1*q->q1 + q->q2*q->q2 - q->q3*q->q3 - q->q4*q->q4);
//	 *pitch = asinf(-2*(q->q2*q->q4 - q->q1*q->q3));
//	 *roll = atan2f(2*(q->q3*q->q4 + q->q1*q->q2), q->q1*q->q1 - q->q2*q->q2 - q->q3*q->q3 + q->q4*q->q4);

//	 this is zxy, part of which is what we want
//	*yaw = atan2f(-2 * (q->q2 * q->q3 - q->q1 * q->q4),
//			q->q1 * q->q1 - q->q2 * q->q2 + q->q3 * q->q3 - q->q4 * q->q4);
//	*pitch = asinf(2 * (q->q3 * q->q4 + q->q1 * q->q2));
//	*roll = atan2f(-2 * (q->q2 * q->q4 - q->q1 * q->q3),
//			q->q1 * q->q1 - q->q2 * q->q2 - q->q3 * q->q3 + q->q4 * q->q4);

	// zyz nope
//	  *roll = atan2f( 2*(q->q3*q->q4 - q->q1*q->q2),      2*(q->q2*q->q4 + q->q1*q->q3));
//	 *pitch = acosf (      q->q1*q->q1 - q->q2*q->q2 - q->q3*q->q3 + q->q4*q->q4);
//	 *yaw = atan2f(      2*(q->q3*q->q4 + q->q1*q->q2),-2*(q->q2*q->q4 - q->q1*q->q3));
//
	// zxz nupe
//	  *roll = atan2f( 2*(q->q2*q->q4 + q->q1*q->q3),-2*(q->q3*q->q4 - q->q1*q->q2));
//	 *pitch = acosf (q->q1*q->q1 - q->q2*q->q2 - q->q3*q->q3 + q->q4*q->q4);
//	 *yaw = atan2f(2*(q->q2*q->q4 - q->q1*q->q3),2*(q->q3*q->q4 + q->q1*q->q2));
//
////	// yxy nupe
//	  *roll = atan2f( 2*(q->q2*q->q3 - q->q1*q->q4),2*(q->q3*q->q4 + q->q1*q->q2));
//	 *pitch = acosf (q->q1*q->q1 - q->q2*q->q2 + q->q3*q->q3 - q->q4*q->q4);
//	 *yaw = atan2f(2*(q->q2*q->q3 + q->q1*q->q4),-2*(q->q3*q->q4 - q->q1*q->q2));
//
////	// yzx
//	  *roll = atan2f( -2*(q->q2*q->q4 - q->q1*q->q3),q->q1*q->q1 + q->q2*q->q2 - q->q3*q->q3 - q->q4*q->q4); // this works well as roll
//	 *pitch = asinf (2*(q->q2*q->q3 + q->q1*q->q4));
//	 *yaw = atan2f(-2*(q->q3*q->q4 - q->q1*q->q2),q->q1*q->q1 - q->q2*q->q2 + q->q3*q->q3 - q->q4*q->q4);
//
//	//yzy
//	  *roll = atan2f( 2*(q->q3*q->q4 + q->q1*q->q2),-2*(q->q2*q->q3 - q->q1*q->q4));
//	 *pitch = acosf (q->q1*q->q1 - q->q2*q->q2 + q->q3*q->q3 - q->q4*q->q4);
//	 *yaw = atan2f(2*(q->q3*q->q4 - q->q1*q->q2),2*(q->q2*q->q3 + q->q1*q->q4)); // this kinda works as roll, except inverted by 90 degs
//
//	// xyx nope
//	  *roll = atan2f( 2*(q->q2*q->q3 + q->q1*q->q4),-2*(q->q2*q->q4 - q->q1*q->q3));
//	 *pitch = acosf (q->q1*q->q1 + q->q2*q->q2 - q->q3*q->q3 - q->q4*q->q4);
//	 *yaw = atan2f(2*(q->q2*q->q3 - q->q1*q->q4),2*(q->q2*q->q4 + q->q1*q->q3));
//
//	//xzy
//	  *roll = atan2f( 2*(q->q3*q->q4 + q->q1*q->q2),q->q1*q->q1 - q->q2*q->q2 + q->q3*q->q3 - q->q4*q->q4); // this works as a good pitch
//	  // but its positives and negatives are affected by init position, and possibly instability at certain angles
//	 *pitch = asinf (-2*(q->q2*q->q3 - q->q1*q->q4));
//	 *yaw = atan2f(2*(q->q2*q->q4 + q->q1*q->q3),q->q1*q->q1 + q->q2*q->q2 - q->q3*q->q3 - q->q4*q->q4);

//	// xzx
//	  *roll = atan2f( 2*(q->q2*q->q4 - q->q1*q->q3),2*(q->q2*q->q3 + q->q1*q->q4));
//	 *pitch = acosf (q->q1*q->q1 + q->q2*q->q2 - q->q3*q->q3 - q->q4*q->q4);
//	 *yaw = atan2f(2*(q->q2*q->q4 + q->q1*q->q3),-2*(q->q2*q->q3 - q->q1*q->q4));


//	// combining these systems, we obtain
//	*roll = atan2f((2*q->q3*q->q4 - 2*q->q1*q->q2), (2*q->q1*q->q1 + 2*q->q4*q->q4 -1)); // from xyz/zyx
//	*pitch = asinf(2 * (q->q3 * q->q4 + q->q1 * q->q2)); // from zxy
//	*yaw = atan2f(-2 * (q->q2 * q->q3 - q->q1 * q->q4), q->q1 * q->q1 - q->q2 * q->q2 + q->q3 * q->q3 - q->q4 * q->q4); // from zxy

	// hopefully better
	*roll = atan2f(-2 * (q->q2 * q->q4 - q->q1 * q->q3),
			q->q1 * q->q1 - q->q2 * q->q2 - q->q3 * q->q3 + q->q4 * q->q4);
	*pitch = atan2f( 2*(q->q3*q->q4 + q->q1*q->q2),q->q1*q->q1 - q->q2*q->q2 + q->q3*q->q3 - q->q4*q->q4); // xzy
	*yaw = atan2f(-2 * (q->q2 * q->q3 - q->q1 * q->q4), q->q1 * q->q1 - q->q2 * q->q2 + q->q3 * q->q3 - q->q4 * q->q4);


	*yaw *= (180.0f / PI);
	*pitch *= (180.0f / PI);
	*roll *= (180.0f / PI);

}

