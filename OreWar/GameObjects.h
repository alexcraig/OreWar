#ifndef __GameObjects_h_
#define __GameObjects_h_

#include <vector>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreMath.h>
#include "PhysicsEngine.h"

using namespace Ogre;

/**
 * PlayerShip represents a ship controllable by a human player which
 * can generate projectiles.
 */
class PlayerShip : public SphereCollisionObject
{
private:
	/** The time which should elapsed between projectile generations */
	Real m_reloadTime;

	/** The time which has elapsed since the last projectile generation */
	Real m_lastShotCounter;

	/** Flag determining if the ship's weapons are currently loaded */
	bool m_canShoot;

	/** True if the left weapon should be used next, false if the right should be used */
	bool m_shootLeft;

public:
	/** Construct a PlayerShip with the specified mass at the specified position */
	PlayerShip(Real mass, Vector3 position);

	/** Construct a PlayerShip with the specified mass at the origin */
	PlayerShip(Real mass);

	/** Copy constructor */
	PlayerShip(const PlayerShip& copy);

	/** @return True if the ship has a loaded weapon */
	bool canShoot() const;

	/** Generates a projectile PhysicsObject, and resets the ship's reload counter*/
	SphereCollisionObject generateProjectile();

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
	/** Called whenever a new PhysicsObject is generated in the GameArena */
	virtual void newPhysicsObject(PhysicsObject * object) = 0;

	/** Called just before a PhysicsObject is destroyed in the GameArena */
	virtual void destroyedPhysicsObject(PhysicsObject * object) = 0;

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

	PlayerShip * m_playerShip;

	/** A vector of pointers to dynamically allocated memory for all npc ships in the GameArena */
	std::vector<SphereCollisionObject *> m_npcShips;

	/** A vector of pointers to dynamically allocated memory for all projectiles in the GameArena */
	std::vector<SphereCollisionObject *> m_projectiles;

	/** A vector of pointers to dynamically allocated memory for all projectiles in the GameArena */
	std::vector<Constraint *> m_constraints;

	/** A vector of pointers to GameArenaListener instances registered with the GameArena*/
	std::vector<GameArenaListener *> m_listeners;

	void notifyPhysicsCreation(PhysicsObject * object);
	void notifyPhysicsDestruction(PhysicsObject * object);
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
	 * Adds a PlayerShip to the GameArena.
	 * Note: A copy of the passed PlayerShip is created and stored in dynamic memory.
	 * @return	A pointer to the new copy of the PlayerShip. This pointer will remain valuid
	 *			for the lifetime of the GameArena.
	 */
	PlayerShip * setPlayerShip(const PlayerShip& ship);

	Constraint * addConstraint(const Constraint& constraint);

	std::vector<Constraint * >::iterator destroyConstraint(Constraint * constraint);

	SphereCollisionObject * addNpcShip(const SphereCollisionObject& ship);

	/**
	 * Adds a projectile to the GameArena.
	 * Note: A copy of the passed PhysicsObject is created and stored in dynamic memory.
	 * @return	A pointer to the new copy of the PhysicsObject. This pointer will remain valuid
	 *			for the lifetime of the GameArena.
	 */
	SphereCollisionObject * addProjectile(const SphereCollisionObject& projectile);

	/** Destroys a projectile, erasing it from the vector of stored PhysicsObjects */
	std::vector<SphereCollisionObject * >::iterator destroyProjectile(SphereCollisionObject * projectile);

	std::vector<SphereCollisionObject * >::iterator destroyNpcShip(SphereCollisionObject * npcShip);

	PlayerShip * getPlayerShip();

	/** @return The list of pointers to all active projectiles */
	std::vector<SphereCollisionObject *> * getProjectiles();

	/** @return The list of pointers to all active ships */
	std::vector<SphereCollisionObject *> * getNpcShips();

	/** 
	 * @return A pointer to the PhysicsObject produced by generating a projectile from the passed ship 
	 * and stored in dynamic memory.
	 */
	SphereCollisionObject * fireProjectileFromShip(PlayerShip * ship);

	/** Updates the physics of all ships and projectiles in the arena */
	void updatePhysics(Real timeElapsed);
};

#endif