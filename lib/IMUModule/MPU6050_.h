// MPU6050_.h
//

#ifndef __MPU6050__H__
#define __MPU6050__H__

#include <Arduino.h>
#include <I2CDevice.h>


/* accelerometer parameters */
#define MPU6050_G_TO_MS 		(9.80665)

#define MPU6050_GYRO_FSR 		(2000)
#define MPU6050_ACCEL_FSR 		(4)
#define MPU6050_FIFO_RATE 		(100)

/*  2G ~= 16384 2^14 */
/*  4G ~=  8192 2^13 */
/*  8G ~=  4096 2^12 */
/* 16G ~=  2048 2^11 */
#define MPU6050_ACCEL_SCALE 	(8192.0)
/*  250 degree/s ~= 131 */
/*  500 degree/s ~= 65.5 */
/* 1000 degree/s ~= 32.8 */
/* 2000 degree/s ~= 16.4 */
#define MPU6050_GYRO_SCALE		(16.4)


/* 2^30 */
#define MPU6050_QUAT_SCALE 		(1073741824.0)


///////////////////////////////////////////////////////////////////////////////////////////////
// class MPU6050_

class MPU6050_
{
public:
	MPU6050_();
	
public:
	void				begin(int calibrateGyro = true);
	void				beginCalibration();
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
	
private:
	void				init(int calibrateGyro);
	
private:
	//
//	float				vertAccel;
	float				accumulateAccel;
	int					accumulateCount;

	int					accelUpdated;
	
//	float				calData[3]; // accel calibration data
	
	// raw data
	float				gyroData[3];
	float				accelData[3];
	float				quadData[4];
};

// inline members
inline float * MPU6050_::getRawAccel()
	{ return accelData; }
	
inline float * MPU6050_::getRawGyro()
	{ return gyroData; }
	
//
extern MPU6050_ mpu6050_;


#endif // __MPU6050__H__
