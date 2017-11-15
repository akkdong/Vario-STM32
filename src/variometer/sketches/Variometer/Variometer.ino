// Variometer.ino
//

#include <DefaultSettings.h>
#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <VertVelocity.h>
#include <IMUModule.h>
#include <NmeaParserEx.h>
#include <FunctionKey.h>
#include <InputKey.h>
#include <OutputKey.h>
#include <GlobalConfig.h>
#include <ToneGenerator.h>
#include <TonePlayer.h>
#include <VarioBeeper.h>
#include <SerialEx.h>
#include <VarioSentence.h>
#include <BluetoothMan.h>
#ifdef FEATURE_IGCLOGGING_ENALBE
#include <IGCLogger.h>
#endif // FEATURE_IGCLOGGING_ENALBE
#include <BatteryVoltage.h>
#include <CommandParser.h>
#include <FuncKeyParser.h>
#include <LEDFlasher.h>
#include <SensorReporter.h>
#include <AccelCalibrator.h>

#include <libmaple\nvic.h>

/* not use anymore
// PIN map
//

struct GPIO_PINMODE {
	uint8 			pin;
	WiringPinMode 	mode;
	uint8			state; // output default state : HIGH or LOW
};

GPIO_PINMODE gpio_mode[] = 
{
//	{ PIN_SD_CS			, OUTPUT, HIGH }, // PA4	output, active low
	{ PIN_BT_EN			, OUTPUT, HIGH }, // PB0	output, active low
	{ PIN_GPS_EN		, OUTPUT, HIGH }, // PB1	output, active low
//	{ PIN_USB_EN		, OUTPUT,  LOW }, // PB9	output,	active high
	{ PIN_KILL_PWR		, OUTPUT, HIGH }, // PB14	output, active low
	{ PIN_MCU_STATE		, OUTPUT, HIGH }, // PC13	output, active low(led on)
	{ PIN_MODE_SELECT   , INPUT,  HIGH }, // PC14    input, active HIGH
	{ PIN_FUNC_INPUT	, INPUT,   LOW }, // PB5	 input, active low
	{ PIN_USB_DETECT	, INPUT,  HIGH }, // PB8	 input, active high
	{ PIN_SHDN_INT		, INPUT,   LOW }, // PB15	 input, active low
};
*/

// Objects used by each mode
//
//	Common used objects
//	  Config
//	  ToneGenerator
//	  TonePlayer
//	  FunctionKey
//	  BatteryVoltage
//	  CommandParser(*)
//    LEDNotify(*)
//
//	Vario-mode
//	  IMUModule
//	  IGCLogger
//	  VarioBeeper
//	  TurnOff-Checker(*)
//
//  UMS-mode
//      
//	Calibration-mode
//	  IMUModule
//	
//	Configuration-mode
//	  IMUModule
//	  IGCLogger
//	
//	  
//	Commands
//    by Function Key
//      Mode change : UMS, Calibration(no-interactive calibration)
//      Volume change : Mute, Loud, Quiet
//      Reset
//      
//    by Serial (BT & USB)
//      Mode change : Vario -> Configuration
//      Read/Update configuration parameters
//    	...
//    	accelerometer calibration
//    	vario-tone table
//      Interactive calibration
//      Device monitoring : Sensor(IMU, GPS, ADC, ...) state & data
//      Vario Tone Simulation
//
//	Communication(Serial) Protocol description
//	  1. Device -> Console
//        - nmea sentences
//            SOF($),[SENTENCE],,[DATA1],[DATA2],....[DATAn],[CHKSUM]\r\n
//        - response
//	          SOF(*)[TAG]<,[DATA1],[DATA2],....[DATAn]>\r\n : response data
//            SOF(*)[OK|FAIL]\r\n  : response result
//            SOF(<),[{FILED:VALUE},...] : response parameters
//        - log or message
//            SOF(@)[STRING]\r\n
//    2. Console -> Device
//        - command
//            SOF(#)[CMD],[DATA]\r\n
//        - set parameter
//            SOF(>)[TYPE],[VALUE]\r\n
//
//
//
//
//
//
//

