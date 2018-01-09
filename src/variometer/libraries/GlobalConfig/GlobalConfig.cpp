// GlobalConfig.cpp
//

#include "GlobalConfig.h"


///////////////////////////////////////////////////////////////////////////////////////////////
//

#define DELAY_AFTER_WRITE		(10)		// 100ms

//
// EEPROM block map
//

EEPROM_BlockInfo GlobalConfig::BlockMap[BLOCK_ID_COUNT] = 
{
	{0x0000, BLOCK_MASK_GLIDER_INFO 		, BLOCK_SIZE_GLIDER_INFO 	 	, 0},
	{0x0040, BLOCK_MASK_IGC_LOGGER			, BLOCK_SIZE_IGC_LOGGER	  		, 0},
	{0x0080, BLOCK_MASK_VARIO_SETTINGS		, BLOCK_SIZE_VARIO_SETTINGS	  	, 0},
	{0x00C0, BLOCK_MASK_VARIO_TONE_00		, BLOCK_SIZE_VARIO_TONE_00	  	, 0},
	{0x0100, BLOCK_MASK_VARIO_TONE_01		, BLOCK_SIZE_VARIO_TONE_01	  	, 0},
	{0x0140, BLOCK_MASK_VARIO_TONE_02		, BLOCK_SIZE_VARIO_TONE_02	  	, 0},
	{0x0180, BLOCK_MASK_VARIO_TONE_03		, BLOCK_SIZE_VARIO_TONE_03	  	, 0},
	{0x01C0, BLOCK_MASK_VARIO_TONE_04		, BLOCK_SIZE_VARIO_TONE_04	  	, 0},
	{0x0200, BLOCK_MASK_VARIO_TONE_05		, BLOCK_SIZE_VARIO_TONE_05	  	, 0},
	{0x0240, BLOCK_MASK_VARIO_TONE_06		, BLOCK_SIZE_VARIO_TONE_06	  	, 0},
	{0x0280, BLOCK_MASK_VARIO_TONE_07		, BLOCK_SIZE_VARIO_TONE_07	  	, 0},
	{0x02C0, BLOCK_MASK_VARIO_TONE_08		, BLOCK_SIZE_VARIO_TONE_08	  	, 0},
	{0x0300, BLOCK_MASK_VARIO_TONE_09		, BLOCK_SIZE_VARIO_TONE_09	  	, 0},
	{0x0340, BLOCK_MASK_VARIO_TONE_10		, BLOCK_SIZE_VARIO_TONE_10	  	, 0},
	{0x0380, BLOCK_MASK_VARIO_TONE_11		, BLOCK_SIZE_VARIO_TONE_11	  	, 0},
	{0x03C0, BLOCK_MASK_VOLUMNE_SETTINGS	, BLOCK_SIZE_VOLUMNE_SETTINGS	, 0},
	{0x0400, BLOCK_MASK_THRESHOLD_SETTINGS	, BLOCK_SIZE_THRESHOLD_SETTINGS	, 0},
	{0x0440, BLOCK_MASK_KALMAN_PARAMS		, BLOCK_SIZE_KALMAN_PARAMS	  	, 0},
	{0x0480, BLOCK_MASK_CALIBRATION_DATA	, BLOCK_SIZE_CALIBRATION_DATA 	, 0},	
};

BLOCK_VarioTone GlobalConfig::defaultTone[TONE_TABLE_COUNT] =
{
	/*
	{ -10.0,  200, 200, 100 },
	{  -3.0,  293, 200, 100 },
	{  -2.0,  369, 200, 100 },
	{  -1.0,  440, 200, 100 },
	{   0.0,  400, 600,  50 },
	{   0.8,  495, 500,  50 },
	{  1.10,  786, 288,  50 },
	{  1.63,  925, 219,  50 },
	{  2.68, 1063, 185,  50 },
	{  4.99, 1314, 134,  50 },
	{  7.73, 1570,  91,  50 },
	{ 10.00, 1800,  78,  50 },
	*/
	{ -10.0,  200, 200, 100 },
	{  -3.0,  293, 200, 100 },
	{  -2.0,  369, 200, 100 },
	{  -1.0,  440, 200, 100 },
	{  0.09,  400, 600,  50 },
	{  0.10,  400, 600,  50 },
	{  1.98,  499, 552,  50 },
	{  3.14,  868, 347,  50 },
	{  4.57, 1084, 262,  50 },
	{  6.28, 1354, 185,  50 },
	{  8.15, 1593, 168,  50 },
	{ 10.00, 1800, 150,  50 },
};


///////////////////////////////////////////////////////////////////////////////////////////////
// class GlobalConfig

GlobalConfig::GlobalConfig(EEPROMDriver & driver, unsigned char addr) : eepromDriver(driver), eepromAddr(addr)
{
	//
	reset();
}

