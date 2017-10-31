// ParamsSaver.ino
//

#include <DefaultSettings.h>
#include <GlobalConfig.h>
#include <EEPROMDriver.h>

#define STRSTACK_START		16*1024		// half-memory reserved for string stacks
#define STRSTACK_SIZE		16 * 4		// each string max size -> it can save up to 256 string 

const char * strings[] = // max string size limits to 16 * 4
{
	"blah blah",
	"woopa doopa",
};

HardWire wire(2);
EEPROMDriver eeprom(wire);
GlobalConfig config(eeprom, EEPROM_ADDRESS);


void setup()
{
	// 
	delay(1000);
	
	//
	Serial.begin(115200);
	while (! Serial);
	
	//
	config.writeAll();
	
	//
	for (int i = 0; i < sizeof(strings) / sizeof(strings[0]); i++)
		eeprom.writePage(EEPROM_ADDRESS, STRSTACK_START + STRSTACK_SIZE * i, (unsigned char *)strings[i], strlen(strings[i]) + 1);
}

void loop()
{
}
