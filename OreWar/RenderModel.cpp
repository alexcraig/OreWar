#include "RenderModel.h"
#include <OgreMatrix4.h>

using namespace Ogre;

// ========================================================================
// RenderObject Implementation
// ========================================================================
RenderObject::RenderObject(PhysicsObject * object, SceneNode * node)
	: m_object(object), m_node(node)
{
}

void RenderObject::updateNode()
{
	m_node->setPosition(m_object->getPosition());
	m_node->lookAt(m_object->getPosition() + m_object->getHeading(), Node::TS_WORLD, Vector3::UNIT_Z);
}