// GlobalConfig.cpp
//

#include "GlobalConfig.h"


///////////////////////////////////////////////////////////////////////////////////////////////
//


//
// EEPROM block map
//

EEPROM_BlockInfo BlockMap[] = 
{
	{0x0000, BLOCK_MASK_PROFILE_MODEL 	, BLOCK_SIZE_PROFILE_MODEL 	  },
	{0x0020, BLOCK_MASK_PROFILE_PILOT	, BLOCK_SIZE_PROFILE_PILOT	  },
	{0x0040, BLOCK_MASK_PROFILE_GLIDER	, BLOCK_SIZE_PROFILE_GLIDER	  },
	{0x0060, BLOCK_MASK_VARIO_SETTINGS	, BLOCK_SIZE_VARIO_SETTINGS	  },
	{0x0080, BLOCK_MASK_VARIO_TIMEZONE	, BLOCK_SIZE_VARIO_TIMEZONE	  },
	{0x00A0, BLOCK_MASK_VARIO_VOLUMNE	, BLOCK_SIZE_VARIO_VOLUMNE	  },
	{0x00C0, BLOCK_MASK_VARIO_TONE_00	, BLOCK_SIZE_VARIO_TONE_00	  },
	{0x00E0, BLOCK_MASK_VARIO_TONE_01	, BLOCK_SIZE_VARIO_TONE_01	  },
	{0x0100, BLOCK_MASK_VARIO_TONE_02	, BLOCK_SIZE_VARIO_TONE_02	  },
	{0x0120, BLOCK_MASK_VARIO_TONE_03	, BLOCK_SIZE_VARIO_TONE_03	  },
	{0x0140, BLOCK_MASK_VARIO_TONE_04	, BLOCK_SIZE_VARIO_TONE_04	  },
	{0x0160, BLOCK_MASK_VARIO_TONE_05	, BLOCK_SIZE_VARIO_TONE_05	  },
	{0x0180, BLOCK_MASK_VARIO_TONE_06	, BLOCK_SIZE_VARIO_TONE_06	  },
	{0x01A0, BLOCK_MASK_VARIO_TONE_07	, BLOCK_SIZE_VARIO_TONE_07	  },
	{0x01C0, BLOCK_MASK_VARIO_TONE_08	, BLOCK_SIZE_VARIO_TONE_08	  },
	{0x01E0, BLOCK_MASK_VARIO_TONE_09	, BLOCK_SIZE_VARIO_TONE_09	  },
	{0x0200, BLOCK_MASK_VARIO_TONE_10	, BLOCK_SIZE_VARIO_TONE_10	  },
	{0x0220, BLOCK_MASK_VARIO_TONE_11	, BLOCK_SIZE_VARIO_TONE_11	  },
	{0x0240, BLOCK_MASK_KALMAN_PARAMS	, BLOCK_SIZE_KALMAN_PARAMS	  },
	{0x0260, BLOCK_MASK_CALIBRATION_DATA, BLOCK_SIZE_CALIBRATION_DATA },	
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

	vario_sentence = VARIOMETER_DEFAULT_NMEA_SENTENCE;
	vario_timezone = VARIOMETER_TIME_ZONE; // GMT+9
	
	//
	vario_volume = VARIOMETER_BEEP_VOLUME; // 0 ~ 100
	
	//
	// vario_tone_table 
	
	//
	kalman_sigmaP = POSITION_MEASURE_STANDARD_DEVIATION; // 0.1
	kalman_sigmaA = ACCELERATION_MEASURE_STANDARD_DEVIATION; // 0.3
	
	// accel calibration data
	accel[0] = 0.0; 
	accel[1] = 0.0; 
	accel[2] = 0.0; 
	
	//
	dirty = false;
}
	
void GlobalConfig::readAll()
{
	//
	EEPROM_Block * block = (EEPROM_Block *)&buffer[0];

	// profile_model
	if (readBlock(BLOCK_ID_PROFILE_MODEL, block))
		memcpy(&block->data[0], profile_model, BlockMap[BLOCK_ID_PROFILE_MODEL].length - sizeof(block->mask));
	// profile_pilot
	if (readBlock(BLOCK_ID_PROFILE_PILOT, block))
		memcpy(&block->data[0], profile_pilot, BlockMap[BLOCK_ID_PROFILE_PILOT].length - sizeof(block->mask));
	// profile_glider
	if (readBlock(BLOCK_ID_PROFILE_GLIDER, block))
		memcpy(&block->data[0], profile_glider, BlockMap[BLOCK_ID_PROFILE_GLIDER].length - sizeof(block->mask));

	
	// vario-settings
	if (readBlock(BLOCK_ID_VARIO_SETTINGS, block))
	{
		BLOCK_VarioSettings * vario = (BLOCK_VarioSettings *)block;
		
		vario_sinkThreshold = vario->sinkThreshold;
		vario_climbThreshold = vario->climbThreshold;
		vario_sensitivity = vario->sensitivity;
		
		vario_sentence = vario->sentence;
		vario_timezone = vario->timezone;
	}
	// vario-volume
	if (readBlock(BLOCK_ID_VARIO_VOLUMNE, block))
	{
		BLOCK_VarioVolume * vario = (BLOCK_VarioVolume *)block;
		
		vario_volume = vario->volume;
	}
	// vario-tone-table
	// for (i = 0; i < tone_counts; i++)
	//    readBlock(BLOCK_ID_VARIO_TONE_00+i, block)
	//    table[i] = block->tone_info;

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
		
		accel[0] = data->accel[0];
		accel[1] = data->accel[1];
		accel[2] = data->accel[2];
	}	

	//
	dirty = false;
}

