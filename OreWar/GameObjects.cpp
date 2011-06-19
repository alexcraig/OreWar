#include "GameObjects.h"
#include "OgreMath.h"

using namespace Ogre;

// ========================================================================
// GameObject Implementation
// ========================================================================
GameObject::GameObject(const SphereCollisionObject& object, ObjectType type, Real maxHealth, Real maxEnergy, 
	Real energyRechargeRate, PagedMemoryPool * memoryMgr)
	: mp_memory(memoryMgr), mp_physModel(NULL), m_maxHealth(maxHealth), m_health(maxHealth), m_maxEnergy(maxEnergy), m_energy(maxEnergy),
	m_energyRechargeRate(energyRechargeRate), m_type(type)
{
	mp_physModel = mp_memory->storeObject(SphereCollisionObject(object));
}

GameObject::GameObject(const GameObject& copy)
	: mp_memory(copy.mp_memory), mp_physModel(NULL), m_maxHealth(copy.m_maxHealth), m_health(copy.m_maxHealth), 
	m_maxEnergy(copy.m_maxEnergy), m_energy(copy.m_maxEnergy), m_energyRechargeRate(copy.m_energyRechargeRate),
	m_type(copy.m_type)
{
	mp_physModel = mp_memory->storeObject(SphereCollisionObject(*copy.phys()));
}


GameObject::~GameObject()
{
	mp_memory->destroyObject(mp_physModel);
}

SphereCollisionObject * GameObject::phys() const
{
	return mp_physModel;
}

ObjectType GameObject::type() const
{
	return m_type;
}

PagedMemoryPool * GameObject::memoryManager() const
{
	return mp_memory;
}

Real GameObject::health() const
{
	return m_health;
}

Real GameObject::maxHealth() const
{
	return m_maxHealth;
}

Real GameObject:: energy() const
{
	return m_energy;
}

Real GameObject::maxEnergy() const
{
	return m_maxEnergy;
}

Real GameObject::energyRecharge() const
{
	return m_energyRechargeRate;
}

void GameObject::health(Real health)
{
	m_health = health > m_maxHealth ? m_maxHealth : health;
}

void GameObject::energy(Real energy)
{
	m_energy = energy > m_maxEnergy ? m_maxEnergy : energy;
}

void GameObject::inflictDamage(Real damage)
{
	if(damage < m_energy) {
		m_energy = m_energy - damage;
	} else {
		m_health = m_health - (damage - m_energy);
		m_energy = 0;
	}
}

void GameObject::addEnergy(Real energy)
{
	m_energy = energy + m_energy;
	if(m_energy > m_maxEnergy) {
		m_energy = m_maxEnergy;
	}
}

void GameObject::drainEnergy(Real energy)
{
	m_energy = m_energy - energy;
	if(m_energy < 0) {
		m_energy = 0;
	}
}


// ========================================================================
// Projectile Implementation
// ========================================================================

Projectile::Projectile(const SphereCollisionObject& physModel, ObjectType type, Real damage, PagedMemoryPool * memoryMgr)
	: GameObject(physModel, type, 1, 0, 0, memoryMgr), m_damage(damage)
{
}

void Projectile::updatePhysics(Real timeElapsed)
{
	phys()->updatePhysics(timeElapsed);
}

Real Projectile::damage()
{
	return m_damage;
}


// ========================================================================
// Weapon Implementation
// ========================================================================
Weapon::Weapon(Real reloadTime, Real energyCost, PagedMemoryPool * memoryMgr) 
	: mp_memory(memoryMgr), m_reloadTime(reloadTime), m_lastShotCounter(reloadTime),
	m_canShoot(true), m_energyCost(energyCost)
{
}

Weapon::Weapon(const Weapon& copy) : mp_memory(copy.mp_memory), m_reloadTime(copy.m_reloadTime), m_lastShotCounter(copy.m_lastShotCounter),
	m_canShoot(copy.m_canShoot), m_energyCost(copy.m_energyCost)
{
}

Weapon::~Weapon()
{
}

PagedMemoryPool * Weapon::memoryManager() const
{
	return mp_memory;
}

