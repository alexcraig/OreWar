#ifndef __GameObjects_h_
#define __GameObjects_h_

#include <vector>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreMath.h>
#include "PhysicsEngine.h"
#include "MemoryMgr.h"

using namespace Ogre;

class GameArena;

/**
 * Enumeration used for differentiating between different types of GameObjects
 */
enum ObjectType { SHIP, NPC_SHIP, PROJECTILE, ANCHOR_PROJECTILE, PLANET_CHUNK, STAR, MOON, PLANET };


/**
 * The GameObject class represents any distinct entity in the game world, which
 * may or may not require physics simulation.
 */
class GameObject
{
private:
	/** The memory manager that should be used for any heap allocation required
	 * by this object */
	PagedMemoryPool * mp_memory;

	SphereCollisionObject * mp_physModel;

	Real m_maxHealth;

	Real m_health;

	Real m_maxEnergy;

	Real m_energy;

	// Note: Not implemented in this class, should be used in subclasses
	// implementation of updatePhysics
	Real m_energyRechargeRate;

	/** The type of the object (used for differentiating among derived classes) */
	ObjectType m_type;



public: 
	GameObject(const SphereCollisionObject& object, ObjectType type, 
		Real maxHealth, Real maxEnergy, Real energyRechargeRate, PagedMemoryPool * memoryMgr);

	GameObject(const GameObject& copy);
	~GameObject();

	/** @return The collision object which encapsulates all physics data for this object */
	SphereCollisionObject * phys() const;

	/** @return The type of the object (used for differentiating among derived classes) */
	ObjectType type() const;

	/** @return The memory manager used by this game object for any required heap allocation */
	PagedMemoryPool * memoryManager() const;

	Real health() const;
	Real maxHealth() const;
	Real energy() const;
	Real maxEnergy() const;
	Real energyRecharge() const;

	virtual void updatePhysics(Real timeElapsed) = 0;
	void health(Real health);
	void energy(Real energy);

	void inflictDamage(Real damage);
	void addEnergy(Real energy);
	void drainEnergy(Real Energy);
};


class Projectile : public GameObject
{
private:
	Real m_damage;

public:
	Projectile(const SphereCollisionObject& physModel, ObjectType type, Real damage,
		PagedMemoryPool * memoryMgr);

	void updatePhysics(Real timeElapsed);

	Real damage();
};


class Weapon
{
private:
	// TODO: Reconsider having the memory manager here, might just want to pass
	// a GameArena to fireWeapon

	/** The memory manager that should be used for any heap allocation required
	 * by this object */
	PagedMemoryPool * mp_memory;

	/** The time which should elapse between projectile generations */
	Real m_reloadTime;

	/** The time which has elapsed since the last projectile generation */
	Real m_lastShotCounter;

	/** Flag determining if the ship's weapons are currently loaded */
	bool m_canShoot;

	Real m_energyCost;

public:
	/** Generates a new (loaded) weapon with the specified reload time */
	Weapon(Real reloadTime, Real m_energyCost, PagedMemoryPool * memoryMgr);

	Weapon(const Weapon& copy);
	
	virtual ~Weapon();

	/** @return The memory manager used by this weapon to allocate projectiles */
	PagedMemoryPool * memoryManager() const;

	/** @return True if the weapon is loaded (reload time has expired since last shot) */
	bool canShoot() const;

	void resetShotCounter();

	Real energyCost();

	/** Generates a projectile PhysicsObject, and resets the weapons's reload counter*/
	virtual Projectile fireWeapon(PhysicsObject& origin) = 0;

	void updatePhysics(Real timeElapsed);
};


class PlasmaCannon : public Weapon
{
private:
	/** Flag determing which side the next projectile should be fired from (true if left) */
	bool m_shootLeft;

public:
	PlasmaCannon(PagedMemoryPool * memoryMgr);

	PlasmaCannon(const PlasmaCannon& copy);

	virtual Projectile fireWeapon(PhysicsObject& origin);
};


class AnchorLauncher : public Weapon
{
public:
	AnchorLauncher(PagedMemoryPool * memorygMr);

