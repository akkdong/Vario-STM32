// Variometer.ino
//

#include <Arduino.h>
#include <VarioSettings.h>
#include <VertVelocity.h>

#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>

//#include <IMU.h>


VertVelocity  vertVel;
//IMU				imu;

void setup()
{
	//
	mpu_init(NULL);
	
	//
	// imu.init();
	
	// imu.update()
	// imu.get();
	// ...
}

void loop()
{
}