bool Weapon::canShoot() const
{
	return m_canShoot;
}


void Weapon::resetShotCounter()
{
	m_lastShotCounter = 0;
	m_canShoot = false;
}

Real Weapon::energyCost()
{
	return m_energyCost;
}

void Weapon::updatePhysics(Real timeElapsed) {
	if(!m_canShoot) {
		m_lastShotCounter += timeElapsed;
		if(m_lastShotCounter >= m_reloadTime) {
			m_canShoot = true;
		}
	}
}


// ========================================================================
// PlasmaCannon Implementation
// ========================================================================
PlasmaCannon::PlasmaCannon(PagedMemoryPool * memoryMgr) : Weapon(0.2, 10, memoryMgr), m_shootLeft(true)
{
}

PlasmaCannon::PlasmaCannon(const PlasmaCannon& copy) : Weapon(copy), m_shootLeft(copy.m_shootLeft)
{
}

Projectile PlasmaCannon::fireWeapon(PhysicsObject& origin)
{
	SphereCollisionObject projectilePhysics = SphereCollisionObject(75, 1, origin.position());
	projectilePhysics.velocity(origin.velocity() + origin.heading() * 4000);
	projectilePhysics.applyForce(origin.heading() * 4000);
	projectilePhysics.orientation(origin.orientation());

	if(m_shootLeft) {
		projectilePhysics.position(origin.position() + 
			(origin.orientation() * Vector3(40, -30, -30)));
	} else {
		projectilePhysics.position(origin.position() + 
			(origin.orientation() * Vector3(-40, -30, -30)));
	}

	m_shootLeft = !m_shootLeft;
	resetShotCounter();
	return Projectile(projectilePhysics, ObjectType::PROJECTILE, 35, memoryManager());
}


// ========================================================================
// AnchorLauncher Implementation
// ========================================================================
AnchorLauncher::AnchorLauncher(PagedMemoryPool * memoryMgr) : Weapon(3, 40, memoryMgr)
{
}

AnchorLauncher::AnchorLauncher(const AnchorLauncher& copy) : Weapon(copy)
{
}

Projectile AnchorLauncher::fireWeapon(PhysicsObject& origin)
{
	SphereCollisionObject projectilePhysics = SphereCollisionObject(75, 1, origin.position());
	projectilePhysics.velocity(origin.velocity() + origin.heading() * 4000);
	projectilePhysics.orientation(origin.orientation());

	resetShotCounter();
	return Projectile(projectilePhysics, ObjectType::ANCHOR_PROJECTILE, 0, memoryManager());
}


// ========================================================================
// CelestialBody Implementation
// ========================================================================
CelestialBody::CelestialBody(ObjectType type, Real mass, Real radius, Vector3 position, PagedMemoryPool * memoryMgr)
	: GameObject(SphereCollisionObject(radius, mass, position), type, 10000, 10000,
		1000, memoryMgr), mp_center(NULL), m_radius(radius)
{
}

CelestialBody::CelestialBody(ObjectType type, Real mass, Real radius, CelestialBody * center, 
	Real distance, Real speed, PagedMemoryPool * memoryMgr)
	: GameObject(SphereCollisionObject(radius, mass, Vector3(0,0,0)), type, 10000, 10000,
		1000, memoryMgr), mp_center(center), m_radius(radius)
{
	Real totalDistance = distance + radius + center->radius();
	
	// Generate a random position on a sphere around the center with a radius
	// equal to the specified distance
	Real randAngle = Math::UnitRandom() * (2 * Math::PI);
	Real randMu = Math::RangeRandom(-0.2, 0.2);
	Vector3 pointOnUnitSphere = Vector3(
		Math::Cos(randAngle) * Math::Sqrt(1 - Math::Sqr(randMu)),
		randMu,
		Math::Sin(randAngle) * Math::Sqrt(1 - Math::Sqr(randMu)));
	Vector3 relPosition = pointOnUnitSphere * totalDistance;

	phys()->position(relPosition + center->phys()->position());

	// Generate a velocity for the orbit by taking the cross product of the Y unit vector
	// and the normal vector to the center of the orbit (results in a vector tangent to
	// the sphere)
	Vector3 unitNormal = (center->phys()->position() - phys()->position()).normalisedCopy();
	// Randomize the direction of the orbit
	int reverse = rand() % 2;
	if(reverse) {
		unitNormal = unitNormal * -1;
	}
	Vector3 velocity = (unitNormal.crossProduct(Vector3::UNIT_Y).normalisedCopy() * speed) + center->phys()->velocity();
	phys()->velocity(velocity);


	// Generate a random velocity by generating a random vector lieing in a plane
	// tangent to the sphere around the orbital center
	/*
	Vector3 randomUnitTangent = unitNormal.randomDeviant(Radian(Degree(90))).normalisedCopy();
	Vector3 velocity = (randomUnitTangent * speed) + center->phys()->velocity();
	phys()->velocity(velocity);
	*/

}