void GlobalConfig::reset()
{
	//
	glider.type = GTYPE_PARAGLIDER;
	
	memset(glider.manufacture, 0, sizeof(glider.manufacture));
	memset(glider.model, 0, sizeof(glider.model));
	
	//
	logger.enable = true;
	
	logger.takeoff_speed = FLIGHT_START_MIN_SPEED;		// km/s
	logger.landing_timeout = FLIGHT_LANDING_THRESHOLD;	// ms
	logger.logging_interval = FLIGHT_LOGGING_INTERVAL;	// ms
	
	memset(logger.pilot, 0, sizeof(logger.pilot));
	logger.timezone = VARIOMETER_TIME_ZONE; 			// GMT+9	
	
	//
	vario.sinkThreshold = VARIOMETER_SINKING_THRESHOLD; // -3.0
	vario.climbThreshold = VARIOMETER_CLIMBING_THRESHOLD; // 0.2
	vario.sensitivity = VARIOMETER_SENSITIVITY; // 0.1
	
	vario.sentence = VARIOMETER_DEFAULT_NMEA_SENTENCE;
	vario.baroOnly = true;

	// vario_tone_table 
	memcpy(&toneTable[0], &defaultTone[0], sizeof(defaultTone));
	
	
	//
	volume.vario = VARIOMETER_BEEP_VOLUME; // 0 ~ 100
	volume.effect = VARIOMETER_EFFECT_VOLUME;
	
	//
	threshold.low_battery = LOW_BATTERY_THRESHOLD;
	threshold.shutdown_holdtime = SHUTDOWN_HOLD_TIME;
	threshold.auto_shutdown_vario = AUTO_SHUTDOWN_THRESHOLD;
	threshold.auto_shutdown_ums = AUTO_SHUTDOWN_THRESHOLD;
	
	//
	#if VARIOMETER_CLASSS == CLASS_KALMANVARIO
	kalman.var_zmeas = KF_ZMEAS_VARIANCE;
	kalman.var_zaccel = KF_ZACCEL_VARIANCE;
	kalman.var_accelbias = KF_ACCELBIAS_VARIANCE;	
	#else
	kalman.sigmaP = POSITION_MEASURE_STANDARD_DEVIATION; // 0.1
	kalman.sigmaA = ACCELERATION_MEASURE_STANDARD_DEVIATION; // 0.3
	#endif // VARIOMETER_CLASSS == CLASS_KALMANVARIO
	
	// calibration data
	
	memset(&calData.accel[0], 0, sizeof(calData.accel)); // accelerometer
	memset(&calData.gyro[0], 0, sizeof(calData.gyro)); // gyro
	memset(&calData.mag[0], 0, sizeof(calData.mag)); // magnet
	
	//
	BlockMap[BLOCK_ID_GLIDER_INFO].block		= &glider;
	BlockMap[BLOCK_ID_IGC_LOGGER].block			= &logger;
	BlockMap[BLOCK_ID_VARIO_SETTINGS].block		= &vario;
	BlockMap[BLOCK_ID_VARIO_TONE_00].block		= &toneTable[0];
	BlockMap[BLOCK_ID_VARIO_TONE_01].block		= &toneTable[1];
	BlockMap[BLOCK_ID_VARIO_TONE_02].block		= &toneTable[2];
	BlockMap[BLOCK_ID_VARIO_TONE_03].block		= &toneTable[3];
	BlockMap[BLOCK_ID_VARIO_TONE_04].block		= &toneTable[4];
	BlockMap[BLOCK_ID_VARIO_TONE_05].block		= &toneTable[5];
	BlockMap[BLOCK_ID_VARIO_TONE_06].block		= &toneTable[6];
	BlockMap[BLOCK_ID_VARIO_TONE_07].block		= &toneTable[7];
	BlockMap[BLOCK_ID_VARIO_TONE_08].block		= &toneTable[8];
	BlockMap[BLOCK_ID_VARIO_TONE_09].block		= &toneTable[9];
	BlockMap[BLOCK_ID_VARIO_TONE_10].block		= &toneTable[10];
	BlockMap[BLOCK_ID_VARIO_TONE_11].block		= &toneTable[11];
	BlockMap[BLOCK_ID_VOLUMNE_SETTINGS].block	= &volume;
	BlockMap[BLOCK_ID_THRESHOLD_SETTINGS].block	= &threshold;
	BlockMap[BLOCK_ID_KALMAN_PARAMS].block		= &kalman;
	BlockMap[BLOCK_ID_CALIBRATION_DATA].block	= &calData;
}
	
void GlobalConfig::readAll()
{
	for (int i = 0; i < BLOCK_ID_COUNT; i++)
	{
		readBlock(i);
		//delay(10);
	}
}

void GlobalConfig::writeAll()
{
	for (int i = 0; i < BLOCK_ID_COUNT; i++)
	{
		writeBlock(i);	
		//delay(1);
	}
}

