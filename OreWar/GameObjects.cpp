#include "GameObjects.h"
#include <OgreMatrix4.h>
#include <OgreMath.h>

using namespace Ogre;

// ========================================================================
// BaseObject Implementation
// ========================================================================

BaseObject::BaseObject(Vector3 position, Vector3 heading) :
	m_position(position), m_heading(heading)
{
}

BaseObject::BaseObject(Vector3 position) : m_position(position),
	m_heading(Vector3(1, 0, 0))
{
}

BaseObject::BaseObject() : m_position(Vector3(0, 0, 0)), m_heading(Vector3(1, 0, 0))
{
}

void BaseObject::rotateHeadingAboutY(Real radians)
{
	Matrix4 rotMatrix = Matrix4(
		Math::Cos(radians), 0, -Math::Sin(radians), 0,
		0, 1, 0, 0,
		Math::Sin(radians), 0, Math::Cos(radians), 0,
		0, 0, 0, 1);
	setHeading(rotMatrix * m_heading);
}

void BaseObject::setPosition(Vector3 position)
{
	m_position = position;
}

void BaseObject::setHeading(Vector3 heading)
{
	m_heading = heading;
	m_heading.normalise();
}

Vector3 BaseObject::getPosition()
{
	return m_position;
}

Vector3 BaseObject::getHeading()
{
	return m_heading;
}

// ========================================================================
// BaseObject Implementation
// ========================================================================
PhysicsObject::PhysicsObject(Real mass, Vector3 position, Vector3 heading) :
	BaseObject(position, heading), m_mass(mass), m_velocity(Vector3(0, 0, 0)),
	m_acceleration(Vector3(0, 0, 0)), m_force(Vector3(0, 0, 0))
{
}

PhysicsObject::PhysicsObject(Real mass, Vector3 position) :
	BaseObject(position), m_mass(mass), m_velocity(Vector3(0, 0, 0)),
	m_acceleration(Vector3(0, 0, 0)), m_force(Vector3(0, 0, 0))
{
}

PhysicsObject::PhysicsObject(Real mass) : BaseObject(), m_mass(mass), m_velocity(Vector3(0, 0, 0)),
	m_acceleration(Vector3(0, 0, 0)), m_force(Vector3(0, 0 ,0))
{
}

void PhysicsObject::setVelocity(Vector3 velocity) {
	m_velocity = velocity;
}

void PhysicsObject::setAcceleration(Vector3 acceleration) {
	m_acceleration = acceleration;
}

Vector3 PhysicsObject::getVelocity() {
	return m_velocity;
}

Vector3 PhysicsObject::getAcceleration() {
	return m_acceleration;
}

Vector3 PhysicsObject::getSumForce() {
	return m_force;
}

void PhysicsObject::applyForce(Vector3 force) {
	m_force = m_force + force;
}

void PhysicsObject::clearForces() {
	m_force = Vector3(0, 0, 0);
}

void PhysicsObject::updatePosition(Real timeElapsed) {
	// if(m_mass == 0) {
	// TODO: Throw exception
	// }
	
	m_acceleration = m_force / m_mass;
	m_velocity = m_velocity + (m_acceleration * timeElapsed);
	setPosition(getPosition() + (m_velocity * timeElapsed));
}