//  Packet from Configuration console
//    1. mode switch
//         "#SW,[1|2|3|4]"
//            1 : start interactive calibration
//            2 : start no-interactive calibration
//            3 : start UMS
//            4 : configuraiton(?)
//         response
//            "*OK\r\n"
//            "*FAIL\r\n"
//    2. device status
//         "#DS<,[0|1|2|3>\r\n"
//            0 : all (imu, sd-card, gps, voltage, ...), default
//            1 : imu
//            2 : sd-card
//            3 : gps
//            4 : voltage
//         response
//            "*STS,[DATA1],[DATA2],...,[DATAn]\r\n"
//            DATAx -> {TYPE:VALUE(V|F)}
//            ex) "*STS,IMU:V,SD:V,GPS:F\r\n"
//    2. sensor dump
//         "#DU,[bitmasks]"
//            0    : none (stop)
//            bit0 : accelerometer
//            bit1 : pressure
//            bit2 : temperature
//            bit3 : voltage
//            bit4 : gps
//            bit5 : v-velocity
//         response : send repeatedly
//            "*ACC,x,y,z\r\n"  
//            "*PRS,x\r\n"  
//            "*TEM,x\r\n"  
//            "*VOL,x\r\n"  
//    2.1. nmea sentence
//         "#NM,[0|1]"
//            0 : stop sending
//            1 : start sending
//         response
//            "*OK\r\n"
//            "*FAIL\r\n"
//	  3. vario tone test : configuration mode only(?)
//         "#TT,[0|1]"
//            0 : stop if is running
//            1 : start tone test
//    4. vario sound level
//         "#SL,[0|1|2]"
//            0 : mute
//            1 : quiet`
//            2 : loud
//         response
//            "*OK\r\n"
//            "*FAIL\r\n"
//    5. reset(restart)
//         "#RS<,[0|1]>\r\n"
//            0 : just reset
//            1 : write parameters to eeprom(in configuration mode)
//         response
//            (no response)
//    6. query parameter
//         "#QU,FIELD\r\n"
//            0   : all
//            1~n : each field
//         response
//            "*PARAM,[DATA1],[DATA2],...,[DATAn]\r\n"
//               DATAx -> {FIELD:VALUE}
//    7. update parameter
//         "#UD,FIELD,VALUE\r\n"
//            FIELD : ...
//            VALUE : BYTE, WORD, DWORD, FLOAT, STRING
//         response
//            "*OK\r\n"
//            "*FAIL\r\n"
//
//  Packet from Firmware downloader
//    DFU....


//	LED status
//	  IMU failed
//	  SDCard failed
//	  No-interactive Calibration
//	    measure
//		measure result (valid, invalid)
//		can calibrate(?)
//	  Firmware update
//	
//	Sound Notify
//	  Device ready
//	  Take-off
//	  Landing
//	  Calibration : start, calibration done, calibration failed, measure valid, measure invalid
//	  Command(key) acquire : replay input
//	  Command(key) done
//
//
// 
//


// parameter map


typedef struct tagParameterMap
{
	uint8_t		id;
	uint8_t		type;
	
	void *		dest;
	
} ParameterMap;

enum ParameterId
{
	PARAMID_VARIO_SINK_THRESHOLD,
	PARAMID_VARIO_CLIMB_THRESHOLD,
	PARAMID_VARIO_SENSITIVITY,
	PARAMID_VARIO_TIMEZONE,
	
};

enum ParameterType
{
	PARAMTYPE_INT8,
	PARAMTYPE_INT16,
	PARAMTYPE_INT32,
	PARAMTYPE_UINT8,
	PARAMTYPE_UINT16,
	PARAMTYPE_UINT32,
	PARAMTYPE_FLOAT,
	PARAMTYPE_STRING,
};

