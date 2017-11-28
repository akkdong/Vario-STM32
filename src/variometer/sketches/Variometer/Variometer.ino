// Variometer.ino
//

#define USE_KALMANVARIO	1

#include <DefaultSettings.h>
#include <SerialEx.h>
#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <GlobalConfig.h>
#include <NmeaParserEx.h>
#include <InputKey.h>
#include <OutputKey.h>
#include <FunctionKey.h>
#include <FuncKeyParser.h>
#include <LEDFlasher.h>
#include <ToneGenerator.h>
#include <ToneFrequency.h>
#include <TonePlayer.h>
#if USE_KALMANVARIO
#include <KalmanVario.h>
#else
#include <Variometer.h>
#endif // USE_KALMANVARIO
#include <VarioBeeper.h>
#include <VarioSentence.h>
#include <BluetoothMan.h>
#include <IGCLogger.h>
#include <BatteryVoltage.h>
#include <CommandParser.h>
#include <ResponseSender.h>
#include <SensorReporter.h>
#include <AccelCalibrator.h>
#include <UsbMassStorage.h>


// test-tone delta(inc/dec) calculation
//
// 0 ~ 8s -> 0 ~ VARIOMETER_MAX_CLIMB_VELOCITY(10m/s)
// velocity update frequency -> 50Hz
//   8000 / (1000 / 50) -> update 400 times
//   10 / 400 -> 0.025

#define TT_HALF_PERIOD		(5000.0)	// 5s
#define TT_UPDATE_FREQ		(50.0)

#define TT_UPDATE_DELTA		(VARIOMETER_MAX_CLIMB_VELOCITY / (TT_HALF_PERIOD / (1000.0 / TT_UPDATE_FREQ)))



enum _DeviceMode
{
	DEVICE_MODE_VARIO = 0,		// (0)
	DEVICE_MODE_UMS,			// (1)
	DEVICE_MODE_CALIBRATION,	// (2)
	DEVICE_MODE_SHUTDOWN,		// (3)
};

enum _VarioMode
{
	VARIO_MODE_INIT = 0,		// (0)
	VARIO_MODE_LANDING,			// (1)
	VARIO_MODE_FLYING,			// (2)
//	VARIO_MODE_SHUTDOWN,		// (3)
};

enum _CalibrationMode
{
	CAL_MODE_INIT = 0,
	CAL_MODE_MEASURE_DELAY,
	CAL_MODE_MEASURE,
	CAL_MODE_COMPLETION,
	CAL_MODE_DONE,
};


//
//
//

void board_init();

void changeDeviceMode();

void setup_vario();
void loop_vario();

void setup_ums();
void loop_ums();

void setup_calibration();
void loop_calibration();

void setup_shutdown();
void loop_shutdown();

void processLowBattery();
void processShutdownInterrupt();
void processCommand();

void commandModeSwitch(Command * cmd);
void commandSoundLevel(Command * cmd);
void commandToneTest(Command * cmd);
void commandSendsorDump(Command * cmd);

void queryParam_profileModel(Command * cmd);		// PARAM_PROFILE_MODEL
void queryParam_profilePilot(Command * cmd);		// PARAM_PROFILE_PILOT
void queryParam_profileGlider(Command * cmd);		// PARAM_PROFILE_GLIDER
void queryParam_varioSinkThreshold(Command * cmd);	// PARAM_VARIO_SINK_THRESHOLD
void queryParam_varioClimbThreshold(Command * cmd);	// PARAM_VARIO_CLIMB_THRESHOLD
void queryParam_varioSensitivity(Command * cmd);	// PARAM_VARIO_SENSITIVITY
void queryParam_varioBaroOnly(Command * cmd);		// PARAM_VARIO_BARO_ONLY
void queryParam_varioVolumn(Command * cmd);			// PARAM_VARIO_VOLUMN
void queryParam_varioTone_XX(Command * cmd);		// PARAM_VARIO_TONE_00~11
void queryParam_timeZone(Command * cmd);			// PARAM_TIME_ZONE
void queryParam_kalmanSigma(Command * cmd);			// PARAM_KALMAN_SIGMA
void queryParam_kalmanVariance(Command * cmd);		// PARAM_KALMAN_VARIANCE
void queryParam_calibrationAccel(Command * cmd);	// PARAM_CALIBRATION_ACCEL
void queryParam_calibrationGyro(Command * cmd);		// PARAM_CALIBRATION_GYRO
void queryParam_unsupport(Command * cmd);

void updateParam_profileModel(Command * cmd);		// PARAM_PROFILE_MODEL
void updateParam_profilePilot(Command * cmd);		// PARAM_PROFILE_PILOT
void updateParam_profileGlider(Command * cmd);		// PARAM_PROFILE_GLIDER
void updateParam_varioSinkThreshold(Command * cmd);	// PARAM_VARIO_SINK_THRESHOLD
void updateParam_varioClimbThreshold(Command * cmd);// PARAM_VARIO_CLIMB_THRESHOLD
void updateParam_varioSensitivity(Command * cmd);	// PARAM_VARIO_SENSITIVITY
void updateParam_varioBaroOnly(Command * cmd);		// PARAM_VARIO_BARO_ONLY
void updateParam_varioVolumn(Command * cmd);		// PARAM_VARIO_VOLUMN
void updateParam_varioTone_XX(Command * cmd);		// PARAM_VARIO_TONE_00~11
void updateParam_timeZone(Command * cmd);			// PARAM_TIME_ZONE
void updateParam_kalmanSigma(Command * cmd);		// PARAM_KALMAN_SIGMA
void updateParam_kalmanVariance(Command * cmd);		// PARAM_KALMAN_VARIANCE
void updateParam_calibrationAccel(Command * cmd);	// PARAM_CALIBRATION_ACCEL
void updateParam_calibrationGyro(Command * cmd);	// PARAM_CALIBRATION_GYRO
void updateParam_unsupport(Command * cmd);

