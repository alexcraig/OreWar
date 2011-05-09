#include "RenderModel.h"
#include <OgreMatrix4.h>

using namespace Ogre;


// ========================================================================
// RenderObject Implementation
// ========================================================================
int RenderObject::m_nextRenderIndex = 1;

RenderObject::RenderObject(SceneManager * mgr)
	: mp_mgr(mgr), m_renderIndex(m_nextRenderIndex)
{
	m_nextRenderIndex++;
}

SceneManager * RenderObject::getSceneManager()
{
	return mp_mgr;
}

int RenderObject::getRenderIndex()
{
	return m_renderIndex;
}

bool RenderObject::operator==(const RenderObject &other) const
{
	return (m_renderIndex == other.m_renderIndex);
}


// ========================================================================
// ConstraintRenderObject Implementation
// ========================================================================
bool ConstraintRenderObject::m_resourcesLoaded = false;

ConstraintRenderObject::ConstraintRenderObject(Constraint * constraint, SceneManager * mgr)
	: RenderObject(mgr), mp_constraint(constraint), mp_node(), mp_particle(NULL)
{
}

Constraint * ConstraintRenderObject::getConstraint()
{
	return mp_constraint;
}

void ConstraintRenderObject::updateNode(Real elapsedTime, Quaternion camOrientation)
{
	Vector3 offset = mp_constraint->getEndObject()->getPosition() - mp_constraint->getStartObject()->getPosition();
	mp_node->setPosition(mp_constraint->getStartObject()->getPosition() + (offset * Real(0.5)));
	mp_node->setOrientation(Vector3(0, 0, -1).getRotationTo(offset));

	// Note: This relies on a single Cylinder emitter being present in the Orewar/ConstraintStream script
	mp_particle->getEmitter(0)->setParameter("depth", Ogre::StringConverter::toString(offset.length()));
}

void ConstraintRenderObject::loadSceneResources()
{
}
 
void ConstraintRenderObject::buildNode()
{
	mp_node = getSceneManager()->getRootSceneNode()->createChildSceneNode();

	std::stringstream oss;
	oss << "Constraint" << getRenderIndex();
	mp_particle = getSceneManager()->createParticleSystem(oss.str(), "Orewar/ConstraintStream");
	mp_particle->setEmitting(true);
	mp_node->attachObject(mp_particle);
}

void ConstraintRenderObject::destroyNode()
{
	mp_node->detachAllObjects();
	mp_node->removeAllChildren();
	getSceneManager()->destroyParticleSystem(mp_particle);
	getSceneManager()->destroySceneNode(mp_node);
}


// ========================================================================
// PhysicsRenderObject Implementation
// ========================================================================
PhysicsRenderObject::PhysicsRenderObject(SphereCollisionObject * object, SceneManager * mgr)
	: RenderObject(mgr), mp_object(object)
{
}

SphereCollisionObject * PhysicsRenderObject::getObject() {
	return mp_object;
}

// ========================================================================
// ShipRO Implementation
// ========================================================================
bool ShipRO::m_resourcesLoaded = false;

ShipRO::ShipRO(SpaceShip * ship, SceneManager * mgr)
	: PhysicsRenderObject(ship->getPhysicsModel(), mgr), mp_spaceShip(ship), mp_shipNode(NULL), mp_shipRotateNode(NULL), mp_shipEntity(NULL),
	mp_spotLight(NULL), mp_pointLight(NULL), mp_engineParticles(NULL)
{
}

SpaceShip * ShipRO::getSpaceShip() const
{
	return mp_spaceShip;
}

/** Updates the node based on passed time and camera orientation (useful for sprites) */
void ShipRO::updateNode(Real elapsedTime, Quaternion camOrientation)
{
	mp_shipNode->setPosition(getObject()->getPosition());
	mp_shipNode->setOrientation(getObject()->getOrientation());
}

void ShipRO::loadSceneResources() 
{
	if(!m_resourcesLoaded) {
		m_resourcesLoaded = true;
	}
}

