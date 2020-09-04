// ParamVarMap.cpp
//

#include <Arduino.h>
#include <GlobalConfig.h>

#include "ParamVarMap.h"


PARAM_MappingInfo ParamMap[] =
{
	// GliderInfo
	{ PARAM_GLIDER_TYPE						, PARAM_UINT8_T	, &Config.glider.type 					},
	{ PARAM_GLIDER_MANUFACTURE				, PARAM_STRING	, &Config.glider.manufacture[0] 		},
	{ PARAM_GLIDER_MODEL					, PARAM_STRING	, &Config.glider.model[0] 				},
	// IGCLogger	
	{ PARAM_LOGGER_ENABLE					, PARAM_INT8_T	, &Config.logger.enable 				},
	{ PARAM_LOGGER_TAKEOFF_SPEED			, PARAM_INT32_T	, &Config.logger.takeoff_speed 			},
	{ PARAM_LOGGER_LANDING_TIMEOUT			, PARAM_INT32_T	, &Config.logger.landing_timeout 		},
	{ PARAM_LOGGER_LOGGING_INTERVAL			, PARAM_INT32_T	, &Config.logger.logging_interval 		},
	{ PARAM_LOGGER_PILOT					, PARAM_STRING	, &Config.logger.pilot[0] 				},
	{ PARAM_LOGGER_TIMEZONE					, PARAM_INT8_T	, &Config.logger.timezone 				},
	// VarioSettings
	{ PARAM_VARIO_SINK_THRESHOLD			, PARAM_FLOAT	, &Config.vario.sinkThreshold			},
	{ PARAM_VARIO_CLIMB_THRESHOLD			, PARAM_FLOAT	, &Config.vario.climbThreshold			},
	{ PARAM_VARIO_SENSITIVITY				, PARAM_FLOAT	, &Config.vario.sensitivity				},
	{ PARAM_VARIO_SENTENCE					, PARAM_UINT8_T	, &Config.vario.sentence				},
	{ PARAM_VARIO_BAROONLY					, PARAM_UINT8_T	, &Config.vario.baroOnly				},
	{ PARAM_VARIO_DAMPING_FACTOR			, PARAM_FLOAT	, &Config.vario.dampingFactor			},
	// ToneTables
	{ PARAM_TONETABLE_00_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x00].velocity		},
	{ PARAM_TONETABLE_00_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x00].freq			},
	{ PARAM_TONETABLE_00_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x00].period		},
	{ PARAM_TONETABLE_00_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x00].duty			},
	{ PARAM_TONETABLE_01_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x01].velocity		},
	{ PARAM_TONETABLE_01_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x01].freq			},
	{ PARAM_TONETABLE_01_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x01].period		},
	{ PARAM_TONETABLE_01_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x01].duty			},
	{ PARAM_TONETABLE_02_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x02].velocity		},
	{ PARAM_TONETABLE_02_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x02].freq			},
	{ PARAM_TONETABLE_02_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x02].period		},
	{ PARAM_TONETABLE_02_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x02].duty			},
	{ PARAM_TONETABLE_03_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x03].velocity		},
	{ PARAM_TONETABLE_03_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x03].freq			},
	{ PARAM_TONETABLE_03_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x03].period		},
	{ PARAM_TONETABLE_03_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x03].duty			},
	{ PARAM_TONETABLE_04_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x04].velocity		},
	{ PARAM_TONETABLE_04_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x04].freq			},
	{ PARAM_TONETABLE_04_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x04].period		},
	{ PARAM_TONETABLE_04_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x04].duty			},
	{ PARAM_TONETABLE_05_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x05].velocity		},
	{ PARAM_TONETABLE_05_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x05].freq			},
	{ PARAM_TONETABLE_05_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x05].period		},
	{ PARAM_TONETABLE_05_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x05].duty			},
	{ PARAM_TONETABLE_06_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x06].velocity		},
	{ PARAM_TONETABLE_06_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x06].freq			},
	{ PARAM_TONETABLE_06_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x06].period		},
	{ PARAM_TONETABLE_06_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x06].duty			},
	{ PARAM_TONETABLE_07_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x07].velocity		},
	{ PARAM_TONETABLE_07_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x07].freq			},
	{ PARAM_TONETABLE_07_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x07].period		},
	{ PARAM_TONETABLE_07_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x07].duty			},
	{ PARAM_TONETABLE_08_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x08].velocity		},
	{ PARAM_TONETABLE_08_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x08].freq			},
	{ PARAM_TONETABLE_08_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x08].period		},
	{ PARAM_TONETABLE_08_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x08].duty			},
	{ PARAM_TONETABLE_09_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x09].velocity		},
	{ PARAM_TONETABLE_09_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x09].freq			},
	{ PARAM_TONETABLE_09_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x09].period		},
	{ PARAM_TONETABLE_09_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x09].duty			},
	{ PARAM_TONETABLE_10_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x0A].velocity		},
	{ PARAM_TONETABLE_10_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x0A].freq			},
	{ PARAM_TONETABLE_10_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x0A].period		},
	{ PARAM_TONETABLE_10_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x0A].duty			},
	{ PARAM_TONETABLE_11_VELOCITY			, PARAM_FLOAT	, &Config.toneTable[0x0B].velocity		},
	{ PARAM_TONETABLE_11_FREQ				, PARAM_UINT16_T, &Config.toneTable[0x0B].freq			},
	{ PARAM_TONETABLE_11_PERIOD				, PARAM_UINT16_T, &Config.toneTable[0x0B].period		},
	{ PARAM_TONETABLE_11_DUTY				, PARAM_UINT16_T, &Config.toneTable[0x0B].duty			},
	// VolumeSettings
	{ PARAM_VOLUME_VARIO					, PARAM_UINT8_T	, &Config.volume.vario					},
	{ PARAM_VOLUME_EFFECT					, PARAM_UINT8_T	, &Config.volume.effect					},
	// ThresholdSettings
	{ PARAM_THRESHOLD_LOW_BATTERY			, PARAM_FLOAT	, &Config.threshold.low_battery			},
	{ PARAM_THRESHOLD_SHUTDOWN_HOLDTIME		, PARAM_UINT32_T, &Config.threshold.shutdown_holdtime	},
	{ PARAM_THRESHOLD_AUTO_SHUTDOWN_VARIO	, PARAM_UINT32_T, &Config.threshold.auto_shutdown_vario },
	{ PARAM_THRESHOLD_AUTO_SHUTDOWN_UMS		, PARAM_UINT32_T, &Config.threshold.auto_shutdown_ums 	},
	// KalmanParameters
	#if VARIOMETER_CLASSS == CLASS_KALMANVARIO
	{ PARAM_KALMAN_VAR_ZMEAS				, PARAM_FLOAT	, &Config.kalman.var_zmeas				},
	{ PARAM_KALMAN_VAR_ZACCEL				, PARAM_FLOAT	, &Config.kalman.var_zaccel				},
	{ PARAM_KALMAN_VAR_ACCELBIAS			, PARAM_FLOAT	, &Config.kalman.var_accelbias			},
	#else
	{ PARAM_KALMAN_SIGMA_P					, PARAM_FLOAT	, &Config.kalman.sigmaP					},
	{ PARAM_KALMAN_SIGMA_A					, PARAM_FLOAT	, &Config.kalman.sigmaA					},
	#endif // VARIOMETER_CLASSS == CLASS_KALMANVARIO
	// CalibrationData
	{ PARAM_CALDATA_ACCEL_00				, PARAM_FLOAT	, &Config.calData.accel[0]				},
	{ PARAM_CALDATA_ACCEL_01				, PARAM_FLOAT	, &Config.calData.accel[1]				},
	{ PARAM_CALDATA_ACCEL_02				, PARAM_FLOAT	, &Config.calData.accel[2]				},
	{ PARAM_CALDATA_GYRO_00					, PARAM_FLOAT	, &Config.calData.gyro[0]				},
	{ PARAM_CALDATA_GYRO_01					, PARAM_FLOAT	, &Config.calData.gyro[1]				},
	{ PARAM_CALDATA_GYRO_02					, PARAM_FLOAT	, &Config.calData.gyro[2]				},
	{ PARAM_CALDATA_MAG_00					, PARAM_FLOAT	, &Config.calData.mag[0]				},
	{ PARAM_CALDATA_MAG_01					, PARAM_FLOAT	, &Config.calData.mag[1]				},
	{ PARAM_CALDATA_MAG_02					, PARAM_FLOAT	, &Config.calData.mag[2]				},
	
	{ PARAM_EOF }
};