boolean GlobalConfig::readBlock(unsigned char id)
{
	EEPROM_BlockInfo * info = &BlockMap[id];
	unsigned short mask = 0x0000;
	
	eepromDriver.readBuffer(eepromAddr, info->address, (unsigned char *)&mask, sizeof(mask));
	//Serial.print("readBlock : "); Serial.println(id);
	//Serial.print("block->mask = 0x"); Serial.println(mask, HEX);
	//Serial.print("info->mask = 0x"); Serial.println(info->mask, HEX);
	if (mask != info->mask)
		return false;
		
	eepromDriver.readBuffer(eepromAddr, info->address + sizeof(mask), (unsigned char *)info->block, info->length);
	return true;
}

void GlobalConfig::writeBlock(unsigned char id)
{
	EEPROM_BlockInfo * info = &BlockMap[id];
	
	//Serial.print("writeBlock : "); Serial.println(id);
	//Serial.print("info->address = 0x"); Serial.println(info->address, HEX);
	//Serial.print("info->mask = 0x"); Serial.println(info->mask, HEX);
	//Serial.print("info->length = "); Serial.println(info->length);
	
	EEPROM_Block * block = GlobalConfig::getBlock();
	block->mask = info->mask;
	memcpy(&block->data[0], info->block, info->length);
	
	eepromDriver.writeBuffer(eepromAddr, info->address, (unsigned char *)block, info->length + sizeof(info->mask));
}

void GlobalConfig::updateVarioVolume(int vol)
{
	// 
	volume.vario = vol;
	
	//
	writeBlock(BLOCK_ID_VOLUMNE_SETTINGS);
}

#if CONFIG_DEBUG_DUMP
void GlobalConfig::dump()
{
	Serial.print("Glider.type = "); Serial.println(glider.type);
	Serial.print("Glider.manufacture = "); Serial.println(glider.manufacture);
	Serial.print("Glider.model = "); Serial.println(glider.model);
	
	Serial.print("Logger.enable = "); Serial.println((int)logger.enable);
	Serial.print("Logger.takeoff_speed = "); Serial.println(logger.takeoff_speed);
	Serial.print("Logger.landing_timeout = "); Serial.println(logger.landing_timeout);
	Serial.print("Logger.logging_interval = "); Serial.println(logger.logging_interval);
	Serial.print("Logger.pilot = "); Serial.println(logger.pilot);
	Serial.print("Logger.timezone = "); Serial.println((int)logger.timezone);
	
	Serial.print("Vario.sinkThreshold = "); Serial.println(vario.sinkThreshold);
	Serial.print("Vario.climbThreshold = "); Serial.println(vario.climbThreshold);
	Serial.print("Vario.sensitivity = "); Serial.println(vario.sensitivity);
	Serial.print("Vario.sentence = "); Serial.println(vario.sentence);
	Serial.print("Vario.baroOnly = "); Serial.println(vario.baroOnly);
	
	Serial.print("Volume.vario = "); Serial.println(volume.vario);
	Serial.print("Volume.effect = "); Serial.println(volume.effect);
	
	Serial.print("Threshold.low_battery = "); Serial.println(threshold.low_battery);
	Serial.print("Threshold.shutdown_holdtime = "); Serial.println(threshold.shutdown_holdtime);
	Serial.print("Threshold.auto_shutdown_vario = "); Serial.println(threshold.auto_shutdown_vario);
	Serial.print("Threshold.auto_shutdown_ums = "); Serial.println(threshold.auto_shutdown_ums);
	
	Serial.print("Kalman.var_zmeas = "); Serial.println(kalman.var_zmeas);
	Serial.print("Kalman.var_zaccel = "); Serial.println(kalman.var_zaccel);
	Serial.print("Kalman.var_accelbias = "); Serial.println(kalman.var_accelbias);
	
	Serial.print("CalData.accel[0] = "); Serial.println(calData.accel[0]);
	Serial.print("CalData.accel[0] = "); Serial.println(calData.accel[1]);
	Serial.print("CalData.accel[0] = "); Serial.println(calData.accel[2]);
	Serial.print("CalData.gyro[0] = "); Serial.println(calData.gyro[0]);
	Serial.print("CalData.gyro[0] = "); Serial.println(calData.gyro[1]);
	Serial.print("CalData.gyro[0] = "); Serial.println(calData.gyro[2]);
	Serial.print("CalData.mag[0] = "); Serial.println(calData.mag[0]);
	Serial.print("CalData.mag[0] = "); Serial.println(calData.mag[1]);
	Serial.print("CalData.mag[0] = "); Serial.println(calData.mag[2]);
	
	Serial.println("ToneTable[] = ");
	for (int i = 0; i < sizeof(toneTable) / sizeo(toneTable[0]); i++)
	{
		Serial.print("    [");
		Serial.print(toneTable[i].velocity);
		Serial.print(", ");
		Serial.print(toneTable[i].freq);
		Serial.print(", ");
		Serial.print(toneTable[i].period);
		Serial.print(", ");
		Serial.print(toneTable[i].duty);
		Serial.println("]");
	}
}
#endif // CONFIG_DEBUG_DUMP