CelestialBody::CelestialBody(const CelestialBody & copy)
	: GameObject(copy), mp_center(copy.mp_center), m_radius(copy.m_radius)
{
}

Constraint CelestialBody::constraint() const
{
	// Generate the constraint which maintains the orbit
	return Constraint(phys(), mp_center->phys(), true);
}

bool CelestialBody::hasCenter() const
{
	return mp_center != NULL;
}

void CelestialBody::center(CelestialBody * newCenter)
{
	mp_center = newCenter;
}

CelestialBody * CelestialBody::center() const
{
	return mp_center;
}

Real CelestialBody::radius() const
{
	return phys()->radius();
}

void CelestialBody::updatePhysics(Real timeElapsed)
{
	phys()->updatePhysics(timeElapsed);
	health(maxHealth());
	energy(maxEnergy());
}


// ========================================================================
// SpaceShip Implementation
// ========================================================================
SpaceShip::SpaceShip(ObjectType type, Real mass, Vector3 position, Real energyRecharge, PagedMemoryPool * memoryMgr) : 
	GameObject(SphereCollisionObject(150, mass, position), type, 100, 100, energyRecharge, memoryMgr), mp_weapons()
{
}

SpaceShip::SpaceShip(ObjectType type, Real mass, Vector3 position, PagedMemoryPool * memoryMgr) : 
	GameObject(SphereCollisionObject(150, mass, position), type, 100, 100, 5, memoryMgr), mp_weapons()
{
}

SpaceShip::SpaceShip(ObjectType type, Real mass, PagedMemoryPool * memoryMgr) :
	GameObject(SphereCollisionObject(150, mass), type, 100, 100, 5, memoryMgr), mp_weapons()
{
}

SpaceShip::SpaceShip(const SpaceShip& copy) :
	GameObject(copy), mp_weapons(copy.mp_weapons)
{
}

PlasmaCannon * SpaceShip::addPlasmaCannon(const PlasmaCannon& weapon)
{
	PlasmaCannon * newCannon = memoryManager()->storeObject(PlasmaCannon(weapon));
	mp_weapons.push_back(newCannon);
	return newCannon;
}

AnchorLauncher * SpaceShip::addAnchorLauncher(const AnchorLauncher& weapon)
{
	AnchorLauncher * newLauncher = memoryManager()->storeObject(AnchorLauncher(weapon));
	mp_weapons.push_back(newLauncher);
	return newLauncher;
}

Projectile * SpaceShip::fireWeapon(GameArena& arena, int weaponIndex)
{
	if(weaponIndex < 0 || weaponIndex >= mp_weapons.size()) {
		return NULL;
	}

	if(mp_weapons[weaponIndex]->canShoot() && energy() > mp_weapons[weaponIndex]->energyCost()) {
		drainEnergy(mp_weapons[weaponIndex]->energyCost());
		return arena.addProjectile(mp_weapons[weaponIndex]->fireWeapon(*phys()));
	}
}

void SpaceShip::updatePhysics(Real timeElapsed) 
{
	phys()->updatePhysics(timeElapsed);
	for(std::vector<Weapon * >::iterator weaponIter = mp_weapons.begin(); 
		weaponIter != mp_weapons.end();
		weaponIter++)
	{
		(*weaponIter)->updatePhysics(timeElapsed);
	}
}

