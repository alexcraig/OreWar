#include "RenderModel.h"
#include <OgreMatrix4.h>

using namespace Ogre;


// ========================================================================
// RenderObject Implementation
// ========================================================================
int RenderObject::m_nextRenderId = 1;

RenderObject::RenderObject(SceneManager * mgr)
	: mp_mgr(mgr), m_renderId(m_nextRenderId)
{
	m_nextRenderId++;
}

SceneManager * RenderObject::sceneManager()
{
	return mp_mgr;
}

int RenderObject::renderId() const
{
	return m_renderId;
}

bool RenderObject::operator==(const RenderObject &other) const
{
	return (m_renderId == other.m_renderId);
}


// ========================================================================
// ConstraintRenderObject Implementation
// ========================================================================
bool ConstraintRenderObject::m_resourcesLoaded = false;

ConstraintRenderObject::ConstraintRenderObject(Constraint * constraint, SceneManager * mgr)
	: RenderObject(mgr), mp_constraint(constraint), mp_node(), mp_particle(NULL)
{
}

Constraint * ConstraintRenderObject::constraint()
{
	return mp_constraint;
}

void ConstraintRenderObject::updateEffects(Real elapsedTime, Quaternion camOrientation)
{
	Vector3 offset = mp_constraint->getTarget()->position() - mp_constraint->getOrigin()->position();
	mp_node->setPosition(mp_constraint->getOrigin()->position() + (offset * Real(0.5)));
	mp_node->setOrientation(Vector3(0, 0, -1).getRotationTo(offset));

	// Note: This relies on a single Cylinder emitter being present in the Orewar/ConstraintStream script
	mp_particle->getEmitter(0)->setParameter("depth", Ogre::StringConverter::toString(offset.length()));
}

void ConstraintRenderObject::loadSceneResources()
{
}
 
void ConstraintRenderObject::createEffects()
{
	mp_node = sceneManager()->getRootSceneNode()->createChildSceneNode();

	std::stringstream oss;
	oss << "Constraint" << renderId();
	mp_particle = sceneManager()->createParticleSystem(oss.str(), "Orewar/ConstraintStream");

	if(mp_constraint->isRigid()) {
		mp_particle->setEmitting(false);
	} else {
		mp_particle->setEmitting(true);
	}

	mp_node->attachObject(mp_particle);
}

void ConstraintRenderObject::destroyEffects()
{
	mp_node->detachAllObjects();
	mp_node->removeAllChildren();
	sceneManager()->destroyParticleSystem(mp_particle);
	sceneManager()->destroySceneNode(mp_node);
}


// ========================================================================
// PhysicsRenderObject Implementation
// ========================================================================
PhysicsRenderObject::PhysicsRenderObject(SphereCollisionObject * object, SceneManager * mgr)
	: RenderObject(mgr), mp_object(object)
{
}

SphereCollisionObject * PhysicsRenderObject::physics() {
	return mp_object;
}

// ========================================================================
// ShipRO Implementation
// ========================================================================
bool ShipRO::m_resourcesLoaded = false;

ShipRO::ShipRO(SpaceShip * ship, SceneManager * mgr)
	: PhysicsRenderObject(ship->phys(), mgr), mp_spaceShip(ship), mp_shipNode(NULL), mp_shipRotateNode(NULL), mp_shipEntity(NULL),
	mp_spotLight(NULL), mp_pointLight(NULL), mp_engineParticles(NULL)
{
}

SpaceShip * ShipRO::ship() const
{
	return mp_spaceShip;
}

/** Updates the node based on passed time and camera orientation (useful for sprites) */
void ShipRO::updateEffects(Real elapsedTime, Quaternion camOrientation)
{
	mp_shipNode->setPosition(physics()->position());
	mp_shipNode->setOrientation(physics()->orientation());
}

void ShipRO::loadSceneResources() 
{
	if(!m_resourcesLoaded) {
		m_resourcesLoaded = true;
	}
}

