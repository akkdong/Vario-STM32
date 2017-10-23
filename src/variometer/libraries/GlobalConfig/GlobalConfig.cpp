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
	{ 0x0000, BLOCK_MASK_PROFILE, 		BLOCK_SIZE_PROFILE		},
	{ 0x0100, BLOCK_MASK_VARIO, 		BLOCK_SIZE_VARIO		},
	{ 0x0110, BLOCK_MASK_KALMAN, 		BLOCK_SIZE_KALMAN		},
	{ 0x0120, BLOCK_MASK_CALIBRATION, 	BLOCK_SIZE_CALIBRATION	},
	{ 0x0130, BLOCK_MASK_TIMEZONE, 		BLOCK_SIZE_TIMEZONE		},
	{ 0x0140, BLOCK_MASK_TONE, 			BLOCK_SIZE_TONE			},
};


///////////////////////////////////////////////////////////////////////////////////////////////
// class GlobalConfig

GlobalConfig::GlobalConfig(EEPROMDriver & driver, unsigned char addr) : eepromDriver(driver), eepromAddr(addr)
{
	//
	name = NULL;
	pilot = NULL;
	glider = NULL;
	
	//
	timeZone = 9; // GMT+9
	
	//
	vario_volume = 100; // 0 ~ 100
	
	vario_sinkThreshold = 3.0;
	vario_climbThreshold = 0.2;
	vario_sensitivity = 0.15;
	
	//
	// vario_tone_table 
	
	//
	kalman_sigmaP = 0.1;
	kalman_sigmaA = 0.3;
	
	// accel calibration data
	accel[0] = 0.0; 
	accel[1] = 0.0; 
	accel[2] = 0.0; 
}
	
void GlobalConfig::readAll()
{
	EEPROM_Block * block = (EEPROM_Block *)&buffer[0];

	//
	block->mask = 0x00;
	if (readBlock(BLOCK_ID_PROFILE, block))
	{		
		memcpy(profile, &block->data[0], BlockMap[BLOCK_ID_PROFILE].length);
		
		// !!!! some boundary check code needs
		char * ptr = &profile[0];
		
		if (*ptr)
		{
			name = ptr;			
			for (int i = 0; ptr[i]; ptr++);
			ptr++;
		}
		if (*ptr)
		{
			pilot = ptr;			
			for (int i = 0; ptr[i]; ptr++);
			ptr++;
		}
		if (*ptr)
		{
			glider = ptr;			
			for (int i = 0; ptr[i]; ptr++);
			ptr++;
		}
	}
	
	//
	block->mask = 0x00;
	if (readBlock(BLOCK_ID_VARIO, block))
	{
		BLOCK_VarioSettings * vario = (BLOCK_VarioSettings *)block;
		
		vario_volume = vario->volume;
		vario_sinkThreshold = vario->sinkThreshold;
		vario_climbThreshold = vario->climbThreshold;
		vario_sensitivity = vario->sensitivity;
	}

	//
	// ...
}

boolean GlobalConfig::readBlock(unsigned char id, EEPROM_Block * block)
{
	EEPROM_BlockInfo * info = &BlockMap[id];
	
	block->mask = 0x00;	
	eepromDriver.readBuffer(eepromAddr, info->address, (unsigned char *)&block->mask, sizeof(block->mask));
	
	if (block->mask != info->mask)
		return false;
		
	eepromDriver.readBuffer(eepromAddr, info->address + sizeof(block->mask), (unsigned char *)&block->data[0], info->length);
	return true;
}

void GlobalConfig::updateVarioSettings()
{
	EEPROM_BlockInfo * info = &BlockMap[BLOCK_ID_VARIO];
	BLOCK_VarioSettings * block = (BLOCK_VarioSettings *)&buffer[0];
	
	block->mask = info->mask;
	block->volume = vario_volume;
	block->sinkThreshold = vario_sinkThreshold;
	block->climbThreshold = vario_climbThreshold;
	block->sensitivity = vario_sensitivity;
	
	eepromDriver.writePage(eepromAddr, info->address, (unsigned char *)block, sizeof(BLOCK_VarioSettings));
}