ParameterMap paramMap[] =
{
	{ PARAMID_VARIO_SINK_THRESHOLD, PARAMTYPE_FLOAT, &Config.vario_sinkThreshold },
};



//
//
//

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


uint8_t deviceMode = DEVICE_MODE_VARIO;

uint8_t	varioMode; 		// sub-mode of vario-mode
uint8_t	calibMode;		// sub-mode of calibration-mode

uint32_t deviceTick;	// global tick count

void (* main_loop)(void) = 0;

void vario_setup();
void ums_setup();
void calibration_setup();
void shutdown_setup();

void vario_loop();
void ums_loop();
void calibration_loop();
void shutdown_loop();

// common functions
void processLowBattery();
void processShutdownInterrupt();
void processCommand();


//
// Kalman Filter based Vertical Velocity Calculator
//

VertVelocity vertVel;

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
UnlockCallback I2CDevice::cbUnlock = SensorMS5611::UnlockI2C;

// declare EEPROMDriver
EEPROMDriver eeprom(Wire2);


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
//
//

BluetoothMan btMan(SerialEx1, nmeaParser, varioNmea, sensorReporter);

//
// IGC Logger
//

#ifdef FEATURE_IGCLOGGING_ENALBE
IGCLogger logger;
#endif // FEATURE_IGCLOGGING_ENALBE

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

static Tone startTone[] =
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

static Tone shutdownTone[] = 
{
	{ 460, 500 },
	{   0, 200 },
};

ToneGenerator toneGen;
TonePlayer tonePlayer(toneGen);

VarioBeeper varioBeeper(tonePlayer);


//
//
//

GlobalConfig	Config(eeprom, EEPROM_ADDRESS);


//
//
//

CommandStack cmdStack;

CommandParser cmdParser1(CMD_FROM_USB, Serial, cmdStack); // USB serial parser
CommandParser cmdParser2(CMD_FROM_BT, Serial1, cmdStack); // BT serial parser
FuncKeyParser keyParser(keyFunc, cmdStack, tonePlayer);


//
//
//

void board_init()
{
	// Initialize Serials
	Serial.begin();
	//Serial.begin(BAUDRATE_DEBUG);  	// Serial(USB2Serial) : for debugging
	//while (! Serial);
	
	Serial1.begin(BAUDRATE_BT); 	// Serial1(USART1) : for BT
	while (! Serial1);
	
	Serial2.begin(BAUDRATE_GPS);	// Serial2(USART2) : for GPS
	while (! Serial2);
	
	// Initialize I2C
	Wire1.begin();
	//Wire1.setClock(400000); // 400KHz
	
	Wire2.begin();
	//Wire2.setClock(400000); // 400KHz
	
	// Initialize GPIO
#if 0
	for (int i = 0; i < sizeof(gpio_mode)/sizeof(gpio_mode[0]); i++)
	{
		pinMode(gpio_mode[i].pin, gpio_mode[i].mode);
		
		if (gpio_mode[i].mode == OUTPUT)
			digitalWrite(gpio_mode[i].pin, gpio_mode[i].state);
	}
#else
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
#endif
}

//
//
//

