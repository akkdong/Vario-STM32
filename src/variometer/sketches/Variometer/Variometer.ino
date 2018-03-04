// Variometer.ino
//

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
#if VARIOMETER_CLASSS == CLASS_KALMANVARIO
#include <KalmanVario.h>
#else
#include <Variometer.h>
#endif // VARIOMETER_CLASSS == CLASS_KALMANVARIO
#include <VarioBeeper.h>
#include <VarioSentence.h>
#include <BluetoothMan.h>
#include <IGCLogger.h>
#include <BatteryVoltage.h>
#include <CommandParser.h>
#include <ResponseStack.h>
#include <SensorReporter.h>
#include <AccelCalibrator.h>

#include "ParamVarMap.h"
#include "Sd2CardEx.h"
#include "UMS_mal.h"
#include <libmaple/bkp.h>

#define RESET_FOR_BOOTLOADER		(0x4501)


// test-tone delta(inc/dec) calculation
//
// 0 ~ 8s -> 0 ~ VARIOMETER_MAX_VELOCITY(10m/s)
// velocity update frequency -> 50Hz
//   8000 / (1000 / 50) -> update 400 times
//   10 / 400 -> 0.025

#define TT_HALF_PERIOD		(6000.0)	// 6s
#define TT_UPDATE_FREQ		(50.0)

#define TT_UPDATE_DELTA		(VARIOMETER_MAX_VELOCITY / (TT_HALF_PERIOD / (1000.0 / TT_UPDATE_FREQ)))



enum _DeviceMode
{
	DEVICE_MODE_UNDEF =0,
	DEVICE_MODE_VARIO,			// (1)
	DEVICE_MODE_UMS,			// (2)
	DEVICE_MODE_CALIBRATION,	// (3)
	DEVICE_MODE_SHUTDOWN,		// (4)
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
	CAL_MODE_MEASURE_READY,
	CAL_MODE_MEASURE,
	CAL_MODE_CALIBATE_READY,
	CAL_MODE_CALIBATE,
	CAL_MODE_DONE,
	CAL_MODE_RESET,
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
void commandQueryProperty(Command * cmd);
void commandUpdateProperty(Command * cmd);
void commandDumpProperties(Command * cmd);
void commandAccelerometerCalibration(Command * cmd);

void calibration_changeMode(uint8_t mode);
void calibration_readyMeasure(int beepType);
void calibration_startMeasure();
void calibration_doCalibate();

int pushProperties(int start);


//
//
//

uint8_t deviceMode = DEVICE_MODE_VARIO;

uint8_t	varioMode; 		// sub-mode of vario-mode
uint8_t	calibMode;		// sub-mode of calibration-mode
uint8_t calInteractive;	// flag for interactive calibration, 0 -> non-interactive(default), others -> interactive

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
// dump properties
//

int		dumpProp = -1;


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

TwoWire Wire1(1, I2C_FAST_MODE);
TwoWire Wire2(2, I2C_FAST_MODE);

// initialize some static member of I2CDevice(I2C1)
//

// set wire reference member to I2C1
TwoWire & I2CDevice::Wire = Wire1;

// set unlock callback function
unlockCallback I2CDevice::cbUnlock = MS5611::unlockI2C;

// declare EEPROMDriver
EEPROMDriver eeprom(Wire2);
//
GlobalConfig Config(eeprom, EEPROM_ADDRESS);

//

#if VARIOMETER_CLASSS == CLASS_KALMANVARIO
KalmanVario vario;
#else
Variometer vario;
#endif // VARIOMETER_CLASSS == CLASS_KALMANVARIO


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
#if HW_VERSION == HW_VERSION_V1_REV2
InputKey keyAccelInt;
InputKey keyMagDReady;
#endif // HW_VERSION == HW_VERSION_V1_REV2
// functional input
FunctionKey	keyFunc;
// analog input
BatteryVoltage batVolt;

// generic digital output
OutputKey keyPowerGPS;
OutputKey keyPowerBT;
OutputKey keyPowerDev;
#if HW_VERSION == HW_VERSION_V1_REV2
//OutputKey keyAccelFSync;
OutputKey keyPowerIMU;
OutputKey keyPowerSD;
#endif // HW_VERSION == HW_VERSION_V1_REV2
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
CommandParser cmdParser2(CMD_FROM_BT, SerialEx1, cmdStack); // BT serial parser
FuncKeyParser keyParser(keyFunc, cmdStack, tonePlayer);

//ResponseStack resStackUSB;
ResponseStack resStackBT;

volatile int commandReceiveFlag = 0; // when any new command is occured, set this. 


//
//
//

BluetoothMan btMan(SerialEx1, nmeaParser, varioNmea, sensorReporter, resStackBT);


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
	
	SerialEx1.begin(BAUDRATE_BT); 	// Serial1(USART1) : for BT
	//while (! SerialEx1);
	
	SerialEx2.begin(BAUDRATE_GPS);	// Serial2(USART2) : for GPS
	//while (! SerialEx2);
	