void ShipRO::createEffects()
{
	mp_shipNode = sceneManager()->getRootSceneNode()->createChildSceneNode();
	std::stringstream oss;
	oss << "Ship" << renderId();
	mp_shipEntity = sceneManager()->createEntity(oss.str(), "RZR-002.mesh");
	mp_shipEntity->setCastShadows(true);
	mp_shipNode->setScale(10, 10, 10);
	mp_shipRotateNode = mp_shipNode->createChildSceneNode();
	mp_shipRotateNode->setDirection(Vector3(0, 0, 1));
	mp_shipRotateNode->attachObject(mp_shipEntity);

	// Add a spot light to the ship of it's the player ship
	oss << "L";
	mp_spotLight = sceneManager()->createLight(oss.str());
	mp_spotLight->setType(Light::LT_SPOTLIGHT);
	mp_spotLight->setDiffuseColour(0.8, 0.8, 1.0);
	mp_spotLight->setSpecularColour(0.2, 0.2, 1.0);
	mp_spotLight->setDirection(0, 0, -1);
	mp_spotLight->setPosition(Vector3(0, 30, 0));
	mp_spotLight->setSpotlightRange(Degree(20), Degree(45));
	mp_shipNode->attachObject(mp_spotLight);

	// Add a point light above the ship
	oss << "L";
	mp_pointLight = sceneManager()->createLight(oss.str());
	mp_pointLight->setType(Ogre::Light::LT_POINT);
	mp_pointLight->setPosition(Ogre::Vector3(0, 30, 0));
	if (ship()->type() == ObjectType::SHIP) {
		mp_pointLight->setDiffuseColour(0.4, 0.1, 0.1);
		mp_pointLight->setSpecularColour(0.4, 0.4, 0.4);
	} else {
		mp_pointLight->setDiffuseColour(0.1, 0.1, 0.5);
		mp_pointLight->setSpecularColour(0.4, 0.4, 0.4);
	}
	mp_shipNode->attachObject(mp_pointLight);

	// Add the engine particle stream
	oss << "P";
	mp_engineParticles = sceneManager()->createParticleSystem(oss.str(), "Orewar/EngineStream");
	mp_engineParticles->setEmitting(true);
	mp_shipNode->attachObject(mp_engineParticles);
}

void ShipRO::destroyEffects()
{
	mp_shipNode->removeAllChildren();
	mp_shipNode->detachAllObjects();
	mp_shipRotateNode->removeAllChildren();
	mp_shipRotateNode->detachAllObjects();
	sceneManager()->destroyMovableObject(mp_shipEntity);
	sceneManager()->destroySceneNode(mp_shipNode);
	sceneManager()->destroyLight(mp_spotLight);
	sceneManager()->destroyLight(mp_pointLight);
	sceneManager()->destroyParticleSystem(mp_engineParticles);
}


// ========================================================================
// NpcShipRO Implementation
// ========================================================================
bool NpcShipRO::m_resourcesLoaded = false;

NpcShipRO::NpcShipRO(SpaceShip * ship, SceneManager * mgr)
	: ShipRO(ship, mgr), mp_frameNode(NULL), mp_frameSprite(NULL), mp_screen(NULL),
	mp_healthBar(NULL), mp_energyBar(NULL)
{
}

void NpcShipRO::updateEffects(Real elapsedTime, Quaternion camOrientation)
{
	ShipRO::updateEffects(elapsedTime, camOrientation);
	mp_frameNode->setPosition(physics()->position());
	mp_frameNode->setOrientation(camOrientation);

	mp_healthBar->width((ship()->health() / ship()->maxHealth()) * 25000);
	mp_energyBar->width((ship()->energy() / ship()->maxEnergy()) * 25000);
}


void NpcShipRO::loadSceneResources()
{
	ShipRO::loadSceneResources();
	if(!m_resourcesLoaded) {
		Plane planeTarget = Plane(Ogre::Vector3::UNIT_Z, 0);
		MeshManager::getSingleton().createPlane("targetFrameSprite", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			planeTarget, 250, 250, 1, 1, true, 1, 1, 1, Ogre::Vector3::UNIT_Y);
	}
}