	AnchorLauncher(const AnchorLauncher& copy);

	virtual Projectile fireWeapon(PhysicsObject& origin);
};


/**
 * The CelestialBody class represents all orbiting physics modelled solar objects 
 * such as stars, moons, and planets.
 */
class CelestialBody : public GameObject {
private:
	/** A pointer to the orbital center of the body (NULL if the body is free standing) */
	CelestialBody * mp_center;

	/** The distance the satelite must maintain from its center (if a center is specified) */
	Real m_radius;

public:
	/** 
	 * Constructs a CelestialBody with no orbital physics
	 * type should be one of: ObjectType::STAR, MOON, or PLANET
	 */
	CelestialBody(ObjectType type, Real mass, Real radius, Vector3 position, PagedMemoryPool * memoryMgr);

	/**
	 * Constructs a CelestialBody in a random position in orbit around the
	 * specified CelestialBody at the specified distance (edge to edge, not center
	 * to center) and speed.
	 */
	CelestialBody(ObjectType type, Real mass, Real radius, CelestialBody * center, 
		Real distance, Real speed, PagedMemoryPool * memoryMgr);

	/** Copy Constructor */
	CelestialBody(const CelestialBody & copy);

	/** 
	 * @return A constraint is generated which maintains a circular orbit at the 
	 * body's current velocity if applied.
	 */
	Constraint constraint() const;

	/** @return True if this body is set to orbit another body */
	bool hasCenter() const;

	/** @return The radius of the celestial body */
	Real radius() const;

	/** @see GameObject::updatePhysics(Real) */
	virtual void updatePhysics(Real timeElapsed);
};


/**
 * Represents a human or NPC controllable space ship which
 * can generate projectiles.
 */
class SpaceShip : public GameObject
{
private:
	/** A list of all weapons currently equipped on the ship */
	std::vector<Weapon *> mp_weapons;

public:
	/** Construct a SpaceShip with the specified mass and size at the specified position */
	SpaceShip(ObjectType type, Real mass, Vector3 position, Real energyRecharge, PagedMemoryPool * memoryMgr);

	/** Construct a SpaceShip with the specified mass and size at the specified position */
	SpaceShip(ObjectType type, Real mass, Vector3 position, PagedMemoryPool * memoryMgr);

	/** Construct a SpaceShip with the specified mass and size at the origin */
	SpaceShip(ObjectType type, Real mass, PagedMemoryPool * memoryMgr);

	/** Copy constructor */
	SpaceShip(const SpaceShip& copy);

	PlasmaCannon * addPlasmaCannon(const PlasmaCannon& weapon);

	AnchorLauncher * addAnchorLauncher(const AnchorLauncher& weapon);

	Projectile * fireWeapon(GameArena& arena, int weaponIndex);

	/** Updates the ship's position and reload status */
	void updatePhysics(Real timeElapsed);
};

/**
 * Interface for listening on a GameArena instance.
 * This interface should be extended by classes which are interested in being notified
 * whenever an object is added to or removed from the game arena
 */
class GameArenaListener
{
public:
	/** Called whenever a new GameObject is generated in the GameArena */
	virtual void newGameObject(GameObject * object) = 0;

	/** Called just before a GameObject is destroyed in the GameArena */
	virtual void destroyedGameObject(GameObject * object) = 0;

	/** Called whenever a new Constraint is generated in the GameArena */
	virtual void newConstraint(Constraint * object) = 0;

	/** Called just before a Constraint is destroyed in the GameArena */
	virtual void destroyedConstraint(Constraint * object) = 0;
};

/**
 * GameArena represents a cube of space in which ships, projectiles, and other
 * objects should undergo physics simulation. The GameArena is responsible for
 * storing references to all involved PhysicsObjects, and notifying listeners
 * when new objects are created and destroyed.
 */
class GameArena
{
private:
	/**
	 * The size of the game arena.
	 * Arenas are a cube centered on the origin, and each wall
	 * is m_arenaSize units from the center (i.e. each wall of the cube
	 * is 2 * m_arenaSize long).
	 */
	Real m_arenaSize;