	// Initialize I2C
	Wire1.begin();
	Wire2.begin();
	
	// input pins
	//keyMode.begin(PIN_MODE_SELECT, ACTIVE_LOW); // not used
	keyShutdown.begin(PIN_SHDN_INT, ACTIVE_LOW);
	keyUSB.begin(PIN_USB_DETECT, ACTIVE_HIGH);
	#if HW_VERSION == HW_VERSION_V1_REV2
	keyAccelInt.begin(PIN_IMU_INTA, ACTIVE_HIGH);
	keyMagDReady.begin(PIN_IMU_DRDY, ACTIVE_LOW);
	#endif // HW_VERSION == HW_VERSION_V1_REV2
	// function-key
	keyFunc.begin(PIN_FUNC_INPUT, ACTIVE_HIGH);
	// adc input
	batVolt.begin(PIN_ADC_BATTERY);	
	
	// output pins
	#if HW_VERSION == HW_VERSION_V1
	keyPowerGPS.begin(PIN_GPS_EN, ACTIVE_LOW, OUTPUT_INACTIVE);
	keyPowerBT.begin(PIN_BT_EN, ACTIVE_LOW, OUTPUT_ACTIVE);
	//keyPowerDev.begin(PIN_KILL_PWR, ACTIVE_LOW, OUTPUT_ACTIVE);
	#elif HW_VERSION == HW_VERSION_V1_REV2
	keyPowerGPS.begin(PIN_GPS_EN, ACTIVE_HIGH, OUTPUT_INACTIVE);
	keyPowerBT.begin(PIN_BT_EN, ACTIVE_HIGH, OUTPUT_ACTIVE);
	//keyPowerDev.begin(PIN_KILL_PWR, ACTIVE_LOW, OUTPUT_ACTIVE);
	//keyAccelFSync.begin(PIN_IMU_FSYNC, ACTIVE_HIGH, OUTPUT_INACTIVE);
	keyPowerIMU.begin(PIN_IMU_EN, ACTIVE_HIGH, OUTPUT_INACTIVE);
	keyPowerSD.begin(PIN_SD_EN, ACTIVE_HIGH, OUTPUT_INACTIVE);
	#endif // HW_VERSION == HW_VERSION_V1_REV2
	// state beacon
	ledFlasher.begin(PIN_MCU_STATE, ACTIVE_LOW);
	ledFlasher.turnOn();
}

void board_reset()
{
	//
	tonePlayer.setTone(NOTE_C3, Config.volume.effect);
	delay(2000);
	
	// reset!!
	nvic_sys_reset();
	while(1);	
}

void board_reboot()
{
	#if 0
	// Enable clocks for the backup domain registers
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	// Disable backup register write protection
	PWR_BackupAccessCmd(ENABLE);
	// store value in DR10
	BKP_WriteBackupRegister(BKP_DR10, RESET_FOR_BOOTLOADER);
	// Re-enable backup register write protection
	PWR_BackupAccessCmd(DISABLE);
	#else
	bkp_init();
	//	rcc_clk_enable(RCC_PWR);
	//	rcc_clk_enable(RCC_BKP);
	bkp_enable_writes();
	bkp_write(10, RESET_FOR_BOOTLOADER);
	bkp_disable_writes();
	#endif
	
	// reset!!
	nvic_sys_reset();
	while(1);	
}

//
//
//

void changeDeviceMode(int mode)
{
	// post process
	switch (deviceMode)
	{
	case DEVICE_MODE_VARIO :
		// clean-up something
		vario.end();
		// close logger file if is logging
		logger.end(nmeaParser.getDateTime());
		// turn-off SD & wait a moment
		#if HW_VERSION == HW_VERSION_V1_REV2
		keyPowerSD.disable();
		delay(100);
		#endif // HW_VERSION == HW_VERSION_V1_REV2
		break;
	case DEVICE_MODE_CALIBRATION :
		// nop
		break;
	case DEVICE_MODE_UMS :
		// turn-off SD & wait a moment
		#if HW_VERSION == HW_VERSION_V1_REV2
		keyPowerSD.disable();
		delay(100);
		#endif // HW_VERSION == HW_VERSION_V1_REV2
		break;
	}

	// start new mode
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
	// init global variables
	// ...
	
	//
	board_init();
	
	//
	Config.reset();
	Config.readAll();
	delay(100);
	Config.readAll();
	
	// ToneGenerator uses PIN_PWM_H(PA8 : Timer1, Channel1)
	toneGen.begin(PIN_PWM_H);
	
	//
	tonePlayer.setVolume(Config.volume.vario);	
		
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
	
	//
	if (dumpProp >= 0)
		dumpProp = pushProperties(dumpProp);	
}


//
//
//