typedef void (* PARAM_PROC)(Command * cmd);

PARAM_PROC	queryProc[] =
{
	queryParam_profileModel,		// PARAM_PROFILE_MODEL
	queryParam_profilePilot,		// PARAM_PROFILE_PILOT
	queryParam_profileGlider,		// PARAM_PROFILE_GLIDER
	queryParam_varioSinkThreshold,	// PARAM_VARIO_SINK_THRESHOLD
	queryParam_varioClimbThreshold,	// PARAM_VARIO_CLIMB_THRESHOLD
	queryParam_varioSensitivity,	// PARAM_VARIO_SENSITIVITY
	queryParam_varioBaroOnly,		// PARAM_VARIO_BARO_ONLY
	queryParam_varioVolumn,			// PARAM_VARIO_VOLUMN
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_00
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_01
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_02
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_03
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_04
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_05
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_06
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_07
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_08
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_09
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_10
	queryParam_varioTone_XX,		// PARAM_VARIO_TONE_11
	queryParam_timeZone,			// PARAM_TIME_ZONE
	queryParam_kalmanSigma,			// PARAM_KALMAN_SIGMA
	queryParam_kalmanVariance,		// PARAM_KALMAN_VARIANCE
	queryParam_calibrationAccel,	// PARAM_CALIBRATION_ACCEL
	queryParam_calibrationGyro,		// PARAM_CALIBRATION_GYRO
};

PARAM_PROC updateProc[] = 
{
	updateParam_profileModel,		// PARAM_PROFILE_MODEL
	updateParam_profilePilot,		// PARAM_PROFILE_PILOT
	updateParam_profileGlider,		// PARAM_PROFILE_GLIDER
	updateParam_varioSinkThreshold,	// PARAM_VARIO_SINK_THRESHOLD
	updateParam_varioClimbThreshold,// PARAM_VARIO_CLIMB_THRESHOLD
	updateParam_varioSensitivity,	// PARAM_VARIO_SENSITIVITY
	updateParam_varioBaroOnly,		// PARAM_VARIO_BARO_ONLY
	updateParam_varioVolumn,		// PARAM_VARIO_VOLUMN
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_00
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_01
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_02
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_03
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_04
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_05
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_06
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_07
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_08
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_09
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_10
	updateParam_varioTone_XX,		// PARAM_VARIO_TONE_11
	updateParam_timeZone,			// PARAM_TIME_ZONE
	updateParam_kalmanSigma,		// PARAM_KALMAN_SIGMA
	updateParam_kalmanVariance,		// PARAM_KALMAN_VARIANCE
	updateParam_calibrationAccel,	// PARAM_CALIBRATION_ACCEL
	updateParam_calibrationGyro,	// PARAM_CALIBRATION_GYRO
};



//
//
//

uint8_t deviceMode = DEVICE_MODE_VARIO;

uint8_t	varioMode; 		// sub-mode of vario-mode
uint8_t	calibMode;		// sub-mode of calibration-mode

uint32_t deviceTick;	// global tick-count
uint32_t modeTick;		// mode-specific tick-count

void (* loop_main)(void) = 0;


//
// tone-test
//

uint8_t	toneTestFlag = false;

float	toneTestVelocity;
float	toneTestDelta;


//
//
//

