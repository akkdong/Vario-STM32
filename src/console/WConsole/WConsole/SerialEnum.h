//
// SerialEnum.h
//
//
// Serial enumerator that should work on al W32 platforms
//
// Martin Jansen
//

#ifndef __SERIAL_ENUMERATOR_W32__
#define __SERIAL_ENUMERATOR_W32__

//
// needed for the stl list
// I use the list because it already holds the sort algorithm
//
#include <list>

class CSerialEnum
{
public:
	// Made static for easy use
	static short GetSerialPortsEnum( std::list<int> &nPortArray);
	static short GetSerialPorts( std::list<int> &nPortArray );
	static int FindMenuItem(CMenu* Menu, LPCTSTR MenuString);
	static void UpdatePortMenuItem(CMenu *menu, int com1_id, int selection);

protected: 
	static void CheckNamePort( LPTSTR lpstrPortName, std::list<int> &nPortArray );

};

#endif // #ifndef __SERIAL_ENUMERATOR_W32__

