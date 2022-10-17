// GlobalConfig.h
//

#ifndef __GLOBALCONFIG_H__
#define __GLOBALCONFIG_H__

#include <DefaultSettings.h>
#include <Arduino.h>
#include <EEPROMDriver.h>


//
//
//

#define MAX_CONFIG_BLOCK_SIZE			(64)	// page size
#define MAX_PROFILE_SIZE				(16)
#define MAX_STRING_SIZE					(16)
#define TONE_TABLE_COUNT				(12)

#define CONFIG_DEBUG_DUMP				(0)


//
// EEPROM block map
//

#define BLOCK_SIZE_GLIDER_INFO			(sizeof(BLOCK_GliderInfo))
#define BLOCK_SIZE_IGC_LOGGER			(sizeof(BLOCK_IGCLogger))
#define BLOCK_SIZE_VARIO_SETTINGS		(sizeof(BLOCK_VarioSettings))
#define BLOCK_SIZE_VARIO_TONE_00		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VARIO_TONE_01		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VARIO_TONE_02		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VARIO_TONE_03		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VARIO_TONE_04		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VARIO_TONE_05		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VARIO_TONE_06		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VARIO_TONE_07		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VARIO_TONE_08		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VARIO_TONE_09		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VARIO_TONE_10		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VARIO_TONE_11		(sizeof(BLOCK_VarioTone))
#define BLOCK_SIZE_VOLUMNE_SETTINGS		(sizeof(BLOCK_VolumeSettings))
#define BLOCK_SIZE_THRESHOLD_SETTINGS	(sizeof(BLOCK_ThresholdSettings))
#define BLOCK_SIZE_KALMAN_PARAMS		(sizeof(BLOCK_KalmanParameters))
#define BLOCK_SIZE_CALIBRATION_DATA		(sizeof(BLOCK_CalibrationData))


enum EEPROM_BlockId
{
	BLOCK_ID_GLIDER_INFO = 0,
	BLOCK_ID_IGC_LOGGER,
	BLOCK_ID_VARIO_SETTINGS,
	BLOCK_ID_VARIO_TONE_00,
	BLOCK_ID_VARIO_TONE_01,
	BLOCK_ID_VARIO_TONE_02,
	BLOCK_ID_VARIO_TONE_03,
	BLOCK_ID_VARIO_TONE_04,
	BLOCK_ID_VARIO_TONE_05,
	BLOCK_ID_VARIO_TONE_06,
	BLOCK_ID_VARIO_TONE_07,
	BLOCK_ID_VARIO_TONE_08,
	BLOCK_ID_VARIO_TONE_09,
	BLOCK_ID_VARIO_TONE_10,
	BLOCK_ID_VARIO_TONE_11,
	BLOCK_ID_VOLUMNE_SETTINGS,
	BLOCK_ID_THRESHOLD_SETTINGS,
	BLOCK_ID_KALMAN_PARAMS,
	BLOCK_ID_CALIBRATION_DATA,
	BLOCK_ID_COUNT
};

enum EEPROM_BlockMask
{
	BLOCK_MASK_GLIDER_INFO 			= 0x5A10,
	BLOCK_MASK_IGC_LOGGER			= 0x5A20,
	BLOCK_MASK_VARIO_SETTINGS		= 0x5A31,
	BLOCK_MASK_VARIO_TONE_00		= 0x5A40,
	BLOCK_MASK_VARIO_TONE_01		= 0x5A41,
	BLOCK_MASK_VARIO_TONE_02		= 0x5A42,
	BLOCK_MASK_VARIO_TONE_03		= 0x5A43,
	BLOCK_MASK_VARIO_TONE_04		= 0x5A44,
	BLOCK_MASK_VARIO_TONE_05		= 0x5A45,
	BLOCK_MASK_VARIO_TONE_06		= 0x5A46,
	BLOCK_MASK_VARIO_TONE_07		= 0x5A47,
	BLOCK_MASK_VARIO_TONE_08		= 0x5A48,
	BLOCK_MASK_VARIO_TONE_09		= 0x5A49,
	BLOCK_MASK_VARIO_TONE_10		= 0x5A4A,
	BLOCK_MASK_VARIO_TONE_11		= 0x5A4B,
	BLOCK_MASK_VOLUMNE_SETTINGS		= 0x5A50,
	BLOCK_MASK_THRESHOLD_SETTINGS	= 0x5A60,
	BLOCK_MASK_KALMAN_PARAMS		= 0x5A70,
	BLOCK_MASK_CALIBRATION_DATA		= 0x5A80,	
};