void setup_vario()
{
	//
	varioMode = VARIO_MODE_INIT;
	
	// turn-on All : IMU, SD, GPS, BT
	#if HW_VERSION == HW_VERSION_V1_REV2
	keyPowerIMU.enable();
	keyPowerSD.enable();
	#endif // HW_VERSION == HW_VERSION_V1_REV2
	keyPowerGPS.enable();
	keyPowerBT.enable();
	delay(100);
	
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
				Config.kalman.sigmaP, // POSITION_MEASURE_STANDARD_DEVIATION,
				Config.kalman.igmaA, // ACCELERATION_MEASURE_STANDARD_DEVIATION,
				millis());		
	#endif
	
	#if VARIOMETER_CLASSS == CLASS_KALMANVARIO
	//vario.begin(KF_ZMEAS_VARIANCE, KF_ZACCEL_VARIANCE, KF_ACCELBIAS_VARIANCE);
	vario.begin(Config.kalman.var_zmeas, Config.kalman.var_zaccel, Config.kalman.var_accelbias);
	#else
	vario.begin(Config.kalman.sigmaP, Config.kalman.sigmaA);
	#endif // VARIOMETER_CLASSS == CLASS_KALMANVARIO


	// led flash as init-state
	ledFlasher.blink(BTYPE_LONG_ON_SHORT_OFF);

	// start vario-loop
	//tonePlayer.setMelody(&startTone[0], sizeof(startTone) / sizeof(startTone[0]), 1, PLAY_PREEMPTIVE, Config.volume.effect);
	tonePlayer.setBeep(NOTE_C4, 800, 500, 2, Config.volume.effect);
	
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
				
				if (toneTestVelocity < VARIOMETER_MIN_VELOCITY)
					toneTestDelta = TT_UPDATE_DELTA;
				if (toneTestVelocity > VARIOMETER_MAX_VELOCITY)
					toneTestDelta = -TT_UPDATE_DELTA;
				
				//toneTestTick = millis();
			}

			varioBeeper.setVelocity(toneTestVelocity);
		}
		else
		{
			varioBeeper.setVelocity(velocity);
			//Serial.println(velocity);
		}

		//
		{
			if (velocity < STABLE_SINKING_THRESHOLD || STABLE_CLIMBING_THRESHOLD < velocity)
				deviceTick = millis(); // reset tick because it's not quiet.
			
			if (commandReceiveFlag)
			{
				deviceTick = millis(); // reset tick because it's not quiet.
				commandReceiveFlag = 0;
			}
			
			//if ((millis() - deviceTick) > AUTO_SHUTDOWN_THRESHOLD)
			if ((Config.threshold.auto_shutdown_vario) && ((millis() - deviceTick) > Config.threshold.auto_shutdown_vario))
			{
				Serial.println("Now process auto-shutdown!!");
				
				changeDeviceMode(DEVICE_MODE_SHUTDOWN);
				return;
			}
		}
		
		//
		sensorReporter.update(vario.getAccelData(), vario.getGyroData(), vario.getPressure(), vario.getTemperature());
		
		//
		float altitude = vario.getCalibratedAltitude(); // getCalibratedAltitude or getAltitude
		logger.update(altitude);
		//Serial.print(altitude); Serial.print(", "); Serial.println(vario.getAltitude2());
		
		// update vario sentence periodically
		if (varioNmea.checkInterval())
			varioNmea.begin(altitude/*vario.getCalibratedAltitude()*/, vario.getVelocity(), vario.getTemperature(), batVolt.getVoltage());
		
		//
		vario.flush();
	}	
	
	// read & prase gps sentence
	nmeaParser.update(/*vario.getCalibratedAltitude()*/);
	
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
		tonePlayer.setMelody(&melodyVarioReady[0], sizeof(melodyVarioReady) / sizeof(melodyVarioReady[0]), 1, PLAY_PREEMPTIVE, Config.volume.effect);
	}
	else if (varioMode == VARIO_MODE_LANDING)
	{
		if (nmeaParser.getSpeed() > FLIGHT_START_MIN_SPEED && nmeaParser.getDateTime())
		{
			//
			varioMode = VARIO_MODE_FLYING;
			
			ledFlasher.blink(BTYPE_SHORT_ON_OFF);
			// play take-off melody
			tonePlayer.setMelody(&melodyTakeOff[0], sizeof(melodyTakeOff) / sizeof(melodyTakeOff[0]), 1, PLAY_PREEMPTIVE, Config.volume.effect);
			
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
				tonePlayer.setMelody(&melodyLanding[0], sizeof(melodyLanding) / sizeof(melodyLanding[0]), 1, PLAY_PREEMPTIVE, Config.volume.effect);
				
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

	//
	//while (resStackUSB.available())
	//	Serial.write(resStackUSB.read());	
}


//
//
//


void setup_ums()
{
	// turn-off GPS & IMU
	keyPowerGPS.disable();
	#if HW_VERSION == HW_VERSION_V1_REV2
	keyPowerIMU.disable();
	#endif // HW_VERSION == HW_VERSION_V1_REV2
	// turn-on SD & BT
	#if HW_VERSION == HW_VERSION_V1_REV2
	keyPowerSD.enable();
	#endif // HW_VERSION == HW_VERSION_V1_REV2
	keyPowerBT.enable();
	delay(100);
	
	
	//
	Serial.end();
	
	//
	SdCard.begin(SDCARD_CS, SD_SCK_HZ(F_CPU/2)); // SPI_QUARTER_SPEED);
	
	uint32_t numberOfBlocks = SdCard.cardSize();
	//SerialDbg.print("Number of Blocks = "); SerialDbg.println(numberOfBlocks);
	
	if (numberOfBlocks)
	{
		usb_mal_init(numberOfBlocks);
		//SerialDbg.print("cardSize = "); SerialDbg.println(numberOfBlocks * 512);

		//
		//SerialDbg.println("USBMassStorage.begin");
		USBMassStorage.begin();	
		
		//
		ledFlasher.blink(BTYPE_BLINK_2_LONG_ON);
		//
		tonePlayer.setBeep(NOTE_G4, 500, 300, 2, Config.volume.effect);
		
		// set mode-tick
		modeTick = millis();
	}
	else
	{
		// synchronous beep!!
		tonePlayer.beep(NOTE_C3, 200, 3, Config.volume.effect);
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
	//if ((millis() - modeTick) > AUTO_SHUTDOWN_THRESHOLD)
	if ((Config.threshold.auto_shutdown_ums) && ((millis() - modeTick) > Config.threshold.auto_shutdown_ums))
	{
		// synchronous beep!!
		tonePlayer.beep(NOTE_C4, 400, 2, Config.volume.effect);
		// shutdown now
		changeDeviceMode(DEVICE_MODE_SHUTDOWN);
	}
}

//
//
//

#define HIGH_BEEP_FREQ 			(1000)
#define LOW_BEEP_FREQ 			(100)
#define BASE_BEEP_DURATION 		(100)

#define BEEP_MEASURE_READY		(1)
#define BEEP_MEASURE_SUCCESS	(2)
#define BEEP_MEASURE_FAIL		(3)

#define MEASURE_READY_DELAY 	(3000)
#define CALIBRATE_READY_DELAY	(1000)
#define CALIBATE_RESET_DELAY	(2000)

void calibration_changeMode(uint8_t mode)
{
	// set mode & reset tick
	calibMode = mode;
	deviceTick = millis();
	
	if (calInteractive)
		resStackBT.push(CMD_ACCEL_CALIBRATION, RPARAM_CAL_MODE_CHANGED, mode);
}

void calibration_readyMeasure(int beepType)
{
	// ready measure
	Serial.println("ready measure....");
	
	//
	switch (beepType)
	{
	case BEEP_MEASURE_SUCCESS :
		// HIGH BEEP : BBBB_
		tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 5, BASE_BEEP_DURATION, 1, KEY_VOLUME);
		break;
	case BEEP_MEASURE_FAIL :
		// LOW BEEP : bbbb_
		tonePlayer.setBeep(LOW_BEEP_FREQ, BASE_BEEP_DURATION * 5, BASE_BEEP_DURATION, 1, KEY_VOLUME);
		break;
	case BEEP_MEASURE_READY :
	default :
		// HIGH BEEP : B___B___
		tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 4, BASE_BEEP_DURATION, 2, Config.volume.effect);
		break;
	}
	// 0___0___ ...
	ledFlasher.blink(BTYPE_SHORT_ON_LONG_OFF);

	// change mode
	calibration_changeMode(CAL_MODE_MEASURE_READY);
}

