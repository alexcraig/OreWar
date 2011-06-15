#include "PhysicsEngine.h"
#include <OgreMath.h>
#include <OgrePlane.h>

using namespace Ogre;

// ========================================================================
// BaseObject Implementation
// ========================================================================

BaseObject::BaseObject(Vector3 position) :
	m_position(position), m_orientation(Quaternion::IDENTITY)
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
	orientation(m_orientation * q);
}

void BaseObject::roll(Radian radians)
{
	Quaternion q(Radian(radians), Vector3::UNIT_Z);
	orientation(m_orientation * q);
}

void BaseObject::pitch(Radian radians)
{
	Quaternion q(Radian(radians), Vector3::UNIT_X);
	orientation(m_orientation * q);
}

void BaseObject::position(Vector3 position)
{
	m_position = position;
}

Vector3 BaseObject::displacement(const BaseObject& other) {
	return other.position() - position();
}

Vector3 BaseObject::position() const
{
	return m_position;
}

Vector3 BaseObject::heading() const
{
	return m_orientation * Vector3(0, 0, -1);
}

Vector3 BaseObject::normal() const
{
	return m_orientation * Vector3(0, 1, 0);
}

Quaternion BaseObject::orientation() const 
{
	return m_orientation;
}

void BaseObject::orientation(Quaternion orientation) {
	m_orientation = orientation;
	m_orientation.normalise();
}


// ========================================================================
// PhysicsObject Implementation
// ========================================================================

PhysicsObject::PhysicsObject(Real mass, Vector3 position) :
	BaseObject(position), m_mass(mass), m_velocity(0, 0, 0),
	m_acceleration(0, 0, 0), m_force(0, 0, 0), m_tempForce(0, 0, 0)
{
}

PhysicsObject::PhysicsObject(Real mass) : BaseObject(), m_mass(mass), 
	m_velocity(0, 0, 0), m_acceleration(0, 0, 0), m_force(0, 0 ,0), m_tempForce(0, 0, 0)
{
}

PhysicsObject::PhysicsObject(const PhysicsObject& copy) : BaseObject(copy), 
	m_mass(copy.m_mass), m_velocity(copy.m_velocity), m_acceleration(copy.m_acceleration),
	m_force(copy.m_force), m_tempForce(copy.m_tempForce)
{
}

Real PhysicsObject::mass() const
{
	return m_mass;
}

void PhysicsObject::velocity(Vector3 velocity) 
{
	m_velocity = velocity;
}

void PhysicsObject::acceleration(Vector3 acceleration) 
{
	m_acceleration = acceleration;
}

Vector3 PhysicsObject::velocity() const
{
	return m_velocity;
}

Vector3 PhysicsObject::acceleration() const
{
	return m_acceleration;
}

Vector3 PhysicsObject::sumForces() const
{
	return m_force;
}

Vector3 PhysicsObject::sumTempForces() const
{
	return m_tempForce;
}

void PhysicsObject::applyForce(Vector3 force) 
{
	m_force = m_force + force;
}

void PhysicsObject::applyTempForce(Vector3 force) 
{
	m_tempForce = m_tempForce + force;
}

void PhysicsObject::clearForces() 
{
	m_force = Vector3(0, 0, 0);
	m_tempForce = Vector3(0, 0, 0);
}

void PhysicsObject::updatePhysics(Real timeElapsed) 
{
	// if(m_mass == 0) {
	// TODO: Throw exception
	// }
	
	m_acceleration = (m_force + m_tempForce) / m_mass;
	m_velocity = m_velocity + (m_acceleration * timeElapsed);
	position(position() + (m_velocity * timeElapsed));
	m_tempForce = Vector3(0, 0, 0);
}


// ========================================================================
// Constraint Implementation
// ========================================================================
Constraint::Constraint(PhysicsObject * origin, PhysicsObject * target, bool rigid) :
	m_origin(origin), m_target(target), 
	m_distance(origin->displacement(*((BaseObject *)target)).length()),
	m_rigidSpeed((origin->velocity() - target->velocity()).length()),
	m_rigid(rigid)
{
}

Constraint::Constraint(const Constraint& copy) :
	m_origin(copy.m_origin), m_target(copy.m_target), m_distance(copy.m_distance),
	m_rigidSpeed(copy.m_rigidSpeed), m_rigid(copy.m_rigid)
{
}

PhysicsObject * Constraint::getOrigin()
{
	return m_origin;
}

PhysicsObject * Constraint::getTarget()
{
	return m_target;
}

void Constraint::applyForces(Real timeElapsed)
{
	if(timeElapsed == 0) {
		return;
	}
	// Spring based constraint
	/*
	Real distance = m_origin->displacement(*m_target).length();
	Real appliedForce = Math::Pow((distance - m_distance), 2) * 1 + Math::Abs(distance - m_distance) * 3;
	if(distance < m_distance) {
		appliedForce = -appliedForce;
	}

	m_origin->applyTempForce((m_origin->displacement(*m_target)).normalisedCopy()
		* appliedForce);

	m_target->applyTempForce((m_target->displacement(*m_origin)).normalisedCopy()
		* appliedForce);
	*/

	// Orbit constraint
	Vector3 normalVector = m_target->displacement(*m_origin);
	if(isRigid() || normalVector.length() > m_distance) {
		normalVector.normalise();
		Plane normalPlane = Plane(normalVector, 0);
		normalPlane.normalise();
		Vector3 relVelocity = m_origin->velocity() - m_target->velocity();
		Vector3 desiredVelocity;
		if(isRigid()) {
			m_origin->position(m_target->position() + (m_distance * normalVector));
			desiredVelocity = (normalPlane.projectVector(relVelocity).normalisedCopy() * m_rigidSpeed) + m_target->velocity();
		} else {
			desiredVelocity = (normalPlane.projectVector(relVelocity) + m_target->velocity()).normalisedCopy() * relVelocity.length();
		}

		Vector3 velocityOffset = desiredVelocity - m_origin->velocity();
		m_origin->applyTempForce(((velocityOffset * m_origin->mass()) / timeElapsed));
	}
}


bool Constraint::isRigid() {
	return m_rigid;
}


// ========================================================================
// SphereCollisionObject Implementation
// ========================================================================

SphereCollisionObject::SphereCollisionObject(Real radius, Real mass, Vector3 position)
	: PhysicsObject(mass, position), m_radius(radius)
{
}

SphereCollisionObject::SphereCollisionObject(Real radius, Real mass)
	: PhysicsObject(mass), m_radius(radius)
{
}

SphereCollisionObject::SphereCollisionObject(const SphereCollisionObject& copy)
	: PhysicsObject(copy), m_radius(copy.m_radius)
{
}

Real SphereCollisionObject::radius() const
{
	return m_radius;
}

bool SphereCollisionObject::checkCollision(const SphereCollisionObject& object) const
{ 
	return position().squaredDistance(object.position()) <= Math::Pow(radius() + object.radius(), 2);
}