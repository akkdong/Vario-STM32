//=====================================================================================================
// MahonyAHRS.h
//=====================================================================================================
//
// Madgwick's implementation of Mayhony's AHRS algorithm.
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
//
// Date			Author			Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
//
//=====================================================================================================
#ifndef MahonyAHRS_h
#define MahonyAHRS_h

#define _180DIVPI 57.2957795f

//extern volatile float twoKp;			// 2 * proportional gain (Kp)
//extern volatile float twoKi;			// 2 * integral gain (Ki)
extern volatile float q0, q1, q2, q3;	// quaternion of sensor frame relative to auxiliary frame


void imu_MahonyAHRSupdate(float dt, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void imu_MahonyAHRSupdateIMU(float dt, float gx, float gy, float gz, float ax, float ay, float az);
void imu_Quaternion2YawPitchRoll(float q0, float q1, float q2, float q3, float* pYawDeg, float* pPitchDeg, float* pRollDeg);
float imu_GravityCompensatedAccel(float ax, float ay, float az, float q0, float q1, float q2, float q3);
void imu_MadgwickAHRSupdate6DOF(int bUseAccel, float dt, float gx, float gy, float gz, float ax, float ay, float az);
#endif