void GlobalConfig::writeAll()
{
	// write all parameters
	writeProfile();
	delay(10);

	writeVarioSettings();
	delay(10);

	writeVarioVolume();
	delay(10);

	writeVarioToneTable();
	delay(10);

	writeKalmanParamters();
	delay(10);

	writeCalibrationData();
	delay(10);
	
	//
	dirty = false;
}

boolean GlobalConfig::readBlock(unsigned char id, EEPROM_Block * block)
{
	EEPROM_BlockInfo * info = &BlockMap[id];
	
	block->mask = 0x0000;
	Serial.print("read at "); Serial.println(info->address, HEX);
	eepromDriver.readBuffer(eepromAddr, info->address, (unsigned char *)&block->mask, sizeof(block->mask));
	Serial.print("   block.mask = "); Serial.println(block->mask, HEX);
	Serial.print("   info.mask = "); Serial.println(info->mask, HEX);
	if (block->mask != info->mask)
		return false;
		
	eepromDriver.readBuffer(eepromAddr, info->address + sizeof(block->mask), (unsigned char *)&block->data[0], info->length - sizeof(block->mask));
	return true;
}

boolean GlobalConfig::writeBlock(unsigned char id, EEPROM_Block * block)
{
	EEPROM_BlockInfo * info = &BlockMap[id];
	
	eepromDriver.writePage(eepromAddr, info->address, (unsigned char *)block, info->length);
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
		memcpy(profile_model, block->name, sizeof(profile_model));
		
		writeBlock(BLOCK_ID_PROFILE_MODEL, (EEPROM_Block *)block);
	}
	
	// if not empty
	if (profile_pilot[0]) 
	{
		info = &BlockMap[BLOCK_ID_PROFILE_PILOT];

		block->mask = info->mask;
		memcpy(profile_pilot, block->name, sizeof(profile_pilot));
		
		writeBlock(BLOCK_ID_PROFILE_PILOT, (EEPROM_Block *)block);
	}
	
	// if not empty
	if (profile_glider[0]) 
	{
		info = &BlockMap[BLOCK_ID_PROFILE_GLIDER];

		block->mask = info->mask;
		memcpy(profile_glider, block->name, sizeof(profile_glider));
		
		writeBlock(BLOCK_ID_PROFILE_GLIDER, (EEPROM_Block *)block);
	}
	
	return false;
}

boolean GlobalConfig::writeVarioSettings()
{
	EEPROM_BlockInfo * info = &BlockMap[BLOCK_ID_VARIO_VOLUMNE];
	BLOCK_VarioSettings * block = (BLOCK_VarioSettings *)&buffer[0];
	
	block->mask = info->mask;
	block->sinkThreshold = vario_sinkThreshold;
	block->climbThreshold = vario_climbThreshold;
	block->sensitivity = vario_sensitivity;
	block->sentence = vario_sentence;
	block->timezone = vario_timezone;
	
	return writeBlock(BLOCK_ID_VARIO_VOLUMNE, (EEPROM_Block *)block);
}

boolean GlobalConfig::writeVarioVolume()
{
	EEPROM_BlockInfo * info = &BlockMap[BLOCK_ID_VARIO_VOLUMNE];
	BLOCK_VarioVolume * block = (BLOCK_VarioVolume *)&buffer[0];
	
	block->mask = info->mask;
	block->volume = vario_volume;
	
	return writeBlock(BLOCK_ID_VARIO_VOLUMNE, (EEPROM_Block *)block);
}

boolean GlobalConfig::writeVarioToneTable()
{
	return false;
}

boolean GlobalConfig::writeKalmanParamters()
{
	EEPROM_BlockInfo * info = &BlockMap[BLOCK_ID_KALMAN_PARAMS];
	BLOCK_KalmanParameters * block = (BLOCK_KalmanParameters *)&buffer[0];
	
	block->mask = info->mask;
	block->sigmaP = kalman_sigmaP;
	block->sigmaA = kalman_sigmaA;
	
	return writeBlock(BLOCK_ID_KALMAN_PARAMS, (EEPROM_Block *)block);
}

boolean GlobalConfig::writeCalibrationData()
{
	EEPROM_BlockInfo * info = &BlockMap[BLOCK_ID_CALIBRATION_DATA];
	BLOCK_CalibrationData * block = (BLOCK_CalibrationData *)&buffer[0];
	
	block->mask = info->mask;
	block->accel[0] = accel[0];
	block->accel[1] = accel[1];
	block->accel[2] = accel[2];
	
	return writeBlock(BLOCK_ID_CALIBRATION_DATA, (EEPROM_Block *)block);
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
*/

void GlobalConfig::updateCalibrationData(double * calData)
{
	//
	accel[0] = calData[0];
	accel[1] = calData[1];
	accel[2] = calData[2];
	
	//
	writeCalibrationData();
}
