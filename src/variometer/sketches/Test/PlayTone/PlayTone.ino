// PlayTone.ino
//

#include <ToneGenerator.h>
#include <TonePlayer.h>
#include <VarioBeeper.h>

#define PLAYER_TIMER_ID		(1)
#define PLAYER_TIMER_CH		(1)

static Tone startTone[] = {
	{ 262, 1000 / 4 }, 
	{ 196, 1000 / 8 }, 
	{ 196, 1000 / 8 }, 
	{ 220, 1000 / 4 }, 
	{ 196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ 247, 1000 / 4 }, 
	{ 262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};

ToneGenerator toneGen;
TonePlayer	tonePlayer(toneGen);
VarioBeeper	varioBeeper(tonePlayer);

#if 1

#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <GlobalConfig.h>
#include <IMUModule.h>
#include <VertVelocity.h>

HardWire Wire1(1, I2C_FAST_MODE);
HardWire Wire2(2, I2C_FAST_MODE);

HardWire & I2CDevice::Wire = Wire1;
unlockCallback I2CDevice::cbUnlock = SensorMS5611::UnlockI2C;

EEPROMDriver eeprom(Wire2);
GlobalConfig Config(eeprom, EEPROM_ADDRESS);

IMUModule imu;
VertVelocity vertVel;

float vVelocity = 0.0;
float vInc = 0.04;

uint32_t lastTick;

void setup()
{
	//
	Serial.begin();
	Serial.println("Tone Test!!!");
	delay(500);
	
	//
	Wire1.begin();
	Wire2.begin();
	
	//
	Config.readAll();
	
	// ToneGenerator uses PIN_PWM_H(PA8 : Timer1, Channel1)
	toneGen.begin(PIN_PWM_H);
	tonePlayer.setVolume(5);
	
	//
	imu.init();
	
	for (int i = 0; i < 100; i++)
	{
		while (! imu.dataReady());
		imu.updateData();
	}
	
	vertVel.init(imu.getAltitude(), 
				imu.getVelocity(),
				Config.kalman_sigmaP, // POSITION_MEASURE_STANDARD_DEVIATION,
				Config.kalman_sigmaA, // ACCELERATION_MEASURE_STANDARD_DEVIATION,
				millis());	
	
	//
	varioBeeper.setVelocity(vVelocity);
	lastTick = millis();
}

void loop()
{
	if ((millis() - lastTick) > 100)
	{
		vVelocity += vInc;
		
		if (vVelocity > 10 || vVelocity < 0)
			vInc = -vInc;
		
		varioBeeper.setVelocity(vVelocity);
		lastTick = millis();
		
		//Serial.print("vv = "); Serial.println(vVelocity);
	}
	
	if (imu.dataReady())
	{
		imu.updateData(/* &sensorReporter */);

		vertVel.update(imu.getAltitude(), imu.getVelocity(), millis());	
		
		float velocity = vertVel.getVelocity();
		Serial.println(velocity * 100.0, 2);
	}
	
	tonePlayer.update();
}

#else

unsigned long lastTick;
int playType = 0;


void setup()
{
	//
	delay(200);

	//
	Serial.begin(115200);
	Serial.println("Tone Test!!!");

	//
	toneGen.begin(PA8);
	
	//
	tonePlayer.setMelody(&startTone[0], sizeof(startTone) / sizeof(startTone[0]));
	lastTick = millis();
}

void loop()
{
	static int v = 0;
	
	tonePlayer.update();
	
	if ((millis() - lastTick) > 10000)
	{
		tonePlayer.setVolume(v ? 90 : 10);
		v = 1 - v;
		
		tonePlayer.setMelody(&startTone[0], sizeof(startTone) / sizeof(startTone[0]));
		lastTick = millis();
	}
}

#endif