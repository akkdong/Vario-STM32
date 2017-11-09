// GlobalConfig.cpp
//

#include "GlobalConfig.h"


///////////////////////////////////////////////////////////////////////////////////////////////
//

#define DELAY_AFTER_WRITE		(10)		// 100ms

//
// EEPROM block map
//

EEPROM_BlockInfo BlockMap[] = 
{
	{0x0000, BLOCK_MASK_PROFILE_MODEL 	, BLOCK_SIZE_PROFILE_MODEL 	  },
	{0x0040, BLOCK_MASK_PROFILE_PILOT	, BLOCK_SIZE_PROFILE_PILOT	  },
	{0x0080, BLOCK_MASK_PROFILE_GLIDER	, BLOCK_SIZE_PROFILE_GLIDER	  },
	{0x00C0, BLOCK_MASK_VARIO_SETTINGS	, BLOCK_SIZE_VARIO_SETTINGS	  },
	{0x0100, BLOCK_MASK_VARIO_TIMEZONE	, BLOCK_SIZE_VARIO_TIMEZONE	  },
	{0x0140, BLOCK_MASK_VARIO_VOLUMNE	, BLOCK_SIZE_VARIO_VOLUMNE	  },
	{0x0180, BLOCK_MASK_VARIO_TONE_00	, BLOCK_SIZE_VARIO_TONE_00	  },
	{0x01C0, BLOCK_MASK_VARIO_TONE_01	, BLOCK_SIZE_VARIO_TONE_01	  },
	{0x0200, BLOCK_MASK_VARIO_TONE_02	, BLOCK_SIZE_VARIO_TONE_02	  },
	{0x0240, BLOCK_MASK_VARIO_TONE_03	, BLOCK_SIZE_VARIO_TONE_03	  },
	{0x0280, BLOCK_MASK_VARIO_TONE_04	, BLOCK_SIZE_VARIO_TONE_04	  },
	{0x02C0, BLOCK_MASK_VARIO_TONE_05	, BLOCK_SIZE_VARIO_TONE_05	  },
	{0x0300, BLOCK_MASK_VARIO_TONE_06	, BLOCK_SIZE_VARIO_TONE_06	  },
	{0x0340, BLOCK_MASK_VARIO_TONE_07	, BLOCK_SIZE_VARIO_TONE_07	  },
	{0x0380, BLOCK_MASK_VARIO_TONE_08	, BLOCK_SIZE_VARIO_TONE_08	  },
	{0x03C0, BLOCK_MASK_VARIO_TONE_09	, BLOCK_SIZE_VARIO_TONE_09	  },
	{0x0400, BLOCK_MASK_VARIO_TONE_10	, BLOCK_SIZE_VARIO_TONE_10	  },
	{0x0440, BLOCK_MASK_VARIO_TONE_11	, BLOCK_SIZE_VARIO_TONE_11	  },
	{0x0480, BLOCK_MASK_KALMAN_PARAMS	, BLOCK_SIZE_KALMAN_PARAMS	  },
	{0x04C0, BLOCK_MASK_CALIBRATION_DATA, BLOCK_SIZE_CALIBRATION_DATA },	
};

static VarioTone default_Tone[TONE_TABLE_COUNT] =
{
	{ -10.0,   200, 200, 100 },
	{  -3.0,   293, 200, 100 },
	{  -2.0,   369, 200, 100 },
	{  -1.0,   440, 200, 100 },
	{  -0.5,   475, 600, 100 },
	{   0.0,   493, 600,  50 },
	{   0.37, 1000, 369,  50 },
	{   0.92, 1193, 219,  50 },
	{   1.90, 1324, 151,  50 },
	{   3.01, 1428, 112,  50 },
	{   5.35, 1567, 100,  50 },
	{  10.00, 1687,  83,  50 },
};


///////////////////////////////////////////////////////////////////////////////////////////////
// class GlobalConfig

GlobalConfig::GlobalConfig(EEPROMDriver & driver, unsigned char addr) : eepromDriver(driver), eepromAddr(addr)
{
	//
	memset(profile_model, 0, sizeof(profile_model));
	memset(profile_pilot, 0, sizeof(profile_pilot));
	memset(profile_glider, 0, sizeof(profile_glider));
	
	//
	vario_sinkThreshold = VARIOMETER_SINKING_THRESHOLD; // -3.0
	vario_climbThreshold = VARIOMETER_CLIMBING_THRESHOLD; // 0.2
	vario_sensitivity = VARIOMETER_NEAR_CLIMBING_SENSITIVITY; // 0.5

//	vario_sentence = VARIOMETER_DEFAULT_NMEA_SENTENCE;
	vario_timezone = VARIOMETER_TIME_ZONE; // GMT+9
	
	//
	vario_volume = VARIOMETER_BEEP_VOLUME; // 0 ~ 100
	
	// vario_tone_table 
	memcpy(&vario_tone[0], &default_Tone[0], sizeof(default_Tone));
	
	//
	kalman_sigmaP = POSITION_MEASURE_STANDARD_DEVIATION; // 0.1
	kalman_sigmaA = ACCELERATION_MEASURE_STANDARD_DEVIATION; // 0.3
	
	// accel calibration data
	accel_calData[0] = 0.0; 
	accel_calData[1] = 0.0; 
	accel_calData[2] = 0.0; 
	
	//
	dirty = false;
}
	