static Tone melodyVarioReady[] =
{
	{ 262, 1000 / 4 }, 
	{ 196, 1000 / 8 }, 
	{ 196, 1000 / 8 }, 
	{ 220, 1000 / 4 }, 
	{ 196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ 247, 1000 / 4 }, 
	{ 262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};
static Tone melodyTakeOff[] =
{
	{ 262, 1000 / 4 }, 
	{ 196, 1000 / 8 }, 
	{ 196, 1000 / 8 }, 
	{ 220, 1000 / 4 }, 
	{ 196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ 247, 1000 / 4 }, 
	{ 262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};

static Tone melodyLanding[] =
{
	{ 262, 1000 / 4 }, 
	{ 196, 1000 / 8 }, 
	{ 196, 1000 / 8 }, 
	{ 220, 1000 / 4 }, 
	{ 196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ 247, 1000 / 4 }, 
	{ 262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};

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
unlockCallback I2CDevice::cbUnlock = MS5611::unlockI2C;

// declare EEPROMDriver
EEPROMDriver eeprom(Wire2);
//
GlobalConfig Config(eeprom, EEPROM_ADDRESS);

//

#if USE_KALMANVARIO
KalmanVario vario;
#else
Variometer vario;
#endif // USE_KALMANVARIO


//
// declare Serial relative instance
//
// BT uses Serial1
// GPS uses Serial2

NmeaParserEx nmeaParser(SerialEx2);


//
//
//

VarioSentence varioNmea(VARIOMETER_DEFAULT_NMEA_SENTENCE);


//
//
//

SensorReporter sensorReporter;


//
// IGC Logger
//

IGCLogger logger;


//
// Digital & Analog Input/Output
//

// generic digital input
InputKey keyMode;
InputKey keyShutdown;
InputKey keyUSB;
// functional input
FunctionKey	keyFunc;
// analog input
BatteryVoltage batVolt;

// generic digital output
OutputKey keyPowerGPS;
OutputKey keyPowerBT;
OutputKey keyPowerDev;
// functional output
LEDFlasher  ledFlasher;


//
//
//

ToneGenerator toneGen;
TonePlayer tonePlayer(toneGen);

VarioBeeper varioBeeper(tonePlayer);


//
//
//

CommandStack cmdStack;

CommandParser cmdParser1(CMD_FROM_USB, Serial, cmdStack); // USB serial parser
CommandParser cmdParser2(CMD_FROM_BT, Serial1, cmdStack); // BT serial parser
FuncKeyParser keyParser(keyFunc, cmdStack, tonePlayer);

ResponseSender senderUSB;
ResponseSender senderBT;




//
//
//

BluetoothMan btMan(SerialEx1, nmeaParser, varioNmea, sensorReporter, senderBT);


//
//
//

AccelCalibrator accelCalibrator;



//
//
//

void board_init()
{
	// Initialize Serials
	Serial.begin();
	//while (! Serial);
	
	Serial1.begin(BAUDRATE_BT); 	// Serial1(USART1) : for BT
	//while (! Serial1);
	
	Serial2.begin(BAUDRATE_GPS);	// Serial2(USART2) : for GPS
	//while (! Serial2);
	
	// Initialize I2C
	Wire1.begin();
	Wire2.begin();
	
	// input pins
	//keyMode.begin(PIN_MODE_SELECT, ACTIVE_LOW); // not used
	keyShutdown.begin(PIN_SHDN_INT, ACTIVE_LOW);
	keyUSB.begin(PIN_USB_DETECT, ACTIVE_HIGH);
	// function-key
	keyFunc.begin(PIN_FUNC_INPUT, ACTIVE_HIGH);
	// adc input
	batVolt.begin(PIN_ADC_BATTERY);	
	
	// output pins
	keyPowerGPS.begin(PIN_GPS_EN, ACTIVE_LOW, OUTPUT_INACTIVE);
	keyPowerBT.begin(PIN_BT_EN, ACTIVE_LOW, OUTPUT_INACTIVE);
	//keyPowerDev.begin(PIN_KILL_PWR, ACTIVE_LOW, OUTPUT_ACTIVE);
	// state beacon
	ledFlasher.begin(PIN_MCU_STATE, ACTIVE_LOW);
	ledFlasher.turnOn();
}

void board_reset()
{
	//
	tonePlayer.setTone(NOTE_C3, KEY_VOLUME);
	delay(2000);
	
	// reset!!
	nvic_sys_reset();
	while(1);	
}

//
//
//

void changeDeviceMode(int mode)
{
	//
	if (deviceMode == DEVICE_MODE_VARIO)
	{
		// clean-up something
		//
		vario.end();
		
		// turn-off GPS & BT
		keyPowerGPS.disable();
		keyPowerBT.disable();
		
		//
		if (logger.isLogging())
			logger.end(nmeaParser.getDateTime());		
	}
	
	//
	switch ((deviceMode = mode))
	{
	case DEVICE_MODE_VARIO :
		setup_vario();
		loop_main = loop_vario;	
		break;
	case DEVICE_MODE_UMS :
		setup_ums();
		loop_main = loop_ums;
		break;
	case DEVICE_MODE_CALIBRATION :
		setup_calibration();
		loop_main = loop_calibration;
		break;
	case DEVICE_MODE_SHUTDOWN :
		setup_shutdown();
		loop_main = loop_shutdown;
		break;
	}
}


//
//
//

void setup()
{
	//
	board_init();	
	
	//
	Config.readAll();
	
	// ToneGenerator uses PIN_PWM_H(PA8 : Timer1, Channel1)
	toneGen.begin(PIN_PWM_H);
	
	//
	tonePlayer.setVolume(Config.vario_volume);	
		
	//
	changeDeviceMode(DEVICE_MODE_VARIO);
}


//
//
//

void loop()
{
	// main loop for each mode
	loop_main();

	// beep beep beep!
	tonePlayer.update();	
	// start voltage measurement periodically
	batVolt.update();
	// MCU State : LED Blinking
	ledFlasher.update();
	
	
	//
	// common functions
	//
	
	// process command from serial or key
	processCommand();
	
	// low battery!!
	processLowBattery();
	
	// check shutdown interrupts and prepare shutdown
	processShutdownInterrupt();	
}


//
//
//

void setup_vario()
{
	//
	varioMode = VARIO_MODE_INIT;
	
	//
	logger.init();

	//
	#if 0
	imu.init();
	
	for (int i = 0; i < 100; i++)
	{
		while (! imu.dataReady());
		imu.updateData();
	}
	
	// initialize kalman filtered vertical velocity calculator
	vertVel.init(imu.getAltitude(), 
				imu.getVelocity(),
				Config.kalman_sigmaP, // POSITION_MEASURE_STANDARD_DEVIATION,
				Config.kalman_sigmaA, // ACCELERATION_MEASURE_STANDARD_DEVIATION,
				millis());		
	#endif
	
	#if USE_KALMANVARIO
	
	// Kalman filter configuration
	#define KF_ZMEAS_VARIANCE       400.0f
	#define KF_ZACCEL_VARIANCE      1000.0f
	#define KF_ACCELBIAS_VARIANCE   1.0f	
	
	vario.begin(KF_ZMEAS_VARIANCE, KF_ZACCEL_VARIANCE, KF_ACCELBIAS_VARIANCE);
	#else
	vario.begin(Config.kalman_sigmaP, Config.kalman_sigmaA);
	#endif // USE_KALMANVARIO
	
	// turn-on GPS & BT
	keyPowerGPS.enable();
	keyPowerBT.enable();

	// led flash as init-state
	ledFlasher.blink(BTYPE_LONG_ON_SHORT_OFF);

	// start vario-loop
	//tonePlayer.setMelody(&startTone[0], sizeof(startTone) / sizeof(startTone[0]), 1, PLAY_PREEMPTIVE, KEY_VOLUME);
	tonePlayer.setBeep(NOTE_C4, 800, 500, 2, KEY_VOLUME);
	
	//
	deviceTick = millis();
}

void loop_vario()
{
	//
	vario.update();
	
	if (vario.available())
	{
		//
		float velocity = vario.getVelocity();
		
		if (toneTestFlag)
		{
			//if (millis() - toneTestTick > TONE_TEST_DELAY)
			{
				toneTestVelocity = toneTestVelocity + toneTestDelta;
				
				if (toneTestVelocity < Config.vario_climbThreshold)
					toneTestDelta = TT_UPDATE_DELTA;
				if (toneTestVelocity > VARIOMETER_MAX_CLIMB_VELOCITY)
					toneTestDelta = -TT_UPDATE_DELTA;
				
				//toneTestTick = millis();
			}

			varioBeeper.setVelocity(toneTestVelocity);
		}
		else
		{
			varioBeeper.setVelocity(velocity);
			Serial.println(velocity);
		}

		//
		{
			if (velocity < STABLE_SINKING_THRESHOLD || STABLE_CLIMBING_THRESHOLD < velocity)
				deviceTick = millis(); // reset tick because it's not quiet.
			
			if ((millis() - deviceTick) > AUTO_SHUTDOWN_THRESHOLD)
			{
				Serial.println("Now process auto-shutdown!!");
				
				changeDeviceMode(DEVICE_MODE_SHUTDOWN);
				return;
			}
		}
		
		//
		sensorReporter.update(vario.getAccelData(), vario.getGyroData(), vario.getPressure(), vario.getTemperature());
		
		//
		float altitude = vario.getAltitude2(); // getCalibratedAltitude or getAltitude
		logger.update(altitude);
		//Serial.print(vario.getAltitude()); Serial.print(", "); Serial.println(vario.getAltitude2());
		
		// update vario sentence periodically
		if (varioNmea.checkInterval())
			varioNmea.begin(altitude/*vario.getCalibratedAltitude()*/, vario.getVelocity(), vario.getTemperature(), batVolt.getVoltage());
		
		//
		vario.flush();
	}	
	
	// read & prase gps sentence
	nmeaParser.update();
	
	// send any prepared sentence to BT
	btMan.update();	

	// IGC setence is available when it received a valid GGA. -> altitude is valid
	if (varioMode == VARIO_MODE_INIT  && nmeaParser.availableIGC())
	{
		// do position calibration
		//vertVel.calibratePosition(nmeaParser.getAltitude());
		vario.calibrateAltitude(nmeaParser.getAltitude());
		
		// now ready to fly~~~
		varioMode = VARIO_MODE_LANDING;

		//		
		ledFlasher.blink(BTYPE_LONG_ON_OFF);
		// play ready melody~~~
		tonePlayer.setMelody(&melodyVarioReady[0], sizeof(melodyVarioReady) / sizeof(melodyVarioReady[0]), 1, PLAY_PREEMPTIVE, KEY_VOLUME);
	}
	else if (varioMode == VARIO_MODE_LANDING)
	{
		if (nmeaParser.getSpeed() > FLIGHT_START_MIN_SPEED)
		{
			//
			varioMode = VARIO_MODE_FLYING;
			
			ledFlasher.blink(BTYPE_SHORT_ON_OFF);
			// play take-off melody
			tonePlayer.setMelody(&melodyTakeOff[0], sizeof(melodyTakeOff) / sizeof(melodyTakeOff[0]), 1, PLAY_PREEMPTIVE, KEY_VOLUME);
			
			// start logging & change mode
			logger.begin(nmeaParser.getDateTime());
			
			// set mode-tick
			modeTick = millis();
		}
	}
	else if (varioMode == VARIO_MODE_FLYING)
	{
		if (nmeaParser.getSpeed() < FLIGHT_START_MIN_SPEED)
		{
			if ((millis() - modeTick) > FLIGHT_LANDING_THRESHOLD)
			{
				//
				varioMode = VARIO_MODE_LANDING;
				
				//
				ledFlasher.blink(BTYPE_LONG_ON_OFF);
				// play landing melody
				tonePlayer.setMelody(&melodyLanding[0], sizeof(melodyLanding) / sizeof(melodyLanding[0]), 1, PLAY_PREEMPTIVE, KEY_VOLUME);
				
				// stop logging & change mode
				logger.end(nmeaParser.getDateTime());
			}
		}
		else
		{
			// reset modeTick
			modeTick = millis();
		}
	}
	
	// check logging state
	if (logger.isLogging())
	{
		// nmeaParser parses GPS sentence and converts it to IGC sentence
		
		//static unsigned long tick = millis();
		//static int index = 0;
		
		//if ((millis()-tick) > time_interval)
		{		
			while (nmeaParser.availableIGC())
				logger.write(nmeaParser.readIGC());
		}
	}	
}


//
//
//

class Sd2CardEx : public SdSpiCard
{
public:
	Sd2CardEx() {
		m_spi.setPort(1);
	}
	Sd2CardEx(uint8_t spiPort) {
		m_spi.setPort(spiPort);
	}

	/** Initialize the SD card.
	* \param[in] csPin SD chip select pin.
	* \param[in] settings SPI speed, mode, and bit order.
	* \return true for success else false.
	*/
	bool begin(uint8_t csPin = SS, SPISettings settings = SD_SCK_MHZ(50)) {
		return SdSpiCard::begin(&m_spi, csPin, settings);
	}
	
private:
	SdFatSpiDriver m_spi;
};

Sd2CardEx SdCard(SDCARD_CHANNEL);

uint32_t MAL_massBlockCount[2];
uint32_t MAL_massBlockSize[2];

void setup_ums()
{
	//
	Serial.end();
	
	//
	SdCard.begin(SDCARD_CS, SD_SCK_HZ(F_CPU/2)); // SPI_QUARTER_SPEED);
	
	uint32_t numberOfBlocks = SdCard.cardSize();
	//SerialDbg.print("Number of Blocks = "); SerialDbg.println(numberOfBlocks);
	
	if (numberOfBlocks)
	{
		MAL_massBlockCount[0] = numberOfBlocks;
		MAL_massBlockCount[1] = 0;
		MAL_massBlockSize[0] = 512;
		MAL_massBlockSize[1] = 0;
		//SerialDbg.print("cardSize = "); SerialDbg.println(numberOfBlocks * 512);

		//
		//SerialDbg.println("USBMassStorage.begin");
		USBMassStorage.begin();	
		
		//
		ledFlasher.blink(BTYPE_BLINK_2_LONG_ON);
		//
		tonePlayer.setBeep(NOTE_G4, 500, 300, 2, KEY_VOLUME);
		
		// set mode-tick
		modeTick = millis();
	}
	else
	{
		// synchronous beep!!
		tonePlayer.beep(NOTE_C3, 200, 3, KEY_VOLUME);
		// return to vario mode
		changeDeviceMode(DEVICE_MODE_VARIO);
	}
}

void loop_ums()
{
	//
	USBMassStorage.loop();
	
	//
	ledFlasher.update();
	tonePlayer.update();
	
	// reset mode-tick when usb is plugged
	if (keyUSB.read() == INPUT_ACTIVE)
		modeTick = millis(); 

	// turn-off if it have been unplugged while a threshold
	if ((millis() - modeTick) > AUTO_SHUTDOWN_THRESHOLD)
	{
		// synchronous beep!!
		tonePlayer.beep(NOTE_C4, 400, 2, KEY_VOLUME);
		// shutdown now
		changeDeviceMode(DEVICE_MODE_SHUTDOWN);
	}
}


extern "C" uint16_t usb_mass_mal_init(uint8_t lun)
{
	return 0;
}

extern "C" uint16_t usb_mass_mal_get_status(uint8_t lun)
{
	return SdCard.errorCode();
}

extern "C" uint16_t usb_mass_mal_write_memory(uint8_t lun, uint32_t memoryOffset, uint8_t *writebuff, uint16_t transferLength)
{
	uint32_t block = memoryOffset / 512;

	if (lun != 0)
		return USB_MASS_MAL_FAIL;

	if (SdCard.writeBlock(block, writebuff))
		return USB_MASS_MAL_SUCCESS;

	return USB_MASS_MAL_FAIL;
}

extern "C" uint16_t usb_mass_mal_read_memory(uint8_t lun, uint32_t memoryOffset, uint8_t *readbuff, uint16_t transferLength)
{
	if (lun != 0)
		return USB_MASS_MAL_FAIL;

	if (SdCard.readBlock(memoryOffset / 512, readbuff))
		return USB_MASS_MAL_SUCCESS;

	return USB_MASS_MAL_FAIL;
}

extern "C" void usb_mass_mal_format()
{
}
	

//
//
//

#define HIGH_BEEP_FREQ 			(1000)
#define LOW_BEEP_FREQ 			(100)
#define BASE_BEEP_DURATION 		(100)

#define MEASURE_DELAY 			(3000)

void setup_calibration()
{
	//
	calibMode = CAL_MODE_INIT;
	
	//
	accelCalibrator.init();
	
	//
	ledFlasher.blink(BTYPE_BLINK_3_LONG_ON);
	tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 2, BASE_BEEP_DURATION, 3, KEY_VOLUME);
	
	calibMode = CAL_MODE_MEASURE_DELAY;
	deviceTick = millis();
}

void loop_calibration()
{
	//
	if (calibMode == CAL_MODE_MEASURE_DELAY)
	{
		if (millis() - deviceTick > MEASURE_DELAY)
			calibMode = CAL_MODE_MEASURE;
	}
	else if (calibMode == CAL_MODE_MEASURE)
	{
		Serial.println("start measure....");
		
		// make measure
		accelCalibrator.measure(&ledFlasher);
		
		// get orientation
		int orient = accelCalibrator.getMeasureOrientation();
		Serial.print("  orientation = "); Serial.println(orient);
		
		if (orient != ACCEL_CALIBRATOR_ORIENTATION_EXCEPTION)
		{
			// push measure
			boolean measureValid = accelCalibrator.pushMeasure();
			Serial.print("  measurement "); Serial.println(measureValid?"valid":"invalid");

			// make corresponding beep
			if (measureValid)
				tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 6, BASE_BEEP_DURATION * 3, 1, KEY_VOLUME);
			else 
				tonePlayer.setBeep(LOW_BEEP_FREQ, BASE_BEEP_DURATION * 6, BASE_BEEP_DURATION * 3, 1, KEY_VOLUME);	
			
			// go back measure delay
			calibMode = CAL_MODE_MEASURE_DELAY;
			// reset delay tick
			deviceTick = millis();
			Serial.println("wait to next measurement!");
		}
		else
		{
			//
			if( accelCalibrator.canCalibrate() )
			{
				Serial.println("calibrate!!!");
				// calibrate & save result
				accelCalibrator.calibrate();

				// play completion melody & confirm
				calibMode = CAL_MODE_COMPLETION;
				
				tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 2, BASE_BEEP_DURATION, 3, KEY_VOLUME);
				ledFlasher.blink(BTYPE_SHORT_ON_OFF);
			}
			else
			{
				// go back measure delay
				calibMode = CAL_MODE_MEASURE_DELAY;
				// reset delay tick
				deviceTick = millis();
				Serial.println("wait to next measurement!");
			}
		}
	}
	else if (calibMode == CAL_MODE_COMPLETION)
	{
		if (millis() - deviceTick > MEASURE_DELAY)
		{
			Serial.println("calibrate complete!!!");
			Serial.println("reset now....");
			
			// jobs done. reset now!
			board_reset();
		}				
	}
}


