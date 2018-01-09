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
#include <UsbMassStorage.h>

#include "ParamVarMap.h"


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
void commandQueryParameter(Command * cmd);
void commandUpdateParameter(Command * cmd);
void commandDumpParameters(Command * cmd);

int pushParameters(int start);


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
// dump parameters
//

int		dumpParam = -1;


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

//ResponseStack resStackUSB;
ResponseStack resStackBT;

int	commandReceiveFlag = 0; // when any new command is occured, set this. 


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
	tonePlayer.setTone(NOTE_C3, Config.volume.effect);
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
	if (dumpParam >= 0)
		dumpParam = pushParameters(dumpParam);	
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
	
	// turn-on GPS & BT
	keyPowerGPS.enable();
	keyPowerBT.enable();

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
	tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 2, BASE_BEEP_DURATION, 3, Config.volume.effect);
	
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
				tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 6, BASE_BEEP_DURATION * 3, 1, Config.volume.effect);
			else 
				tonePlayer.setBeep(LOW_BEEP_FREQ, BASE_BEEP_DURATION * 6, BASE_BEEP_DURATION * 3, 1, Config.volume.effect);	
			
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
				
				tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 2, BASE_BEEP_DURATION, 3, Config.volume.effect);
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
		
		switch(cmd.code)
		{
		case CMD_STATUS 	:
			resStackBT.push(RCODE_NOT_READY);
			break;
		case CMD_RESET 	:
			// reset!!
			board_reset();
			break;
		case CMD_SHUTDOWN:
			// shutdown!!
			changeDeviceMode(DEVICE_MODE_SHUTDOWN);
			break;
		case CMD_FIRMWARE_VERSION :
			resStackBT.push(RCODE_NOT_READY);
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
		case CMD_DUMP_PARAMETERS :
			commandDumpParameters(&cmd);
			break;
//		case CMD_BLOCK_GPS_NMEA :
//			btMan.blockNmeaSentence(cmd.param ? );
//			break;
//		case CMD_BLOCK_VARIO_NMEA :
//			btMan.blockNmeaSentence(cmd.param ? );
//			break;
		case CMD_QUERY_PARAM 	:
			commandQueryParameter(&cmd);
			break;
		case CMD_UPDATE_PARAM 	:
			commandUpdateParameter(&cmd);
			break;
		case CMD_SAVE_PARAM :
			Config.writeAll();
			resStackBT.push(RCODE_OK);
			break;
		case CMD_RESTORE_PARAM :
			Config.reset();
			Config.readAll();
			resStackBT.push(RCODE_OK);
			break;
		case CMD_FACTORY_RESET :
			Config.reset();
			Config.writeAll();
			resStackBT.push(RCODE_OK);
		#if CONFIG_DEBUG_DUMP
		case CMD_DUMP_CONFIG :
			Config.dump();
			break;
		#endif // CONFIG_DEBUG_DUMP
			
		default :
			resStackBT.push(RCODE_UNAVAILABLE);		
			break;
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
			resStackBT.push(RCODE_UNAVAILABLE);
			break;
		case PARAM_SW_CALIBRATION  :
			changeDeviceMode(DEVICE_MODE_CALIBRATION);
			resStackBT.push(RCODE_OK);
			break;
		case PARAM_SW_UMS          :
			if (/*keyUSB.read() == INPUT_ACTIVE &&*/ logger.isInitialized())
			{
				changeDeviceMode(DEVICE_MODE_UMS);			
				resStackBT.push(RCODE_OK);
			}
			else
			{
				// sd-init failed!! : warning beep~~
				tonePlayer.beep(NOTE_C3, 200, 4, Config.volume.effect);
				resStackBT.push(RCODE_FAIL);
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
	case PARAM_LV_MEDIUM :
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
		//
		resStackBT.push(RCODE_OK);
	}
	else
	{
		resStackBT.push(RCODE_FAIL);
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
	
	resStackBT.push(RCODE_OK);
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

void commandQueryParameter(Command * cmd)
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
				resStackBT.push(RCODE_QUERY_PARAM, cmd->param, *((int8_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((int8_t *)info->ref));	
				break;
			case PARAM_INT16_T	:
				resStackBT.push(RCODE_QUERY_PARAM, cmd->param, *((int16_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((int16_t *)info->ref));	
				break;
			case PARAM_INT32_T	:
				resStackBT.push(RCODE_QUERY_PARAM, cmd->param, *((int32_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((int32_t *)info->ref));	
				break;
			case PARAM_UINT8_T	:
				resStackBT.push(RCODE_QUERY_PARAM, cmd->param, *((uint8_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((uint8_t *)info->ref));	
				break;
			case PARAM_UINT16_T	:
				resStackBT.push(RCODE_QUERY_PARAM, cmd->param, *((uint16_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((uint16_t *)info->ref));	
				break;
			case PARAM_UINT32_T	:
				resStackBT.push(RCODE_QUERY_PARAM, cmd->param, *((uint32_t *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((uint32_t *)info->ref));	
				break;
			case PARAM_FLOAT	:
				resStackBT.push(RCODE_QUERY_PARAM, cmd->param, *((float *)info->ref));
				//Serial.print("QP:"); Serial.print(cmd->param); Serial.print(" -> "); Serial.println(*((float *)info->ref), MAX_FLOAT_PRECISION);	
				break;
			case PARAM_STRING	:
				resStackBT.push(RCODE_QUERY_PARAM, cmd->param, (char *)info->ref);
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
	
	resStackBT.push(RCODE_ERROR/*, error code*/);
	//Serial.println("%ER");
}	

void commandUpdateParameter(Command * cmd)
{
	for (int i = 0; ParamMap[i].id != PARAM_EOF; i++)
	{
		PARAM_MappingInfo * info = &ParamMap[i];
		
		if (info->id == cmd->param)
		{
			switch (info->type)
			{
			case PARAM_INT8_T	:
				*((int8_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(RCODE_UPDATE_PARAM, cmd->param, *((int8_t *)info->ref));
				break;
			case PARAM_INT16_T	:
				*((int16_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(RCODE_UPDATE_PARAM, cmd->param, *((int16_t *)info->ref));
				break;
			case PARAM_INT32_T	:
				*((int32_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(RCODE_UPDATE_PARAM, cmd->param, *((int32_t *)info->ref));
				break;
			case PARAM_UINT8_T	:
				*((uint8_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(RCODE_UPDATE_PARAM, cmd->param, *((uint8_t *)info->ref));
				break;
			case PARAM_UINT16_T	:
				*((uint16_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(RCODE_UPDATE_PARAM, cmd->param, *((uint16_t *)info->ref));
				break;
			case PARAM_UINT32_T	:
				*((uint32_t *)info->ref) = atoi((char *)cmd->valData);
				resStackBT.push(RCODE_UPDATE_PARAM, cmd->param, *((uint32_t *)info->ref));
				break;
			case PARAM_FLOAT	:
				//Serial.print("UP:"); Serial.print(cmd->param, HEX); Serial.print(" -> "); Serial.println((char *)cmd->valData);
				*((float *)info->ref) = atof((char *)cmd->valData);
				resStackBT.push(RCODE_UPDATE_PARAM, cmd->param, *((float *)info->ref));
				break;
			case PARAM_STRING	:
				int len = cmd->valLen < MAX_STRING_SIZE ? cmd->valLen : MAX_STRING_SIZE;
				memset((char *)info->ref, 0, MAX_STRING_SIZE);
				memcpy((char *)info->ref, (char *)cmd->valData, len);
				resStackBT.push(RCODE_UPDATE_PARAM, cmd->param, (char *)info->ref);
				break;
			}
			
			return;
		}
	}
	
	resStackBT.push(RCODE_ERROR/*, error code*/);
	//Serial.println("%ER");
}

void commandDumpParameters(Command * cmd)
{
	//Serial.print("commandDumpParameters: "); Serial.println(dumpParam);
	
	if (dumpParam < 0)
		dumpParam = pushParameters(0);
}

int pushParameters(int start)
{
	int i;
	
	//Serial.print("push from "); Serial.println(start);
	
	for (i = start; ParamMap[i].id != PARAM_EOF && ! resStackBT.isFull(); i++)
	{
		//Serial.print("push QP,0x"); Serial.println(ParamMap[i].id, HEX);
		
		switch (ParamMap[i].type)
		{
		case PARAM_INT8_T	:
			resStackBT.push(RCODE_DUMP_PARAM, ParamMap[i].id, *((int8_t *)ParamMap[i].ref));
			break;
		case PARAM_INT16_T	:
			resStackBT.push(RCODE_DUMP_PARAM, ParamMap[i].id, *((int16_t *)ParamMap[i].ref));
			break;
		case PARAM_INT32_T	:
			resStackBT.push(RCODE_DUMP_PARAM, ParamMap[i].id, *((int32_t *)ParamMap[i].ref));
			break;
		case PARAM_UINT8_T	:
			resStackBT.push(RCODE_DUMP_PARAM, ParamMap[i].id, *((uint8_t *)ParamMap[i].ref));
			break;
		case PARAM_UINT16_T	:
			resStackBT.push(RCODE_DUMP_PARAM, ParamMap[i].id, *((uint16_t *)ParamMap[i].ref));
			break;
		case PARAM_UINT32_T	:
			resStackBT.push(RCODE_DUMP_PARAM, ParamMap[i].id, *((uint32_t *)ParamMap[i].ref));
			break;
		case PARAM_FLOAT	:
			resStackBT.push(RCODE_DUMP_PARAM, ParamMap[i].id, *((float *)ParamMap[i].ref));
			break;
		case PARAM_STRING	:
			resStackBT.push(RCODE_DUMP_PARAM, ParamMap[i].id, (char *)ParamMap[i].ref);
			break;
		} 
	}
	
	if (ParamMap[i].id == PARAM_EOF && ! resStackBT.isFull())
	{
		resStackBT.push(RCODE_DUMP_PARAM, PARAM_EOF);
		i = -1;
	}

	return i; // ParamMap[i].id != PARAM_EOF ? i : -1;
}