void calibration_startMeasure()
{
	// start measure
	Serial.println("start measure....");
	accelCalibrator.startMeasure();
	
	// HIGH BEEP : BBB___BBB___BBB___
	tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 6, BASE_BEEP_DURATION * 3, 3, Config.volume.effect);
	// O_O_ ...
	ledFlasher.blink(BTYPE_SHORT_ON_OFF);

	// change mode
	calibration_changeMode(CAL_MODE_MEASURE);
}

void calibration_readyCalibrate()
{
	// HIGH BEEP : BBBB_
	tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 5, BASE_BEEP_DURATION, 1, KEY_VOLUME);
	
	// change mode
	calibration_changeMode(CAL_MODE_CALIBATE_READY);
}

void calibration_startCalibrate()
{
	// play complete melody
	// HIGH BEEP : BBB_BBB_BBB_
	tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 4, BASE_BEEP_DURATION * 3, 3, Config.volume.effect);
	// OOO_OOO_ ...
	ledFlasher.blink(BTYPE_LONG_ON_OFF);
	
	// change mode
	calibration_changeMode(CAL_MODE_CALIBATE);
}

void setup_calibration()
{
	// turn-off GPS, SD
	keyPowerGPS.disable();
	#if HW_VERSION == HW_VERSION_V1_REV2
	keyPowerSD.disable();
	#endif // HW_VERSION == HW_VERSION_V1_REV2
	// turn-on IMU & BT
	#if HW_VERSION == HW_VERSION_V1_REV2
	keyPowerIMU.enable();
	#endif // HW_VERSION == HW_VERSION_V1_REV2
	keyPowerBT.enable();
	delay(100);
	
	//
	//calibMode = CAL_MODE_INIT;
	resStackBT.push(CMD_ACCEL_CALIBRATION, RPARAM_CAL_START);

	//
	accelCalibrator.init();

	//
	calibration_readyMeasure(BEEP_MEASURE_READY);
}

