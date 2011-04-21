#include "GameObjects.h"

using namespace Ogre;

// ========================================================================
// PlayerShip Implementation
// ========================================================================

PlayerShip::PlayerShip(Real mass, Vector3 position) : 
	PhysicsObject(ObjectType::SHIP, mass, position), m_reloadTime(0.2), m_lastShotCounter(0),
	m_canShoot(true), m_shootLeft(true)
{
}

PlayerShip::PlayerShip(Real mass) :
	PhysicsObject(ObjectType::SHIP, mass), m_reloadTime(0.2), m_lastShotCounter(0),
	m_canShoot(true), m_shootLeft(true)
{
}

PlayerShip::PlayerShip(const PlayerShip& copy) :
	PhysicsObject(ObjectType::SHIP, copy.getMass(), copy.getPosition()), m_reloadTime(copy.m_reloadTime), 
	m_lastShotCounter(copy.m_lastShotCounter), m_canShoot(copy.canShoot()), m_shootLeft(copy.m_shootLeft)
{
}

bool PlayerShip::canShoot() const
{
	return m_canShoot;
}

PhysicsObject PlayerShip::generateProjectile()
{
	PhysicsObject projectile = PhysicsObject(ObjectType::PROJECTILE, 1, getPosition());
	projectile.setVelocity(getVelocity() + getHeading() * 1000);
	projectile.applyForce(getHeading() * 2000);
	projectile.setOrientation(getOrientation());

	if(m_shootLeft) {
		projectile.setPosition(getPosition() + 
			(getOrientation() * Vector3(80, -30, -30)));
	} else {
		projectile.setPosition(getPosition() + 
			(getOrientation() * Vector3(-80, -30, -30)));
	}

	m_shootLeft = !m_shootLeft;
	m_canShoot = false;
	m_lastShotCounter = 0;
	return projectile;
}

void PlayerShip::updatePhysics(Real timeElapsed) 
{
	PhysicsObject::updatePhysics(timeElapsed);
	if(!m_canShoot) {
		m_lastShotCounter += timeElapsed;
		if(m_lastShotCounter >= m_reloadTime) {
			m_canShoot = true;
		}
	}
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

PlayerShip * GameArena::addShip(PlayerShip ship)
{
	PlayerShip * p_ship = new PlayerShip(ship);
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

PhysicsObject * GameArena::fireProjectileFromShip(PlayerShip * ship)
{
	PhysicsObject projectile = ship->generateProjectile();
	return addProjectile(projectile);
}

std::vector<PhysicsObject *> * GameArena::getProjectiles() {
	return & m_projectiles;
}

std::vector<PlayerShip *> * GameArena::getShips() {
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