// ========================================================================
// GameArena Implementation
// ========================================================================
GameArena::GameArena(Real size, int pageSize, int initPages) : m_arenaSize(size), mp_playerShip(NULL), mp_npcShips(), mp_projectiles(),
	mp_bodies(), mp_constraints(), mp_listeners(), m_memory(pageSize, initPages)
{
}

GameArena::~GameArena() 
{
	if(mp_playerShip != NULL) {
		m_memory.destroyObject(mp_playerShip);
	}

	for(std::vector<SpaceShip * >::iterator delIter =  mp_npcShips.begin(); 
		delIter != mp_npcShips.end();
		delIter++) 
	{
		m_memory.destroyObject(*delIter);
	}

	for(std::vector<CelestialBody * >::iterator delIter =  mp_bodies.begin(); 
		delIter != mp_bodies.end();
		delIter++) 
	{
		m_memory.destroyObject(*delIter);
	}

	for(std::vector<Projectile * >::iterator delIter =  mp_projectiles.begin(); 
		delIter != mp_projectiles.end();
		delIter++) 
	{
		m_memory.destroyObject(*delIter);
	}

	for(std::vector<Constraint * >::iterator delIter =  mp_constraints.begin(); 
		delIter != mp_constraints.end();
		delIter++) 
	{
		m_memory.destroyObject(*delIter);
	}
	
}

void GameArena::notifyObjectCreation(GameObject * object)
{
	for(std::vector<GameArenaListener * >::iterator listenerIter = mp_listeners.begin(); 
		listenerIter != mp_listeners.end();
		listenerIter++)
	{
		(*listenerIter)->newGameObject(object);
	}
}

void GameArena::notifyObjectDestruction(GameObject * object)
{
	for(std::vector<GameArenaListener * >::iterator listenerIter = mp_listeners.begin(); 
		listenerIter != mp_listeners.end();
		listenerIter++)
	{
		(*listenerIter)->destroyedGameObject(object);
	}
}

void GameArena::notifyConstraintCreation(Constraint * constraint)
{
	for(std::vector<GameArenaListener * >::iterator listenerIter = mp_listeners.begin(); 
		listenerIter != mp_listeners.end();
		listenerIter++)
	{
		(*listenerIter)->newConstraint(constraint);
	}
}

void GameArena::notifyConstraintDestruction(Constraint * constraint)
{
	for(std::vector<GameArenaListener * >::iterator listenerIter = mp_listeners.begin(); 
		listenerIter != mp_listeners.end();
		listenerIter++)
	{
		(*listenerIter)->destroyedConstraint(constraint);
	}
}

void GameArena::addGameArenaListener(GameArenaListener * listener) 
{
	mp_listeners.push_back(listener);
}

void GameArena::removeGameArenaListener(GameArenaListener * listener)
{
	mp_listeners.erase(std::remove(mp_listeners.begin(), mp_listeners.end(), listener), mp_listeners.end());
}

Real GameArena::size() const {
	return m_arenaSize;
}

SpaceShip * GameArena::setPlayerShip(const SpaceShip& ship) {
	if(mp_playerShip != NULL) {
		notifyObjectDestruction(mp_playerShip);
		m_memory.destroyObject(&mp_playerShip);
		mp_playerShip = NULL;
	}

	// TODO: Deconstructor needs to handle deleting this
	// memory
	mp_playerShip = memoryManager()->storeObject(SpaceShip(ship));
	notifyObjectCreation(mp_playerShip);

	return mp_playerShip;
}

SpaceShip * GameArena::addNpcShip(const SpaceShip& ship)
{
	SpaceShip * p_ship = m_memory.storeObject(ship);
	mp_npcShips.push_back(p_ship);
	notifyObjectCreation(p_ship);
	return p_ship;
}

Projectile * GameArena::addProjectile(const Projectile& projectile)
{
	Projectile * p_projectile = m_memory.storeObject(projectile);
	mp_projectiles.push_back(p_projectile);
	notifyObjectCreation(p_projectile);
	return p_projectile;
}