void NpcShipRO::createEffects()
{
	ShipRO::createEffects();
	mp_frameNode = sceneManager()->getRootSceneNode()->createChildSceneNode();

	std::stringstream oss;
	oss << "TargetFrame" << renderId();
	mp_frameSprite = sceneManager()->createEntity(oss.str(), "targetFrameSprite");
	mp_frameSprite->setMaterialName("Orewar/TargetFrame");
	mp_frameSprite->setCastShadows(false);
	mp_frameNode->attachObject(mp_frameSprite);

	
	Viewport * p_vp = sceneManager()->getCamera("Camera")->getViewport();
	Gorilla::Silverback * gorilla = Gorilla::Silverback::getSingletonPtr();
	mp_screen = gorilla->createScreenRenderable(Vector2(250, 440), "dejavu");
	mp_frameNode->attachObject(mp_screen);

	Gorilla::Layer * layer = mp_screen->createLayer(10);

	mp_healthBar = layer->createRectangle(Vector2(0, 0), Vector2(25000, 2000));
	mp_healthBar->background_colour(Gorilla::Colours::Red);
	mp_healthBar->border_colour(Gorilla::Colours::Red);

	Gorilla::Rectangle * healthBarBorder = layer->createRectangle(Vector2(0, 0), Vector2(25000, 2000));
	healthBarBorder->no_background();
	healthBarBorder->border_colour(Gorilla::Colours::Red);
	healthBarBorder->border_width(200);

	mp_energyBar = layer->createRectangle(Vector2(0, 2500), Vector2(25000, 2000));
	mp_energyBar->background_colour(Gorilla::Colours::Blue);
	mp_energyBar->border_colour(Gorilla::Colours::Blue);

	Gorilla::Rectangle * energyBarBorder = layer->createRectangle(Vector2(0, 2500), Vector2(25000, 2000));
	energyBarBorder->no_background();
	energyBarBorder->border_colour(Gorilla::Colours::Blue);
	energyBarBorder->border_width(200);
}

void NpcShipRO::destroyEffects()
{
	ShipRO::destroyEffects();
	mp_frameNode->detachAllObjects();
	mp_frameNode->removeAllChildren();
	sceneManager()->destroyMovableObject(mp_frameSprite);
	sceneManager()->destroySceneNode(mp_frameNode);
	Gorilla::Silverback::getSingletonPtr()->destroyScreenRenderable(mp_screen);
}


// ========================================================================
// CelestialBodyRO Implementation
// ========================================================================
bool CelestialBodyRO::m_resourcesLoaded = false;

/** Constructor */
CelestialBodyRO::CelestialBodyRO(CelestialBody * body, SceneManager * mgr) 
	: PhysicsRenderObject(body->phys(), mgr), mp_body(body), mp_bodyNode(NULL),
	mp_model(NULL), mp_pointLight(NULL), mp_particles(NULL)
{
}

/** @return The CelestialBody game object represented by this entity */
CelestialBody * CelestialBodyRO::body() const
{
	return mp_body;
}

/** Updates the node based on passed time and camera orientation (useful for sprites) */
void CelestialBodyRO::updateEffects(Real elapsedTime, Quaternion camOrientation)
{
	mp_bodyNode->setPosition(mp_body->phys()->position());
}

/** @see RenderObject::loadSceneResources() */ 
void CelestialBodyRO::loadSceneResources()
{
	if(!m_resourcesLoaded) {
		m_resourcesLoaded = true;
	}
}

