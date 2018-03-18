// VarioSettings.h
//

#ifndef __VARIOSETTINGS_H__
#define __VARIOSETTINGS_H__


/////////////////////////////////////////////////////////////////////////////
//

// command code
#define CMD_STATUS				'ST'
#define CMD_RESET				'RS'
#define CMD_SHUTDOWN			'SD'
#define CMD_FIRMWARE_VERSION	'FV'
#define CMD_MODE_SWITCH			'MS'
#define CMD_SOUND_LEVEL			'SL'
#define CMD_TONE_TEST			'TT'
#define CMD_DUMP_SENSOR			'DS'
#define CMD_DUMP_PARAMETERS		'DP'
#define CMD_BLOCK_GPS_NMEA		'BG'
#define CMD_BLOCK_VARIO_NMEA	'BV'
#define CMD_FACTORY_RESET		'FR'
#define CMD_RESTORE_PARAM		'RP'
#define CMD_SAVE_PARAM			'SP'
#define CMD_QUERY_PARAM			'QP'
#define CMD_UPDATE_PARAM		'UP'

// response code
#define RCODE_RESULT			'RS'
#define RCODE_OK				'OK'
#define RCODE_FAIL				'FA'
#define RCODE_ERROR				'ER'
#define RCODE_NOT_READY			'NR'
#define RCODE_UNAVAILABLE		'UA'
#define RCODE_DUMP_PARAM		'DP'
#define RCODE_QUERY_PARAM		'QP'
#define RCODE_UPDATE_PARAM		'UP'


// mode switch
#define PARAM_SW_ICALIBRATION	(1)		// interactive
#define PARAM_SW_CALIBRATION	(2)		// no-interactive
#define PARAM_SW_UMS			(3)
#define PARAM_SW_CONFIG			(4)

// device status
#define PARAM_DS_ALL			(0)
#define PARAM_DS_IMU			(1)
#define PARAM_DS_SDCARD			(2)
#define PARAM_DS_GPS			(3)
#define PARAM_DS_VOLTAGE		(4)

// sensor dump
#define PARAM_DU_NONE			(0)
#define PARAM_DU_ACCELEROMETER	(1<<0)
#define PARAM_DU_GYROSCOPE		(1<<1)
#define PARAM_DU_PRESSURE		(1<<2)
#define PARAM_DU_TEMPERATURE	(1<<3)
#define PARAM_DU_ALL			(0x0F)

// nmea sentence
#define PARAM_NM_UNBLOCK		(0)
#define PARAM_NM_BLOCK			(1)

//  tone test
#define PARAM_TT_STOP			(0)
#define PARAM_TT_START			(1)

// sound level
#define PARAM_LV_MUTE			(0)
#define PARAM_LV_MEDIUM			(1)
#define PARAM_LV_LOUD			(2)

// device reset
#define PARAM_RS_NOW			(0)
#define PARAM_RS_AFTER_SAVE		(1)

#define MAX_PROFILE_SIZE		(16)
#define MAX_STRING_SIZE			(16)
#define TONE_TABLE_COUNT		(12)


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

enum VarioSentence
{
	VSENTENCE_LK8,
	VSENTENCE_LXNAV,
};

enum PARAM_VarType
{
	PARAM_INT8_T,
	PARAM_INT16_T,
	PARAM_INT32_T,
	PARAM_UINT8_T,
	PARAM_UINT16_T,
	PARAM_UINT32_T,
	PARAM_FLOAT,
	PARAM_STRING
};