void changeDeviceMode(int mode)
{
	switch ((deviceMode = mode))
	{
	case DEVICE_MODE_VARIO :
		vario_setup();
		main_loop = vario_loop;	
		break;
	#ifdef FEATURE_IGCLOGGING_ENALBE
	case DEVICE_MODE_UMS :
		ums_setup();
		main_loop = ums_loop;
		break;
	#endif // FEATURE_IGCLOGGING_ENALBE
	case DEVICE_MODE_CALIBRATION :
		calibration_setup();
		main_loop = calibration_loop;
		break;
	case DEVICE_MODE_SHUTDOWN :
		shutdown_setup();
		main_loop = shutdown_loop;
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
	delay(100); // some delay for peripheral
	
	//
	Config.readAll();

	// initialize imu module & measure first data
	imu.init();
	
	#if 1
	for (int i = 0; i < 100; i++)
	{
		while (! imu.dataReady());
		imu.updateData();
	}
	#else
	while (! imu.dataReady());
	imu.updateData();
	#endif
	
	// initialize kalman filtered vertical velocity calculator
	vertVel.init(imu.getAltitude(), 
				imu.getVelocity(),
				Config.kalman_sigmaP, // POSITION_MEASURE_STANDARD_DEVIATION,
				Config.kalman_sigmaA, // ACCELERATION_MEASURE_STANDARD_DEVIATION,
				millis());
	
	// Initialize IGC Logger
	#ifdef FEATURE_IGCLOGGING_ENALBE
	logger.init();
	#endif // FEATURE_IGCLOGGING_ENALBE
	
	// enable BT & GPS if it's startted inactive state
	//		keyPowerBT.begin(PIN_KILL_PWR, ACTIVE_LOW, OUTPUT_INACTIVE);
	// 		keyPowerGPS.begin(PIN_KILL_PWR, ACTIVE_LOW, OUTPUT_INACTIVE);
	//
	// keyPowerBT.enable();
	// keyPowerGPS.enable();
	
	// ToneGenerator uses PIN_PWM_H(PA8 : Timer1, Channel1)
	toneGen.begin(PIN_PWM_H);
	
	//
	tonePlayer.setVolume(Config.vario_volume);

	//
	changeDeviceMode(DEVICE_MODE_VARIO);
}

void loop()
{
	// main-loop for each mode
	main_loop();

	
	//
	// commont functions
	//
	
	// process command from serial or key
	processCommand();
	
	// low battery!!
	processLowBattery();
	
	// check shutdown interrupts and prepare shutdown
	processShutdownInterrupt();	
}


//
// process low battery
//

void processLowBattery()
{
	if (deviceMode != DEVICE_MODE_SHUTDOWN)
	{
		if (batVolt.getVoltage() < LOW_BATTERY_THRESHOLD)
		{
			Serial.println("!!Alert!!");
			Serial.println("It's low battery. Device will be shutdown now!!");

			changeDeviceMode(DEVICE_MODE_SHUTDOWN);
		}
	}
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
		#ifdef FEATURE_IGCLOGGING_ENALBE
		if (logger.isLogging())
			logger.end();
		#endif // FEATURE_IGCLOGGING_ENALBE
		
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
	while(cmdStack.getSize())
	{
		Command cmd = cmdStack.dequeue();
		
		switch(cmd.code)
		{
		case CMD_MODE_SWITCH 	:
			// change current mode
			if (deviceMode != cmd.param)
			{
				switch (cmd.param)
				{
				case PARAM_SW_ICALIBRATION :
					// setup
					//setup_calibration();
					// loop
					//main_loop = icalibration_loop();
					return;
				case PARAM_SW_CALIBRATION  :
					changeDeviceMode(DEVICE_MODE_CALIBRATION);
					return;
				#ifdef FEATURE_IGCLOGGING_ENALBE
				case PARAM_SW_UMS          :
					if (keyUSB.read() && logger.isInitialized())
					{
						changeDeviceMode(DEVICE_MODE_UMS);
						return;
					}
					// else cann't change mode : warning beep~~
					break;
				#endif // FEATURE_IGCLOGGING_ENALBE
				}
			}
			break;
		case CMD_DEVICE_STATUS 	:
			break;
		case CMD_SENSOR_DUMP 	:
			btMan.blockSensorData(cmd.param);
			break;
		case CMD_NMEA_SENTENCE  :
			btMan.blockNmeaSentence(cmd.param);
			break;
		case CMD_TONE_TEST 		:
			break;
		case CMD_SOUND_LEVEL 	:
			switch (cmd.param)
			{
			case PARAM_LV_LOUD 	:
				tonePlayer.setVolume(MAX_VOLUME);
				tonePlayer.setBeep(460, 800, 400, 3);
				break;
			case PARAM_LV_QUIET :
				tonePlayer.setVolume(MID_VOLUME);
				tonePlayer.setBeep(460, 800, 400, 3);
				break;
			case PARAM_LV_MUTE	:
			default				: 
				tonePlayer.setVolume(MIN_VOLUME);
				break;
			}

			// save volume
			//Config.vario_volume = tonePlayer.getVolume();
			//Config.writeVarioVolume();
			Config.updateVarioVolume(tonePlayer.getVolume());
			break;
		case CMD_DEVICE_RESET 	:
			break;
		case CMD_QUERY_PARAM 	:
			switch (cmd.param)
			{
			case PARAM_QU_VARIO_SINK_THRESHOLD	: // (1)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.vario_sinkThreshold);
				break;
			case PARAM_QU_VARIO_CLIMB_THRESHOLD	: // (2)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.vario_climbThreshold);
				break;
			case PARAM_QU_VARIO_SENSITIVITY		: // (3)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.vario_sensitivity);
				break;
			//case PARAM_QU_NMEA_SENTENCE			: // (4)
			//	Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.vario_sentence);
			//	break;
			case PARAM_QU_TIME_ZONE             : // (5)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.vario_timezone);
				break;
			case PARAM_QU_KALMAN_SIGMA_P		: // (6)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.kalman_sigmaP);
				break;
			case PARAM_QU_KALMAN_SIGMA_A		: // (7)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.kalman_sigmaA);
				break;
			case PARAM_QU_CALIBRATION_X			: // (8)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.accel_calData[0]);
				break;
			case PARAM_QU_CALIBRATION_Y			: // (9)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.accel_calData[1]);
				break;
			case PARAM_QU_CALIBRATION_Z			: // (10)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.accel_calData[2]);
				break;
			case PARAM_QU_PROFILE_MODEL			: // (11)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.profile_model);
				break;
			case PARAM_QU_PROFILE_PILOT			: // (12)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.profile_pilot);
				break;
			case PARAM_QU_PROFILE_GLIDER		: // (13)
				Serial.print("%QU,"); Serial.print(cmd.param); Serial.print(","); Serial.println(Config.profile_glider);
				break;
			case PARAM_QU_TONE_TABLE			: // (14)
				break;
			default                             :
				Serial.println("!ERR");
				break;
			}
			break;
		case CMD_UPDATE_PARAM 	:
			break;
		}
	}	
}

//
// variometer main function
//

uint32_t lastTick;

void vario_setup()
{
	//
	varioMode = VARIO_MODE_INIT;
	
	// turn-on GPS & BT
	keyPowerGPS.enable();
	keyPowerBT.enable();

	// led flash as init-state
	ledFlasher.blink(BTYPE_LONG_ON_SHORT_OFF);

	// start vario-loop
	tonePlayer.setMelody(&startTone[0], sizeof(startTone) / sizeof(startTone[0]), 1, PLAY_PREEMPTIVE, KEY_VOLUME);
	//lastTick = millis();
}

void vario_loop()
{
	//
	if (imu.dataReady())
	{
		//
		imu.updateData(/* &sensorReporter */);

		//
		vertVel.update(imu.getAltitude(), imu.getVelocity(), millis());
		
		//Serial.println(millis() -  lastTick);
		//lastTick = millis();

		//
		float velocity = vertVel.getVelocity();
		varioBeeper.setVelocity(velocity);
		#if 1
		Serial.println(velocity * 100.0, 2);
		#endif
		
		#ifdef FEATURE_IGCLOGGING_ENALBE
		float position = vertVel.getCalibratedPosition(); // vertVel.getPosition();
		logger.update(position);
		#endif // FEATURE_IGCLOGGING_ENALBE
		
		//
		//if (varioMode != VARIO_MODE_SHUTDOWN)
		{
			static uint32_t tick = millis();

			if (velocity < STABLE_SINKING_THRESHOLD || STABLE_CLIMBING_THRESHOLD < velocity)
				tick = millis(); // reset tick because it's not quiet.
			
			if ((millis() - tick) > AUTO_SHUTDOWN_THRESHOLD)
			{
				Serial.println("Now process auto-shutdown!!");
				
				changeDeviceMode(DEVICE_MODE_SHUTDOWN);
				return;
			}
		}
	}
	
	// read & prase gps sentence
	nmeaParser.update();
	
	// update vario sentence periodically
	if (varioNmea.checkInterval())
		varioNmea.begin(vertVel.getPosition(), vertVel.getVelocity(), imu.getTemperature(), batVolt.getVoltage());

	// send any prepared sentence to BT
	btMan.update();
	
	// IGC setence is available when it received a valid GGA. -> altitude is valid
	if (varioMode == VARIO_MODE_INIT  && nmeaParser.availableIGC())
	{
		// do position calibration
		vertVel.calibratePosition(nmeaParser.getAltitude());
		
		// now ready to fly~~~
		varioMode = VARIO_MODE_LANDING;

		//		
		ledFlasher.blink(BTYPE_LONG_ON_OFF);
		// play reday melody~~~
		//
	}
	else if (varioMode == VARIO_MODE_LANDING)
	{
		if (nmeaParser.getSpeed() > FLIGHT_START_MIN_SPEED)
		{
			//
			varioMode = VARIO_MODE_FLYING;
			
			ledFlasher.blink(BTYPE_SHORT_ON_OFF);
			// play take-off melody
			// ...
			
			#ifdef FEATURE_IGCLOGGING_ENALBE
			// start logging & change mode
			logger.begin(nmeaParser.getDateTime());
			#endif // FEATURE_IGCLOGGING_ENALBE
			
			//
			deviceTick = millis();
		}
	}
	else if (varioMode == VARIO_MODE_FLYING)
	{
		if (nmeaParser.getSpeed() < FLIGHT_START_MIN_SPEED)
		{
			if ((millis() - deviceTick) > FLIGHT_LANDING_THRESHOLD)
			{
				//
				varioMode = VARIO_MODE_LANDING;
				
				//
				ledFlasher.blink(BTYPE_LONG_ON_OFF);
				// play landing melody
				// ...
				
				#ifdef FEATURE_IGCLOGGING_ENALBE
				// stop logging & change mode
				logger.end();
				#endif // FEATURE_IGCLOGGING_ENALBE
			}
		}
		else
		{
			// reset deviceTick
			deviceTick = millis();
		}
	}
	//else if (varioMode == VARIO_MODE_SHUTDOWN)
	//{
	//	if (millis() - deviceTick > SHUTDOWN_HOLD_TIME)
	//	{
	//		tonePlayer.setMute();
	//		keyPowerDev.begin(PIN_KILL_PWR, ACTIVE_LOW, OUTPUT_ACTIVE);
	//		
	//		while(1);
	//	}
	//}

	#ifdef FEATURE_IGCLOGGING_ENALBE
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
	#endif // FEATURE_IGCLOGGING_ENALBE
	
	// beep beep beep!
	tonePlayer.update();
	
	// start voltage measurement periodically
	//batVolt.update();
	
	// MCU State : LED Blinking
	ledFlasher.update();
}


//
//
//

#ifdef FEATURE_IGCLOGGING_ENALBE
void ums_setup()
{
	//
	ledFlasher.blink(BTYPE_BLINK_2_LONG_ON);
	//tonePlayer.setMelody(&startUMS[0], sizeof(startUMS) / sizeof(startUMS[0]), 1, PLAY_PREEMPTIVE, KEY_VOLUME);	
}

void ums_loop()
{
	ledFlasher.update();
	tonePlayer.update();
}
#endif // FEATURE_IGCLOGGING_ENALBE


//
// !!! calibration !!!
//

#define HIGH_BEEP_FREQ 			(1000)
#define LOW_BEEP_FREQ 			(100)
#define BASE_BEEP_DURATION 		(100)

#define MEASURE_DELAY 			(3000)


void calibration_setup()
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

void calibration_loop()
{
	//
	ledFlasher.update();
	tonePlayer.update();
	keyFunc.update();
	
	if (calibMode == CAL_MODE_MEASURE_DELAY)
	{
		if (millis() - deviceTick > MEASURE_DELAY)
			calibMode = CAL_MODE_MEASURE;
	}
	else if (calibMode == CAL_MODE_MEASURE)
	{
		// make measure
		accelCalibrator.measure();
		
		// get orientation
		int orient = accelCalibrator.getMeasureOrientation();
		
		if (orient == ACCEL_CALIBRATOR_ORIENTATION_EXCEPTION)
		{
			// the reversed position launch calibration 
			//
			if( !accelCalibrator.canCalibrate() )
			{
				// can't calibrate. try again~
				tonePlayer.setBeep(LOW_BEEP_FREQ, BASE_BEEP_DURATION * 2, BASE_BEEP_DURATION, 3, KEY_VOLUME);
			}
			else
			{
				// calibrate & save result
				accelCalibrator.calibrate();

				// play completion melody & confirm
				calibMode = CAL_MODE_COMPLETION;
				
				tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 2, BASE_BEEP_DURATION, 3, KEY_VOLUME);
				ledFlasher.blink(BTYPE_SHORT_ON_OFF);
			}
		}
		else
		{
			// push measure
			boolean measureValid = accelCalibrator.pushMeasure();

			// make corresponding beep
			if (measureValid)
				tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 6, BASE_BEEP_DURATION * 3, 1, KEY_VOLUME);
			else 
				tonePlayer.setBeep(LOW_BEEP_FREQ, BASE_BEEP_DURATION * 6, BASE_BEEP_DURATION * 3, 1, KEY_VOLUME);
		}

		if (calibMode == CAL_MODE_MEASURE)
		{
			// go back measure delay
			calibMode = CAL_MODE_MEASURE_DELAY;
			// reset delay tick
			deviceTick = millis();
		}
	}
	else if (calibMode == CAL_MODE_COMPLETION)
	{
		if (keyFunc.fired())
		{
			uint8_t value = keyFunc.getValue();
			
			if (value == 0x21)
			{
				// jobs done. reset now!
				tonePlayer.setTone(LOW_BEEP_FREQ, KEY_VOLUME);
				delay(BASE_BEEP_DURATION * 4);
				
				// reset!!
				nvic_sys_reset();
				while(1);
			}
			
			if (value == 0x40)
			{
				calibMode = CAL_MODE_MEASURE_DELAY;

				// re-calibration
				tonePlayer.setBeep(HIGH_BEEP_FREQ, BASE_BEEP_DURATION * 2, BASE_BEEP_DURATION, 3, KEY_VOLUME);
				// reset delay tick
				deviceTick = millis();		
			}
		}
	}
}

void icalibration_loop()
{
}


//
// shutdown : alsert, clean-up & power-off
//

void shutdown_setup()
{
	//
	ledFlasher.blink(BTYPE_BLINK_3_LONG_OFF);
	tonePlayer.setMelody(&shutdownTone[0], sizeof(shutdownTone) / sizeof(shutdownTone[0]), 10, PLAY_PREEMPTIVE, KEY_VOLUME);

	//
	#ifdef FEATURE_IGCLOGGING_ENALBE
	if (logger.isLogging())
		logger.end();
	#endif // FEATURE_IGCLOGGING_ENALBE
	
	//
	deviceTick = millis();
}

void shutdown_loop()
{
	if (millis() - deviceTick > SHUTDOWN_HOLD_TIME)
	{
		tonePlayer.setMute();
		keyPowerDev.begin(PIN_KILL_PWR, ACTIVE_LOW, OUTPUT_ACTIVE);
		
		while(1);
	}
}
