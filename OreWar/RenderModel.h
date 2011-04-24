#ifndef __RenderModel_h_
#define __RenderModel_h_

#include <Ogre.h>
#include <vector>
#include <sstream>
#include "GameObjects.h"

using namespace Ogre;

/**
 * A RenderObject represents a single logical object from the game model
 * that should be rendered to screen. The RenderObject combines a reference
 * to the PhysicsObject of the model with a reference to the root sceneNode
 * that should be used to render the entity through OGRE.
 */
class RenderObject
{
private:
	/** A pointer to the model object */
	PhysicsObject * mp_object;

	SceneManager * mp_mgr;
public:
	/** Constructs a new RenderObject */
	RenderObject(PhysicsObject * object, SceneManager * mgr);

	/** @return A pointer to the model object */
	PhysicsObject * getObject();

	SceneManager * getSceneManager();

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateNode(Real elapsedTime, Quaternion camOrientation) = 0;

	virtual void loadSceneResources() = 0;

	virtual void buildNode(int entityIndex) = 0;

	virtual void destroyNode() = 0;

	bool operator==(const RenderObject &other) const;
};

class ShipRO : public RenderObject
{
private:
	SceneNode * mp_shipNode;
	SceneNode * mp_shipRotateNode;
	Entity * mp_shipEntity;
	Light * mp_spotLight;
	Light * mp_pointLight;

public:
	ShipRO(PhysicsObject * object, SceneManager * mgr);

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateNode(Real elapsedTime, Quaternion camOrientation);

	virtual void loadSceneResources();

	virtual void buildNode(int entityIndex);

	virtual void destroyNode();
};

class NpcShipRO : public ShipRO
{
private:
	SceneNode * mp_frameNode;
	Entity * mp_frameSprite;

	static bool m_resourcesLoaded;
public:
	NpcShipRO(PhysicsObject * object, SceneManager * mgr);

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateNode(Real elapsedTime, Quaternion camOrientation);

	virtual void loadSceneResources();

	virtual void buildNode(int entityIndex);

	virtual void destroyNode();
};

class ProjectileRO : public RenderObject
{
private:
	SceneNode * mp_projNode;

	Entity * mp_projSprite;
	Light * mp_pointLight;

	static bool m_resourcesLoaded;
public:
	ProjectileRO(PhysicsObject * object, SceneManager * mgr);

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateNode(Real elapsedTime, Quaternion camOrientation);

	virtual void loadSceneResources();

	virtual void buildNode(int entityIndex);

	virtual void destroyNode();
};

/**
 * The RenderModel stores the complete list of RenderObjects that should be rendered each frame
 * for a scene.
 */
class RenderModel : public GameArenaListener
{
private:
	/** Reference to the GameArena object that should be observed */
	GameArena & m_model;

	/** List of all RenderObjects that should be updated and rendered each frame */
	std::vector<RenderObject * > m_renderList;

	/** The SceneManager for the scene represented by the RenderModel */
	SceneManager * mp_mgr;

	/** Counter used to ensure entities are generated with unique identifiers */
	int m_entityIndex;

public:
	/**
	 * Constructs a RenderModel which observes the specified GameArena, and creates
	 * scene nodes through the passed SceneManager.
	 */
	RenderModel(GameArena& model, SceneManager * mgr);

	/**
	 * Generates a RenderObject to represent the passed PhysicsObject, and adds it to the render list 
	 */
	void generateRenderObject(PhysicsObject * object);

	/**
	 * Searches the render list for a RenderObject representing the passed object, and deletes it if
	 * it exists.
	 */
	void destroyRenderObject(PhysicsObject * object);

	/** Calls the updateNode() method of all RenderObjects stored in the RenderModel's render list */
	void updateRenderList(Real elapsedTime, Quaternion camOrientation);

	/** Called whenever a new PhysicsObject is created by the observed GameArena */
	virtual void newPhysicsObject(PhysicsObject * object);

	/** Called whenever a PhysicsObject is destroyed by the observed GameArena */
	virtual void destroyedPhysicsObject(PhysicsObject * object);

	int getNumObjects() const;
};

#endif