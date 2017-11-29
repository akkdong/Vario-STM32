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

#define MAX_CONFIG_BLOCK_SIZE		(64)
#define MAX_PROFILE_SIZE			(16)
#define TONE_TABLE_COUNT			(12)


//
// EEPROM block map
//

#define BLOCK_SIZE_PROFILE_MODEL 	(sizeof(BLOCK_Profile))	// x + sizeof(mask)
#define BLOCK_SIZE_PROFILE_PILOT	(sizeof(BLOCK_Profile))	// x + sizeof(mask)
#define BLOCK_SIZE_PROFILE_GLIDER	(sizeof(BLOCK_Profile))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_SETTINGS	(sizeof(BLOCK_VarioSettings))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TIMEZONE	(sizeof(BLOCK_VarioTimezone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_VOLUMNE	(sizeof(BLOCK_VarioVolume))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_00	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_01	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_02	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_03	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_04	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_05	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_06	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_07	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_08	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_09	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_10	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_VARIO_TONE_11	(sizeof(BLOCK_VarioTone))	// x + sizeof(mask)
#define BLOCK_SIZE_KALMAN_PARAMS	(sizeof(BLOCK_KalmanParameters))	// x + sizeof(mask)
#define BLOCK_SIZE_CALIBRATION_DATA	(sizeof(BLOCK_CalibrationData))	// x + sizeof(mask)

enum EEPROM_BlockId
{
	BLOCK_ID_PROFILE_MODEL = 0,
	BLOCK_ID_PROFILE_PILOT,
	BLOCK_ID_PROFILE_GLIDER,
	BLOCK_ID_VARIO_SETTINGS,
	BLOCK_ID_VARIO_TIMEZONE,
	BLOCK_ID_VARIO_VOLUMNE,
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
	BLOCK_ID_KALMAN_PARAMS,
	BLOCK_ID_CALIBRATION_DATA,
	BLOCK_ID_COUNT
};

enum EEPROM_BlockMask
{
	BLOCK_MASK_PROFILE_MODEL 	= 0x5301,
	BLOCK_MASK_PROFILE_PILOT	= 0x5302,
	BLOCK_MASK_PROFILE_GLIDER	= 0x5303,
	BLOCK_MASK_VARIO_SETTINGS	= 0x5425,
	BLOCK_MASK_VARIO_TIMEZONE	= 0x5411,
	BLOCK_MASK_VARIO_VOLUMNE	= 0x5451,
	BLOCK_MASK_VARIO_TONE_00	= 0x55E1,
	BLOCK_MASK_VARIO_TONE_01	= 0x55E2,
	BLOCK_MASK_VARIO_TONE_02	= 0x55E3,
	BLOCK_MASK_VARIO_TONE_03	= 0x55E4,
	BLOCK_MASK_VARIO_TONE_04	= 0x55E5,
	BLOCK_MASK_VARIO_TONE_05	= 0x55E6,
	BLOCK_MASK_VARIO_TONE_06	= 0x55E7,
	BLOCK_MASK_VARIO_TONE_07	= 0x55E8,
	BLOCK_MASK_VARIO_TONE_08	= 0x55E9,
	BLOCK_MASK_VARIO_TONE_09	= 0x55EA,
	BLOCK_MASK_VARIO_TONE_10	= 0x55EB,
	BLOCK_MASK_VARIO_TONE_11	= 0x55EC,
	BLOCK_MASK_KALMAN_PARAMS	= 0xA602,
	BLOCK_MASK_CALIBRATION_DATA	= 0xBC06,
};

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

typedef struct tagBLOCK_Profile
{
	//
	unsigned short mask;
	
	//
	unsigned char  name[MAX_PROFILE_SIZE];

} BLOCK_Profile;

typedef struct tagBLOCK_VarioSettings
{
	//
	unsigned short mask;
	
	//
	float sinkThreshold;
	float climbThreshold;
	float sensitivity;
	
	uint8_t baroOnly;

} BLOCK_VarioSettings;

typedef struct tagBLOCK_VarioTimezone
{
	//
	unsigned short mask;
	
	//
	unsigned char timezone;

} BLOCK_VarioTimezone;

