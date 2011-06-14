#include "GameObjects.h"
#include "OgreMath.h"

using namespace Ogre;

// ========================================================================
// GameObject Implementation
// ========================================================================
GameObject::GameObject(const SphereCollisionObject& object, ObjectType type, Real maxHealth, Real maxEnergy, Real energyRechargeRate)
	: mp_physModel(NULL), m_maxHealth(maxHealth), m_health(maxHealth), m_maxEnergy(maxEnergy), m_energy(maxEnergy),
	m_energyRechargeRate(energyRechargeRate), m_type(type)
{
	mp_physModel = new SphereCollisionObject(object);
}

GameObject::GameObject(const GameObject& copy)
	: mp_physModel(NULL), m_maxHealth(copy.m_maxHealth), m_health(copy.m_maxHealth), 
	m_maxEnergy(copy.m_maxEnergy), m_energy(copy.m_maxEnergy), m_energyRechargeRate(copy.m_energyRechargeRate),
	m_type(copy.m_type)
{
	mp_physModel = new SphereCollisionObject(*copy.phys());
}


GameObject::~GameObject()
{
	delete mp_physModel;
}

SphereCollisionObject * GameObject::phys() const
{
	return mp_physModel;
}

ObjectType GameObject::type() const
{
	return m_type;
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

Projectile::Projectile(const SphereCollisionObject& physModel, ObjectType type, Real damage)
	: GameObject(physModel, type, 1, 0, 0), m_damage(damage)
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
Weapon::Weapon(Real reloadTime, Real energyCost) : m_reloadTime(reloadTime), m_lastShotCounter(reloadTime),
	m_canShoot(true), m_energyCost(energyCost)
{
}

Weapon::Weapon(const Weapon& copy) : m_reloadTime(copy.m_reloadTime), m_lastShotCounter(copy.m_lastShotCounter),
	m_canShoot(copy.m_canShoot), m_energyCost(copy.m_energyCost)
{
}

Weapon::~Weapon()
{
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
PlasmaCannon::PlasmaCannon() : Weapon(0.2, 10), m_shootLeft(true)
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
	return Projectile(projectilePhysics, ObjectType::PROJECTILE, 35);
}


// ========================================================================
// AnchorLauncher Implementation
// ========================================================================
AnchorLauncher::AnchorLauncher() : Weapon(3, 40)
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
	return Projectile(projectilePhysics, ObjectType::ANCHOR_PROJECTILE, 0);
}


// ========================================================================
// CelestialBody Implementation
// ========================================================================
CelestialBody::CelestialBody(ObjectType type, Real mass, Real radius, Vector3 position)
	: GameObject(SphereCollisionObject(radius, mass, position), type, 10000, 10000,
		1000), mp_center(NULL), m_radius(0)
{
}

