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

void GameArena::notifyCreation(PhysicsObject * object)
{
	for(std::vector<GameArenaListener * >::iterator listenerIter = m_listeners.begin(); 
		listenerIter != m_listeners.end();
		listenerIter++)
	{
		(*listenerIter)->newPhysicsObject(object);
	}
}

void GameArena::notifyDestruction(PhysicsObject * object)
{
	for(std::vector<GameArenaListener * >::iterator listenerIter = m_listeners.begin(); 
		listenerIter != m_listeners.end();
		listenerIter++)
	{
		(*listenerIter)->destroyedPhysicsObject(object);
	}
}

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
	m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
}

Real GameArena::getSize() const {
	return m_arenaSize;
}

PlayerShip * GameArena::setPlayerShip(const PlayerShip& ship) {
	if(m_playerShip != NULL) {
		notifyDestruction(m_playerShip);
		delete m_playerShip;
		m_playerShip = NULL;
	}

	// TODO: Deconstructor needs to handle deleting this
	// memory
	m_playerShip = new PlayerShip(ship);
	notifyCreation(m_playerShip);

	return m_playerShip;
}

SphereCollisionObject * GameArena::addNpcShip(const SphereCollisionObject& ship)
{
	SphereCollisionObject * p_ship = new SphereCollisionObject(ship);
	m_npcShips.push_back(p_ship);
	notifyCreation(p_ship);
	return p_ship;
}

SphereCollisionObject * GameArena::addProjectile(const SphereCollisionObject& projectile)
{
	SphereCollisionObject * p_projectile = new SphereCollisionObject(projectile);
	m_projectiles.push_back(p_projectile);
	notifyCreation(p_projectile);
	return p_projectile;
}

std::vector<SphereCollisionObject * >::iterator GameArena::destroyProjectile(SphereCollisionObject * projectile) 
{
	for(std::vector<SphereCollisionObject * >::iterator iter =  m_projectiles.begin(); 
		iter != m_projectiles.end();
		iter++)
	{
		if(*iter == projectile) {
			notifyDestruction(projectile);
			delete projectile;
			return m_projectiles.erase(iter);
		}
	}
	
	// TODO: Throw exception, ship not found
	return m_projectiles.end();
}

std::vector<SphereCollisionObject * >::iterator GameArena::destroyNpcShip(SphereCollisionObject * npcShip) 
{
	for(std::vector<SphereCollisionObject * >::iterator iter =  m_npcShips.begin(); 
		iter != m_npcShips.end();
		iter++)
	{
		if(*iter == npcShip) {
			notifyDestruction(npcShip);
			delete npcShip;
			return m_npcShips.erase(iter);
		}
	}

	// TODO: Throw exception, ship not found
	return m_npcShips.end();
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
	// Update the player ship's physics, and reverse its velocity if it passes a wall
	if(m_playerShip != NULL) {
		m_playerShip->updatePhysics(timeElapsed);
		if(m_playerShip->getPosition().x > m_arenaSize || m_playerShip->getPosition().x < - m_arenaSize
			|| m_playerShip->getPosition().y > m_arenaSize || m_playerShip->getPosition().y < - m_arenaSize
			|| m_playerShip->getPosition().z > m_arenaSize || m_playerShip->getPosition().z < - m_arenaSize) 
		{
			m_playerShip->setVelocity(m_playerShip->getVelocity() * Vector3(-1, -1, -1));
		}
	}

	// Update physics for all NPC ships
	for(std::vector<SphereCollisionObject * >::iterator shipIter =  m_npcShips.begin(); 
		shipIter != m_npcShips.end();
		shipIter++) {

		(*shipIter)->updatePhysics(timeElapsed);

		if((*shipIter)->getPosition().x > m_arenaSize || (*shipIter)->getPosition().x < - m_arenaSize
			|| (*shipIter)->getPosition().y > m_arenaSize || (*shipIter)->getPosition().y < - m_arenaSize
			|| (*shipIter)->getPosition().z > m_arenaSize || (*shipIter)->getPosition().z < - m_arenaSize) 
		{
			(*shipIter)->setVelocity((*shipIter)->getVelocity() * Vector3(-1, -1, -1));
			(*shipIter)->setOrientation(Vector3(0, 0, -1).getRotationTo((*shipIter)->getVelocity()));
		}
	}

	// Update physics for projectiles and check for collisions
	for(std::vector<SphereCollisionObject * >::iterator projIter =  m_projectiles.begin(); 
		projIter != m_projectiles.end(); )
	{
		(*projIter)->updatePhysics(timeElapsed);

		if((*projIter)->getPosition().x > m_arenaSize || (*projIter)->getPosition().x < - m_arenaSize
			|| (*projIter)->getPosition().y > m_arenaSize || (*projIter)->getPosition().y < - m_arenaSize
			|| (*projIter)->getPosition().z > m_arenaSize || (*projIter)->getPosition().z < - m_arenaSize) {

			projIter = destroyProjectile((*projIter));
			continue;
		}

		bool projDestroyed = false;
		for(std::vector<SphereCollisionObject * >::iterator shipIter =  m_npcShips.begin(); 
			shipIter != m_npcShips.end();
			shipIter++) {
			if((*projIter)->checkCollision(*(*shipIter))) {
				projIter = destroyProjectile(*projIter);
				destroyNpcShip(*shipIter);
				projDestroyed = true;
				break;
			}
		}

		if(!projDestroyed) {
			projIter++;
		}
	}
}
