// CommandStack.h
//

#ifndef __COMMANDSTACK_H__
#define __COMMANDSTACK_H__

#define MAX_STACKSIZE			(10)

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




/////////////////////////////////////////////////////////////////////////////
// 

class Command
{
public:
	Command() 
		{ from = code = param = 0; }
	Command(uint8_t f, uint16_t c, uint8_t p)
		{ from = f; code = c; param = p; }
	
public:
	uint16_t	code;	// command code;
	uint8_t		from; 	// this command received from BT or USB or KEY
	uint32_t	param;	// command specific parameter
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
