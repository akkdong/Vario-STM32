// VarioTest.ino
//

#include <DefaultSettings.h>
#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <GlobalConfig.h>
#include <Variometer.h>
#include <KalmanVario.h>
#include <AccelCalibrator.h>
#include <MPU60X0.h>

#define TEST_MPU60X0		0
#define TEST_KALMANVARIO	1
#define TEST_VARIOMETER 	0

//
HardWire Wire1(1, I2C_FAST_MODE);
HardWire Wire2(2, I2C_FAST_MODE);

// set wire reference member to I2C1
HardWire & I2CDevice::Wire = Wire1;

// set unlock callback function
unlockCallback I2CDevice::cbUnlock = MS5611::unlockI2C;

// EEPROMDriver
EEPROMDriver eeprom(Wire2);

// global configuration
GlobalConfig Config(eeprom, EEPROM_ADDRESS);

//
#if TEST_VARIOMETER
Variometer vario;
#elif TEST_KALMANVARIO
KalmanVario vario;
#endif // TEST_VARIOMETER

//
AccelCalibrator calibrator;


//
//
//

#if TEST_MPU60X0

void setup()
{
	//
	Serial.begin(115200);
	while (! Serial);
	delay(1000);
	
	Serial.println("MPU60X0 Test!!!");
	
	// scan
	Wire1.begin();
	Wire2.begin();
	
	//
	Config.readAll();
	
	//
	mpu60x0_.begin();
	
	mpu60x0_.calibateGyro();
}

void loop()
{
	mpu60x0_.update();
	
	if (mpu60x0_.available())
	{
		#if 0
		float accel[3], gyro[3];
		
		mpu60x0_.read(accel, gyro);
		Serial.print(accel[0]); Serial.print(",");
		Serial.print(accel[1]); Serial.print(",");
		Serial.print(accel[2]); Serial.print(",");
		Serial.print(gyro[0]); Serial.print(",");
		Serial.print(gyro[1]); Serial.print(",");
		Serial.print(gyro[2]); Serial.println("");
		#else
		float va;
	
		mpu60x0_.read(&va);
		Serial.print(va); Serial.println("");
		#endif
	}
}

#endif


//
//
//

#if TEST_KALMANVARIO || TEST_VARIOMETER

// Kalman filter configuration
#define KF_ZMEAS_VARIANCE       400.0f
#define KF_ZACCEL_VARIANCE      1000.0f
#define KF_ACCELBIAS_VARIANCE   1.0f


void setup()
{
	//
	Serial.begin(115200);
	while (! Serial);
	delay(1000);
	
	Serial.println("Vario Test!!!");
	
	// scan
	Wire1.begin();
	Wire2.begin();
	
	//
	Config.readAll();
	
	//
	#if TEST_KALMANVARIO
	vario.begin(KF_ZMEAS_VARIANCE, KF_ZACCEL_VARIANCE, KF_ACCELBIAS_VARIANCE);
	#else
	vario.begin(Config.kalman.sigmaP, Config.kalman.sigmaA);
	#endif
}

void loop()
{
	//
	vario.update();
	
	//
	if (vario.available())
	{
		Serial.println(vario.getVelocity() * 100.0);
		
		vario.flush();
	}
}

#endif