void GlobalConfig::readAll()
{
	//
	EEPROM_Block * block = (EEPROM_Block *)&buffer[0];

	// profile_model
	if (readBlock(BLOCK_ID_PROFILE_MODEL, block))
		memcpy(profile_model, &block->data[0], BlockMap[BLOCK_ID_PROFILE_MODEL].length - sizeof(block->mask));
	// profile_pilot
	if (readBlock(BLOCK_ID_PROFILE_PILOT, block))
		memcpy(profile_pilot, &block->data[0], BlockMap[BLOCK_ID_PROFILE_PILOT].length - sizeof(block->mask));
	// profile_glider
	if (readBlock(BLOCK_ID_PROFILE_GLIDER, block))
		memcpy(profile_glider, &block->data[0], BlockMap[BLOCK_ID_PROFILE_GLIDER].length - sizeof(block->mask));

	
	// vario-settings
	if (readBlock(BLOCK_ID_VARIO_SETTINGS, block))
	{
		BLOCK_VarioSettings * vario = (BLOCK_VarioSettings *)block;
		
		vario_sinkThreshold = vario->sinkThreshold;
		vario_climbThreshold = vario->climbThreshold;
		vario_sensitivity = vario->sensitivity;
		
		//vario_sentence = vario->sentence;
		//vario_timezone = vario->timezone;
	}
	// vario-timezone
	if (readBlock(BLOCK_ID_VARIO_TIMEZONE, block))
	{
		BLOCK_VarioTimezone * vario = (BLOCK_VarioTimezone *)block;
		
		vario_timezone = vario->timezone;
	}
	// vario-volume
	if (readBlock(BLOCK_ID_VARIO_VOLUMNE, block))
	{
		BLOCK_VarioVolume * vario = (BLOCK_VarioVolume *)block;
		
		vario_volume = vario->volume;
	}
	// vario-tone-table
	for (int i = 0; i < TONE_TABLE_COUNT; i++)
	{
		if (readBlock(BLOCK_ID_VARIO_TONE_00+i, block))
		{
			BLOCK_VarioTone * vario = (BLOCK_VarioTone *)block;
			
			memcpy(&vario_tone[i], &vario->tone, sizeof(VarioTone));
		}
	}

	//
	if (readBlock(BLOCK_ID_KALMAN_PARAMS, block))
	{
		BLOCK_KalmanParameters * kalman = (BLOCK_KalmanParameters *)block;
		
		kalman_sigmaP = kalman->sigmaP;
		kalman_sigmaA = kalman->sigmaA;
	}

	//
	if (readBlock(BLOCK_ID_CALIBRATION_DATA, block))
	{
		BLOCK_CalibrationData * data = (BLOCK_CalibrationData *)block;
		
		accel_calData[0] = data->accel[0];
		accel_calData[1] = data->accel[1];
		accel_calData[2] = data->accel[2];
	}	

	//
	dirty = false;
}

void GlobalConfig::writeAll()
{
	// write all parameters
	writeProfile();
	writeVarioSettings();
	writeVarioTimezone();
	writeVarioVolume();
	writeVarioToneTable();
	writeKalmanParamters();
	writeCalibrationData();
	
	//
	dirty = false;
}

boolean GlobalConfig::readBlock(unsigned char id, EEPROM_Block * block)
{
	EEPROM_BlockInfo * info = &BlockMap[id];
	
	block->mask = 0x0000;
	eepromDriver.readBuffer(eepromAddr, info->address, (unsigned char *)&block->mask, sizeof(block->mask));
	//Serial.print("block->mask = 0x"); Serial.println(block->mask, HEX);
	//Serial.print("info->mask = 0x"); Serial.println(info->mask, HEX);
	if (block->mask != info->mask)
		return false;
		
	eepromDriver.readBuffer(eepromAddr, info->address + sizeof(block->mask), (unsigned char *)&block->data[0], info->length - sizeof(block->mask));
	return true;
}

boolean GlobalConfig::writeBlock(unsigned char id, EEPROM_Block * block)
{
	EEPROM_BlockInfo * info = &BlockMap[id];
	
	eepromDriver.writePage(eepromAddr, info->address, (unsigned char *)block, info->length);
	delay(DELAY_AFTER_WRITE);
	
	return true;
}

