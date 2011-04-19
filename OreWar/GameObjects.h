#ifndef __GameObjects_h_
#define __GameObjects_h_

#include <vector>
#include <OgreVector3.h>

using namespace Ogre;

/**
 * The BaseObject class represents an entity in the OreWar game world.
 * BaseObjects use an X, Y, Z coordinate system following Ogre's axis conventions.
 * (see http://www.ogre3d.org/tikiwiki/Basic+Tutorial+1&structure=Tutorials)
 * The coordinate system is centered with 0,0,0 in the exact center of the game
 * world.
 */
class BaseObject
{
private:
	/** The current position of the object */
	Vector3 m_position;

	/** The current heading of the object (not normalized) */
	Vector3 m_heading;

public:
	/** Construct a new BaseObject at a specified position with a specified heading */
	BaseObject(Vector3 position, Vector3 heading);

	/** Construct a new BaseObject at a specified position with default heading (<1,0,0>) */
	BaseObject(Vector3 position);

	/** Construct a new BaseObject at the origin with default heading (<1,0,0>) */
	BaseObject();

	/** Rotates the objects heading clockwise about the Y axis by the specified number of radians */
	void rotateHeadingAboutY(Real radians);

	/** Sets the position of the object */
	void setPosition(Vector3 position);

	/** Sets the heading of the object (not normalized)*/
	void setHeading(Vector3 heading);

	/** @return The position of the object */
	Vector3 getPosition();

	/** @return The heading of the object (not normalized) */
	Vector3 getHeading();
};

/**
 * The PhysicsObject class represents an object in the OreWar gameworld which is subject
 * to physics simulation. Physics objects have a set mass, and forces can be applied
 * to produce motion when the updatePosition() method is called. The velocity and acceleration
 * can also be directly manipulated in cases where forces are unnecessary. All properties
 * use the same coordinate orientation as those in the BaseObject class.
 */
class PhysicsObject : public BaseObject
{
private:
	/** The mass of the object */
	Real m_mass;

	/** The velocity of the object */
	Vector3 m_velocity;

	/** The acceleration of the object */
	Vector3 m_acceleration;

	/** The sum vector of all forces current applied on the object */
	Vector3 m_force;

public:
	/** 
	 * Construct a PhysicsObject at the given position coordinates with the
	 * specified mass and heading.
	 */
	PhysicsObject(Real mass, Vector3 position, Vector3 heading);

	/** 
	 * Construct a PhysicsObject at the given position coordinates with the
	 * specified mass and default heading (<1,0,0>).
	 */
	PhysicsObject(Real mass, Vector3 position);

	/**
	Construct a PhysicsObject at the origin with the specified mass 
	 * with default heading (<1,0,0>)
	 */
	PhysicsObject(Real mass);

	/** Sets the velocity of the object */
	void setVelocity(Vector3 velocity);

	/** Set the Y (vertical) velocity of the object */
	void setAcceleration(Vector3 acceleration);

	/** @return The velocity of the object */
	Vector3 getVelocity();

	/** @return The acceleration of the object */
	Vector3 getAcceleration();

	/** @return The vector sum of all forces on the object */
	Vector3 getSumForce();

	/** 
	 * Applies an addititve force on the object which will be taken into account
	 * on the next physics update.
	 */
	void applyForce(Vector3 force);

	/** Cancel all force currently applied to the object */
	void clearForces();

	/**
	 * Updates the object's position, taking all physics parameters into
	 * account as well as the time elapsed since the last position update
	 * (in seconds).
	 */
	void updatePosition(Real timeElapsed);
};

#endif