Constraint * GameArena::addConstraint(const Constraint& constraint)
{
	Constraint * p_constraint = m_memory.storeObject(constraint);
	mp_constraints.push_back(p_constraint);
	notifyConstraintCreation(p_constraint);
	return p_constraint;
}

CelestialBody * GameArena::addBody(const CelestialBody& body)
{
	CelestialBody * p_body = m_memory.storeObject(body);
	if(p_body->hasCenter()) {
		addConstraint(p_body->constraint());
	}
	mp_bodies.push_back(p_body);
	notifyObjectCreation(p_body);
	return p_body;
}

std::vector<CelestialBody * >::iterator GameArena::destroyBody(CelestialBody * body)
{
	std::vector<CelestialBody * >::iterator returnIter;
	CelestialBody * bodyCenter = body->center();
	bool foundBody = false;

	for(std::vector<CelestialBody * >::iterator iter =  mp_bodies.begin(); 
		iter != mp_bodies.end();)
	{
		if(bodyCenter != NULL && (*iter)->hasCenter() && (*iter)->center() == body) {
			(*iter)->center(bodyCenter);
		}

		if(*iter == body) {
			if(body->hasCenter()) {
				// Ensure any constraints attached to this body are also destroyed
				for(std::vector<Constraint * >::iterator conIter =  mp_constraints.begin(); 
					conIter != mp_constraints.end();)
				{
					SphereCollisionObject * bodyPhys = body->phys();
					if((*conIter)->getTarget() == bodyPhys || (*conIter)->getOrigin() == bodyPhys)
					{
						conIter = destroyConstraint(*conIter);
					} else {
						conIter++;
					}
				}
			}

			notifyObjectDestruction(body);
			m_memory.destroyObject(body);
			returnIter = mp_bodies.erase(iter);
			iter = returnIter;
			foundBody = true;
		} else {
			iter++;
		}
	}

	if(foundBody) {
		return returnIter;
	} else {
		// TODO: Throw exception, constraint not found
		return mp_bodies.end();
	}
}

std::vector<Constraint * >::iterator GameArena::destroyConstraint(Constraint * constraint) 
{
	for(std::vector<Constraint * >::iterator iter =  mp_constraints.begin(); 
		iter != mp_constraints.end();
		iter++)
	{
		if(*iter == constraint) {
			notifyConstraintDestruction(constraint);
			m_memory.destroyObject(constraint);
			return mp_constraints.erase(iter);
		}
	}
	
	// TODO: Throw exception, constraint not found
	return mp_constraints.end();
}

std::vector<Projectile * >::iterator GameArena::destroyProjectile(Projectile * projectile) 
{
	for(std::vector<Projectile * >::iterator iter =  mp_projectiles.begin(); 
		iter != mp_projectiles.end();
		iter++)
	{
		if(*iter == projectile) {
			notifyObjectDestruction(projectile);
			m_memory.destroyObject(projectile);
			return mp_projectiles.erase(iter);
		}
	}
	
	// TODO: Throw exception, projectile not found
	return mp_projectiles.end();
}

std::vector<SpaceShip * >::iterator GameArena::destroyNpcShip(SpaceShip * npcShip) 
{
	for(std::vector<SpaceShip * >::iterator iter =  mp_npcShips.begin(); 
		iter != mp_npcShips.end();
		iter++)
	{
		if(*iter == npcShip) 
		{
			// Ensure any constraints attached to this ship are also destroyed
			for(std::vector<Constraint * >::iterator conIter =  mp_constraints.begin(); 
				conIter != mp_constraints.end();)
			{
				SphereCollisionObject * shipPhys = npcShip->phys();
				if((*conIter)->getTarget() == shipPhys || (*conIter)->getOrigin() == shipPhys)
				{
					conIter = destroyConstraint(*conIter);
				} else {
					conIter++;
				}
			}
			notifyObjectDestruction(npcShip);
			m_memory.destroyObject(npcShip);
			return mp_npcShips.erase(iter);
		}
	}

	// TODO: Throw exception, ship not found
	return mp_npcShips.end();
}