boolean GlobalConfig::writeProfile()
{
	EEPROM_BlockInfo * info;
	BLOCK_Profile * block = (BLOCK_Profile *)&buffer[0];
	
	// if not empty
	if (profile_model[0]) 
	{
		info = &BlockMap[BLOCK_ID_PROFILE_MODEL];

		block->mask = info->mask;
		memcpy(block->name, profile_model, sizeof(profile_model));
		
		//Serial.println("write model");
		writeBlock(BLOCK_ID_PROFILE_MODEL, (EEPROM_Block *)block);
	}
	
	// if not empty
	if (profile_pilot[0]) 
	{
		info = &BlockMap[BLOCK_ID_PROFILE_PILOT];

		block->mask = info->mask;
		memcpy(block->name, profile_pilot, sizeof(profile_pilot));
		
		//Serial.println("write pilot");
		writeBlock(BLOCK_ID_PROFILE_PILOT, (EEPROM_Block *)block);
	}
	
	// if not empty
	if (profile_glider[0]) 
	{
		info = &BlockMap[BLOCK_ID_PROFILE_GLIDER];

		block->mask = info->mask;
		memcpy(block->name, profile_glider, sizeof(profile_glider));
		
		//Serial.println("write glider");
		writeBlock(BLOCK_ID_PROFILE_GLIDER, (EEPROM_Block *)block);
	}
	
	return true;
}

boolean GlobalConfig::writeVarioSettings()
{
	EEPROM_BlockInfo * info = &BlockMap[BLOCK_ID_VARIO_SETTINGS];
	BLOCK_VarioSettings * block = (BLOCK_VarioSettings *)&buffer[0];

	block->mask = info->mask;
	block->sinkThreshold = vario_sinkThreshold;
	block->climbThreshold = vario_climbThreshold;
	block->sensitivity = vario_sensitivity;
	
//	block->sentence = vario_sentence;
//	block->timezone = vario_timezone;
	
	//Serial.println("write vario settings");
	return writeBlock(BLOCK_ID_VARIO_SETTINGS, (EEPROM_Block *)block);
}

boolean GlobalConfig::writeVarioTimezone()
{
	EEPROM_BlockInfo * info = &BlockMap[BLOCK_ID_VARIO_TIMEZONE];
	BLOCK_VarioTimezone * block = (BLOCK_VarioTimezone *)&buffer[0];
	
	block->mask = info->mask;
	block->timezone = vario_timezone;
	
	//Serial.println("write timezone");
	return writeBlock(BLOCK_ID_VARIO_TIMEZONE, (EEPROM_Block *)block);
}

boolean GlobalConfig::writeVarioVolume()
{
	EEPROM_BlockInfo * info = &BlockMap[BLOCK_ID_VARIO_VOLUMNE];
	BLOCK_VarioVolume * block = (BLOCK_VarioVolume *)&buffer[0];
	
	block->mask = info->mask;
	block->volume = vario_volume;
	
	//Serial.println("write vario volume");
	return writeBlock(BLOCK_ID_VARIO_VOLUMNE, (EEPROM_Block *)block);
}

boolean GlobalConfig::writeVarioToneTable()
{
	for (int i = 0; i < TONE_TABLE_COUNT; i++)
	{
		EEPROM_BlockInfo * info = &BlockMap[BLOCK_ID_VARIO_TONE_00+i];
		BLOCK_VarioTone * vario = (BLOCK_VarioTone *)&buffer[0];
		
		vario->mask = info->mask;
		memcpy(&vario->tone, &vario_tone[i], sizeof(VarioTone));
		
		//Serial.print("write vario tone #"); Serial.println(i+1);
		if (! writeBlock(BLOCK_ID_VARIO_TONE_00+i, (EEPROM_Block *)vario))
			return false;
	}
	
	return true;
}

boolean GlobalConfig::writeKalmanParamters()
{
	EEPROM_BlockInfo * info = &BlockMap[BLOCK_ID_KALMAN_PARAMS];
	BLOCK_KalmanParameters * block = (BLOCK_KalmanParameters *)&buffer[0];
	
	block->mask = info->mask;
	block->sigmaP = kalman_sigmaP;
	block->sigmaA = kalman_sigmaA;
	
	//Serial.println("write kalman filter parameters");
	return writeBlock(BLOCK_ID_KALMAN_PARAMS, (EEPROM_Block *)block);
}

boolean GlobalConfig::writeCalibrationData()
{
	EEPROM_BlockInfo * info = &BlockMap[BLOCK_ID_CALIBRATION_DATA];
	BLOCK_CalibrationData * block = (BLOCK_CalibrationData *)&buffer[0];
	
	block->mask = info->mask;
	block->accel[0] = accel_calData[0];
	block->accel[1] = accel_calData[1];
	block->accel[2] = accel_calData[2];
	
	//Serial.println("write accelerometer calibration data");
	return writeBlock(BLOCK_ID_CALIBRATION_DATA, (EEPROM_Block *)block);
}

void GlobalConfig::updateVarioVolume(int volume)
{
	// 
	vario_volume = volume;
	
	//
	writeVarioVolume();
}

/*
void GlobalConfig::updateVarioSettings(float sink, float climb, float sensitive, char vol)
{
	//
	vario_sinkThreshold = sink;
	vario_climbThreshold = climb;
	vario_sensitivity = sensitive;
	
	vario_volume = vol;
	
	//
	writeVarioSettings();
}

void GlobalConfig::updateCalibrationData(float * calData)
{
	//
	accel_calData[0] = calData[0];
	accel_calData[1] = calData[1];
	accel_calData[2] = calData[2];
	
	//
	writeCalibrationData();
}
*/
