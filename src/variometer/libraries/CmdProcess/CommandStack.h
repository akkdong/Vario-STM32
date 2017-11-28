// CommandStack.h
//

#ifndef __COMMANDSTACK_H__
#define __COMMANDSTACK_H__

#define MAX_STACKSIZE			(10)
#define MAX_VALUE_STRING		(32)

#define CMD_FROM_KEY			(0)
#define CMD_FROM_BT				(1)
#define CMD_FROM_USB			(2)

#define CMD_XXXX				(0xXXXX)	// consist of two bytes. each byte denotes alphabet 'A' to 'Z'
										// ex) 0x5357 -> 'SW' : mode switch
#define CMD_MODE_SWITCH			'SW'
#define CMD_DEVICE_STATUS		'DS'
#define CMD_SENSOR_DUMP			'DU'
#define CMD_NMEA_SENTENCE		'NM'
#define CMD_TONE_TEST			'TT'
#define CMD_SOUND_LEVEL			'LV'
#define CMD_DEVICE_RESET		'RS'
#define CMD_QUERY_PARAM			'QU'
#define CMD_UPDATE_PARAM		'UD'
#define CMD_SAVE_PARAM			'SV'
#define CMD_RESTORE_PARAM		'RE'

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
#define PARAM_LV_QUIET			(1)
#define PARAM_LV_LOUD			(2)

// device reset
#define PARAM_RS_NOW			(0)
#define PARAM_RS_AFTER_SAVE		(1)

// query paramater
enum VarioParameters
{
	PARAM_PROFILE_MODEL,
	PARAM_PROFILE_PILOT,
	PARAM_PROFILE_GLIDER,
	PARAM_VARIO_SINK_THRESHOLD,
	PARAM_VARIO_CLIMB_THRESHOLD,
	PARAM_VARIO_SENSITIVITY,
	PARAM_VARIO_BARO_ONLY,
	PARAM_VARIO_VOLUMN,
	PARAM_VARIO_TONE_00,
	PARAM_VARIO_TONE_01,
	PARAM_VARIO_TONE_02,
	PARAM_VARIO_TONE_03,
	PARAM_VARIO_TONE_04,
	PARAM_VARIO_TONE_05,
	PARAM_VARIO_TONE_06,
	PARAM_VARIO_TONE_07,
	PARAM_VARIO_TONE_08,
	PARAM_VARIO_TONE_09,
	PARAM_VARIO_TONE_10,
	PARAM_VARIO_TONE_11,
	PARAM_TIME_ZONE,
	PARAM_KALMAN_SIGMA,
	PARAM_KALMAN_VARIANCE,
	PARAM_CALIBRATION_ACCEL,
	PARAM_CALIBRATION_GYRO,
	PARAM_COUNT,
};


/////////////////////////////////////////////////////////////////////////////
// 

class Command
{
public:
	Command() {
		from = code = param = valLen = 0;
	}
	Command(const Command & c) {
		from = c.from; code = c.code; param = c.param; valLen = c.valLen;

		memset(valData, 0, sizeof(valData));
		if (valLen)
			memcpy(valData, c.valData, valLen);
	}
	Command(uint16_t f, uint16_t c, uint32_t p, uint8_t * d = 0, uint8_t n = 0) { 
		from = f; code = c; param = p; valLen = n;

		memset(valData, 0, sizeof(valData));
		if (d && n)
			memcpy(valData, d, n);
	}

public:
	uint16_t	from; 	// this command received from BT or USB or KEY
	uint16_t	code;	// command code;
	uint32_t	param;	// command specific parameter
	
	uint8_t		valData[MAX_VALUE_STRING];
	uint8_t		valLen;
};


/////////////////////////////////////////////////////////////////////////////
//

enum RESDATA_TYPE
{
	RESDATA_NONE,
	RESDATA_NUMBER,
	RESDATA_FLOAT,
	RESDATA_ARRAY,
	RESDATA_STRING,
};

class Response
{
public:
	Response();
	
public:
	uint16_t	to;		// this response is transmitted to BT or USB
	uint16_t	code;	// response code
	uint32_t	param;	// response specific paramater
	
	union
	{
		uint32 	uData;
		float	fData;
		uint32	aData[4];
		uint8_t	sData[MAX_VALUE_STRING];
	};
	uint8_t dataType;	// response data type
};



/////////////////////////////////////////////////////////////////////////////
// class CommandStack

class CommandStack
{
public:
	CommandStack();
	
public:
	void		enqueue(Command cmd);
	Command		dequeue();
	
	int8_t		getSize();
	
	int8_t		isEmpty();
	int8_t		isFull();
	
private:
	Command		fifo[MAX_STACKSIZE];
	int8_t		front;
	int8_t		rear;
};


#endif // __COMMANDSTACK_H__