void ShipRO::buildNode()
{
	mp_shipNode = getSceneManager()->getRootSceneNode()->createChildSceneNode();
	std::stringstream oss;
	oss << "Ship" << getRenderIndex();
	mp_shipEntity = getSceneManager()->createEntity(oss.str(), "RZR-002.mesh");
	mp_shipEntity->setCastShadows(true);
	mp_shipNode->setScale(10, 10, 10);
	mp_shipRotateNode = mp_shipNode->createChildSceneNode();
	mp_shipRotateNode->setDirection(Vector3(0, 0, 1));
	mp_shipRotateNode->attachObject(mp_shipEntity);

	// Add a spot light to the ship of it's the player ship
	oss << "L";
	mp_spotLight = getSceneManager()->createLight(oss.str());
	mp_spotLight->setType(Light::LT_SPOTLIGHT);
	mp_spotLight->setDiffuseColour(0.8, 0.8, 1.0);
	mp_spotLight->setSpecularColour(0.2, 0.2, 1.0);
	mp_spotLight->setDirection(0, 0, -1);
	mp_spotLight->setPosition(Vector3(0, 30, 0));
	mp_spotLight->setSpotlightRange(Degree(20), Degree(45));
	mp_shipNode->attachObject(mp_spotLight);

	// Add a point light above the ship
	oss << "L";
	mp_pointLight = getSceneManager()->createLight(oss.str());
	mp_pointLight->setType(Ogre::Light::LT_POINT);
	mp_pointLight->setPosition(Ogre::Vector3(0, 30, 0));
	if (getObject()->getType() == ObjectType::SHIP) {
		mp_pointLight->setDiffuseColour(0.4, 0.1, 0.1);
		mp_pointLight->setSpecularColour(0.4, 0.4, 0.4);
	} else {
		mp_pointLight->setDiffuseColour(0.1, 0.1, 0.5);
		mp_pointLight->setSpecularColour(0.4, 0.4, 0.4);
	}
	mp_shipNode->attachObject(mp_pointLight);

	// Add the engine particle stream
	oss << "P";
	mp_engineParticles = getSceneManager()->createParticleSystem(oss.str(), "Orewar/EngineStream");
	mp_engineParticles->setEmitting(true);
	mp_shipNode->attachObject(mp_engineParticles);
}

