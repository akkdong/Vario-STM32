// ParamsSaver.ino
//

#include <DefaultSettings.h>
#include <GlobalConfig.h>
#include <EEPROMDriver.h>
#include <Digit.h>

#define STRSTACK_START		16*1024		// half-memory reserved for string stacks
#define STRSTACK_SIZE		16 * 4		// each string max size -> it can save up to 256 string 

const char * pilot = "AKKDONG";
const char * manufacture = "Niviuk";
const char * model = "IcePeak 6";

const char * strings[] = // max string size limits to 16 * 4
{
	"blah blah",
	"woopa doopa",
};

TwoWire wire(2, I2C_FAST_MODE);
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

void printParams()
{
	//
	// GliderInfo
	Serial.println("Glider Info"); 
	Serial.print("   "); 
	switch(Config.glider.type)
	{
	case GTYPE_PARAGLIDER	:
		Serial.println("Paraglider");
		break;
	case GTYPE_HG_FLEX		:
		Serial.println("Hanglider Flex");
		break;
	case GTYPE_HG_RIGID		:	
		Serial.println("Hanglider Rigid");
		break;
	case GTYPE_SAILPLANE	:
		Serial.println("Sailplane");
		break;
	case GTYPE_PPG_FOOT		:
		Serial.println("Powered Paraglider Foot");
		break;
	case GTYPE_PPG_WHEEL	:
		Serial.println("Powered Paraglider Wheel");
		break;
	case GTYPE_UNKNOWNM		:
	default :
		Serial.println("Unset");
		break;
	}
	Serial.print("   "); 
	Serial.print(Config.glider.manufacture);
	Serial.print(" "); 
	Serial.println(Config.glider.model);
	Serial.println("");
	
	// Logger
	Serial.println("IGC-Logger"); 
	Serial.print("  "); Serial.println(Config.logger.enable ? "Enable" : "Disable");
	Serial.print("  takeoff speed : "); Serial.println(Config.logger.takeoff_speed);
	Serial.print("  landing timeout : "); Serial.println(Config.logger.landing_timeout);
	Serial.print("  logging interval : "); Serial.println(Config.logger.logging_interval);
	Serial.print("  pilot : "); Serial.println(Config.logger.pilot);
	Serial.print("  timezone : GMT"); Serial.print(Config.logger.timezone > 0 ? "+" : ""); Serial.println((int)Config.logger.timezone);
	Serial.println("");
	
	// Vario Settings
	Serial.println("Vario Settings");
	Serial.print("  sink threshold : "); Serial.println(Config.vario.sinkThreshold);
	Serial.print("  climb threshold : "); Serial.println(Config.vario.climbThreshold);
	Serial.print("  sensitivity : "); Serial.println(Config.vario.sensitivity);
	Serial.print("  sentence : "); Serial.println(Config.vario.sentence == VARIOMETER_LK8_SENTENCE ? "LK8" : "LxNav");
	Serial.print("  baroOnly: "); Serial.println(Config.vario.baroOnly ? "Yes" : "No");
	Serial.println("");
	
	// Tone Table
	Serial.println("Tone Tables:");
	Serial.println("  vel, freq, period, duty");
	for (int i = 0; i < TONE_TABLE_COUNT; i++)
	{
		Serial.print("  "); 
		Serial.print(Config.toneTable[i].velocity);
		Serial.print(", ");
		Serial.print(Config.toneTable[i].freq);
		Serial.print(", ");
		Serial.print(Config.toneTable[i].period);
		Serial.print(", ");
		Serial.print(Config.toneTable[i].duty);
		Serial.println("");	
	}
	Serial.println("");
	
	// Volume Settings
	Serial.println("Volume Settings");
	Serial.print("  vario : "); Serial.println(Config.volume.vario);
	Serial.print("  effect : "); Serial.println(Config.volume.effect);
	Serial.println("");
	
	// Threshold Settings
	Serial.println("Threshold Settings");
	Serial.print("  low battery : "); Serial.println(Config.threshold.low_battery);
	Serial.print("  shutdown hold-time : "); Serial.println(Config.threshold.shutdown_holdtime);
	Serial.println("");
	
	// Kalman Parameters
	Serial.println("Kalman Parameters");
	#if VARIOMETER_CLASSS == CLASS_KALMANVARIO
	Serial.print("  variance zmeas : "); Serial.println(Config.kalman.var_zmeas);
	Serial.print("  variance zaccel : "); Serial.println(Config.kalman.var_zaccel);
	Serial.print("  variance accelbias : "); Serial.println(Config.kalman.var_accelbias);
	#else
	Serial.print("  sigma-P : "); Serial.println(Config.kalman.sigmaP);
	Serial.print("  sigma-A : "); Serial.println(Config.kalman.sigmaA);
	#endif // VARIOMETER_CLASSS == CLASS_KALMANVARIO
	Serial.println("");
	
	// Calibration data
	Serial.println("Calibration data");
	Serial.print("  accel : "); Serial.print(Config.calData.accel[0]);
	Serial.print(", "); Serial.print(Config.calData.accel[1]);
	Serial.print(", "); Serial.println(Config.calData.accel[2]);
	Serial.print("  gyro : "); Serial.print(Config.calData.gyro[0]);
	Serial.print(", "); Serial.print(Config.calData.gyro[1]);
	Serial.print(", "); Serial.println(Config.calData.gyro[2]);
	Serial.print("  mag : "); Serial.print(Config.calData.mag[0]);
	Serial.print(", "); Serial.print(Config.calData.mag[1]);
	Serial.print(", "); Serial.println(Config.calData.mag[2]);
	Serial.println("");
}


void writeParams()
{
	//
	Serial.println("reset parameters");
	Config.reset();
	
	//
	Serial.println("update to default parameters");
	strcpy(Config.glider.manufacture, manufacture);
	strcpy(Config.glider.model, model);
	
	strcpy(Config.logger.pilot, pilot);
	
	
	// 
	Config.writeAll();
	
	//
	Config.reset();
	Config.readAll();
	
	//
	Serial.println("save all....");
	Serial.println("");
	
	printParams();
	
	Serial.println("");
	Serial.println("done.");
}

void dumpParams()
{
	//
	Config.reset();
	Config.readAll();
	
	//
	Serial.println("dump all parameters");
	Serial.println("");
	
	printParams();
	
	Serial.println("");
	Serial.println("done.");	
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
	Serial.println("");
	Serial.println("enter (w)rite or (d)ump...");
}

void loop()
{
	if (Serial.available())
	{
		int ch = Serial.read();
		
		switch(ch)
		{
		case 'w' :
		case 'W' :
			writeParams();
			break;
		case 'd' :
		case 'D' :
			dumpParams();
			break;
		}
	}
}
