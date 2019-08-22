#ifndef OCULUSHMD_H
#define OCULUSHMD_H

#include "system/EventListener.h"

namespace Input
{

	struct OculusHMD : System::EventListener
	{
		glm::quat headLocalOrientation;
		glm::vec3 headLocalTranslation;

		glm::quat eyeLocalOrientation[ET_COUNT];
		glm::vec3 eyeLocalTranslation[ET_COUNT];
	};
}

#endif // OCULUSHMD_H
