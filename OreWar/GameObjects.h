#ifndef __GameObjects_h_
#define __GameObjects_h_

#include <vector>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreMath.h>
#include "PhysicsEngine.h"

using namespace Ogre;


class GameArena;


class GameObject
{
private:
	// TODO: The object type should probably move into this class and out
	//	     of the physics subsystem.

	SphereCollisionObject * mp_physModel;

	Real m_maxHealth;

	Real m_health;

	Real m_maxShields;

	Real m_shields;

public: 
	GameObject(const SphereCollisionObject& object, Real maxHealth, Real maxShields);
	GameObject(const GameObject& copy);
	~GameObject();

	SphereCollisionObject * getPhysicsModel() const;

	ObjectType getType() const;

	Real getHealth() const;
	Real getMaxHealth() const;
	Real getShields() const;
	Real getMaxShields() const;

	virtual void updatePhysics(Real timeElapsed) = 0;
	void setHealth(Real health);
	void setShields(Real shields);
};


class Projectile : public GameObject
{
private:
	Real m_damage;

public:
	Projectile(const SphereCollisionObject& physModel, Real damage);

	void updatePhysics(Real timeElapsed);

	Real getDamage();
};


class Weapon
{
private:
	/** The time which should elapse between projectile generations */
	Real m_reloadTime;

	/** The time which has elapsed since the last projectile generation */
	Real m_lastShotCounter;

	/** Flag determining if the ship's weapons are currently loaded */
	bool m_canShoot;

public:
	/** Generates a new (loaded) weapon with the specified reload time */
	Weapon(Real reloadTime);

	Weapon(const Weapon& copy);
	
	virtual ~Weapon();

	/** @return True if the weapon is loaded (reload time has expired since last shot) */
	bool canShoot() const;

	void resetShotCounter();

	/** Generates a projectile PhysicsObject, and resets the weapons's reload counter*/
	virtual Projectile generateProjectile(PhysicsObject& origin) = 0;

	void updatePhysics(Real timeElapsed);
};


class PlasmaCannon : public Weapon
{
private:
	/** Flag determing which side the next projectile should be fired from (true if left) */
	bool m_shootLeft;

public:
	PlasmaCannon();

	PlasmaCannon(const PlasmaCannon& copy);

	virtual Projectile generateProjectile(PhysicsObject& origin);
};


class AnchorLauncher : public Weapon
{
public:
	AnchorLauncher();

	AnchorLauncher(const AnchorLauncher& copy);

	virtual Projectile generateProjectile(PhysicsObject& origin);
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
	SpaceShip(ObjectType type, Real mass, Vector3 position);

	/** Construct a SpaceShip with the specified mass and size at the origin */
	SpaceShip(ObjectType type, Real mass);

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

	/** A vector of pointers to dynamically allocated memory for all projectiles in the GameArena */
	std::vector<Constraint *> m_constraints;

	/** A vector of pointers to GameArenaListener instances registered with the GameArena*/
	std::vector<GameArenaListener *> m_listeners;

	void notifyObjectCreation(GameObject * object);
	void notifyObjectDestruction(GameObject * object);
	void notifyConstraintCreation(Constraint * object);
	void notifyConstraintDestruction(Constraint * object);
public:
	/** Constructs a new, empty GameArena with the specified size. */
	GameArena(Real size);

	/** Registers a GameArenaListener with the GameArena */
	void addGameArenaListener(GameArenaListener * listener);

	/** Unregisters a GameArenaListener from the GameArena */
	void removeGameArenaListener(GameArenaListener * listener);

	Real getSize() const;

	/**
	 * Adds a SpaceShip to the GameArena.
	 * Note: A copy of the passed SpaceShip is created and stored in dynamic memory.
	 * @return	A pointer to the new copy of the SpaceShip. This pointer will remain valuid
	 *			for the lifetime of the GameArena.
	 */
	SpaceShip * setPlayerShip(const SpaceShip& ship);

	Constraint * addConstraint(const Constraint& constraint);

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

	SpaceShip * getPlayerShip();

	/** @return The list of pointers to all active projectiles */
	std::vector<Projectile *> * getProjectiles();

	/** @return The list of pointers to all active ships */
	std::vector<SpaceShip *> * getNpcShips();

	/** 
	 * @return A pointer to the PhysicsObject produced by generating a projectile from the passed ship 
	 * and stored in dynamic memory.
	 */
	Projectile * fireProjectileFromShip(SpaceShip * ship, int weaponIndex);

	/** Updates the physics of all ships and projectiles in the arena */
	void updatePhysics(Real timeElapsed);
};

#endif