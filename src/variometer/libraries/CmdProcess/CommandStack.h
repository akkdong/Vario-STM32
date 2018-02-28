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
#define CMD_STATUS				'ST'
#define CMD_RESET				'RS'
#define CMD_SHUTDOWN			'SD'
#define CMD_FIRMWARE_VERSION	'FV'
#define CMD_MODE_SWITCH			'MS'
#define CMD_SOUND_LEVEL			'SL'
#define CMD_TONE_TEST			'TT'
#define CMD_DUMP_SENSOR			'DS'
#define CMD_DUMP_PROPERTY		'DP'
#define CMD_DUMP_CONFIG			'DC'
#define CMD_BLOCK_GPS_NMEA		'BG'
#define CMD_BLOCK_VARIO_NMEA	'BV'
#define CMD_FACTORY_RESET		'FR'
#define CMD_RESTORE_PROPERTY	'RP'
#define CMD_SAVE_PROPERTY		'SP'
#define CMD_QUERY_PROPERTY		'QP'
#define CMD_UPDATE_PROPERTY		'UP'
#define CMD_ACCEL_CALIBRATION	'AC'
#define CMD_RUN_BOOTLOADER		'RB'


// mode switch
#define PARAM_MS_QUERY			(0)	// response with current mode
#define PARAM_MS_VARIO			(1)
#define PARAM_MS_UMS			(2)
#define PARAM_MS_CALIBRATION	(3)

// device status
/*
#define PARAM_DS_ALL			(0)
#define PARAM_DS_IMU			(1)
#define PARAM_DS_SDCARD			(2)
#define PARAM_DS_GPS			(3)
#define PARAM_DS_VOLTAGE		(4)
*/

// sensor dump
#define PARAM_DU_NONE			(0)
#define PARAM_DU_ACCELEROMETER	(1<<0)
#define PARAM_DU_GYROSCOPE		(1<<1)
#define PARAM_DU_PRESSURE		(1<<2)
#define PARAM_DU_TEMPERATURE	(1<<3)
#define PARAM_DU_ALL			(0x0F)

// nmea sentence
/*
#define PARAM_NM_UNBLOCK		(0)
#define PARAM_NM_BLOCK			(1)
*/

//  vario tone test
#define PARAM_TT_STOP			(0)
#define PARAM_TT_START			(1)

// sound level
#define PARAM_SL_ALL			(0)
#define PARAM_SL_VARIO			(1)
#define PARAM_SL_EFFECT			(2)

#define PARAM_SL_MUTE			(0x1000)
#define PARAM_SL_MEDIUM			(0x1001)
#define PARAM_SL_LOUD			(0x1002)


// device reset
#define PARAM_RS_NOW			(0)
#define PARAM_RS_AFTER_SAVE		(1)

// accelerometer calibration
#define PARAM_AC_MEASURE		(1)
#define PARAM_AC_CALIBRATE		(2)
#define PARAM_AC_STOP			(3)
#define PARAM_AC_QUERY_STATUS	(4)
#define PARAM_AC_RESET			(5)


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