typedef struct tagBLOCK_VarioVolume
{
	//
	unsigned short mask;
	
	//
	char volume;

} BLOCK_VarioVolume;


typedef struct tagVarioTone
{
	float 	velocity;
	unsigned short 	freq;
	unsigned short 	period;
	unsigned short 	duty;
	
} VarioTone;

typedef struct tagBLOCK_VarioTone
{
	//
	unsigned short mask;
	
	//
	VarioTone tone;

} BLOCK_VarioTone;

typedef struct tagBLOCK_KalmanParameters
{
	//
	unsigned short mask;
	
	//
	float sigmaP;
	float sigmaA;
	
} BLOCK_KalmanParameters;

typedef struct tagBLOCK_CalibrationData
{
	//
	unsigned short mask;
	
	//
	float accel[3];
	float gyro[3];
	
} BLOCK_CalibrationData;



///////////////////////////////////////////////////////////////////////////////////////////////
// class GlobalConfig

class GlobalConfig
{
public:
	GlobalConfig(EEPROMDriver & driver, unsigned char addr);
	
	void				reset();
	
	void				readAll();
	void				writeAll();
	
	boolean				readBlock(unsigned char blockId, EEPROM_Block * block);
//	EEPROM_Block *		readBlock(unsigned char blockId);
	boolean				writeBlock(unsigned char blockId, EEPROM_Block * block);
	
	EEPROM_Block *		getBlock(); // return free block

	boolean				writeProfile();
	boolean				writeVarioSettings();
	boolean				writeVarioTimezone();
	boolean				writeVarioVolume();
	boolean				writeVarioToneTable();
	boolean				writeKalmanParamters();
	boolean 			writeCalibrationData();
	
	void				updateVarioVolume(int volume);
//	void				updateProfile();
//	void				updateVarioSettings(float sink, float climb, float sensitive, char vol);
//	void				updateVarioToneTable();
//	void				updateKalmanParamters(float sigmaP, float sigmaA);
//	void 				updateCalibrationData(float * calData);

public:
	// block: profile_model
	char				profile_model[MAX_PROFILE_SIZE];
	
	// block: profile_pilot
	char				profile_pilot[MAX_PROFILE_SIZE];
	
	// block: profile_glider
	char				profile_glider[MAX_PROFILE_SIZE];

	// block: vario_settings
	float				vario_sinkThreshold;
	float				vario_climbThreshold;
	float				vario_sensitivity;
//	unsigned char		vario_sentence;		// sentence type: LK8 or LXNAV
	uint8_t				vario_baroOnly;

	// block: vario_timezone
	char 				vario_timezone; 	// GMT+9
	
	// block: vario_volume
	unsigned char		vario_volume;
	
	// vario_tone_table 
	VarioTone			vario_tone[TONE_TABLE_COUNT];
	
	// threshold
	// int threshold_auto_shutdown;
	// int threshold_stable_sinking;
	// int threshold_stable_climibing;	
	// int threshold_start_flight;
	// int threshold_landing_timeout;
	// int threshold_shutdown_holdtime;
	// int threshold_low_battery;
	//
	// int threshold_auto_shutdown_vario;
	// int threshold_auto_shutdown_ums;
	
	// block: kalman_paramters
	float				kalman_sigmaP;
	float				kalman_sigmaA;
	
	// kalman-vario-parameters
	// float variance_zmeas;
	// float variance_acccel;
	// float variance_accelbias;
	
	// bloack: accelerometer/gyro calibration data
	float				accel_calData[3]; 
	float				gyro_calData[3];
	
	// dirty flag
	char				dirty;
	
private:
	EEPROMDriver &		eepromDriver;
	unsigned char 		eepromAddr;
	
	unsigned char		buffer[MAX_CONFIG_BLOCK_SIZE];
};

// inline members

inline EEPROM_Block * GlobalConfig::getBlock()
	{ return (EEPROM_Block *)&buffer[0]; }

	
//
//
//

extern GlobalConfig Config;


#endif // __GLOBALCONFIG_H__
