// CommandStack.h
//

#ifndef __COMMANDSTACK_H__
#define __COMMANDSTACK_H__

#define MAX_STACKSIZE			(10)
#define MAX_COMMAND_STRING		(32)

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
#define PARAM_DU_PRESSURE		(1<<1)
#define PARAM_DU_TEMPERATURE	(1<<2)
#define PARAM_DU_VOLTAGE		(1<<3)
#define PARAM_DU_GPS			(1<<4)
#define PARAM_DU_V_VELOCITY		(1<<5)
#define PARAM_DU_ALL			(0x3F)

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
#define PARAM_QU_VARIO_SINK_THRESHOLD	(1)
#define PARAM_QU_VARIO_CLIMB_THRESHOLD	(2)
#define PARAM_QU_VARIO_SENSITIVITY		(3)
#define PARAM_QU_NMEA_SENTENCE			(4)
#define PARAM_QU_TIME_ZONE              (5)
#define PARAM_QU_KALMAN_SIGMA_P			(6)
#define PARAM_QU_KALMAN_SIGMA_A			(7)
#define PARAM_QU_CALIBRATION_X			(8)
#define PARAM_QU_CALIBRATION_Y			(9)
#define PARAM_QU_CALIBRATION_Z			(10)
#define PARAM_QU_PROFILE_MODEL			(11)
#define PARAM_QU_PROFILE_PILOT			(12)
#define PARAM_QU_PROFILE_GLIDER			(13)
#define PARAM_QU_TONE_TABLE				(14)

// update paramater
#define PARAM_UD_VARIO_SINK_THRESHOLD	(1)
#define PARAM_UD_VARIO_CLIMB_THRESHOLD	(2)
#define PARAM_UD_VARIO_SENSITIVITY		(3)
#define PARAM_UD_NMEA_SENTENCE			(4)
#define PARAM_UD_TIME_ZONE              (5)
#define PARAM_UD_KALMAN_SIGMA_P			(6)
#define PARAM_UD_KALMAN_SIGMA_A			(7)
#define PARAM_UD_CALIBRATION_X			(8)
#define PARAM_UD_CALIBRATION_Y			(9)
#define PARAM_UD_CALIBRATION_Z			(10)
#define PARAM_UD_PROFILE_MODEL			(100) // 100 ~ 1xx : each character of string
#define PARAM_UD_PROFILE_PILOT			(200) // 200 ~ 2xx : each character of string
#define PARAM_UD_PROFILE_GLIDER			(300) // 300 ~ 3xx : each character of string
#define PARAM_UD_TONE_TABLE_VELOCITY	(400) // 400 ~ 411
#define PARAM_UD_TONE_TABLE_FREQUENCY	(420) // 420 ~ 431
#define PARAM_UD_TONE_TABLE_PERIOD		(440) // 440 ~ 451
#define PARAM_UD_TONE_TABLE_DUTY		(460) // 460 ~ 471


/////////////////////////////////////////////////////////////////////////////
// 

union CommandValue
{
	int32_t		sData;
	uint32_t	uData;
	float		fData;
	
	uint32_t	aData[MAX_COMMAND_STRING/sizeof(uint32_t)];
	uint8_t		strData[MAX_COMMAND_STRING];
};

/////////////////////////////////////////////////////////////////////////////
// 

class Command
{
public:
	Command() 
		{ from = code = param = 0; }
	Command(uint16_t f, uint16_t c, uint32_t p, uint32_t v = 0)
		{ from = f; code = c; param = p; value.sData = v; }
//	Command(uint16_t f, uint16_t c, uint32_t p, float v = 0)
//		{ from = f; code = c; param = p; value.fData = v; }	
public:
	uint16_t	code;	// command code;
	uint16_t	from; 	// this command received from BT or USB or KEY
	uint32_t	param;	// command specific parameter
//	uint32_t	value;	// 
	CommandValue value;
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
