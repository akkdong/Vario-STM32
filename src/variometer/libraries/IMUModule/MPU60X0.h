// MPU60X0.h
//

#ifndef __MPU60X0_H__
#define __MPU60X0_H__

#include <Arduino.h>
#include <I2CDevice.h>


/* accelerometer parameters */
#define MPU60X0_G_TO_MS 		(9.80665)

#define MPU60X0_GYRO_FSR 		(2000)
#define MPU60X0_ACCEL_FSR 		(4)
#define MPU60X0_FIFO_RATE 		(100)

/*  2G ~= 16384 2^14 */
/*  4G ~=  8192 2^13 */
/*  8G ~=  4096 2^12 */
/* 16G ~=  2048 2^11 */
#define MPU60X0_ACCEL_SCALE 	(8192.0)
/*  250 degree/s ~= 131 */
/*  500 degree/s ~= 65.5 */
/* 1000 degree/s ~= 32.8 */
/* 2000 degree/s ~= 16.4 */
#define MPU60X0_GYRO_SCALE		(16.4)


/* 2^30 */
#define MPU60X0_QUAT_SCALE 		(1073741824.0)


///////////////////////////////////////////////////////////////////////////////////////////////
// class MPU60X0

class MPU60X0
{
public:
	MPU60X0();
	
public:
	void				begin();
	void				end();
	
	int					available();
	void				flush();
	
	void				update(int calibrateAccel = true);
	
	int					read(float * va);
	int					read(float * accel, float * gyro);
	
	float *				getRawAccel();
	float *				getRawGyro();
	
	void				setCalibration(float * cal);
	float *				getCalibration();
	
	void				calibateGyro();
	
private:
	//
	float				accumulateAccel;
	int					accumulateCount;
	
	//
	int					accelUpdated;
	
//	float				calData[3]; // accel calibration data
	
	// raw data
	float				gyroData[3];
	float				accelData[3];
	
	float				gyro_calData[3];
	float				accel_calData[3];
};

// inline members
inline float * MPU60X0::getRawAccel()
	{ return accelData; }
	
inline float * MPU60X0::getRawGyro()
	{ return gyroData; }
	
//
extern MPU60X0 mpu60x0_;


#endif // __MPU60X0_H__
