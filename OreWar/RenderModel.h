#ifndef __RenderModel_h_
#define __RenderModel_h_

#include <Ogre.h>
#include "GameObjects.h"

using namespace Ogre;

class RenderObject
{
private:
	PhysicsObject * m_object;
	SceneNode * m_node;
public:
	RenderObject(PhysicsObject * object, SceneNode * node);
	void updateNode();
};

#endif