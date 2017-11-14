// IVerticalVelocity.h
//

#ifndef __IVERTICALVELOCITY_H__
#define __IVERTICALVELOCITY_H__

////////////////////////////////////////////////////////////////////////////////////
// interaface IVerticalVelocity

struct IVerticalVelocity
{
	virtual int			begin(float p, float a, float sigmaP, float sigmaA, uint32_t tick) = 0;
	virtual int 		update(float p, float a, uint32_t tick) = 0;
	
	virtual int			ready() = 0;
	
	virtual uint32_t 	getTimestamp() = 0;
	virtual float		getVelocity() = 0;
	virtual float		getAltitude() = 0;
	virtual float		getCalibratedAltitude() = 0;
	
	virtual float		calibrateAltitude(float refAltitude) = 0;
};

#endif // __IVERTICALVELOCITY_H__