/** @see RenderObject::createEffects() */ 
void CelestialBodyRO::createEffects()
{
	Real modelSizeScale = 1.0 / 100;
	std::stringstream oss;
	oss << "CelestialBody" << renderId();

	mp_bodyNode = sceneManager()->getRootSceneNode()->createChildSceneNode();
	mp_bodyNode->setPosition(mp_body->phys()->position());

	mp_model = sceneManager()->createEntity(oss.str(), "sphere.mesh");

	if(mp_body->type() == ObjectType::STAR) {
		mp_model->setMaterialName("Orewar/Star");
	} else if(mp_body->type() == ObjectType::PLANET) {
		mp_model->setMaterialName("Orewar/Planet");
	} if(mp_body->type() == ObjectType::MOON) {
		mp_model->setMaterialName("Orewar/Moon");
	}

	mp_bodyNode->attachObject(mp_model);
	mp_bodyNode->setScale(
		mp_body->radius() * modelSizeScale,
		mp_body->radius() * modelSizeScale, 
		mp_body->radius() * modelSizeScale);
	
	if(mp_body->type() == ObjectType::STAR) {
		oss << "L";
		mp_pointLight = sceneManager()->createLight(oss.str());
		mp_pointLight->setType(Ogre::Light::LT_POINT);
		mp_pointLight->setDiffuseColour(0.9, 0.6, 0.05);
		mp_pointLight->setSpecularColour(1, 1, 1);
		mp_pointLight->setAttenuation(40000, 1.0, 0.007, 0.00014);
		mp_pointLight->setCastShadows(false);
		mp_bodyNode->attachObject(mp_pointLight);

		oss << "P";
		mp_particles = sceneManager()->createParticleSystem(oss.str(), "Orewar/StarFlare");
		mp_bodyNode->attachObject(mp_particles);
	}
}

/** @see RenderObject::destroyEffects() */ 
void CelestialBodyRO::destroyEffects()
{
	mp_bodyNode->removeAllChildren();
	mp_bodyNode->detachAllObjects();
	sceneManager()->destroyMovableObject(mp_model);
	sceneManager()->destroySceneNode(mp_bodyNode);

	if(mp_body->type() == ObjectType::STAR) {
		sceneManager()->destroyLight(mp_pointLight);
		sceneManager()->destroyParticleSystem(mp_particles);
	}
}

// ========================================================================
// ProjectileRO Implementation
// ========================================================================
bool ProjectileRO::m_resourcesLoaded = false;

ProjectileRO::ProjectileRO(Projectile * proj, SceneManager * mgr)
	: PhysicsRenderObject(proj->phys(), mgr), mp_projectile(proj), mp_projNode(NULL), 
	mp_pointLight(NULL), mp_particle(NULL)
{
}

Projectile * ProjectileRO::projectile() const
{
	return mp_projectile;
}

void ProjectileRO::updateEffects(Real elapsedTime, Quaternion camOrientation)
{
	mp_projNode->setPosition(physics()->position());
	mp_projNode->setOrientation(Vector3(0, 0, -1).getRotationTo(physics()->velocity()));
}


void ProjectileRO::loadSceneResources()
{
	if(!m_resourcesLoaded) {
		m_resourcesLoaded = true;
	}
}

void ProjectileRO::createEffects()
{
	mp_projNode = sceneManager()->getRootSceneNode()->createChildSceneNode();

	std::stringstream oss;
	oss << "Projectile" << renderId();

	// Dynamic point lights on projectiles
	mp_pointLight = sceneManager()->createLight(oss.str());
	mp_pointLight->setType(Ogre::Light::LT_POINT);
	mp_pointLight->setPosition(Ogre::Vector3(0, 60, 0));
	mp_pointLight->setAttenuation(3250, 1.0, 0.0014, 0.000007);
	mp_pointLight->setCastShadows(false);
	mp_projNode->attachObject(mp_pointLight);

	oss << "P";
	if(mp_projectile->type() == ObjectType::PROJECTILE) {
		mp_particle = sceneManager()->createParticleSystem(oss.str(), "Orewar/PlasmaStream");
		mp_pointLight->setDiffuseColour(0.0, 1, 0.0);
		mp_pointLight->setSpecularColour(0.2, 0.7, 0.2);
	} else if (mp_projectile->type() == ObjectType::ANCHOR_PROJECTILE) {
		mp_particle = sceneManager()->createParticleSystem(oss.str(), "Orewar/Anchor");
		mp_pointLight->setDiffuseColour(1, 0.0, 0.0);
		mp_pointLight->setSpecularColour(0.7, 0.2, 0.2);
	}
	mp_particle->setEmitting(true);
	mp_projNode->attachObject(mp_particle);
}

