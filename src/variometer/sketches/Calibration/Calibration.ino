// Variometer.ino
//

#include <DefaultSettings.h>
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <IMUModule.h>
#include <GlobalConfig.h>
#include <AccelCalibrator.h>


#define count_of(X)		(sizeof(X) / sizeof(X[0]))


const char usage01[] 						= "-------------------------";
const char usage02[] 						= "ACCELEROMETER CALIBRATION";
const char usage03[] 						= "-------------------------";
const char usage04[] 						= "";
const char usage05[] 						= "Commands :";
const char usage06[] 						= "  d : display current accel vector";
const char usage07[] 						= "  m : measure accel vertor for calibration";
const char usage08[] 						= "  c : start calibration";
const char usage09[] 						= "  r : restart calibration to the beginning";
const char usage10[] 						= "";
const char usage11[] 						= "Procedure :";
const char usage12[] 						= "  The accelerometer calibration procedure";
const char usage13[] 						= "  need five accel vectors from the five upper ";
const char usage14[] 						= "  orientations. That is to say with the";
const char usage15[] 						= "  accelerometer pointing :";
const char usage16[] 						= "    -> toward the sky, flat on the groud";
const char usage17[] 						= "    -> toward the left on it's left side";
const char usage18[] 						= "    -> toward the right on it's right side";
const char usage19[] 						= "    -> toward you on it's bottom side";
const char usage20[] 						= "    -> back to you, on it's top side";
const char usage21[] 						= "";
const char usage22[] 						= "  For each measure :";
const char usage23[] 						= "  1) You can check the current accel vector";
const char usage24[] 						= "     with multiple 'd' command.";
const char usage25[] 						= "  2) Record the accel vector with the 'm' command";
const char usage26[] 						= "     This can be done multiple times for each orientations";
const char usage27[] 						= "";
const char usage28[] 						= "  Once the five orientations recorded. You can";
const char usage29[] 						= "  launch the calibration procedure with the 'c'";
const char usage30[] 						= "  command. Use the 'd' command  to check the result.";
const char usage31[] 						= "  The distance must be as close as possible to 1.000.";
const char usage32[] 						= "  If you are not satisfied by the result. Make more";
const char usage33[] 						= "  measures or reset teh calibration with the 'r' command.";
const char usage34[] 						= "-------------------------";
const char usage35[] 						= "";

const char* const usage[]		 			= { usage01, usage02, usage03, usage04, usage05,
												usage06, usage07, usage08, usage09, usage10,
												usage11, usage12, usage13, usage14, usage15,
												usage16, usage17, usage18, usage19, usage20,
												usage21, usage22, usage23, usage24, usage25,
												usage26, usage27, usage28, usage29, usage30,
												usage31, usage32, usage33, usage34, usage35
											   };

const char separatorMsg[] 					= "-------------------------";
const char measureReadyMsg[] 				= "Ready to make measure.";
const char measureAndCalibrateReadyMsg[] 	= "Ready to make measure or calibration.";
const char calibrationNotReadyMsg[] 		= "Not enough measures to calibrate !";

const char measureValidMsg[] 				= "GOOD measure, recorded for calibration";
const char measureInvalidMsg01[] 			= "BAD measure, not recorded, possible problems are :";
const char measureInvalidMsg02[] 			= "-> ambiguous orientation";
const char measureInvalidMsg03[] 			= "-> orientation already done with lower standard deviation";

const char* const measureInvalidMsg[] 		= {measureInvalidMsg01, measureInvalidMsg02, measureInvalidMsg03};

const char resetMsg[] 						= "Reset calibration !";
const char waitMsg[] 						= "Don't move the accelerometer and wait...";
const char recordMsg[] 						= "Starting measure...";

const char calMsgB01[] 						= "-------------------------";
const char calMsgB02[] 						= "CALIBRATION DONE ";
const char calMsgB03[] 						= "-------------------------";
const char calMsgB04[] 						= "";
const char calMsgB05[] 						= "Here the new calibration coefficients :";
const char calMsgB06[] 						= "";

const char* const calMsgB[] 				= {calMsgB01, calMsgB02, calMsgB03, calMsgB04, calMsgB05, calMsgB06};

const char calMsgE01[] 						= "";
const char calMsgE02[] 						= "Run multiple 'd' command to check the result.";
const char calMsgE03[] 						= "-------------------------";

const char* const calMsgE[] 				= {calMsgE01, calMsgE02, calMsgE03};


//
// InertialMeasurementUnit Module : measure accelerometer & gyro & do calibration for accelerometer
//    It internally uses I2CDevice 
//

IMUModule imu;

AccelCalibrator accelCalibrator(imu);

//
// declare I2C instance
//
// I2C1 is used by IMU : I2CDevice is proxy interface of I2C1
// I2C2 is used by EEPROM
//

HardWire Wire1(1, I2C_FAST_MODE);
HardWire Wire2(2, I2C_FAST_MODE);

// initialize some static member of I2CDevice(I2C1)
//

// set wire reference member to I2C1
HardWire & I2CDevice::Wire = Wire1;

// set unlock callback function
unlockCallback I2CDevice::cbUnlock = SensorMS5611::UnlockI2C;

// declare EEPROMDriver
EEPROMDriver eeprom(Wire2);

