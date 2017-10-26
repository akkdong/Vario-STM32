// VarioSentence.h
//

#ifndef __VARIOSENTENCE_H__
#define __VARIOSENTENCE_H__

#include <Digit.h>

#define USE_LK8_SENTENCE			(0)
#define USE_LXNAV_SENTENCE			(1)

#define VARIOMETER_SENTENCE_DELAY	(1000)


/////////////////////////////////////////////////////////////////////////////
// interface IVarioSentence

class IVarioSentence
{
public:
	virtual void		begin(double height, double vel, double temp, double bat);
	
	virtual int			available();
	virtual int			read();
};


/////////////////////////////////////////////////////////////////////////////
// class LK8Sentence

class LK8Sentence : public IVarioSentence
{
public:
	virtual void		begin(double height, double vel, double temp, double bat);
	
	virtual int			available();
	virtual int			read();
	
private:
	double				altimeter;
	double				vario;
	double				temperature;
	double				voltage;
	
	Digit 				valueDigit;
	HexDigit 			parityDigit;
	
	unsigned char		tagPos;
	unsigned char		parity;
	
	static const char *	LK8Tag;
};


/////////////////////////////////////////////////////////////////////////////
// class LxNavSentence

class LxNavSentence : public IVarioSentence
{
public:
	virtual void		begin(double height, double vel, double temp, double bat);
	
	virtual int			available();
	virtual int			read();
	
private:
	double				vario;
	
	Digit 				valueDigit;
	HexDigit 			parityDigit;
	
	unsigned char		tagPos;
	unsigned char		parity;
	
	static const char *	LxNavTag;
};


/////////////////////////////////////////////////////////////////////////////
// class VarioSentence

class VarioSentence
{
public:
	VarioSentence(char type);
	
public:
	void				begin(double height, double vel, double temp, double bat);
	
	int					available();	
	int					read();
	
	void				use(char type)	{ sentenceType = type; }
	
	int					checkInterval();
	
private:
	IVarioSentence *	varioSentence;
	
	LK8Sentence			LK8;
	LxNavSentence		LxNav;

	char				sentenceType;
	unsigned long		lastTick;
};

#endif // __VARIOSENTENCE_H__