enum GliderType
{
	GTYPE_UNKNOWNM,
	GTYPE_PARAGLIDER,
	GTYPE_HG_FLEX,
	GTYPE_HG_RIGID,
	GTYPE_SAILPLANE,
	GTYPE_PPG_FOOT,
	GTYPE_PPG_WHEEL
};

// enum VarioSentence
// {
// 	VSENTENCE_UNKNOWN,
// 	VSENTENCE_LK8,
// 	VSENTENCE_LXNAV
// };


//
// block relative structure definition
//

typedef struct tagEEPROM_BlockInfo
{
	unsigned short 	address;
	unsigned short 	mask;

	unsigned char 	length;		// payload size
	void *			block;
	
} EEPROM_BlockInfo;

typedef struct tagEEPROM_Block
{
	unsigned short	mask;
	unsigned char 	data[1];
	
} EEPROM_Block;

typedef struct tagBLOCK_GliderInfo
{
	unsigned char	type;
	
	char			manufacture[MAX_STRING_SIZE];
	char			model[MAX_STRING_SIZE];
	
} BLOCK_GliderInfo;

typedef struct tagBLOCK_IGCLogger
{
	char			enable;
	
	int				takeoff_speed;
	int				landing_timeout;
	int				logging_interval;
	
	char			pilot[MAX_STRING_SIZE];
	char			timezone;
		
} BLOCK_IGCLogger;

typedef struct tagBLOCK_VarioSettings
{
	float 			sinkThreshold;
	float 			climbThreshold;
	float 			sensitivity;
	
	unsigned char	sentence;
	unsigned char	baroOnly;

	float			dampingFactor;

} BLOCK_VarioSettings;


typedef struct tagBLOCK_VarioTone
{
	float 			velocity;
	
	unsigned short 	freq;
	unsigned short 	period;
	unsigned short 	duty;

} BLOCK_VarioTone;

typedef struct tagBLOCK_VolumeSettings
{
	unsigned char	vario;
	unsigned char	effect;

	unsigned char	autoTurnOn;

} BLOCK_VolumeSettings;


typedef struct tagBLOCK_ThresholdSettings
{
	float			low_battery;
	unsigned int	shutdown_holdtime;
	
	unsigned int	auto_shutdown_vario;
	unsigned int	auto_shutdown_ums;

} BLOCK_ThresholdSettings;

typedef struct tagBLOCK_KalmanParameters
{
	#if VARIOMETER_CLASSS == CLASS_KALMANVARIO
	float var_zmeas;
	float var_zaccel;
	float var_accelbias;
	#else
	float sigmaP;
	float sigmaA;
	#endif  // VARIOMETER_CLASSS == CLASS_KALMANVARIO
	
} BLOCK_KalmanParameters;

typedef struct tagBLOCK_CalibrationData
{
	float accel[3];
	float gyro[3];
	float mag[3];
	
} BLOCK_CalibrationData;


///////////////////////////////////////////////////////////////////////////////////////////////
// class GlobalConfig

class GlobalConfig
{
public:
	GlobalConfig(EEPROMDriver & driver, unsigned char addr);
	
	void					reset();
	
	void					writeBlock(unsigned char blockId);
	boolean					readBlock(unsigned char blockId);

	void					writeAll();
	void					readAll();
	
	EEPROM_Block *			getBlock(); // return free block

	void					updateVarioVolume(int volume);
	
	#if CONFIG_DEBUG_DUMP
	void					dump();
	#endif // CONFIG_DEBUG_DUMP

public:
	// blocks
	BLOCK_GliderInfo		glider;
	BLOCK_IGCLogger			logger;
	BLOCK_VarioSettings		vario;
	BLOCK_VarioTone			toneTable[TONE_TABLE_COUNT];
	BLOCK_VolumeSettings 	volume;
	BLOCK_ThresholdSettings	threshold;
	BLOCK_KalmanParameters	kalman;
	BLOCK_CalibrationData	calData;
	
private:
	EEPROMDriver &			eepromDriver;
	unsigned char 			eepromAddr;
	
	unsigned char			buffer[MAX_CONFIG_BLOCK_SIZE];
	
	static EEPROM_BlockInfo BlockMap[BLOCK_ID_COUNT];
	static BLOCK_VarioTone	defaultTone[TONE_TABLE_COUNT];
};

// inline members

inline EEPROM_Block * GlobalConfig::getBlock()
	{ return (EEPROM_Block *)&buffer[0]; }

	
//
//
//

extern GlobalConfig Config;


#endif // __GLOBALCONFIG_H__
