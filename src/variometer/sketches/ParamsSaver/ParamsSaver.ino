// ParamsSaver.ino
//

#include <DefaultSettings.h>
#include <GlobalConfig.h>
#include <EEPROMDriver.h>
#include <Digit.h>

#define STRSTACK_START		16*1024		// half-memory reserved for string stacks
#define STRSTACK_SIZE		16 * 4		// each string max size -> it can save up to 256 string 

const char * model = "NRC BTVario v1";
const char * pilot = "AKKDONG";
const char * glider = "IcePeak 6";

const char * strings[] = // max string size limits to 16 * 4
{
	"blah blah",
	"woopa doopa",
};

HardWire wire(2, I2C_FAST_MODE);
EEPROMDriver eeprom(wire);
GlobalConfig Config(eeprom, EEPROM_ADDRESS);

void printDigit(double value, int width)
{
	Digit digit;
	
	digit.begin(value, 2);
	int size = digit.size();
	for (int i = 0; i < width - size; i++)
		Serial.print(" ");
	while (digit.available())
		Serial.write(digit.get());
}

void printDigit(long value, int width)
{
	Digit digit;
	
	digit.begin(value);
	int size = digit.size();
	for (int i = 0; i < width - size; i++)
		Serial.print(" ");
	while (digit.available())
		Serial.write(digit.get());
}

void setup()
{
	// 
	delay(4000);
	
	//
	wire.begin();
	
	//
	Serial.begin(115200);
	while (! Serial);
	
	Serial.println("Write default configuration paramters for variometer...");
	Serial.println("");
	
	//
	strcpy(Config.profile_model, model);
	strcpy(Config.profile_pilot, pilot);
	strcpy(Config.profile_glider, glider);
	
	// write default configuration parameters...
	Config.writeAll();
	Serial.println("");
	
	// & write strings...
//	for (int i = 0; i < sizeof(strings) / sizeof(strings[0]); i++)
//		eeprom.writePage(EEPROM_ADDRESS, STRSTACK_START + STRSTACK_SIZE * i, (unsigned char *)strings[i], strlen(strings[i]) + 1);
	
	
	// confirm writing...
	EEPROM_Block * block = Config.getBlock();

	// read profile
	// profile_model
	Serial.print("Model: "); 
	if (Config.readBlock(BLOCK_ID_PROFILE_MODEL, block))
	{		
		BLOCK_Profile * profile = (BLOCK_Profile *)block;
		Serial.println((char *)profile->name);
	}
	else
	{
		Serial.println("N/A");
	}
	
	// profile_pilot
	Serial.print("Pilot: "); 
	if (Config.readBlock(BLOCK_ID_PROFILE_PILOT, block))
	{		
		BLOCK_Profile * profile = (BLOCK_Profile *)block;
		Serial.println((char *)profile->name);
	}
	else
	{
		Serial.println("N/A");
	}
	
	// profile_glider
	Serial.print("Glider: "); 
	if (Config.readBlock(BLOCK_ID_PROFILE_GLIDER, block))
	{		
		BLOCK_Profile * profile = (BLOCK_Profile *)block;
		Serial.println((char *)profile->name);
	}
	else
	{
		Serial.println("N/A");
	}
	
	// vario-settings
	Serial.print("Vario settings");
	if (Config.readBlock(BLOCK_ID_VARIO_SETTINGS, block))
	{
		BLOCK_VarioSettings * vario = (BLOCK_VarioSettings *)block;
		
		Serial.println("...");
		Serial.print("  Sink threshold: "); Serial.println(vario->sinkThreshold, 2);
		Serial.print("  Climb threshold: "); Serial.println(vario->climbThreshold, 2);
		Serial.print("  Sensitivity: "); Serial.println(vario->sensitivity, 2);
	}
	else
	{
		Serial.println(": N/A");
	}
	
	// vario-timezone
	Serial.print("Vario time-zone: ");
	if (Config.readBlock(BLOCK_ID_VARIO_TIMEZONE, block))
	{
		BLOCK_VarioTimezone * vario = (BLOCK_VarioTimezone *)block;
		Serial.print("GMT");
		if (vario->timezone > 0)
			Serial.print("+");
		Serial.println(vario->timezone);
	}
	else
	{
		Serial.println("N/A");
	}
	
	// vario-volume
	if (Config.readBlock(BLOCK_ID_VARIO_VOLUMNE, block))
	{
		BLOCK_VarioVolume * vario = (BLOCK_VarioVolume *)block;
		Serial.print("Vario volume: "); printDigit((long)vario->volume, 3); Serial.println("%");
	}
	else
	{
		Serial.println("N/A");
	}
	
	// vario-tone-table
	Serial.println("Vario tone table...");
	for (int i = 0; i < TONE_TABLE_COUNT; i++)
	{
		Serial.print("  [");
		if (Config.readBlock(BLOCK_ID_VARIO_TONE_00+i, block))
		{
			BLOCK_VarioTone * vario = (BLOCK_VarioTone *)block;
			
			printDigit(vario->tone.velocity, 6);		Serial.print(", ");
			printDigit((long)vario->tone.freq, 4);		Serial.print(", ");
			printDigit((long)vario->tone.period, 3);	Serial.print(", ");
			printDigit((long)vario->tone.duty, 3);		Serial.println("]");
		}
		else
		{
			Serial.println("N/A]");
		}
	}

	//
	Serial.print("Kalman filter parameters");
	if (Config.readBlock(BLOCK_ID_KALMAN_PARAMS, block))
	{
		BLOCK_KalmanParameters * kalman = (BLOCK_KalmanParameters *)block;
		
		Serial.println("...");
		Serial.print("  sigmaP: "); Serial.println(kalman->sigmaP, 2);
		Serial.print("  sigmaA: "); Serial.println(kalman->sigmaA, 2);
	}
	else
	{
		Serial.println(": N/A");
	}

	//
	Serial.print("Accelerometer calibration data: ");
	if (Config.readBlock(BLOCK_ID_CALIBRATION_DATA, block))
	{
		BLOCK_CalibrationData * data = (BLOCK_CalibrationData *)block;
		
		Serial.print("[");
		printDigit(data->accel[0], 5);
		Serial.print(", ");
		printDigit(data->accel[1], 5);
		Serial.print(", ");
		printDigit(data->accel[2], 5);
		Serial.println("]");
	}	
	else
	{
		Serial.println("N/A");
	}
	
	Serial.println("");
	Serial.println("");
	Serial.println("end.");
}

void loop()
{
}