//
//
//

void setup_shutdown()
{
	//
	ledFlasher.blink(BTYPE_BLINK_3_LONG_OFF);
	tonePlayer.setBeep(NOTE_C3, 800, 800);

	//
	if (logger.isLogging())
		logger.end(nmeaParser.getDateTime());
	
	//
	deviceTick = millis();
}

void loop_shutdown()
{
	if (millis() - deviceTick > SHUTDOWN_HOLD_TIME)
	{
		tonePlayer.setMute();
		keyPowerDev.begin(PIN_KILL_PWR, ACTIVE_LOW, OUTPUT_ACTIVE);
		
		while(1);
	}
}


//
// process low battery
//

void processLowBattery()
{
	#if 0
	if (deviceMode != DEVICE_MODE_SHUTDOWN)
	{
		if (batVolt.getVoltage() < LOW_BATTERY_THRESHOLD)
		{
			Serial.println("!!Alert!!");
			Serial.println("It's low battery. Device will be shutdown now!!");

			changeDeviceMode(DEVICE_MODE_SHUTDOWN);
		}
	}
	#endif
}


//
// process shutdown interrupt (from LTC2950)
//

void processShutdownInterrupt()
{
	if (keyShutdown.read() == INPUT_ACTIVE)
	{
		// shutdown interrupt trigged by LTC2950
		// clean-up & wait power-off(LTC2750 will turn off power)
		if (logger.isLogging())
			logger.end(nmeaParser.getDateTime());
		
		// beep~
		//tonePlayer.setBeep(420, 0, 0, KEY_VOLUME);
		tonePlayer.setTone(360, KEY_VOLUME);
		while(1)
			tonePlayer.update();
	}
}


