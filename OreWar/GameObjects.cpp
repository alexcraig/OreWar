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
	m_heading(1, 0, 0)
{
}

BaseObject::BaseObject() : m_position(0, 0, 0), m_heading(1, 0, 0)
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
	BaseObject(position, heading), m_mass(mass), m_velocity(0, 0, 0),
	m_acceleration(0, 0, 0), m_force(0, 0, 0)
{
}

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
	PhysicsObject projectile = PhysicsObject(1, ship.getPosition(), ship.getHeading());
	projectile.setVelocity(ship.getVelocity() + ship.getHeading() * 1000);
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