void ProjectileRO::destroyEffects()
{
	mp_projNode->detachAllObjects();
	mp_projNode->removeAllChildren();
	sceneManager()->destroyLight(mp_pointLight);
	sceneManager()->destroySceneNode(mp_projNode);
	sceneManager()->destroyParticleSystem(mp_particle);
}


// ========================================================================
// RenderModel Implementation
// ========================================================================
RenderModel::RenderModel(GameArena& model, SceneManager * mgr) : m_model(model), m_physicsRenderList(),
	mp_mgr(mgr)
{
	m_model.addGameArenaListener(this);
}


void RenderModel::updateRenderList(Real elapsedTime, Quaternion camOrientation)
{
	for(std::vector<PhysicsRenderObject *>::iterator physIter = m_physicsRenderList.begin();
		physIter != m_physicsRenderList.end();
		physIter++) 
	{
		(*(*physIter)).updateEffects(elapsedTime, camOrientation);
	}

	for(std::vector<ConstraintRenderObject *>::iterator conIter = m_constraintRenderList.begin();
		conIter != m_constraintRenderList.end();
		conIter++) 
	{
		(*(*conIter)).updateEffects(elapsedTime, camOrientation);
	}
}

void RenderModel::newGameObject(GameObject * object)
{
	PhysicsRenderObject * p_renderObj = NULL;
	if(object->type() == ObjectType::SHIP) {
		p_renderObj = new ShipRO((SpaceShip*)object, mp_mgr);
	} else if (object->type() == ObjectType::NPC_SHIP) {
		p_renderObj = new NpcShipRO((SpaceShip*)object, mp_mgr);
	} else if (object->type() == ObjectType::PROJECTILE
		|| object->type() == ObjectType::ANCHOR_PROJECTILE) 
	{
		p_renderObj = new ProjectileRO((Projectile*)object, mp_mgr);
	} else if (object->type() == ObjectType::STAR
		|| object->type() == ObjectType::PLANET
		|| object->type() == ObjectType::MOON) 
	{
		p_renderObj = new CelestialBodyRO((CelestialBody*)object, mp_mgr);
	}

	p_renderObj->loadSceneResources();
	p_renderObj->createEffects();
	m_physicsRenderList.push_back(p_renderObj);
}

void RenderModel::destroyedGameObject(GameObject * object)
{
	for(std::vector<PhysicsRenderObject *>::iterator renderIter =  m_physicsRenderList.begin(); 
		renderIter != m_physicsRenderList.end();
		renderIter++) {

			if((*(*renderIter)).physics() == object->phys()) {
			(*(*renderIter)).destroyEffects();
			delete (*renderIter);
			m_physicsRenderList.erase(std::remove(m_physicsRenderList.begin(), 
				m_physicsRenderList.end(), (*renderIter)), m_physicsRenderList.end());
			return;
		}
	}
}

void RenderModel::newConstraint(Constraint * constraint)
{
	ConstraintRenderObject * p_renderObj = new ConstraintRenderObject(constraint, mp_mgr);

	p_renderObj->loadSceneResources();
	p_renderObj->createEffects();
	m_constraintRenderList.push_back(p_renderObj);
}

void RenderModel::destroyedConstraint(Constraint * constraint)
{
	for(std::vector<ConstraintRenderObject *>::iterator renderIter =  m_constraintRenderList.begin(); 
		renderIter != m_constraintRenderList.end();
		renderIter++) {

		if((*(*renderIter)).constraint() == constraint) {
			(*(*renderIter)).destroyEffects();
			delete (*renderIter);
			m_constraintRenderList.erase(std::remove(m_constraintRenderList.begin(), 
				m_constraintRenderList.end(), (*renderIter)), m_constraintRenderList.end());
			return;
		}
	}
}

int RenderModel::getNumObjects() const
{
	return m_physicsRenderList.size() + m_constraintRenderList.size();
}