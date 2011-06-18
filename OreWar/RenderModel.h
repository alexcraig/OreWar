#ifndef __RenderModel_h_
#define __RenderModel_h_

#include <Ogre.h>
#include <vector>
#include <sstream>
#include <OgreParticleSystem.h>
#include "GameObjects.h"
#include "Gorilla.h"
#include "MemoryMgr.h"

using namespace Ogre;

/**
 * The RenderObject class represents any entity in the game world which
 * will be rendered through the OgreScene manager (typically attached to
 * SceneNodes)
 */
class RenderObject
{
private:
	/** The lowest integer value that is guaranteed to be unused as a render index */
	static int m_nextRenderId;

	/** The SceneManager that should be used to manage OGRE resources */
	SceneManager * mp_mgr;

	/** An integer identifier which is unique to this instance of RenderObject */
	int m_renderId;

public:
	/** Constructs a new RenderObject which uses the specified SceneManager */
	RenderObject(SceneManager * mgr);

	/** @return The OGRE scene manager used to manage resources for this object */
	SceneManager * sceneManager();

	/** @return The unique render identifier of this RenderObject */
	int renderId() const;

	/** Updates the entities effects based on passed time and camera orientation (useful for sprites) */
	virtual void updateEffects(Real elapsedTime, Quaternion camOrientation) = 0;

	/**  Loads any resources (sprites, particles, etc) that will be neccesary to render the entity. */
	virtual void loadSceneResources() = 0;

	/** Performs first time creation of required OGRE effects */
	virtual void createEffects() = 0;

	/** Performs destruction of all generated OGRE effects */
	virtual void destroyEffects() = 0;

	/** @return True if the passed RenderObject has the same render id */
	bool operator==(const RenderObject &other) const;
};

/**
 * The ConstraintRenderObject represents the graphical representation
 * of a Constraint physics object */
class ConstraintRenderObject : public RenderObject
{
private:
	/** The constraint object to render */
	Constraint * mp_constraint;
	
	/** SceneNode to anchor effects to */
	SceneNode * mp_node;

	/** Particle system for rope effect */
	ParticleSystem * mp_particle;

	/** Static flag to ensure resources are loaded only once */
	static bool m_resourcesLoaded;
public:
	/** Constructs a new PhysicsRenderObject */
	ConstraintRenderObject(Constraint * constraint, SceneManager * mgr);

	/** @return A pointer to the model object */
	Constraint * constraint();

	/** #see RenderObject::updateEffects() */
	virtual void updateEffects(Real elapsedTime, Quaternion camOrientation);

	/** @see RenderObject::loadSceneResources() */ 
	virtual void loadSceneResources();

	/** @see RenderObject::createEffects() */ 
	virtual void createEffects();

	/** @see RenderObject::destroyEffects() */ 
	virtual void destroyEffects();
};


/**
 * A PhysicsRenderObject represents a single logical object from the game model
 * that should be rendered to screen.
 */
class PhysicsRenderObject : public RenderObject
{
private:
	/** A pointer to the model object */
	SphereCollisionObject * mp_object;

public:
	/** Constructs a new PhysicsRenderObject */
	PhysicsRenderObject(SphereCollisionObject * object, SceneManager * mgr);

	/** @return A pointer to the model object */
	SphereCollisionObject * physics();

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateEffects(Real elapsedTime, Quaternion camOrientation) = 0;

	/** @see RenderObject::loadSceneResources() */ 
	virtual void loadSceneResources() = 0;

	/** @see RenderObject::createEffects() */ 
	virtual void createEffects() = 0;

	/** @see RenderObject::destroyEffects() */ 
	virtual void destroyEffects() = 0;

	/** @see RenderObject::operator==() */
	bool operator==(const RenderObject &other) const;
};


/**
 * The ShipRO (ShipRenderObject) class manages the graphical representation
 * of any instance of a SpaceShip (human or player controlled)
 */
class ShipRO : public PhysicsRenderObject
{
private:
	/** The game object represented by this render object */
	SpaceShip * mp_spaceShip;

	/** The scene node to anchor effects onto */
	SceneNode * mp_shipNode;

	/** 
	 * The scene node which rotates to face the same direction as the
	 * represented game object.
	 */
	SceneNode * mp_shipRotateNode;
	Entity * mp_shipEntity;
	Light * mp_spotLight;
	Light * mp_pointLight;
	ParticleSystem * mp_engineParticles;

	static bool m_resourcesLoaded;
public:
	ShipRO(SpaceShip * ship, SceneManager * mgr);

