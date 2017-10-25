// VarioSentence.h
//

#ifndef __VARIOSENTENCE_H__
#define __VARIOSENTENCE_H__

#define USE_LXNAV_SENTENCE		(1)
#define USE_LK8_SENTENCE		(0)


/////////////////////////////////////////////////////////////////////////////
// class VarioSentence

class VarioSentence
{
public:
	VarioSentence(char type);
	
public:
	void				update(float v, float h, float t);
	
	int					available();	
	int					read();
	
	void				use(char type)	{ sentenceType = type; }
	
private:
//	LK8Sentence			lk8;
//	LxNavSentence		lxnav;

	char				sentenceType;
};

#endif // __VARIOSENTENCE_H__
