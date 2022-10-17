// Sd2CardEx.h
//

#ifndef __SD2CARDEX_H__
#define __SD2CARDEX_H__

#include <Arduino.h>
#include <SdFat.h>
#include <DefaultSettings.h>


////////////////////////////////////////////////////////////////////////////////////
// class Sd2CardEx

class Sd2CardEx : public SdSpiCard
{
public:
	Sd2CardEx() {
		m_spi.setPort(nullptr);
	}
	Sd2CardEx(SPIClass* spiPort) {
		m_spi.setPort(spiPort);
	}

	/** Initialize the SD card.
	* \param[in] csPin SD chip select pin.
	* \param[in] settings SPI speed, mode, and bit order.
	* \return true for success else false.
	*/
	bool begin(uint8_t csPin = SS, SPISettings settings = SD_SCK_MHZ(50)) {
		return SdSpiCard::begin(&m_spi, csPin, settings);
	}
	
private:
	SdFatSpiDriver m_spi;
};


////////////////////////////////////////////////////////////////////////////////////
//

extern Sd2CardEx SdCard;


#endif // __SD2CARDEX_H__
