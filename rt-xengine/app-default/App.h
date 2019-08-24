#ifndef APP_H
#define APP_H

#include "AppBase.h"

// the custom AppBase implementation used by the engine

class App : public AppBase {
public:
	App()
		: AppBase()
	{
		m_handleControllers = true;
	}

};

#endif //APP_H