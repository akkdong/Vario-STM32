// ParamVarMap.h
//

#ifndef __PARAMVARMAP_H__
#define __PARAMVARMAP_H__

#include "GlobalConfig.h"
#include "CommandStack.h"

/////////////////////////////////////////////////////////////////////////////
//

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
	PARAM_GLIDER_TYPE 					= 0x9001,
	PARAM_GLIDER_MANUFACTURE 			= 0x9002,
	PARAM_GLIDER_MODEL 					= 0x9003,
	// IGCLogger
	PARAM_LOGGER_ENABLE 				= 0x9101,
	PARAM_LOGGER_TAKEOFF_SPEED 			= 0x9102,
	PARAM_LOGGER_LANDING_TIMEOUT 		= 0x9103,
	PARAM_LOGGER_LOGGING_INTERVAL 		= 0x9104,
	PARAM_LOGGER_PILOT 					= 0x9105,
	PARAM_LOGGER_TIMEZONE 				= 0x9106,
	// VarioSettings
	PARAM_VARIO_SINK_THRESHOLD			= 0x9201,
	PARAM_VARIO_CLIMB_THRESHOLD			= 0x9202,
	PARAM_VARIO_SENSITIVITY				= 0x9203,
	PARAM_VARIO_SENTENCE				= 0x9204,
	PARAM_VARIO_BAROONLY				= 0x9205,
	PARAM_VARIO_DAMPING_FACTOR			= 0x9206,
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
	PARAM_VOLUME_TURNON_AT_TAKEOFF		= 0x9403,
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
	PARAM_CALDATA_GYRO_00				= 0x9711,
	PARAM_CALDATA_GYRO_01				= 0x9712,
	PARAM_CALDATA_GYRO_02				= 0x9713,
	PARAM_CALDATA_MAG_00				= 0x9721,
	PARAM_CALDATA_MAG_01				= 0x9722,
	PARAM_CALDATA_MAG_02				= 0x9723,
	
	//
	PARAM_EOF							= 0xFFFF
};

struct PARAM_MappingInfo
{
	uint16_t	id;
	uint16_t	type;
	
	void *		ref;
	
};

extern PARAM_MappingInfo ParamMap[];


/////////////////////////////////////////////////////////////////////////////
//



#endif // __PARAMVARMAP_H__