	SpaceShip * m_playerShip;

	/** A vector of pointers to dynamically allocated memory for all npc ships in the GameArena */
	std::vector<SpaceShip *> m_npcShips;

	/** A vector of pointers to dynamically allocated memory for all projectiles in the GameArena */
	std::vector<Projectile *> m_projectiles;

	/** A vector of pointers to dynamically allocated memory for all celestial bodies in the GameArena */
	std::vector<CelestialBody *> m_bodies;

	/** A vector of pointers to dynamically allocated memory for all projectiles in the GameArena */
	std::vector<Constraint *> m_constraints;

	/** A vector of pointers to GameArenaListener instances registered with the GameArena*/
	std::vector<GameArenaListener *> m_listeners;

	/** The paged memory pool which will store game objects */
	PagedMemoryPool m_memory;

	void notifyObjectCreation(GameObject * object);
	void notifyObjectDestruction(GameObject * object);
	void notifyConstraintCreation(Constraint * object);
	void notifyConstraintDestruction(Constraint * object);
public:
	/** 
	 * Constructs a new, empty GameArena with the specified size and inital
	 * number of memory pages at the set page size
	 */
	GameArena(Real size, int pageSize, int initPages);

	/** Deconstructor */
	~GameArena();

	/** Registers a GameArenaListener with the GameArena */
	void addGameArenaListener(GameArenaListener * listener);

	/** Unregisters a GameArenaListener from the GameArena */
	void removeGameArenaListener(GameArenaListener * listener);

	Real size() const;

	/**
	 * Adds a SpaceShip to the GameArena.
	 * Note: A copy of the passed SpaceShip is created and stored in dynamic memory.
	 * @return	A pointer to the new copy of the SpaceShip. This pointer will remain valuid
	 *			for the lifetime of the GameArena.
	 */
	SpaceShip * setPlayerShip(const SpaceShip& ship);

	Constraint * addConstraint(const Constraint& constraint);

	/**
	 * Adds the specified body to the game arena.
	 * The corresponding constraint is also generated and added.
	 * @return A pointer to the newly generated copy
	 */
	CelestialBody * addBody(const CelestialBody& body);

	/** 
	 * Destroys a celestial body, erasing it from the vector of stored bodies.
	 * Any attached constraints are also destroyed.
	 */
	std::vector<CelestialBody * >::iterator destroyBody(CelestialBody * body);

	std::vector<Constraint * >::iterator destroyConstraint(Constraint * constraint);

	SpaceShip * addNpcShip(const SpaceShip& ship);

	/**
	 * Adds a projectile to the GameArena.
	 * Note: A copy of the passed PhysicsObject is created and stored in dynamic memory.
	 * @return	A pointer to the new copy of the PhysicsObject. This pointer will remain valuid
	 *			for the lifetime of the GameArena.
	 */
	Projectile * addProjectile(const Projectile& projectile);

	/** Destroys a projectile, erasing it from the vector of stored PhysicsObjects */
	std::vector<Projectile * >::iterator destroyProjectile(Projectile * projectile);

	std::vector<SpaceShip * >::iterator destroyNpcShip(SpaceShip * npcShip);

	/** @return A pointer to the player's ship */
	SpaceShip * playerShip();

	/** @return The list of pointers to all active projectiles */
	std::vector<Projectile *> * projectiles();

	/** @return The list of pointers to all active ships */
	std::vector<SpaceShip *> * npcShips();

	/** @return The list of pointers to all celestial bodies */
	std::vector<CelestialBody *> * bodies();

	/** 
	 * @return A pointer to the PhysicsObject produced by generating a projectile from the passed ship 
	 * and stored in dynamic memory.
	 */
	Projectile * fireProjectileFromShip(SpaceShip * ship, int weaponIndex);

	/** Updates the physics of all ships and projectiles in the arena */
	void updatePhysics(Real timeElapsed);

	/** Generates a randomly distributed solar system (collection of celestial objects) */
	void generateSolarSystem();

	/** @return The memory manager used by this GameArena */
	PagedMemoryPool * memoryManager();
};

#endif