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

ObjectType PhysicsObject::getType()
{
	return m_type;
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

GameArena::GameArena(Real size) : m_arenaSize(size), m_ships(), m_projectiles(),
	m_listeners()
{
}

void GameArena::addGameArenaListener(GameArenaListener * listener) 
{
	m_listeners.push_back(listener);
}

void GameArena::removeGameArenaListener(GameArenaListener * listener)
{
	for(int i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == listener) {
			m_listeners.erase(m_listeners.begin() + i);
		}
	}
}

PhysicsObject * GameArena::addShip(PhysicsObject ship)
{
	PhysicsObject * p_ship = new PhysicsObject(ship);
	m_ships.push_back(p_ship);
	for(int i = 0; i < m_listeners.size(); i++) {
		m_listeners[i]->newPhysicsObject(p_ship);
	}
	return p_ship;
}

PhysicsObject * GameArena::addProjectile(PhysicsObject projectile)
{
	PhysicsObject * p_projectile = new PhysicsObject(projectile);
	m_projectiles.push_back(p_projectile);
	for(int i = 0; i < m_listeners.size(); i++) {
		m_listeners[i]->newPhysicsObject(p_projectile);
	}
	return p_projectile;
}

bool GameArena::destroyProjectile(PhysicsObject * projectile) 
{
	for(int i = 0; i < m_projectiles.size(); i++) {
		if(m_projectiles[i] == projectile) {
			delete projectile;
			m_projectiles.erase(m_projectiles.begin() + i);
			return true;
		}
	}
	return false;
}

PhysicsObject * GameArena::fireProjectileFromShip(PhysicsObject ship)
{
	PhysicsObject projectile = PhysicsObject(ObjectType::PROJECTILE, 1, ship.getPosition());
	projectile.setVelocity(ship.getVelocity() + ship.getHeading() * 1000);
	projectile.setOrientation(ship.getOrientation());
	return addProjectile(projectile);
}

std::vector<PhysicsObject *> * GameArena::getProjectiles() {
	return & m_projectiles;
}

std::vector<PhysicsObject *> * GameArena::getShips() {
	return & m_ships;
}

void GameArena::updatePhysics(Real timeElapsed)
{
	int i;
	for(i = 0; i < m_ships.size(); i++) {
		m_ships[i]->updatePhysics(timeElapsed);
		if(m_ships[i]->getPosition().x > m_arenaSize) {
			m_ships[i]->setPosition(m_ships[i]->getPosition() + Vector3(-(2 * m_arenaSize), 0, 0));
		} else if (m_ships[i]->getPosition().x < - m_arenaSize) {
			m_ships[i]->setPosition(m_ships[i]->getPosition() + Vector3((2 * m_arenaSize), 0, 0));
		}

		if(m_ships[i]->getPosition().y > m_arenaSize) {
			m_ships[i]->setPosition(m_ships[i]->getPosition() + Vector3(0, -(2 * m_arenaSize), 0));
		} else if (m_ships[i]->getPosition().y < - m_arenaSize) {
			m_ships[i]->setPosition(m_ships[i]->getPosition() + Vector3(0, (2 * m_arenaSize), 0));
		}

		if(m_ships[i]->getPosition().z > m_arenaSize) {
			m_ships[i]->setPosition(m_ships[i]->getPosition() + Vector3(0, 0, -(2 * m_arenaSize)));
		} else if (m_ships[i]->getPosition().z < - m_arenaSize) {
			m_ships[i]->setPosition(m_ships[i]->getPosition() + Vector3(0, 0, (2 * m_arenaSize)));
		}
	}

	for(i = 0; i < m_projectiles.size(); i++) {
		m_projectiles[i]->updatePhysics(timeElapsed);

		if(m_projectiles[i]->getPosition().x > m_arenaSize || m_projectiles[i]->getPosition().x < - m_arenaSize
			|| m_projectiles[i]->getPosition().y > m_arenaSize || m_projectiles[i]->getPosition().y < - m_arenaSize
			|| m_projectiles[i]->getPosition().z > m_arenaSize || m_projectiles[i]->getPosition().z < - m_arenaSize) 
		{
			destroyProjectile(m_projectiles[i]);
			i--; // Kludgey, should fix this
		}
	}
}