//
// process command from serial or key
//

void processCommand()
{
	//
	cmdParser1.update();
	cmdParser2.update();
	keyParser.update();
	
	//
	//while(cmdStack.getSize())
	if(cmdStack.getSize())
	{
		Command cmd = cmdStack.dequeue();
		
		switch(cmd.code)
		{
		case CMD_MODE_SWITCH 	:
			commandModeSwitch(&cmd);
			break;
//		case CMD_DEVICE_STATUS 	:
//			break;
		case CMD_SENSOR_DUMP 	:
			commandSendsorDump(&cmd);
			break;
		case CMD_NMEA_SENTENCE  :
			btMan.blockNmeaSentence(cmd.param);
			break;
		case CMD_TONE_TEST 		:
			commandToneTest(&cmd);
			break;
		case CMD_SOUND_LEVEL 	:
			commandSoundLevel(&cmd);
			break;
		case CMD_DEVICE_RESET 	:
			// reset!!
			board_reset();
			break;
		case CMD_QUERY_PARAM 	:
			if (cmd.param < PARAM_COUNT)
				queryProc[cmd.param](&cmd);
			else
				queryParam_unsupport(&cmd);
			break;
		case CMD_UPDATE_PARAM 	:
			if (cmd.param < PARAM_COUNT)
				updateProc[cmd.param](&cmd);
			else
				updateParam_unsupport(&cmd);
			break;
		case CMD_SAVE_PARAM :
			Config.writeAll();
			break;
		case CMD_RESTORE_PARAM :
			Config.reset();
			Config.readAll();
			break;;
		}
	}	
}