void loop_calibration()
{
	// led(measure-ready)		BTYPE_SHORT_ON_LONG_OFF
	// led(measure-run)			BTYPE_SHORT_ON_OFF
	// led(complete)			BTYPE_LONG_ON_OFF
	
	// beep(measure-ready)		삑~___삑~___
	// beep(measure-enter)		삑~~~___삑~~~___삑~~~___
	// beep(measure-success)  	삑~~~~~_
	// beep(measure-fail)     	뷕~~~~~_
	// beep(complete)			삑~~~_삑~~~_
	
	//
	if (calibMode == CAL_MODE_MEASURE_READY)
	{
		// empty the FIFO and stabilize the accelerometer
		accelCalibrator.prepareMeasure();

		// 
		if (calInteractive == 0 && millis() - deviceTick > MEASURE_READY_DELAY)
		{
			// start measure
			calibration_startMeasure();
		}
		// else : wait measure trigger
	}
	else if (calibMode == CAL_MODE_MEASURE)
	{
		if (accelCalibrator.continueMeasure())
			return; // continue

		//
		Serial.println("done measure....");
		accelCalibrator.finishMeasure();
		
		// get orientation
		boolean measureValid = false;
		int orient = accelCalibrator.getMeasureOrientation();
		Serial.print("  orientation = "); Serial.println(orient);
		
		if (orient != ACCEL_CALIBRATOR_ORIENTATION_EXCEPTION)
		{
			// push measure
			measureValid = accelCalibrator.pushMeasure();
			Serial.print("  measurement "); Serial.println(measureValid?"valid":"invalid");
			
			// report measured data
			if (calInteractive)
			{
				resStackBT.push(CMD_ACCEL_CALIBRATION, 
								RPARAM_CAL_MEASURED_RESULT, 
								measureValid, 
								orient, 
								accelCalibrator.measuredAccelSD);
			}
		}
		
		if (! accelCalibrator.canCalibrate())
		{
			// next~
			calibration_readyMeasure(measureValid ? BEEP_MEASURE_SUCCESS : BEEP_MEASURE_FAIL);
		}
		else
		{
			// now we can calibate
			calibration_readyCalibrate();
		}
	}
	else if (calibMode == CAL_MODE_CALIBATE_READY)
	{
		if (calInteractive == 0 && millis() - deviceTick > CALIBRATE_READY_DELAY)
		{
			// start calibrate
			calibration_startCalibrate();
		}
		// else : wait calibrate trigger
	}
	else if (calibMode == CAL_MODE_CALIBATE)
	{
		// calibrate & save result
		Serial.println("calibrate!!!");
		accelCalibrator.calibrate();
		
		// report calibrate result
		if (calInteractive)
		{
			resStackBT.push(CMD_ACCEL_CALIBRATION, 
							RPARAM_CAL_DONE,
							accelCalibrator.calibration[0],
							accelCalibrator.calibration[1],
							accelCalibrator.calibration[2]);
		}

		// change mode
		calibration_changeMode((calInteractive == 0) ? CAL_MODE_RESET : CAL_MODE_DONE);
		
		// calibrated accelerometer is reported in CAL_MODE_DONE mode.
		if (calInteractive)
			accelCalibrator.startMeasure();
	}
	else if (calibMode == CAL_MODE_DONE)
	{
		if (accelCalibrator.continueMeasure())
			return; // continue

		//
		float accel[3];
		
		accelCalibrator.finishMeasure();
		accelCalibrator.getCalibratedMeasure(accel);
		accelCalibrator.startMeasure();
		
		// send calibrated measured accelerometer
		resStackBT.push(CMD_ACCEL_CALIBRATION,
						RPARAM_CAL_ACCELEROMETER,
						accel[0], 
						accel[1], 
						accel[2]);
	}
	else if (calibMode == CAL_MODE_RESET)
	{
		if (millis() - deviceTick > CALIBATE_RESET_DELAY)
		{
			Serial.println("calibrate complete!!!");
			Serial.println("reset now....");
			
			// ??? beep
			//
			
			// jobs done. reset now!
			board_reset();
			
			while(1) {}
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
	logger.end(nmeaParser.getDateTime());
	
	//
	deviceTick = millis();
}

void loop_shutdown()
{
	//if (millis() - deviceTick > SHUTDOWN_HOLD_TIME)
	if ((millis() - deviceTick) > Config.threshold.shutdown_holdtime)
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
		logger.end(nmeaParser.getDateTime());
		
		// beep~
		//tonePlayer.setBeep(420, 0, 0, Config.volume.effect);
		tonePlayer.setTone(360, Config.volume.effect);
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
		commandReceiveFlag = 1; //
		
		//Serial.print("dequeue command: ");
		//Serial.write((char)(cmd.code >> 8)); Serial.write((char)(cmd.code & 0x00FF));
		//Serial.print(", "); 
		//Serial.println(cmd.param);
		
		switch(cmd.code)
		{
		case CMD_STATUS 	:
			resStackBT.push(cmd.code, RPARAM_UNAVAILABLE);
			break;
		case CMD_RESET 	:
			// reset!!
			board_reset();
			break;
		case CMD_RUN_BOOTLOADER :
			// reboot -> goto bootloader
			//if (cmd.from != CMD_FROM_KEY)
			//	resStackBT.push(cmd.code, RPARAM_SUCCESS);
			board_reboot();
			break;
		case CMD_SHUTDOWN:
			// shutdown!!
			changeDeviceMode(DEVICE_MODE_SHUTDOWN);
			break;
		case CMD_FIRMWARE_VERSION :
			resStackBT.push(cmd.code, RPARAM_UNAVAILABLE);
			break;
		case CMD_MODE_SWITCH 	:
			commandModeSwitch(&cmd);
			break;
		case CMD_SOUND_LEVEL 	:
			commandSoundLevel(&cmd);
			break;
		case CMD_TONE_TEST 		:
			commandToneTest(&cmd);
			break;
		case CMD_DUMP_SENSOR 	:
			commandSendsorDump(&cmd);
			break;
		case CMD_DUMP_PROPERTY :
			commandDumpProperties(&cmd);
			break;
//		case CMD_BLOCK_GPS_NMEA :
//			btMan.blockNmeaSentence(cmd.param ? );
//			break;
//		case CMD_BLOCK_VARIO_NMEA :
//			btMan.blockNmeaSentence(cmd.param ? );
//			break;
		case CMD_QUERY_PROPERTY 	:
			commandQueryProperty(&cmd);
			break;
		case CMD_UPDATE_PROPERTY 	:
			commandUpdateProperty(&cmd);
			break;
		case CMD_SAVE_PROPERTY :
			Config.writeAll();
			resStackBT.push(cmd.code, RPARAM_SUCCESS);
			break;
		case CMD_RESTORE_PROPERTY :
			Config.reset();
			Config.readAll();
			resStackBT.push(cmd.code, RPARAM_SUCCESS);
			break;
		case CMD_FACTORY_RESET :
			Config.reset();
			Config.writeAll();
			resStackBT.push(cmd.code, RPARAM_SUCCESS);
		#if CONFIG_DEBUG_DUMP
		case CMD_DUMP_CONFIG :
			Config.dump();
			break;
		#endif // CONFIG_DEBUG_DUMP
		case CMD_ACCEL_CALIBRATION :
			commandAccelerometerCalibration(&cmd);
			break;
			
		default :
			resStackBT.push(cmd.code, RPARAM_INVALID_COMMAND);		
			break;
		}
	}	
}

// CMD_MODE_SWITCH
void commandModeSwitch(Command * cmd)
{
	// return current device-mode
	if (cmd->param == PARAM_MS_QUERY)
	{
		resStackBT.push(cmd->code, RPARAM_SW_BASE + deviceMode);
		return;
	}
	
	// change device mode
	if (deviceMode != cmd->param && deviceMode != DEVICE_MODE_SHUTDOWN)
	{
		switch (cmd->param)
		{
		case PARAM_MS_VARIO :
			changeDeviceMode(DEVICE_MODE_VARIO);
			
			if (cmd->from != CMD_FROM_KEY)
				resStackBT.push(cmd->code, RPARAM_SUCCESS);
			break;
		case PARAM_MS_CALIBRATION :
			calInteractive = (cmd->from == CMD_FROM_KEY ? 0 : 1);
			changeDeviceMode(DEVICE_MODE_CALIBRATION);
			
			if (cmd->from != CMD_FROM_KEY)
				resStackBT.push(cmd->code, RPARAM_SUCCESS);
			break;
		case PARAM_MS_UMS :
			if (/*keyUSB.read() == INPUT_ACTIVE &&*/ logger.isInitialized())
			{
				changeDeviceMode(DEVICE_MODE_UMS);			
				
				if (cmd->from != CMD_FROM_KEY)
					resStackBT.push(cmd->code, RPARAM_SUCCESS);
			}
			else
			{
				// sd-init failed!! : warning beep~~
				tonePlayer.beep(NOTE_C3, 200, 4, Config.volume.effect);
				
				if (cmd->from != CMD_FROM_KEY)
					resStackBT.push(cmd->code, RPARAM_FAIL);
			}
			break;

		default :
			if (cmd->from != CMD_FROM_KEY)
				resStackBT.push(cmd->code, RPARAM_INVALID_COMMAND);
			break;
		}
	}
	else
	{
		if (cmd->from != CMD_FROM_KEY)
			resStackBT.push(cmd->code, RPARAM_NOT_ALLOWED);
	}
}

// CMD_SOUND_LEVEL
void commandSoundLevel(Command * cmd)
{
	int volume = -1;
	
	switch (cmd->param)
	{
	case PARAM_SL_ALL : 
	case PARAM_SL_VARIO :
	case PARAM_SL_EFFECT :
		//if (cmd->valLen == 1)
		//	volume = cmd->valData[0];
		if (cmd->valLen > 0)
			volume = atoi((char *)cmd->valData);
		break;
	
	case PARAM_SL_LOUD 	:
		volume = MAX_VOLUME;
		break;
	case PARAM_SL_MEDIUM :
		volume = MID_VOLUME;
		break;
	case PARAM_SL_MUTE	:
		volume = MIN_VOLUME;
		break;
	}

	if (MIN_VOLUME <= volume && volume <= MAX_VOLUME)
	{
		switch (cmd->param)
		{
		case PARAM_SL_ALL : 
		case PARAM_SL_LOUD 	:
		case PARAM_SL_MEDIUM :
		case PARAM_SL_MUTE	:
			Config.volume.vario = volume;
			Config.volume.effect = volume;
			break;
			
		case PARAM_SL_VARIO :
			Config.volume.vario = volume;
			break;
		case PARAM_SL_EFFECT :
			Config.volume.effect = volume;
			break;
		}
		
		// update global configuration
		Config.writeBlock(BLOCK_ID_VOLUMNE_SETTINGS);
		// beep~
		tonePlayer.setBeep(460, 800, 400, 3, volume);
		// report success
		if (cmd->from != CMD_FROM_KEY)
			resStackBT.push(cmd->code, RPARAM_SUCCESS);
	}
	else
	{
		if (cmd->from != CMD_FROM_KEY)
			resStackBT.push(cmd->code, RPARAM_INVALID_COMMAND);
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

		toneTestVelocity = 0; // Config.vario.climbThreshold;
		toneTestDelta = TT_UPDATE_DELTA;
	}
	else
	{
		toneTestFlag = false;
	}
	
	resStackBT.push(cmd->code, RPARAM_SUCCESS);
}


// CMD_DUMP_SENSOR
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

void commandQueryProperty(Command * cmd)
{
	for (int i = 0; ParamMap[i].id != PARAM_EOF; i++)
	{
		PARAM_MappingInfo * info = &ParamMap[i];
		
		if (info->id == cmd->param)
		{
			//Serial.print("%QP,"); 
			//Serial.print(cmd->param); 
			//Serial.print(","); 
			
			switch (info->type)
			{
			case PARAM_INT8_T	:
				resStackBT.push(cmd->code, cmd->param, *((int8_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((int8_t *)info->ref));	
				break;
			case PARAM_INT16_T	:
				resStackBT.push(cmd->code, cmd->param, *((int16_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((int16_t *)info->ref));	
				break;
			case PARAM_INT32_T	:
				resStackBT.push(cmd->code, cmd->param, *((int32_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((int32_t *)info->ref));	
				break;
			case PARAM_UINT8_T	:
				resStackBT.push(cmd->code, cmd->param, *((uint8_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((uint8_t *)info->ref));	
				break;
			case PARAM_UINT16_T	:
				resStackBT.push(cmd->code, cmd->param, *((uint16_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((uint16_t *)info->ref));	
				break;
			case PARAM_UINT32_T	:
				resStackBT.push(cmd->code, cmd->param, *((uint32_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((uint32_t *)info->ref));	
				break;
			case PARAM_FLOAT	:
				resStackBT.push(cmd->code, cmd->param, *((float *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((float *)info->ref), MAX_FLOAT_PRECISION);	
				break;
			case PARAM_STRING	:
				resStackBT.push(cmd->code, cmd->param, (char *)info->ref);
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); 
				//{
				//	char * ptr = (char *)info->ref;
				//	
				//	for(int i = 0; *ptr && i < MAX_STRING_SIZE; i++)
				//		Serial.write(ptr[i]);
				//	Serial.println("");
				//}					
				break;
			}

			return;
		}
	}
	
	resStackBT.push(cmd->code, RPARAM_INVALID_PROPERTY/*, error code*/);
	//Serial.println("%ER");
}	

void commandUpdateProperty(Command * cmd)
{
	//Serial.print("commandUpdateParam(");
	//Serial.print("code:"); Serial.print(cmd->code);
	//Serial.print(", param: "); Serial.print(cmd->param);
	//Serial.println(")");
	
	for (int i = 0; ParamMap[i].id != PARAM_EOF; i++)
	{
		PARAM_MappingInfo * info = &ParamMap[i];
		
		if (info->id == cmd->param)
		{
			switch (info->type)
			{
			case PARAM_INT8_T	:
				*((int8_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(cmd->code, cmd->param, *((int8_t *)info->ref));
				break;
			case PARAM_INT16_T	:
				*((int16_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(cmd->code, cmd->param, *((int16_t *)info->ref));
				break;
			case PARAM_INT32_T	:
				*((int32_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(cmd->code, cmd->param, *((int32_t *)info->ref));
				break;
			case PARAM_UINT8_T	:
				*((uint8_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(cmd->code, cmd->param, *((uint8_t *)info->ref));
				break;
			case PARAM_UINT16_T	:
				*((uint16_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(cmd->code, cmd->param, *((uint16_t *)info->ref));
				break;
			case PARAM_UINT32_T	:
				*((uint32_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(cmd->code, cmd->param, *((uint32_t *)info->ref));
				break;
			case PARAM_FLOAT	:
				//Serial.print("UP:"); Serial.print(cmd->param, HEX); Serial.print(" -> "); Serial.println((char *)cmd->valData);
				*((float *)info->ref) = atof((char *)cmd->valData);
				resStackBT.push(cmd->code, cmd->param, *((float *)info->ref));
				break;
			case PARAM_STRING	:
				int len = cmd->valLen < MAX_STRING_SIZE ? cmd->valLen : MAX_STRING_SIZE;
				memset((char *)info->ref, 0, MAX_STRING_SIZE);
				memcpy((char *)info->ref, (char *)cmd->valData, len);
				resStackBT.push(cmd->code, cmd->param, (char *)info->ref);
				break;
			}
			
			return;
		}
	}
	
	//Serial.println("  -> unmatched command");
	resStackBT.push(cmd->code, RPARAM_INVALID_PROPERTY/*, error code*/);
}

void commandDumpProperties(Command * cmd)
{
	//Serial.print("commandDumpProperties: "); Serial.println(dumpProp);
	
	if (dumpProp < 0)
		dumpProp = pushProperties(0);
}

void commandAccelerometerCalibration(Command * cmd)
{
	if (deviceMode == DEVICE_MODE_CALIBRATION && calInteractive != 0)
	{
		switch (cmd->param)
		{
		case PARAM_AC_MEASURE :
			if (calibMode == CAL_MODE_MEASURE_READY || calibMode == CAL_MODE_CALIBATE_READY || calibMode == CAL_MODE_DONE)
				calibration_startMeasure();
			else
				resStackBT.push(cmd->code, RPARAM_NOT_ALLOWED);
			break;
			
		case PARAM_AC_CALIBRATE :
			if (calibMode == CAL_MODE_CALIBATE_READY)
				calibration_startCalibrate();
			else
				resStackBT.push(cmd->code, RPARAM_NOT_ALLOWED);
			break;
			
		case PARAM_AC_STOP :
			// ...
			break;
			
		case PARAM_AC_QUERY_STATUS :
			// ...
			break;
			
		case PARAM_AC_RESET :
			// ...
			break;
		}
	}
	else
	{
		resStackBT.push(cmd->code, RPARAM_INVALID_COMMAND);
		return;
	}
}

int pushProperties(int start)
{
	int i;
	
	//Serial.print("push from "); Serial.println(start);
	
	for (i = start; ParamMap[i].id != PARAM_EOF && ! resStackBT.isFull(); i++)
	{
		//Serial.print("push QP,0x"); Serial.println(ParamMap[i].id, HEX);
		
		switch (ParamMap[i].type)
		{
		case PARAM_INT8_T	:
			resStackBT.push(CMD_DUMP_PROPERTY, ParamMap[i].id, *((int8_t *)ParamMap[i].ref));
			break;
		case PARAM_INT16_T	:
			resStackBT.push(CMD_DUMP_PROPERTY, ParamMap[i].id, *((int16_t *)ParamMap[i].ref));
			break;
		case PARAM_INT32_T	:
			resStackBT.push(CMD_DUMP_PROPERTY, ParamMap[i].id, *((int32_t *)ParamMap[i].ref));
			break;
		case PARAM_UINT8_T	:
			resStackBT.push(CMD_DUMP_PROPERTY, ParamMap[i].id, *((uint8_t *)ParamMap[i].ref));
			break;
		case PARAM_UINT16_T	:
			resStackBT.push(CMD_DUMP_PROPERTY, ParamMap[i].id, *((uint16_t *)ParamMap[i].ref));
			break;
		case PARAM_UINT32_T	:
			resStackBT.push(CMD_DUMP_PROPERTY, ParamMap[i].id, *((uint32_t *)ParamMap[i].ref));
			break;
		case PARAM_FLOAT	:
			resStackBT.push(CMD_DUMP_PROPERTY, ParamMap[i].id, *((float *)ParamMap[i].ref));
			break;
		case PARAM_STRING	:
			resStackBT.push(CMD_DUMP_PROPERTY, ParamMap[i].id, (char *)ParamMap[i].ref);
			break;
		} 
	}
	
	if (ParamMap[i].id == PARAM_EOF && ! resStackBT.isFull())
	{
		resStackBT.push(CMD_DUMP_PROPERTY, PARAM_EOF);
		i = -1;
	}

	return i; // ParamMap[i].id != PARAM_EOF ? i : -1;
}
