#ifndef __RenderModel_h_
#define __RenderModel_h_

#include <Ogre.h>
#include <vector>
#include <sstream>
#include <OgreParticleSystem.h>
#include "GameObjects.h"
#include "Gorilla.h"

using namespace Ogre;

class RenderObject
{
private:
	SceneManager * mp_mgr;

	static int m_nextRenderIndex;

	int m_renderIndex;

public:
	RenderObject(SceneManager * mgr);

	SceneManager * getSceneManager();

	int getRenderIndex();

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateNode(Real elapsedTime, Quaternion camOrientation) = 0;

	virtual void loadSceneResources() = 0;

	virtual void buildNode() = 0;

	virtual void destroyNode() = 0;

	bool operator==(const RenderObject &other) const;
};


class ConstraintRenderObject : public RenderObject
{
private:
	Constraint * mp_constraint;

	SceneNode * mp_node;
	ParticleSystem * mp_particle;

	static bool m_resourcesLoaded;
public:
	/** Constructs a new PhysicsRenderObject */
	ConstraintRenderObject(Constraint * constraint, SceneManager * mgr);

	/** @return A pointer to the model object */
	Constraint * getConstraint();

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateNode(Real elapsedTime, Quaternion camOrientation);

	virtual void loadSceneResources();

	virtual void buildNode();

	virtual void destroyNode();
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
	SphereCollisionObject * getObject();

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateNode(Real elapsedTime, Quaternion camOrientation) = 0;

	virtual void loadSceneResources() = 0;

	virtual void buildNode() = 0;

	virtual void destroyNode() = 0;

	bool operator==(const RenderObject &other) const;
};


class ShipRO : public PhysicsRenderObject
{
private:
	SpaceShip * mp_spaceShip;

	SceneNode * mp_shipNode;
	SceneNode * mp_shipRotateNode;
	Entity * mp_shipEntity;
	Light * mp_spotLight;
	Light * mp_pointLight;
	ParticleSystem * mp_engineParticles;

	static bool m_resourcesLoaded;
public:
	ShipRO(SpaceShip * ship, SceneManager * mgr);

	SpaceShip * getSpaceShip() const;

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateNode(Real elapsedTime, Quaternion camOrientation);

	virtual void loadSceneResources();

	virtual void buildNode();

	virtual void destroyNode();
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
	virtual void updateNode(Real elapsedTime, Quaternion camOrientation);

	virtual void loadSceneResources();

	virtual void buildNode();

	virtual void destroyNode();
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

	Projectile * getProjectile() const;

	/** Updates the node based on passed time and camera orientation (useful for sprites) */
	virtual void updateNode(Real elapsedTime, Quaternion camOrientation);

	virtual void loadSceneResources();

	virtual void buildNode();

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

	/** List of all PhysicsRenderObjects that should be updated and rendered each frame */
	std::vector<PhysicsRenderObject * > m_physicsRenderList;

	std::vector<ConstraintRenderObject * > m_constraintRenderList;

	/** The SceneManager for the scene represented by the RenderModel */
	SceneManager * mp_mgr;

public:
	/**
	 * Constructs a RenderModel which observes the specified GameArena, and creates
	 * scene nodes through the passed SceneManager.
	 */
	RenderModel(GameArena& model, SceneManager * mgr);

	/** Calls the updateNode() method of all RenderObjects stored in the RenderModel's render list */
	void updateRenderList(Real elapsedTime, Quaternion camOrientation);

	/** Called whenever a new GameObject is created by the observed GameArena */
	virtual void newGameObject(GameObject * object);

	/** Called whenever a GameObject is destroyed by the observed GameArena */
	virtual void destroyedGameObject(GameObject * object);

	/** Called whenever a new Constraint is generated in the GameArena */
	virtual void newConstraint(Constraint * object);

	/** Called just before a Constraint is destroyed in the GameArena */
	virtual void destroyedConstraint(Constraint * object);

	int getNumObjects() const;
};

#endif