// CMD_MODE_SWITCH
void commandModeSwitch(Command * cmd)
{
	// change current mode
	if (deviceMode != cmd->param)
	{
		switch (cmd->param)
		{
		case PARAM_SW_ICALIBRATION :
			// setup
			//setup_calibration();
			// loop
			//main_loop = icalibration_loop();
			break;
		case PARAM_SW_CALIBRATION  :
			changeDeviceMode(DEVICE_MODE_CALIBRATION);
			break;
		case PARAM_SW_UMS          :
			if (/*keyUSB.read() == INPUT_ACTIVE &&*/ logger.isInitialized())
			{
				changeDeviceMode(DEVICE_MODE_UMS);
				return;
			}
			else
			{
				// sd-init failed!! : warning beep~~
				tonePlayer.beep(NOTE_C3, 200, 4, KEY_VOLUME);
			}
			break;
		}
	}
}

// CMD_SOUND_LEVEL
void commandSoundLevel(Command * cmd)
{
	int volume = -1;
	
	switch (cmd->param)
	{
	case PARAM_LV_LOUD 	:
		volume = MAX_VOLUME;
		break;
	case PARAM_LV_QUIET :
		volume = MID_VOLUME;
		break;
	case PARAM_LV_MUTE	:
		volume = MIN_VOLUME;
		break;
	}
	
	if (MIN_VOLUME <= volume && volume != tonePlayer.getVolume())
	{
		//
		tonePlayer.setVolume(volume);
		// save volume
		Config.updateVarioVolume(tonePlayer.getVolume());

		//
		tonePlayer.setBeep(460, 800, 400, 3);
	}
}