CelestialBody::CelestialBody(ObjectType type, Real mass, Real radius, CelestialBody * center, 
	Real distance, Real speed)
	: GameObject(SphereCollisionObject(radius, mass, Vector3(0,0,0)), type, 10000, 10000,
		1000), mp_center(center), m_radius(radius)
{
	Real totalDistance = distance + radius + center->radius();
	
	// Generate a random position on a sphere around the center with a radius
	// equal to the specified distance
	Real randAngle = Math::UnitRandom() * (2 * Math::PI);
	Real randMu = Math::RangeRandom(-0.1, 0.1);
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
SpaceShip::SpaceShip(ObjectType type, Real mass, Vector3 position, Real energyRecharge) : 
	GameObject(SphereCollisionObject(150, mass, position), type, 100, 100, energyRecharge), mp_weapons()
{
}

SpaceShip::SpaceShip(ObjectType type, Real mass, Vector3 position) : 
	GameObject(SphereCollisionObject(150, mass, position), type, 100, 100, 5), mp_weapons()
{
}

SpaceShip::SpaceShip(ObjectType type, Real mass) :
	GameObject(SphereCollisionObject(150, mass), type, 100, 100, 5), mp_weapons()
{
}

SpaceShip::SpaceShip(const SpaceShip& copy) :
	GameObject(copy), mp_weapons(copy.mp_weapons)
{
}

PlasmaCannon * SpaceShip::addPlasmaCannon(const PlasmaCannon& weapon)
{
	PlasmaCannon * newCannon = new PlasmaCannon(weapon);
	mp_weapons.push_back(newCannon);
	return newCannon;
}

AnchorLauncher * SpaceShip::addAnchorLauncher(const AnchorLauncher& weapon)
{
	AnchorLauncher * newLauncher = new AnchorLauncher(weapon);
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
GameArena::GameArena(Real size) : m_arenaSize(size), m_playerShip(NULL), m_npcShips(), m_projectiles(),
	m_bodies(), m_constraints(), m_listeners()
{
}

void GameArena::notifyObjectCreation(GameObject * object)
{
	for(std::vector<GameArenaListener * >::iterator listenerIter = m_listeners.begin(); 
		listenerIter != m_listeners.end();
		listenerIter++)
	{
		(*listenerIter)->newGameObject(object);
	}
}

void GameArena::notifyObjectDestruction(GameObject * object)
{
	for(std::vector<GameArenaListener * >::iterator listenerIter = m_listeners.begin(); 
		listenerIter != m_listeners.end();
		listenerIter++)
	{
		(*listenerIter)->destroyedGameObject(object);
	}
}

void GameArena::notifyConstraintCreation(Constraint * constraint)
{
	for(std::vector<GameArenaListener * >::iterator listenerIter = m_listeners.begin(); 
		listenerIter != m_listeners.end();
		listenerIter++)
	{
		(*listenerIter)->newConstraint(constraint);
	}
}

void GameArena::notifyConstraintDestruction(Constraint * constraint)
{
	for(std::vector<GameArenaListener * >::iterator listenerIter = m_listeners.begin(); 
		listenerIter != m_listeners.end();
		listenerIter++)
	{
		(*listenerIter)->destroyedConstraint(constraint);
	}
}

void GameArena::addGameArenaListener(GameArenaListener * listener) 
{
	m_listeners.push_back(listener);
}

void GameArena::removeGameArenaListener(GameArenaListener * listener)
{
	m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
}

Real GameArena::size() const {
	return m_arenaSize;
}

SpaceShip * GameArena::setPlayerShip(const SpaceShip& ship) {
	if(m_playerShip != NULL) {
		notifyObjectDestruction(m_playerShip);
		delete m_playerShip;
		m_playerShip = NULL;
	}

	// TODO: Deconstructor needs to handle deleting this
	// memory
	m_playerShip = new SpaceShip(ship);
	notifyObjectCreation(m_playerShip);

	return m_playerShip;
}

SpaceShip * GameArena::addNpcShip(const SpaceShip& ship)
{
	SpaceShip * p_ship = new SpaceShip(ship);
	m_npcShips.push_back(p_ship);
	notifyObjectCreation(p_ship);
	return p_ship;
}

Projectile * GameArena::addProjectile(const Projectile& projectile)
{
	Projectile * p_projectile = new Projectile(projectile);
	m_projectiles.push_back(p_projectile);
	notifyObjectCreation(p_projectile);
	return p_projectile;
}

Constraint * GameArena::addConstraint(const Constraint& constraint)
{
	Constraint * p_constraint = new Constraint(constraint);
	m_constraints.push_back(p_constraint);
	notifyConstraintCreation(p_constraint);
	return p_constraint;
}

CelestialBody * GameArena::addBody(const CelestialBody& body)
{
	CelestialBody * p_body = new CelestialBody(body);
	if(p_body->hasCenter()) {
		addConstraint(p_body->constraint());
	}
	m_bodies.push_back(p_body);
	notifyObjectCreation(p_body);
	return p_body;
}

std::vector<CelestialBody * >::iterator GameArena::destroyBody(CelestialBody * body)
{
	for(std::vector<CelestialBody * >::iterator iter =  m_bodies.begin(); 
		iter != m_bodies.end();
		iter++)
	{
		if(*iter == body) {
			if(body->hasCenter()) {
				// Ensure any constraints attached to this body are also destroyed
				for(std::vector<Constraint * >::iterator conIter =  m_constraints.begin(); 
					conIter != m_constraints.end();)
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
			delete body;
			return m_bodies.erase(iter);
		}
	}
	
	// TODO: Throw exception, constraint not found
	return m_bodies.end();
}

std::vector<Constraint * >::iterator GameArena::destroyConstraint(Constraint * constraint) 
{
	for(std::vector<Constraint * >::iterator iter =  m_constraints.begin(); 
		iter != m_constraints.end();
		iter++)
	{
		if(*iter == constraint) {
			notifyConstraintDestruction(constraint);
			delete constraint;
			return m_constraints.erase(iter);
		}
	}
	
	// TODO: Throw exception, constraint not found
	return m_constraints.end();
}

std::vector<Projectile * >::iterator GameArena::destroyProjectile(Projectile * projectile) 
{
	for(std::vector<Projectile * >::iterator iter =  m_projectiles.begin(); 
		iter != m_projectiles.end();
		iter++)
	{
		if(*iter == projectile) {
			notifyObjectDestruction(projectile);
			delete projectile;
			return m_projectiles.erase(iter);
		}
	}
	
	// TODO: Throw exception, projectile not found
	return m_projectiles.end();
}

std::vector<SpaceShip * >::iterator GameArena::destroyNpcShip(SpaceShip * npcShip) 
{
	for(std::vector<SpaceShip * >::iterator iter =  m_npcShips.begin(); 
		iter != m_npcShips.end();
		iter++)
	{
		if(*iter == npcShip) 
		{
			// Ensure any constraints attached to this ship are also destroyed
			for(std::vector<Constraint * >::iterator conIter =  m_constraints.begin(); 
				conIter != m_constraints.end();)
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
			delete npcShip;
			return m_npcShips.erase(iter);
		}
	}

	// TODO: Throw exception, ship not found
	return m_npcShips.end();
}

SpaceShip * GameArena::playerShip()
{
	return m_playerShip;
}

Projectile * GameArena::fireProjectileFromShip(SpaceShip * ship, int weaponIndex)
{
	return ship->fireWeapon(*this, weaponIndex);
}

std::vector<Projectile *> * GameArena::projectiles() {
	return & m_projectiles;
}

std::vector<SpaceShip *> * GameArena::npcShips() {
	return & m_npcShips;
}

void GameArena::updatePhysics(Real timeElapsed)
{
	// Apply forces from constraints
	for(std::vector<Constraint * >::iterator conIter =  m_constraints.begin(); 
		conIter != m_constraints.end();
		conIter++)
	{
		(*conIter)->applyForces(timeElapsed);
	}

	// Update physics for orbiting bodies
	for(std::vector<CelestialBody * >::iterator bodyIter =  m_bodies.begin(); 
		bodyIter != m_bodies.end();
		bodyIter++) 
	{
		(*bodyIter)->updatePhysics(timeElapsed);
	}

	// Update the player ship's physics, and reverse its velocity if it passes a wall
	if(m_playerShip != NULL) 
	{
		m_playerShip->updatePhysics(timeElapsed);
		m_playerShip->addEnergy(m_playerShip->energyRecharge() * timeElapsed);
		SphereCollisionObject * playerShipPhys = m_playerShip->phys();

		if(playerShipPhys->position().x > m_arenaSize || playerShipPhys->position().x < - m_arenaSize
			|| playerShipPhys->position().y > m_arenaSize || playerShipPhys->position().y < - m_arenaSize
			|| playerShipPhys->position().z > m_arenaSize || playerShipPhys->position().z < - m_arenaSize) 
		{
			// playerShipPhys->velocity(playerShipPhys->velocity() * Vector3(-1, -1, -1));
			
			// Slowly drain health if outside game boundaries
			// m_playerShip->health(m_playerShip->health() - (timeElapsed * 5));
		}
	}

	// Update physics for all NPC ships
	for(std::vector<SpaceShip * >::iterator shipIter =  m_npcShips.begin(); 
		shipIter != m_npcShips.end();
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
	for(std::vector<Projectile * >::iterator projIter =  m_projectiles.begin(); 
		projIter != m_projectiles.end(); )
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
		for(std::vector<SpaceShip * >::iterator shipIter =  m_npcShips.begin(); 
			shipIter != m_npcShips.end();
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
	for(std::vector<CelestialBody * >::iterator bodyIter =  m_bodies.begin(); 
		bodyIter != m_bodies.end();
		bodyIter++) 
	{
		if((*bodyIter)->phys()->checkCollision(*(m_playerShip->phys()))) {
			m_playerShip->inflictDamage(500);
		}

		for(std::vector<Projectile * >::iterator projIter =  m_projectiles.begin(); 
			projIter != m_projectiles.end(); )
		{
			if((*bodyIter)->phys()->checkCollision(*(*projIter)->phys())) {
				projIter = destroyProjectile(*projIter);
			} else {
				projIter++;
			}
		}
		
		for(std::vector<SpaceShip * >::iterator shipIter =  m_npcShips.begin(); 
		shipIter != m_npcShips.end();) 
		{
			if((*bodyIter)->phys()->checkCollision(*(*shipIter)->phys())) {
				shipIter = destroyNpcShip(*shipIter);
			} else {
				shipIter++;
			}
		}
	}

	// After all projectile collisions, remove any ships with less than 0 health
	for(std::vector<SpaceShip * >::iterator shipIter =  m_npcShips.begin(); 
		shipIter != m_npcShips.end();) 
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
	if(m_playerShip->health() <= 0) {
		m_playerShip->health(m_playerShip->maxHealth());
		m_playerShip->phys()->velocity(Vector3(0, 0, 0));
		m_playerShip->phys()->position(Vector3(10000, 10000, 10000));
	}
}


void GameArena::generateSolarSystem() 
{
	// Generate a star in the middle of the arena
	CelestialBody * star = addBody(CelestialBody(ObjectType::STAR, 100000, 10000, Vector3(0, 0, 0)));
	srand(time(NULL));
	int numPlanets = rand() % 10 + 4;

	// Add a random number of planents
	for(int i = 0; i < numPlanets; i++) {
		// For each planet, generate a random distance, radius, speed, and number of moons
		Real planet_radius = Math::RangeRandom(500, 4000);
		Real distance = 10000 + Math::RangeRandom(0, 34000);
		Real speed = Math::RangeRandom(2000, 15000);

		CelestialBody * planet = addBody(CelestialBody(ObjectType::PLANET, 10000, planet_radius,
			star, distance, speed));

		int numMoons = rand() % 3;
		for(int j = 0; j < 2; j++) {
			Real moon_radius = Math::RangeRandom(planet_radius * 0.1, planet_radius * 0.7);
			distance = Math::RangeRandom(planet_radius * 0.5, planet_radius * 2);
			speed = Math::RangeRandom(1, 3) * distance;
			CelestialBody * moon = addBody(CelestialBody(ObjectType::MOON, 1000, moon_radius,
				planet, distance, speed));
		}
	}
}