// global configuration
GlobalConfig Config(eeprom, EEPROM_ADDRESS);



/*********************/               
/* display functions */
/*********************/
char stringBuffer[65];

void displayText( const char* const* text, int textLinesCount)
{
	for(int i = 0; i<textLinesCount; i++)
	{
		//strcpy_P(stringBuffer, (char*)pgm_read_word(&(text[i])));
		//Serial.println(stringBuffer);
		Serial.println(text[i]);
	}
}

void displayString(const char* string)
{
	//strcpy_P(stringBuffer, string);
	//Serial.println(stringBuffer);
	Serial.println(string);
}


/*-----------------*/
/*      SETUP      */
/*-----------------*/
void setup()
{
	//
	delay(400);
	
	/***************/
	/* init serial */
	/***************/
	Serial.begin(115200);
	
	//
	Wire1.begin();
	Wire2.begin();
	
	//
	Config.readAll();
	
	/*************************************/
	/* init accelerometer and calibrator */
	/*************************************/
	accelCalibrator.init();

	/*********************/
	/* display procedure */
	/*********************/
	displayText(usage, count_of(usage));

	/* display current calibration coeffs */
	Serial.print("Current calibration coefficients : \nx : ");
	Serial.print(accelCalibrator.calibration[0], 5);
	Serial.print("\ny : ");
	Serial.print(accelCalibrator.calibration[1], 5);
	Serial.print("\nz : ");
	Serial.print(accelCalibrator.calibration[2], 5);
	Serial.print("\n\n");

	displayString(measureReadyMsg);
}

/*----------------*/
/*      LOOP      */
/*----------------*/
void loop()
{
	/******************/
	/* check commands */
	/******************/
	while( Serial.available() )
	{
		uint8_t c;
		//while( Serial.available() )
		//{
			c = Serial.read();
		//}
		
		if( c == 'r' )
		{
			/*********/
			/* reset */
			/*********/
			accelCalibrator.reset();
			displayString(resetMsg);
		}
		else if( c == 'd' )
		{
			/*******************/
			/* display measure */
			/*******************/

			/* make measure */
			displayString(waitMsg);
			displayString(recordMsg);
			
			if( ! accelCalibrator.calibrated )
			{
				accelCalibrator.measure();
			}
			else
			{
				accelCalibrator.calibratedMeasure();
			}

			/* display the measure */
			Serial.print("accel=(");
			Serial.print(accelCalibrator.measuredAccel[0], 5);
			Serial.print(", ");
			Serial.print(accelCalibrator.measuredAccel[1], 5);
			Serial.print(", ");
			Serial.print(accelCalibrator.measuredAccel[2], 5);
			Serial.print(")\n");
			Serial.print("Standard deviation = ");
			Serial.print(accelCalibrator.measuredAccelSD, 5);
			Serial.print("\n");

			/* if calibrated display the distance */
			if( accelCalibrator.calibrated )
			{
				double dist = sqrt(accelCalibrator.measuredAccel[0]*accelCalibrator.measuredAccel[0] + accelCalibrator.measuredAccel[1]*accelCalibrator.measuredAccel[1] + accelCalibrator.measuredAccel[2]*accelCalibrator.measuredAccel[2]);
				
				Serial.print("calibrated distance : ");
				Serial.print(dist, 5);
				Serial.print("\n");
			}
		}
		else if( c == 'm' )
		{
			/******************/
			/* record measure */
			/******************/

			/* make measure */
			displayString(waitMsg);
			displayString(recordMsg);
			accelCalibrator.measure();

			/* display the measure */
			Serial.print("accel=(");
			Serial.print(accelCalibrator.measuredAccel[0], 5);
			Serial.print(", ");
			Serial.print(accelCalibrator.measuredAccel[1], 5);
			Serial.print(", ");
			Serial.print(accelCalibrator.measuredAccel[2], 5);
			Serial.print(")\n");
			Serial.print("Standard deviation = ");
			Serial.print(accelCalibrator.measuredAccelSD, 5);
			Serial.print("\n");

			/* record the result */
			boolean measureValid = accelCalibrator.pushMeasure();
			if( measureValid )
			{
				displayString(measureValidMsg);
			}
			else
			{
				displayText(measureInvalidMsg, count_of(measureInvalidMsg));
			}
		}
		else if( c == 'c' )
		{
			/*************/
			/* calibrate */
			/*************/
			if( !accelCalibrator.canCalibrate() )
			{
				displayString(calibrationNotReadyMsg);
			}
			else
			{
				/* calibrate */
				accelCalibrator.calibrate();

				/* display result */
				displayText(calMsgB, count_of(calMsgB));
				Serial.print("x : ");
				Serial.print(accelCalibrator.calibration[0], 5);
				Serial.print("\ny : ");
				Serial.print(accelCalibrator.calibration[1], 5);
				Serial.print("\nz : ");
				Serial.print(accelCalibrator.calibration[2], 5);
				Serial.print("\n");
				displayText(calMsgE, count_of(calMsgE));
			}
		}
		else
		{
			// ignore unacceptable characters
			continue;
		}

		/* next */
		displayString(separatorMsg);
		if( ! accelCalibrator.canCalibrate() )
		{
			displayString(measureReadyMsg);
		}
		else
		{
			displayString(measureAndCalibrateReadyMsg);
		}
	}
}