SpaceShip * GameArena::playerShip()
{
	return mp_playerShip;
}

Projectile * GameArena::fireProjectileFromShip(SpaceShip * ship, int weaponIndex)
{
	return ship->fireWeapon(*this, weaponIndex);
}

std::vector<Projectile *> * GameArena::projectiles() {
	return & mp_projectiles;
}

std::vector<SpaceShip *> * GameArena::npcShips() {
	return & mp_npcShips;
}

std::vector<CelestialBody *> * GameArena::bodies() {
	return & mp_bodies;
}

void GameArena::updatePhysics(Real timeElapsed)
{
	// Apply forces from constraints
	for(std::vector<Constraint * >::iterator conIter =  mp_constraints.begin(); 
		conIter != mp_constraints.end();
		conIter++)
	{
		(*conIter)->applyForces(timeElapsed);
	}

	// Update physics for orbiting bodies
	for(std::vector<CelestialBody * >::iterator bodyIter =  mp_bodies.begin(); 
		bodyIter != mp_bodies.end();
		bodyIter++) 
	{
		(*bodyIter)->updatePhysics(timeElapsed);
	}

	// Update the player ship's physics, and reverse its velocity if it passes a wall
	if(mp_playerShip != NULL) 
	{
		mp_playerShip->updatePhysics(timeElapsed);
		mp_playerShip->addEnergy(mp_playerShip->energyRecharge() * timeElapsed);
		SphereCollisionObject * playerShipPhys = mp_playerShip->phys();

		if(playerShipPhys->position().x > m_arenaSize || playerShipPhys->position().x < - m_arenaSize
			|| playerShipPhys->position().y > m_arenaSize || playerShipPhys->position().y < - m_arenaSize
			|| playerShipPhys->position().z > m_arenaSize || playerShipPhys->position().z < - m_arenaSize) 
		{
			// playerShipPhys->velocity(playerShipPhys->velocity() * Vector3(-1, -1, -1));
			
			// Slowly drain health if outside game boundaries
			// mp_playerShip->health(mp_playerShip->health() - (timeElapsed * 5));
		}
	}

	// Update physics for all NPC ships
	for(std::vector<SpaceShip * >::iterator shipIter =  mp_npcShips.begin(); 
		shipIter != mp_npcShips.end();
		shipIter++) 
	{
		(*shipIter)->updatePhysics(timeElapsed);
		(*shipIter)->addEnergy((*shipIter)->energyRecharge() * timeElapsed);
		SphereCollisionObject * shipPhys = (*shipIter)->phys();

		if(shipPhys->position().x > m_arenaSize || shipPhys->position().x < - m_arenaSize
			|| shipPhys->position().y > m_arenaSize || shipPhys->position().y < - m_arenaSize
			|| shipPhys->position().z > m_arenaSize || shipPhys->position().z < - m_arenaSize) 
		{
			shipPhys->velocity(shipPhys->velocity() * Vector3(-1, -1, -1));
		}
		shipPhys->orientation(Vector3(0, 0, -1).getRotationTo(shipPhys->velocity()));
	}

	// Update physics for projectiles and check for collisions
	for(std::vector<Projectile * >::iterator projIter =  mp_projectiles.begin(); 
		projIter != mp_projectiles.end(); )
	{
		(*projIter)->updatePhysics(timeElapsed);
		SphereCollisionObject * projPhys = (*projIter)->phys();

		if(projPhys->position().x > m_arenaSize || projPhys->position().x < - m_arenaSize
			|| projPhys->position().y > m_arenaSize || projPhys->position().y < - m_arenaSize
			|| projPhys->position().z > m_arenaSize || projPhys->position().z < - m_arenaSize) 
		{
			projIter = destroyProjectile((*projIter));
			continue;
		}

		bool projDestroyed = false;
		for(std::vector<SpaceShip * >::iterator shipIter =  mp_npcShips.begin(); 
			shipIter != mp_npcShips.end();
			shipIter++) 
		{
			if(projPhys->checkCollision(*(*shipIter)->phys())) 
			{
				(*shipIter)->inflictDamage((*projIter)->damage());
				projIter = destroyProjectile(*projIter);
				projDestroyed = true;
				break;
			}
		}

		if(!projDestroyed) 
		{
			projIter++;
		}
	}

	// Deal fatal damage to any entity that collides with a celestial body
	for(std::vector<CelestialBody * >::iterator bodyIter =  mp_bodies.begin(); 
		bodyIter != mp_bodies.end(); ) 
	{
		if((*bodyIter)->phys()->checkCollision(*(mp_playerShip->phys()))) {
			mp_playerShip->inflictDamage(500);
		}

		for(std::vector<Projectile * >::iterator projIter =  mp_projectiles.begin(); 
			projIter != mp_projectiles.end(); )
		{
			if((*bodyIter)->phys()->checkCollision(*(*projIter)->phys())) {
				projIter = destroyProjectile(*projIter);
			} else {
				projIter++;
			}
		}
		
		for(std::vector<SpaceShip * >::iterator shipIter =  mp_npcShips.begin(); 
		shipIter != mp_npcShips.end();) 
		{
			if((*bodyIter)->phys()->checkCollision(*(*shipIter)->phys())) {
				shipIter = destroyNpcShip(*shipIter);
			} else {
				shipIter++;
			}
		}

		bool gotCollision = false;

		for(std::vector<CelestialBody * >::iterator colBodyIter =  mp_bodies.begin(); 
			colBodyIter != mp_bodies.end(); )
		{
			if((*bodyIter) != (*colBodyIter)
				&& (*bodyIter)->phys()->checkCollision(*(*colBodyIter)->phys())) {
				// Two celestial bodies collided, destroy the smaller and generate 25
				// random projectiles inside the remains of each
				gotCollision = true;
				Vector3 center, centerVelocity;
				Real radius;

				if((*bodyIter)->radius() > (*colBodyIter)->radius()) {
					radius = (*colBodyIter)->radius();
					center = (*colBodyIter)->phys()->position();
					centerVelocity = (*colBodyIter)->phys()->velocity();
					bodyIter = destroyBody((*colBodyIter));

					// Restart collision checking when a hit is found? Have to find
					// a better way to do this whole thing
					bodyIter = mp_bodies.begin();

				} else {
					radius = (*bodyIter)->radius();
					center = (*bodyIter)->phys()->position();
					centerVelocity = (*bodyIter)->phys()->velocity();

					bodyIter = destroyBody((*bodyIter));
				}

				for(int i = 0; i < 20; i++) {
					Real randAngle = Math::UnitRandom() * (2 * Math::PI);
					Real randMu = Math::RangeRandom(-1, 1);
					Vector3 pointOnUnitSphere = Vector3(
						Math::Cos(randAngle) * Math::Sqrt(1 - Math::Sqr(randMu)),
						randMu,
						Math::Sin(randAngle) * Math::Sqrt(1 - Math::Sqr(randMu)));
					Vector3 relOffset = pointOnUnitSphere * radius * Math::RangeRandom(0, 1);
					
					SphereCollisionObject projectilePhysics = SphereCollisionObject(500, 1, relOffset + center);
					projectilePhysics.velocity(relOffset.normalisedCopy() * 4000 + centerVelocity);
					addProjectile(Projectile(projectilePhysics, ObjectType::PLANET_CHUNK, 50, &m_memory));
				}

				break;
			} else {
				colBodyIter++;
			}
		}

		if(!gotCollision) {
			bodyIter++;
		}
	}

	// After all projectile collisions, remove any ships with less than 0 health
	for(std::vector<SpaceShip * >::iterator shipIter =  mp_npcShips.begin(); 
		shipIter != mp_npcShips.end();) 
	{
			if ((*shipIter)->health() <= 0)
			{
				shipIter = destroyNpcShip((*shipIter));
			}
			else
			{
				shipIter++;
			}
	}

	// Reset the player ship if they "die"
	if(mp_playerShip->health() <= 0) {
		mp_playerShip->health(mp_playerShip->maxHealth());
		mp_playerShip->phys()->velocity(Vector3(0, 0, 0));
		mp_playerShip->phys()->position(Vector3(10000, 10000, 10000));
	}
}


