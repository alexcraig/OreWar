#include "GameObjects.h"
#include <OgreMatrix4.h>

using namespace Ogre;

// ========================================================================
// BaseObject Implementation
// ========================================================================

BaseObject::BaseObject(Vector3 position) :
	m_position(position), m_orientation(Radian(1), Vector3(0, 0, 0))
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

Vector3 BaseObject::getPosition()
{
	return m_position;
}

Vector3 BaseObject::getHeading()
{
	return m_orientation * Vector3(0, 0, -1);
}

Quaternion BaseObject::getOrientation() 
{
	return m_orientation;
}

void BaseObject::setOrientation(Quaternion orientation) {
	m_orientation = orientation;
	m_orientation.normalise();
}

// ========================================================================
// BaseObject Implementation
// ========================================================================
PhysicsObject::PhysicsObject(Real mass, Vector3 position) :
	BaseObject(position), m_mass(mass), m_velocity(0, 0, 0),
	m_acceleration(0, 0, 0), m_force(0, 0, 0)
{
}

PhysicsObject::PhysicsObject(Real mass) : BaseObject(), m_mass(mass), m_velocity(0, 0, 0),
	m_acceleration(0, 0, 0), m_force(Vector3(0, 0 ,0))
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

void PhysicsObject::updatePhysics(Real timeElapsed) {
	// if(m_mass == 0) {
	// TODO: Throw exception
	// }
	
	m_acceleration = m_force / m_mass;
	m_velocity = m_velocity + (m_acceleration * timeElapsed);
	setPosition(getPosition() + (m_velocity * timeElapsed));
}


// ========================================================================
// GameArena Implementation
// ========================================================================

GameArena::GameArena(Real size) : m_arenaSize(size), m_ships(), m_projectiles()
{
	m_projectiles.reserve(1000); // TESTING
}

void GameArena::addShip(PhysicsObject ship)
{
	m_ships.push_back(ship);
}

void GameArena::addProjectile(PhysicsObject projectile)
{
	m_projectiles.push_back(projectile);
}

void GameArena::fireProjectileFromShip(PhysicsObject ship)
{
	PhysicsObject projectile = PhysicsObject(1, ship.getPosition());
	projectile.setVelocity(ship.getVelocity() + ship.getHeading() * 1000);
	projectile.setOrientation(ship.getOrientation());
	addProjectile(projectile);
}

std::vector<PhysicsObject> * GameArena::getProjectiles() {
	return & m_projectiles;
}

std::vector<PhysicsObject> * GameArena::getShips() {
	return & m_ships;
}

void GameArena::updatePhysics(Real timeElapsed)
{
	int i;
	for(i = 0; i < m_ships.size(); i++) {
		m_ships[i].updatePhysics(timeElapsed);
		if(m_ships[i].getPosition().x > m_arenaSize) {
			m_ships[i].setPosition(m_ships[i].getPosition() + Vector3(-(2 * m_arenaSize), 0, 0));
		} else if (m_ships[i].getPosition().x < - m_arenaSize) {
			m_ships[i].setPosition(m_ships[i].getPosition() + Vector3((2 * m_arenaSize), 0, 0));
		}

		if(m_ships[i].getPosition().y > m_arenaSize) {
			m_ships[i].setPosition(m_ships[i].getPosition() + Vector3(0, -(2 * m_arenaSize), 0));
		} else if (m_ships[i].getPosition().y < - m_arenaSize) {
			m_ships[i].setPosition(m_ships[i].getPosition() + Vector3(0, (2 * m_arenaSize), 0));
		}

		if(m_ships[i].getPosition().z > m_arenaSize) {
			m_ships[i].setPosition(m_ships[i].getPosition() + Vector3(0, 0, -(2 * m_arenaSize)));
		} else if (m_ships[i].getPosition().z < - m_arenaSize) {
			m_ships[i].setPosition(m_ships[i].getPosition() + Vector3(0, 0, (2 * m_arenaSize)));
		}
	}

	for(i = 0; i < m_projectiles.size(); i++) {
		m_projectiles[i].updatePhysics(timeElapsed);
	}
}
