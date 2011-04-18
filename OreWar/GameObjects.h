#ifndef __GameObjects_h_
#define __GameObjects_h_

/**
 * The BaseObject class represents an entity in the OreWar game world.
 * A BaseObject has an X and Y position coordinate, where X represents the
 * horizontal axis (0 at origin, positive to the right) and Y represents 
 * the vertical axis (0 at origin, positive in the "up" direction). The
 * coordinate system is centered with 0,0 in the exact center of the game
 * world.
 */
class BaseObject
{
private:
	/** The horizontal position coordinate of the object (0 at origin, positive to the right) */
	float m_xPos;
	/** The vertical position coordinate of the object (0 at origin, positive in the "up" direction) */
	float m_yPos;

public:
	/** Construct a new BaseObject at a specified position */
	BaseObject(float xPos, float yPos);

	/** Construct a new BaseObject at the origin */
	BaseObject();

	/** Set the X (horizontal) position coordinate of the object */
	void setXPos(float newPos);

	/** Set the Y (vertical) position coordinate of the object */
	void setYPos(float newPos);

	/** @return The X (horizontal) position coordinate of the object */
	float getXPos();

	/** @return The Y (vertical) position coordinate of the object */
	float getYPos();

	/** Deconstructor */
	virtual ~BaseObject();
};

/**
 * The PhysicsObject class represents an object in the OreWar gameworld which is subject
 * to physics simulation. Physics objects have a set mass, and forces can be applied
 * to produce motion when the updatePosition() method is called. The velocity and acceleration
 * can also be directly manipulated if cases where forces are unnecessary. All properties
 * use the same coordinate orientation as those in the BaseObject class.
 */
class PhysicsObject : public BaseObject
{
private:
	/** The mass of the object */
	float m_mass;
	/** The horizontal velocity of the object */
	float m_xVel;
	/** The vertical velocity of the object */
	float m_yVel;
	/** The horizontal acceleration of the object */
	float m_xAccel;
	/** The vertical acceleration of the object */
	float m_yAccel;
	/** The horizontal force applied on the object */
	float m_xForce;
	/** The vertical force applied on the object */
	float m_yForce;

public:
	/**Construct a PhysicsObject at the origin with the specified mass */
	PhysicsObject(float mass);

	/** 
	 * Construct a PhysicsObject at the given position coordinates with the
	 * specified mass.
	 */
	PhysicsObject(float mass, float xPos, float yPos);

	/** Set the X (horizontal) velocity of the object */
	void setXVel(float newX);

	/** Set the Y (vertical) velocity of the object */
	void setYVel(float newY);

	/** @return The X (horizontal) velocity of the object */
	float getXVel();

	/** @return The Y (vertical) velocity of the object */
	float getYVel();

	/** Set the X (horizontal) acceleration of the object */
	void setXAccel(float newX);

	/** Set the Y (vertical)acceleration of the object */
	void setYAccel(float newY);

	/** @return The X (horizontal) acceleration of the object */
	float getXAccel();

	/** @return The Y (vertical) accceleration of the object */
	float getYAccel();

	/** Applies a force on the object with the specified X and Y components */
	void applyForce(float xForce, float yForce);

	/** Cancel all force currently applied to the object */
	void clearForces();

	/**
	 * Updates the object's position, taking all physics parameters into
	 * account as well as the time elapsed since the last position update
	 * (in seconds).
	 */
	void updatePosition(float timeElapsed);
};

#endif