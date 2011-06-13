#ifndef __PhysicsEngine_h_
#define __PhysicsEngine_h_

#include <vector>
#include <OgreVector3.h>
#include <OgreQuaternion.h>

using namespace Ogre;

/**
 * The BaseObject class represents an entity in the OreWar game world.
 * BaseObjects use an X, Y, Z coordinate system following Ogre's axis conventions.
 * (see http://www.ogre3d.org/tikiwiki/Basic+Tutorial+1&structure=Tutorials)
 * The coordinate system is centered with 0,0,0 in the exact center of the game
 * world. Orientation is stored as a quaternion with <0, 0, -1> as the base
 * heading (to correspond with the default for Ogre cameras, scene nodes, etc.)
 */
class BaseObject
{
private:
	/** The current position of the object */
	Vector3 m_position;

	/** The current orientation of the object (as a rotation from the base <0, 0, -1> heading) */
	Quaternion m_orientation;

public:
	/** Construct a new BaseObject at a specified position with default heading (<0, 0,-1>) */
	BaseObject(Vector3 position);

	/** Copy constructor */
	BaseObject(const BaseObject& copy);

	/** Construct a new BaseObject at the origin with default heading (<0, 0,-1>) */
	BaseObject();

	/** Yaw the object by the specified number of radians (positive for right yaw) */
	void yaw(Radian radians);

	/** Roll the object by the specified number of radians (positive for counter-clockwise roll) */
	void roll(Radian radians);

	/** Pitch the object by the specified number of radians (positive for forward pitch) */
	void pitch(Radian radians);

	/** Sets the position of the object */
	void position(Vector3 position);

	/** Calculates the relative displacement between this object and the specified object */
	Vector3 displacement(const BaseObject& other);

	/** @return The position of the object */
	Vector3 position() const;

	/** @return The heading of the object (normalised vector pointing "forward" fom it's current orientation) */
	Vector3 heading() const;

	/** @return The normal vector of the object (normalised vector pointing "up" from it's current orientation) */
	Vector3 normal() const;

	/** @return The orientation quaternion of the object (with <0, 0, -1> as the base heading) */
	Quaternion orientation() const;

	/** Sets the orientation of the object */
	void orientation(Quaternion orientation);
};


/**
 * The PhysicsObject class represents an object in the OreWar game world which is subject
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

	/** The sum vector of all persistent forces current applied on the object. */
	Vector3 m_force;

	/** 
	 * The sum vector of all temporary forces currently applied on the object.
	 * Temporary forces are cleared on each physics update.
	 */
	Vector3 m_tempForce;

public:
	/** 
	 * Construct a PhysicsObject at the given position coordinates with the
	 * specified mass and default heading (<0, 0, -1>).
	 */
	PhysicsObject(Real mass, Vector3 position);

	/**
	 * Construct a PhysicsObject at the origin with the specified mass 
	 * with default heading (<0, 0, -1>)
	 */
	PhysicsObject(Real mass);

	/** Copy constructor */
	PhysicsObject(const PhysicsObject& copy);

	/** @return The mass of the object */
	Real mass() const;

	/** Sets the velocity of the object */
	void velocity(Vector3 velocity);

	/** Sets the acceleration of the object */
	void acceleration(Vector3 acceleration);

	/** @return The velocity of the object */
	Vector3 getVelocity() const;

	/** @return The acceleration of the object */
	Vector3 acceleration() const;

	/** @return The vector sum of permanent all (not cleared on physics update) forces on the object */
	Vector3 sumForces() const;

	/** @return The vector sum of all temporary (cleared on physics update) forces on the object */
	Vector3 sumTempForces() const;

	/** 
	 * Applies an addititve force on the object which will be taken into account
	 * on the next physics update.
	 */
	void applyForce(Vector3 force);

	/** 
	 * Applies an addititve force on the object which will be taken into account
	 * and cleared on the next physics update.
	 */
	void applyTempForce(Vector3 force);

	/** Remove all forces (temporary and persistent) all force currently applied to the object */
	void clearForces();

	/**
	 * Updates the object's position, taking all physics parameters into
	 * account as well as the time elapsed since the last position update
	 * (in seconds).
	 */
	virtual void updatePhysics(Real timeElapsed);
};

/**
 * The Constaint class represents a connection between two physics objects
 * which should apply force based on some condition (ropes or springs for
 * example.
 */
class Constraint
{
private:
	/** The originating object of the constraint */
	PhysicsObject * m_origin;

	/** The target object of the constraint */
	PhysicsObject * m_target;

	/** The distance between the two objects at the time of creation */
	Real m_distance;

public:
	/** Construct a constraint between the two provided objects */
	Constraint(PhysicsObject * origin, PhysicsObject * target);

	/** Copy constructor */
	Constraint(const Constraint& copy);

	/** @return The origin object of the constraint */
	PhysicsObject * getOrigin();

	/** @return The target object of the constraint */
	PhysicsObject * getTarget();

	/** Applies temporary forces on one or both of the constraint objects based on the elapsed time */
	void applyForces(Real timeElapsed);
};


/**
 * The SphereCollisionObject represents a physical object whose collisions should be
 * modelled as a sphere.
 */
class SphereCollisionObject : public PhysicsObject
{
private:
	/** The radius of the object */
	Real m_radius;
public:
	/** 
	 * Construct a SphereCollisionObject at the given position coordinates with the
	 * specified mass and default heading (<0, 0, -1>).
	 */
	SphereCollisionObject( Real radius, Real mass, Vector3 position);

	/**
	 * Construct a SphereCollisionObject at the origin with the specified mass 
	 * with default heading (<0, 0, -1>)
	 */
	SphereCollisionObject(Real radius, Real mass);

	/** Copy constructor */
	SphereCollisionObject(const SphereCollisionObject& copy);

	/** @return The radius of the object */
	Real radius() const;

	/** @return True if the passed object intersects with the sphere */
	bool checkCollision(const SphereCollisionObject& object) const;
};

#endif