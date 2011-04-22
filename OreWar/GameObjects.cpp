#include "GameObjects.h"

using namespace Ogre;

// ========================================================================
// PlayerShip Implementation
// ========================================================================

PlayerShip::PlayerShip(Real mass, Vector3 position) : 
	SphereCollisionObject(ObjectType::SHIP, 150, mass, position), m_reloadTime(0.2), m_lastShotCounter(0),
	m_canShoot(true), m_shootLeft(true)
{
}

PlayerShip::PlayerShip(Real mass) :
	SphereCollisionObject(ObjectType::SHIP, 150, mass), m_reloadTime(0.2), m_lastShotCounter(0),
	m_canShoot(true), m_shootLeft(true)
{
}

PlayerShip::PlayerShip(const PlayerShip& copy) :
	SphereCollisionObject(ObjectType::SHIP, 150, copy.getMass(), copy.getPosition()), m_reloadTime(copy.m_reloadTime), 
	m_lastShotCounter(copy.m_lastShotCounter), m_canShoot(copy.canShoot()), m_shootLeft(copy.m_shootLeft)
{
}

bool PlayerShip::canShoot() const
{
	return m_canShoot;
}

SphereCollisionObject PlayerShip::generateProjectile()
{
	SphereCollisionObject projectile = SphereCollisionObject(ObjectType::PROJECTILE, 75, 1, getPosition());
	projectile.setVelocity(getVelocity() + getHeading() * 4000);
	projectile.applyForce(getHeading() * 4000);
	projectile.setOrientation(getOrientation());

	if(m_shootLeft) {
		projectile.setPosition(getPosition() + 
			(getOrientation() * Vector3(40, -30, -30)));
	} else {
		projectile.setPosition(getPosition() + 
			(getOrientation() * Vector3(-40, -30, -30)));
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

GameArena::GameArena(Real size) : m_arenaSize(size), m_playerShip(NULL), m_npcShips(), m_projectiles(),
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

Real GameArena::getSize() const {
	return m_arenaSize;
}

PlayerShip * GameArena::setPlayerShip(const PlayerShip& ship) {
	if(m_playerShip != NULL) {
		for(int i = 0; i < m_listeners.size(); i++) {
			m_listeners[i]->destroyedPhysicsObject(m_playerShip);
		}
		delete m_playerShip;
		m_playerShip = NULL;
	}

	// TODO: Deconstructor needs to handle deleting this
	// memory
	m_playerShip = new PlayerShip(ship);
	for(int i = 0; i < m_listeners.size(); i++) {
		m_listeners[i]->newPhysicsObject(m_playerShip);
	}

	return m_playerShip;
}

SphereCollisionObject * GameArena::addNpcShip(const SphereCollisionObject& ship)
{
	SphereCollisionObject * p_ship = new SphereCollisionObject(ship);
	m_npcShips.push_back(p_ship);
	for(int i = 0; i < m_listeners.size(); i++) {
		m_listeners[i]->newPhysicsObject(p_ship);
	}
	return p_ship;
}


SphereCollisionObject * GameArena::addProjectile(const SphereCollisionObject& projectile)
{
	SphereCollisionObject * p_projectile = new SphereCollisionObject(projectile);
	m_projectiles.push_back(p_projectile);
	for(int i = 0; i < m_listeners.size(); i++) {
		m_listeners[i]->newPhysicsObject(p_projectile);
	}
	return p_projectile;
}

bool GameArena::destroyProjectile(SphereCollisionObject * projectile) 
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

bool GameArena::destroyNpcShip(SphereCollisionObject * npcShip) 
{
	for(int i = 0; i < m_npcShips.size(); i++) {
		if(m_npcShips[i] == npcShip) {
			delete npcShip;
			m_npcShips.erase(m_npcShips.begin() + i);
			return true;
		}
	}
	return false;
}

PlayerShip * GameArena::getPlayerShip()
{
	return m_playerShip;
}

SphereCollisionObject * GameArena::fireProjectileFromShip(PlayerShip * ship)
{
	SphereCollisionObject projectile = ship->generateProjectile();
	return addProjectile(projectile);
}

std::vector<SphereCollisionObject *> * GameArena::getProjectiles() {
	return & m_projectiles;
}

std::vector<SphereCollisionObject *> * GameArena::getNpcShips() {
	return & m_npcShips;
}

void GameArena::updatePhysics(Real timeElapsed)
{
	int i, j;
	if(m_playerShip != NULL) {
		m_playerShip->updatePhysics(timeElapsed);
		if(m_playerShip->getPosition().x > m_arenaSize || m_playerShip->getPosition().x < - m_arenaSize
			|| m_playerShip->getPosition().y > m_arenaSize || m_playerShip->getPosition().y < - m_arenaSize
			|| m_playerShip->getPosition().z > m_arenaSize || m_playerShip->getPosition().z < - m_arenaSize) 
		{
			m_playerShip->setVelocity(m_playerShip->getVelocity() * Vector3(-1, -1, -1));
		}
	}

	for(i = 0; i < m_npcShips.size(); i++) {
		m_npcShips[i]->updatePhysics(timeElapsed);

		if(m_npcShips[i]->getPosition().x > m_arenaSize || m_npcShips[i]->getPosition().x < - m_arenaSize
			|| m_npcShips[i]->getPosition().y > m_arenaSize || m_npcShips[i]->getPosition().y < - m_arenaSize
			|| m_npcShips[i]->getPosition().z > m_arenaSize || m_npcShips[i]->getPosition().z < - m_arenaSize) 
		{
			m_npcShips[i]->setVelocity(m_npcShips[i]->getVelocity() * Vector3(-1, -1, -1));
			m_npcShips[i]->setOrientation(Vector3(0, 0, -1).getRotationTo(m_npcShips[i]->getVelocity()));
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
			continue;
		}

		for(j = 0; j < m_npcShips.size(); j++) {
			if(m_projectiles[i]->checkCollision(*m_npcShips[j])) {
				destroyProjectile(m_projectiles[i]);
				destroyNpcShip(m_npcShips[j]);
				i--; // Kludgey, should fix this
				break;
			}
		}
	}
}
