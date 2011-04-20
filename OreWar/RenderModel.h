#ifndef __RenderModel_h_
#define __RenderModel_h_

#include <Ogre.h>
#include <vector>
#include <sstream>
#include "GameObjects.h"

using namespace Ogre;

class RenderObject
{
private:
	PhysicsObject * m_object;
	SceneNode * m_node;
public:
	RenderObject(PhysicsObject * object, SceneNode * node);

	void updateNode(Real elapsedTime);

	PhysicsObject * getObject();

	SceneNode * getRootNode();
};


class RenderModel : public GameArenaListener
{
private:
	GameArena & m_model;
	std::vector<RenderObject> m_renderList;
	SceneManager * m_mgr;
	int m_entityIndex;

public:
	RenderModel(GameArena& model, SceneManager * mgr);

	void generateRenderObject(PhysicsObject * object);

	void destroyRenderObject(PhysicsObject * object);

	void updateRenderList(Real elapsedTime);

	virtual void newPhysicsObject(PhysicsObject * object);

	virtual void destroyedPhysicsObject(PhysicsObject * object);
};

#endif