void GameArena::generateSolarSystem() 
{
	// Generate a star in the middle of the arena
	CelestialBody * star = addBody(CelestialBody(ObjectType::STAR, 100000, 10000, Vector3(0, 0, 0), &m_memory));
	Real totalDistance = 5000;

	// TODO: This should be refactored to remove copy/pasting code

	// Inner planets
	int numInnerPlanets = rand() % 5 + 3;
	// Add a random number of planents
	for(int i = 0; i < numInnerPlanets; i++) {
		totalDistance += Math::RangeRandom(4000, 7000);
		Real planetRadius = Math::RangeRandom(500, 2000);
		Real speed = Math::RangeRandom(2000, 8000);

		CelestialBody * planet = addBody(CelestialBody(ObjectType::PLANET, 10000, planetRadius,
			star, totalDistance, speed, &m_memory));

		int numMoons = rand() % 3;
		Real moonDistance = planetRadius * 0.3;
		for(int j = 0; j < 2; j++) {
			Real moonRadius = Math::RangeRandom(planetRadius * 0.1, planetRadius * 0.7);
			moonDistance += Math::RangeRandom(planetRadius * 0.5, planetRadius * 1);
			speed = Math::RangeRandom(1, 3) * moonDistance;
			CelestialBody * moon = addBody(CelestialBody(ObjectType::MOON, 1000, moonRadius,
				planet, moonDistance, speed, &m_memory));
		}
	}

	// Outer planets - giants
	int numOuterPlanets = rand() % 4 + 2;
	for(int i = 0; i < numOuterPlanets; i++) {
		totalDistance += Math::RangeRandom(8000, 14000);
		Real planetRadius = Math::RangeRandom(2000, 8000);
		Real speed = Math::RangeRandom(8000, 15000);

		CelestialBody * planet = addBody(CelestialBody(ObjectType::PLANET, 10000, planetRadius,
			star, totalDistance, speed, &m_memory));

		int numMoons = rand() % 5 + 2;
		Real moonDistance = planetRadius * 0.3;
		for(int j = 0; j < 2; j++) {
			Real moonRadius = Math::RangeRandom(planetRadius * 0.1, planetRadius * 0.3);
			moonDistance += Math::RangeRandom(planetRadius * 0.2, planetRadius * 0.4);
			speed = Math::RangeRandom(2, 4) * moonDistance;
			CelestialBody * moon = addBody(CelestialBody(ObjectType::MOON, 1000, moonRadius,
				planet, moonDistance, speed, &m_memory));
		}
	}

	// Outer planets - tiny
	int numTinyPlanets = rand() % 3;
	totalDistance += Math::RangeRandom(8000, 12000);
	for(int i = 0; i < numTinyPlanets; i++) {
		totalDistance += Math::RangeRandom(8000, 14000);
		Real planetRadius = Math::RangeRandom(500, 1500);
		Real speed = Math::RangeRandom(20000, 25000);

		CelestialBody * planet = addBody(CelestialBody(ObjectType::PLANET, 10000, planetRadius,
			star, totalDistance, speed, &m_memory));

		int numMoons = rand() % 2;
		Real moonDistance = planetRadius * 0.3;
		for(int j = 0; j < 2; j++) {
			Real moonRadius = Math::RangeRandom(planetRadius * 0.8, planetRadius * 1.2);
			moonDistance += Math::RangeRandom(planetRadius * 0.2, planetRadius * 0.4);
			speed = Math::RangeRandom(2, 4) * moonDistance;
			CelestialBody * moon = addBody(CelestialBody(ObjectType::MOON, 1000, moonRadius,
				planet, moonDistance, speed, &m_memory));
		}
	}
}

void GameArena::clearSolarSystem() {
	for(std::vector<CelestialBody * >::iterator iter =  mp_bodies.begin(); 
		iter != mp_bodies.end();)
	{
		iter = destroyBody(*iter);
	}
}

PagedMemoryPool * GameArena::memoryManager()
{
	return &m_memory;
}