	/** @return The SpaceShip game object represented by this entity */
	SpaceShip * ship() const;

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateEffects(Real elapsedTime, Quaternion camOrientation);

	/** @see RenderObject::loadSceneResources() */ 
	virtual void loadSceneResources();

	/** @see RenderObject::createEffects() */ 
	virtual void createEffects();

	/** @see RenderObject::destroyEffects() */ 
	virtual void destroyEffects();
};


class NpcShipRO : public ShipRO
{
private:
	SceneNode * mp_frameNode;
	Entity * mp_frameSprite;
	Gorilla::ScreenRenderable * mp_screen;
	Gorilla::Rectangle * mp_healthBar;
	Gorilla::Rectangle * mp_energyBar;

	static bool m_resourcesLoaded;
public:
	NpcShipRO(SpaceShip * ship, SceneManager * mgr);

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateEffects(Real elapsedTime, Quaternion camOrientation);

	/** @see RenderObject::loadSceneResources() */ 
	virtual void loadSceneResources();

	/** @see RenderObject::createEffects() */ 
	virtual void createEffects();

	/** @see RenderObject::destroyEffects() */ 
	virtual void destroyEffects();
};

/**
 * The CelestialBodyRO (CelestialBodyObject) class manages the graphical representation
 * of any instance of a CelestialBody
 */
class CelestialBodyRO : public PhysicsRenderObject
{
private:
	/** The celestial body represented by this render object */
	CelestialBody * mp_body;

	/** The scene node to anchor effects onto */
	SceneNode * mp_bodyNode;
	
	/** Entity for star/planet/moon model */
	Entity * mp_model;

	/** Point light to use for stars */
	Light * mp_pointLight;

	/** Particle system to use for stars */
	ParticleSystem * mp_particles;

	/** @see RenderObject::m_resourcesLoaded */
	static bool m_resourcesLoaded;

public:
	/** Constructor */
	CelestialBodyRO(CelestialBody * body, SceneManager * mgr);

	/** @return The CelestialBody game object represented by this entity */
	CelestialBody * body() const;

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateEffects(Real elapsedTime, Quaternion camOrientation);

	/** @see RenderObject::loadSceneResources() */ 
	virtual void loadSceneResources();

	/** @see RenderObject::createEffects() */ 
	virtual void createEffects();

	/** @see RenderObject::destroyEffects() */ 
	virtual void destroyEffects();
};


class ProjectileRO : public PhysicsRenderObject
{
private:
	Projectile * mp_projectile;

	SceneNode * mp_projNode;

	Light * mp_pointLight;

	ParticleSystem * mp_particle;

	static bool m_resourcesLoaded;
public:
	ProjectileRO(Projectile * proj, SceneManager * mgr);

	Projectile * projectile() const;

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateEffects(Real elapsedTime, Quaternion camOrientation);

	virtual void loadSceneResources();

	virtual void createEffects();

	virtual void destroyEffects();
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

	/** List of all PhysicsRenderObjects that should be updated and rendered each frame */
	std::vector<PhysicsRenderObject * > m_physicsRenderList;

	std::vector<ConstraintRenderObject * > m_constraintRenderList;

	/** The SceneManager for the scene represented by the RenderModel */
	SceneManager * mp_mgr;

	/** The memory pool which will handle all RenderObjects */
	PagedMemoryPool m_memory;

public:
	/**
	 * Constructs a RenderModel which observes the specified GameArena, and creates
	 * scene nodes through the passed SceneManager. The specified number of inital pages
	 * at the specified page size (in bytes) will be created in the memory manager.
	 */
	RenderModel(GameArena& model, SceneManager * mgr, int pageSize, int initPages);

	/** Calls the updateEffects() method of all RenderObjects stored in the RenderModel's render list */
	void updateRenderList(Real elapsedTime, Quaternion camOrientation);

	/** Called whenever a new GameObject is created by the observed GameArena */
	virtual void newGameObject(GameObject * object);

	/** Called whenever a GameObject is destroyed by the observed GameArena */
	virtual void destroyedGameObject(GameObject * object);

	/** Called whenever a new Constraint is generated in the GameArena */
	virtual void newConstraint(Constraint * object);

	/** Called just before a Constraint is destroyed in the GameArena */
	virtual void destroyedConstraint(Constraint * object);

	/** @return THe number of render objects currently managed by this RenderModel */
	int getNumObjects() const;

	/** @return The memory manager used by this RenderModel */
	PagedMemoryPool memoryManager();
};

#endif