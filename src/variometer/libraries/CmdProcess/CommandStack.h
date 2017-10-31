// CommandStack.h
//

#ifndef __COMMANDSTACK_H__
#define __COMMANDSTACK_H__

#define MAX_STACKSIZE		(10)

#define CMD_FROM_KEY		(0)
#define CMD_FROM_BT			(1)
#define CMD_FROM_USB		(2)

#define CMD_XXXX			(0xXXXX)	// consist of two bytes. each byte denotes alphabet 'A' to 'Z'
										// ex) 0x5357 -> 'SW' : mode switch
#define CMD_MODE_SWITCH		'SW'
#define CMD_DEVICE_STATUS	'DS'
#define CMD_SENSOR_DUMP		'DU'
#define CMD_TONE_TEST		'TT'
#define CMD_SOUND_LEVEL		'SL'
#define CMD_DEVICE_RESET	'RS'
#define CMD_QUERY_PARAM		'QU'
#define CMD_UPDATE_PARAM	'UD'


/////////////////////////////////////////////////////////////////////////////
// 

typedef struct tagCommand
{
	uint16_t	code;	// command code;
	uint8_t		from; 	// this command received from BT or USB or KEY
	uint8_t		param;	// command specific parameter
} Command;


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
