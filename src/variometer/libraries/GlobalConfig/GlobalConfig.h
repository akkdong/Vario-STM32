// GlobalConfig.h
//

#ifndef __GLOBALCONFIG_H__
#define __GLOBALCONFIG_H__

#include <DefaultSettings.h>
#include <Arduino.h>
#include <EEPROMDriver.h>


//
// EEPROM block map
//

#define BLOCK_ID_PROFILE			(0)
#define BLOCK_ID_VARIO				(1)
#define BLOCK_ID_KALMAN				(2)
#define BLOCK_ID_CALIBRATION		(3)
#define BLOCK_ID_TIMEZONE			(4)
#define BLOCK_ID_TONE				(5)

#define BLOCK_MASK_PROFILE			0x4215
#define BLOCK_MASK_VARIO			0x7424
#define BLOCK_MASK_KALMAN			0x1642
#define BLOCK_MASK_CALIBRATION		0x2782
#define BLOCK_MASK_TIMEZONE			0x9048
#define BLOCK_MASK_TONE				0x8203

#define BLOCK_SIZE_PROFILE			254
#define BLOCK_SIZE_VARIO			13
#define BLOCK_SIZE_KALMAN			8
#define BLOCK_SIZE_CALIBRATION		12
#define BLOCK_SIZE_TIMEZONE			1
#define BLOCK_SIZE_TONE				0			// not support yet


//
//
//

#define MAX_CONFIG_BLOCK_SIZE		(256)


//
// block relative structure definition
//

typedef struct tagEEPROM_BlockInfo
{
	unsigned short address;
	unsigned short mask;

	unsigned char length;		// payload size
	
} EEPROM_BlockInfo;

typedef struct tagEEPROM_Block
{
	unsigned short	mask;
	unsigned char 	data[1];
	
} EEPROM_Block;

typedef struct tagBLOCK_VarioSettings
{
	//
	unsigned char mask;
	
	//
	float sinkThreshold;
	float climbThreshold;
	float sensitivity;
	
	char volume;	

} BLOCK_VarioSettings;



///////////////////////////////////////////////////////////////////////////////////////////////
// class GlobalConfig

class GlobalConfig
{
public:
	GlobalConfig(EEPROMDriver & driver, unsigned char addr);
	
	void				readAll();
	void				writeAll();
	
	boolean				readBlock(unsigned char blockId, EEPROM_Block * block);
	
	void				updateVarioSettings();
//	void				updateProfile();
//	void				updateKalmanParamter();
//	void				updateTimeZone();
//	void				updateToneTable();

public:
	//
	char				profile[64];
	char *				name;
	char *				pilot;
	char *				glider;
	
	//
	unsigned char 		timeZone; // GMT+9
	
	//
	char				vario_volume;
	float				vario_sinkThreshold;
	float				vario_climbThreshold;
	float				vario_sensitivity;
	
	// vario_tone_table 
	// ...
	
	// vario nmea sentence format
	char				vario_sentence;
	
	//
	float				kalman_sigmaP;
	float				kalman_sigmaA;
	
	// accel calibration data
	float				accel[3]; 
	
	// dirty flag
	char				dirty;
	
private:
	EEPROMDriver &		eepromDriver;
	unsigned char 		eepromAddr;
	
	unsigned char		buffer[MAX_CONFIG_BLOCK_SIZE];
};


//
//
//

extern GlobalConfig Config;


#endif // __GLOBALCONFIG_H__