// CMD_TONE_TEST
//
// #TT,[0|1]
//     0 : stop
//     1 : start
void commandToneTest(Command * cmd)
{
	if (cmd->param)
	{
		toneTestFlag = true;

		toneTestVelocity = Config.vario_climbThreshold;
		toneTestDelta = TT_UPDATE_DELTA;
	}
	else
	{
		toneTestFlag = false;
	}
}

// CMD_SENSOR_DUMP
//
// #DU,[bitmask]
//     0 : stop
//     1 : accel
//     2 : gyro
//     4 : pressure
//     8 : temperature
// response
//     $SENSOR,ax,ay,az,gx,gy,gz,p,t*XX\r\n
	

void commandSendsorDump(Command * cmd)
{
	if (cmd->param)
	{
		uint8_t enable = 0;
		
		if (cmd->param & PARAM_DU_ACCELEROMETER)
			enable |= ENABLE_ACCELEROMETER;
		if (cmd->param & PARAM_DU_GYROSCOPE)
			enable |= ENABLE_GYROSCOPE;
		if (cmd->param & PARAM_DU_PRESSURE)
			enable |= ENABLE_PRESSURE;
		if (cmd->param & PARAM_DU_TEMPERATURE)
			enable |= ENABLE_TEMPERATURE;
		
		sensorReporter.enable(enable);
	}
	else
	{
		sensorReporter.enable(ENABLE_NONE);
	}
	
	//sensorReporter.enable(cmd.param);
	//btMan.blockSensorData(cmd.param);		
}

// PARAM_PROFILE_MODEL
void queryParam_profileModel(Command * cmd)
{	
	Serial.print("%QU,"); 
	Serial.print(cmd->param); 
	Serial.print(","); 
	Serial.println(Config.profile_model);	
}

// PARAM_PROFILE_PILOT
void queryParam_profilePilot(Command * cmd)
{	
	Serial.print("%QU,"); 
	Serial.print(cmd->param); 
	Serial.print(","); 
	Serial.println(Config.profile_pilot);	
}

// PARAM_PROFILE_GLIDER
void queryParam_profileGlider(Command * cmd)
{	
	Serial.print("%QU,"); 
	Serial.print(cmd->param); 
	Serial.print(","); 
	Serial.println(Config.profile_glider);	
}

// PARAM_VARIO_SINK_THRESHOLD
void queryParam_varioSinkThreshold(Command * cmd)
{
	Serial.print("%QU,"); 
	Serial.print(cmd->param); 
	Serial.print(","); 
	Serial.println(Config.vario_sinkThreshold);	
}

// PARAM_VARIO_CLIMB_THRESHOLD
void queryParam_varioClimbThreshold(Command * cmd)
{
	Serial.print("%QU,"); 
	Serial.print(cmd->param); 
	Serial.print(","); 
	Serial.println(Config.vario_climbThreshold);
}