enum PARAM_Id
{
	// GliderInfo
	PARAM_GLIDER_TYPE					= 0x9001,
	PARAM_GLIDER_MANUFACTURE			= 0x9002,
	PARAM_GLIDER_MODEL					= 0x9003,
	// IGCLogger
	PARAM_LOGGER_ENABLE					= 0x9101,
	PARAM_LOGGER_TAKEOFF_SPEED			= 0x9102,
	PARAM_LOGGER_LANDING_TIMEOUT		= 0x9103,
	PARAM_LOGGER_LOGGING_INTERVAL		= 0x9104,
	PARAM_LOGGER_PILOT					= 0x9105,
	PARAM_LOGGER_TIMEZONE				= 0x9106,
	// VarioSettings
	PARAM_VARIO_SINK_THRESHOLD			= 0x9201,
	PARAM_VARIO_CLIMB_THRESHOLD			= 0x9202,
	PARAM_VARIO_SENSITIVITY				= 0x9203,
	PARAM_VARIO_SENTENCE				= 0x9204,
	PARAM_VARIO_BAROONLY				= 0x9205,
	// ToneTables 
	PARAM_TONETABLE_00_VELOCITY			= 0x9301,
	PARAM_TONETABLE_00_FREQ				= 0x9302,
	PARAM_TONETABLE_00_PERIOD			= 0x9303,
	PARAM_TONETABLE_00_DUTY				= 0x9304,
	PARAM_TONETABLE_01_VELOCITY			= 0x9311,
	PARAM_TONETABLE_01_FREQ				= 0x9312,
	PARAM_TONETABLE_01_PERIOD			= 0x9313,
	PARAM_TONETABLE_01_DUTY				= 0x9314,
	PARAM_TONETABLE_02_VELOCITY			= 0x9321,
	PARAM_TONETABLE_02_FREQ				= 0x9322,
	PARAM_TONETABLE_02_PERIOD			= 0x9323,
	PARAM_TONETABLE_02_DUTY				= 0x9324,
	PARAM_TONETABLE_03_VELOCITY			= 0x9331,
	PARAM_TONETABLE_03_FREQ				= 0x9332,
	PARAM_TONETABLE_03_PERIOD			= 0x9333,
	PARAM_TONETABLE_03_DUTY				= 0x9334,
	PARAM_TONETABLE_04_VELOCITY			= 0x9341,
	PARAM_TONETABLE_04_FREQ				= 0x9342,
	PARAM_TONETABLE_04_PERIOD			= 0x9343,
	PARAM_TONETABLE_04_DUTY				= 0x9344,
	PARAM_TONETABLE_05_VELOCITY			= 0x9351,
	PARAM_TONETABLE_05_FREQ				= 0x9352,
	PARAM_TONETABLE_05_PERIOD			= 0x9353,
	PARAM_TONETABLE_05_DUTY				= 0x9354,
	PARAM_TONETABLE_06_VELOCITY			= 0x9361,
	PARAM_TONETABLE_06_FREQ				= 0x9362,
	PARAM_TONETABLE_06_PERIOD			= 0x9363,
	PARAM_TONETABLE_06_DUTY				= 0x9364,
	PARAM_TONETABLE_07_VELOCITY			= 0x9371,
	PARAM_TONETABLE_07_FREQ				= 0x9372,
	PARAM_TONETABLE_07_PERIOD			= 0x9373,
	PARAM_TONETABLE_07_DUTY				= 0x9374,
	PARAM_TONETABLE_08_VELOCITY			= 0x9381,
	PARAM_TONETABLE_08_FREQ				= 0x9382,
	PARAM_TONETABLE_08_PERIOD			= 0x9383,
	PARAM_TONETABLE_08_DUTY				= 0x9384,
	PARAM_TONETABLE_09_VELOCITY			= 0x9391,
	PARAM_TONETABLE_09_FREQ				= 0x9392,
	PARAM_TONETABLE_09_PERIOD			= 0x9393,
	PARAM_TONETABLE_09_DUTY				= 0x9394,
	PARAM_TONETABLE_10_VELOCITY			= 0x93A1,
	PARAM_TONETABLE_10_FREQ				= 0x93A2,
	PARAM_TONETABLE_10_PERIOD			= 0x93A3,
	PARAM_TONETABLE_10_DUTY				= 0x93A4,
	PARAM_TONETABLE_11_VELOCITY			= 0x93B1,
	PARAM_TONETABLE_11_FREQ				= 0x93B2,
	PARAM_TONETABLE_11_PERIOD			= 0x93B3,
	PARAM_TONETABLE_11_DUTY				= 0x93B4,
	// VolumeSettings
	PARAM_VOLUME_VARIO					= 0x9401,
	PARAM_VOLUME_EFFECT					= 0x9402,
	// ThresholdSettings
	PARAM_THRESHOLD_LOW_BATTERY			= 0x9501,
	PARAM_THRESHOLD_SHUTDOWN_HOLDTIME	= 0x9502,
	PARAM_THRESHOLD_AUTO_SHUTDOWN_VARIO	= 0x9503,
	PARAM_THRESHOLD_AUTO_SHUTDOWN_UMS	= 0x9504,
	// KalmanParameters
	PARAM_KALMAN_VAR_ZMEAS				= 0x9601,
	PARAM_KALMAN_VAR_ZACCEL				= 0x9602,
	PARAM_KALMAN_VAR_ACCELBIAS			= 0x9603,
	PARAM_KALMAN_SIGMA_P				= 0x9611,
	PARAM_KALMAN_SIGMA_A				= 0x9612,
	// CalibrationData
	PARAM_CALDATA_ACCEL_00				= 0x9701,
	PARAM_CALDATA_ACCEL_01				= 0x9702,
	PARAM_CALDATA_ACCEL_02				= 0x9703,
	PARAM_CALDATA_GYRO					= 0x9710,
	PARAM_CALDATA_GYRO_00				= 0x9711,
	PARAM_CALDATA_GYRO_01				= 0x9712,
	PARAM_CALDATA_GYRO_02				= 0x9713,
	PARAM_CALDATA_MAG					= 0x9720,
	PARAM_CALDATA_MAG_00				= 0x9721,
	PARAM_CALDATA_MAG_01				= 0x9722,
	PARAM_CALDATA_MAG_02				= 0x9723,

	//
	PARAM_EOF							= 0xFFFF
};

struct VarioTone
{
	float 			velocity;

	unsigned short 	freq;
	unsigned short 	period;
	unsigned short 	duty;

};


#endif // __VARIOSETTINGS_H__

