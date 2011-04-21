#include "PhysicsEngine.h"

using namespace Ogre;

// ========================================================================
// BaseObject Implementation
// ========================================================================

BaseObject::BaseObject(Vector3 position) :
	m_position(position), m_orientation(Radian(1), Vector3(0, 0, 0))
{
}

BaseObject::BaseObject(const BaseObject& copy) : m_position(copy.m_position), 
	m_orientation(copy.m_orientation)
{
}

BaseObject::BaseObject() : 
	m_position(0, 0, 0), m_orientation(Radian(1), Vector3(0, 0, 0))
{
}

void BaseObject::yaw(Radian radians)
{
	Quaternion q(Radian(radians), Vector3::UNIT_Y);
	setOrientation(m_orientation * q);
}

void BaseObject::roll(Radian radians)
{
	Quaternion q(Radian(radians), Vector3::UNIT_Z);
	setOrientation(m_orientation * q);
}

void BaseObject::pitch(Radian radians)
{
	Quaternion q(Radian(radians), Vector3::UNIT_X);
	setOrientation(m_orientation * q);
}

void BaseObject::setPosition(Vector3 position)
{
	m_position = position;
}

Vector3 BaseObject::getPosition() const
{
	return m_position;
}

Vector3 BaseObject::getHeading() const
{
	return m_orientation * Vector3(0, 0, -1);
}

Vector3 BaseObject::getNormal() const
{
	return m_orientation * Vector3(0, 1, 0);
}

Quaternion BaseObject::getOrientation() const 
{
	return m_orientation;
}

void BaseObject::setOrientation(Quaternion orientation) {
	m_orientation = orientation;
	m_orientation.normalise();
}

// ========================================================================
// PhysicsObject Implementation
// ========================================================================

PhysicsObject::PhysicsObject(ObjectType type, Real mass, Vector3 position) :
	BaseObject(position), m_type(type), m_mass(mass), m_velocity(0, 0, 0),
	m_acceleration(0, 0, 0), m_force(0, 0, 0)
{
}

PhysicsObject::PhysicsObject(ObjectType type, Real mass) : BaseObject(), m_type(type), m_mass(mass), 
	m_velocity(0, 0, 0), m_acceleration(0, 0, 0), m_force(Vector3(0, 0 ,0))
{
}

PhysicsObject::PhysicsObject(const PhysicsObject& copy) : BaseObject(copy), m_type(copy.m_type), 
	m_mass(copy.m_mass), m_velocity(copy.m_velocity), m_acceleration(copy.m_acceleration),
	m_force(copy.m_force)
{
}

ObjectType PhysicsObject::getType() const
{
	return m_type;
}

Real PhysicsObject::getMass() const
{
	return m_mass;
}

void PhysicsObject::setVelocity(Vector3 velocity) 
{
	m_velocity = velocity;
}

void PhysicsObject::setAcceleration(Vector3 acceleration) 
{
	m_acceleration = acceleration;
}

Vector3 PhysicsObject::getVelocity() const
{
	return m_velocity;
}

Vector3 PhysicsObject::getAcceleration() const
{
	return m_acceleration;
}

Vector3 PhysicsObject::getSumForce() const
{
	return m_force;
}

void PhysicsObject::applyForce(Vector3 force) 
{
	m_force = m_force + force;
}

void PhysicsObject::clearForces() 
{
	m_force = Vector3(0, 0, 0);
}

void PhysicsObject::updatePhysics(Real timeElapsed) 
{
	// if(m_mass == 0) {
	// TODO: Throw exception
	// }
	
	m_acceleration = m_force / m_mass;
	m_velocity = m_velocity + (m_acceleration * timeElapsed);
	setPosition(getPosition() + (m_velocity * timeElapsed));
}