// PARAM_VARIO_SENSITIVITY
void queryParam_varioSensitivity(Command * cmd)
{	
	Serial.print("%QU,"); 
	Serial.print(cmd->param); 
	Serial.print(","); 
	Serial.println(Config.vario_sensitivity);
}

// PARAM_VARIO_BARO_ONLY
void queryParam_varioBaroOnly(Command * cmd)
{
	Serial.print("%QU,"); 
	Serial.print(cmd->param); 
	Serial.print(","); 
	Serial.println(Config.vario_baroOnly);
}

// PARAM_VARIO_VOLUMN
void queryParam_varioVolumn(Command * cmd)
{	
	Serial.print("%QU,"); 
	Serial.print(cmd->param); 
	Serial.print(","); 
	Serial.println(Config.vario_volume);
}

// PARAM_VARIO_TONE_00~11
void queryParam_varioTone_XX(Command * cmd)
{	
	Serial.print("%QU,"); 
	Serial.print(cmd->param);
	Serial.print(","); 
	Serial.print(Config.vario_tone[cmd->param-PARAM_VARIO_TONE_00].velocity);
	Serial.print(","); 
	Serial.print(Config.vario_tone[cmd->param-PARAM_VARIO_TONE_00].freq);
	Serial.print(","); 
	Serial.print(Config.vario_tone[cmd->param-PARAM_VARIO_TONE_00].period);
	Serial.print(","); 
	Serial.println(Config.vario_tone[cmd->param-PARAM_VARIO_TONE_00].duty);
}


// PARAM_TIME_ZONE
void queryParam_timeZone(Command * cmd)
{	
	Serial.print("%QU,");
	Serial.print(cmd->param);
	Serial.print(","); 
	Serial.println(Config.vario_timezone);
}

// PARAM_KALMAN_SIGMA
void queryParam_kalmanSigma(Command * cmd)
{
	Serial.print("%QU,"); 
	Serial.print(cmd->param); 
	Serial.print(","); 
	Serial.print(Config.kalman_sigmaP);
	Serial.print(","); 
	Serial.println(Config.kalman_sigmaA);
}

// PARAM_KALMAN_VARIANCE
void queryParam_kalmanVariance(Command * cmd)
{	
}

// PARAM_CALIBRATION_ACCEL
void queryParam_calibrationAccel(Command * cmd)
{
	Serial.print("%QU,"); 
	Serial.print(cmd->param); 
	Serial.print(","); 
	Serial.print(Config.accel_calData[0]);
	Serial.print(","); 
	Serial.print(Config.accel_calData[1]);
	Serial.print(","); 
	Serial.println(Config.accel_calData[2]);
}

// PARAM_CALIBRATION_GYRO
void queryParam_calibrationGyro(Command * cmd)
{	
	Serial.print("%QU,"); 
	Serial.print(cmd->param); 
	Serial.print(","); 
	Serial.print(Config.gyro_calData[0]);
	Serial.print(","); 
	Serial.print(Config.gyro_calData[1]);
	Serial.print(","); 
	Serial.println(Config.gyro_calData[2]);
}

// UNSUPPORTED PARAMETER
void queryParam_unsupport(Command * cmd)
{	
}


// PARAM_PROFILE_MODEL
void updateParam_profileModel(Command * cmd)
{	
}

// PARAM_PROFILE_PILOT
void updateParam_profilePilot(Command * cmd)
{	
}

// PARAM_PROFILE_GLIDER
void updateParam_profileGlider(Command * cmd)
{	
}

// PARAM_VARIO_SINK_THRESHOLD
void updateParam_varioSinkThreshold(Command * cmd)
{	
//	float value = toFloat(cmd->valData);
//	Config.vario_sensitivity = value;
}

// PARAM_VARIO_CLIMB_THRESHOLD
void updateParam_varioClimbThreshold(Command * cmd)
{	
//	float value = toFloat(cmd->valData);
//	Config.vario_sensitivity = value;
}

// PARAM_VARIO_SENSITIVITY
void updateParam_varioSensitivity(Command * cmd)
{
//	float value = toFloat(cmd->valData);
//	Config.vario_sensitivity = value;
}

// PARAM_VARIO_BARO_ONLY
void updateParam_varioBaroOnly(Command * cmd)
{
}

// PARAM_VARIO_VOLUMN
void updateParam_varioVolumn(Command * cmd)
{	
}

// PARAM_VARIO_TONE_00~11
void updateParam_varioTone_XX(Command * cmd)
{	
}

// PARAM_TIME_ZONE
void updateParam_timeZone(Command * cmd)
{
//	int32_t value = toNum(cmd->valData);
//	Config.vario_timezone = value;
}

// PARAM_KALMAN_SIGMA
void updateParam_kalmanSigma(Command * cmd)
{
}

// PARAM_KALMAN_VARIANCE
void updateParam_kalmanVariance(Command * cmd)
{	
}

// PARAM_CALIBRATION_ACCEL
void updateParam_calibrationAccel(Command * cmd)
{	
}

// PARAM_CALIBRATION_GYRO
void updateParam_calibrationGyro(Command * cmd)
{	
}

// UNSUPPORTED PARAMETER
void updateParam_unsupport(Command * cmd)
{	
}