void ShipRO::destroyNode()
{
	mp_shipNode->removeAllChildren();
	mp_shipNode->detachAllObjects();
	mp_shipRotateNode->removeAllChildren();
	mp_shipRotateNode->detachAllObjects();
	getSceneManager()->destroyMovableObject(mp_shipEntity);

	getSceneManager()->destroyLight(mp_spotLight);
	getSceneManager()->destroyLight(mp_pointLight);
	getSceneManager()->destroyParticleSystem(mp_engineParticles);
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

void NpcShipRO::updateNode(Real elapsedTime, Quaternion camOrientation)
{
	ShipRO::updateNode(elapsedTime, camOrientation);
	mp_frameNode->setPosition(getObject()->getPosition());
	mp_frameNode->setOrientation(camOrientation);

	mp_healthBar->width((getSpaceShip()->getHealth() / getSpaceShip()->getMaxHealth()) * 25000);
	mp_energyBar->width((getSpaceShip()->getShields() / getSpaceShip()->getMaxShields()) * 25000);
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

void NpcShipRO::buildNode()
{
	ShipRO::buildNode();
	mp_frameNode = getSceneManager()->getRootSceneNode()->createChildSceneNode();

	std::stringstream oss;
	oss << "TargetFrame" << getRenderIndex();
	mp_frameSprite = getSceneManager()->createEntity(oss.str(), "targetFrameSprite");
	mp_frameSprite->setMaterialName("Orewar/TargetFrame");
	mp_frameSprite->setCastShadows(false);
	mp_frameNode->attachObject(mp_frameSprite);

	
	Viewport * p_vp = getSceneManager()->getCamera("Camera")->getViewport();
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

void NpcShipRO::destroyNode()
{
	ShipRO::destroyNode();
	mp_frameNode->detachAllObjects();
	mp_frameNode->removeAllChildren();
	getSceneManager()->destroyMovableObject(mp_frameSprite);
	getSceneManager()->destroySceneNode(mp_frameNode);
	Gorilla::Silverback::getSingletonPtr()->destroyScreenRenderable(mp_screen);
}

// ========================================================================
// ProjectileRO Implementation
// ========================================================================
bool ProjectileRO::m_resourcesLoaded = false;

ProjectileRO::ProjectileRO(Projectile * proj, SceneManager * mgr)
	: PhysicsRenderObject(proj->getPhysicsModel(), mgr), mp_projectile(proj), mp_projNode(NULL), 
	mp_pointLight(NULL), mp_particle(NULL)
{
}

Projectile * ProjectileRO::getProjectile() const
{
	return mp_projectile;
}

void ProjectileRO::updateNode(Real elapsedTime, Quaternion camOrientation)
{
	mp_projNode->setPosition(getObject()->getPosition());
	mp_projNode->setOrientation(Vector3(0, 0, -1).getRotationTo(getObject()->getVelocity()));
}


void ProjectileRO::loadSceneResources()
{
	if(!m_resourcesLoaded) {
		m_resourcesLoaded = true;
	}
}

void ProjectileRO::buildNode()
{
	mp_projNode = getSceneManager()->getRootSceneNode()->createChildSceneNode();

	std::stringstream oss;
	oss << "Projectile" << getRenderIndex();

	// Dynamic point lights on projectiles
	mp_pointLight = getSceneManager()->createLight(oss.str());
	mp_pointLight->setType(Ogre::Light::LT_POINT);
	mp_pointLight->setPosition(Ogre::Vector3(0, 60, 0));
	mp_pointLight->setDiffuseColour(0.0, 1, 0.0);
	mp_pointLight->setSpecularColour(0.2, 0.7, 0.2);
	mp_pointLight->setAttenuation(3250, 1.0, 0.0014, 0.000007);
	mp_pointLight->setCastShadows(false);
	mp_projNode->attachObject(mp_pointLight);

	oss << "P";
	mp_particle = getSceneManager()->createParticleSystem(oss.str(), "Orewar/PlasmaStream");
	mp_particle->setEmitting(true);
	mp_projNode->attachObject(mp_particle);
}

void ProjectileRO::destroyNode()
{
	mp_projNode->detachAllObjects();
	mp_projNode->removeAllChildren();
	getSceneManager()->destroyLight(mp_pointLight);
	getSceneManager()->destroySceneNode(mp_projNode);
	getSceneManager()->destroyParticleSystem(mp_particle);
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
		(*(*physIter)).updateNode(elapsedTime, camOrientation);
	}

	for(std::vector<ConstraintRenderObject *>::iterator conIter = m_constraintRenderList.begin();
		conIter != m_constraintRenderList.end();
		conIter++) 
	{
		(*(*conIter)).updateNode(elapsedTime, camOrientation);
	}
}

void RenderModel::newGameObject(GameObject * object)
{
	PhysicsRenderObject * p_renderObj = NULL;
	if(object->getType() == ObjectType::SHIP) {
		p_renderObj = new ShipRO((SpaceShip*)object, mp_mgr);
	} else if (object->getType() == ObjectType::NPC_SHIP) {
		p_renderObj = new NpcShipRO((SpaceShip*)object, mp_mgr);
	} else if (object->getType() == ObjectType::PROJECTILE) {
		p_renderObj = new ProjectileRO((Projectile*)object, mp_mgr);
	}

	p_renderObj->loadSceneResources();
	p_renderObj->buildNode();
	m_physicsRenderList.push_back(p_renderObj);
}

void RenderModel::destroyedGameObject(GameObject * object)
{
	for(std::vector<PhysicsRenderObject *>::iterator renderIter =  m_physicsRenderList.begin(); 
		renderIter != m_physicsRenderList.end();
		renderIter++) {

			if((*(*renderIter)).getObject() == object->getPhysicsModel()) {
			(*(*renderIter)).destroyNode();
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
	p_renderObj->buildNode();
	m_constraintRenderList.push_back(p_renderObj);
}

void RenderModel::destroyedConstraint(Constraint * constraint)
{
	for(std::vector<ConstraintRenderObject *>::iterator renderIter =  m_constraintRenderList.begin(); 
		renderIter != m_constraintRenderList.end();
		renderIter++) {

		if((*(*renderIter)).getConstraint() == constraint) {
			(*(*renderIter)).destroyNode();
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