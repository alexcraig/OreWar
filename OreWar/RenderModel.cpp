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

ConstraintRenderObject::ConstraintRenderObject(Constraint * constraint, SceneManager * mgr,
	int numSprites)
	: RenderObject(mgr), mp_constraint(constraint), m_numSprites(numSprites), m_nodes(),
	m_entities()
{
}

Constraint * ConstraintRenderObject::getConstraint()
{
	return mp_constraint;
}

void ConstraintRenderObject::updateNode(Real elapsedTime, Quaternion camOrientation)
{
	Vector3 offset = mp_constraint->getEndObject()->getPosition() - mp_constraint->getStartObject()->getPosition();

	Real increment = Real(1) / Real(m_numSprites + 2);
	int moveCounter = 1;
	for(std::vector<SceneNode *>::iterator nodeIter = m_nodes.begin();
		nodeIter != m_nodes.end();
		nodeIter++) 
	{
		(*nodeIter)->setPosition(mp_constraint->getStartObject()->getPosition()
		+ (offset * Real(increment * moveCounter)));
		(*nodeIter)->setOrientation(camOrientation);
		moveCounter++;
	}
}

void ConstraintRenderObject::loadSceneResources()
{
	if(!m_resourcesLoaded) {
		Plane planePlasma = Plane(Ogre::Vector3::UNIT_Z, 0);
		MeshManager::getSingleton().createPlane("conSprite", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			planePlasma, 100, 100, 1, 1, true, 1, 1, 1, Ogre::Vector3::UNIT_Y);
		m_resourcesLoaded = true;
	}
}

void ConstraintRenderObject::buildNode()
{
	int i;

	for(i = 0; i < m_numSprites; i++)
	{
		SceneNode * p_node = getSceneManager()->getRootSceneNode()->createChildSceneNode();

		std::stringstream oss;
		oss << "Constraint" << getRenderIndex() << "_" << i;
		Entity * p_entity = getSceneManager()->createEntity(oss.str(), "conSprite");
		p_entity->setMaterialName("Orewar/ConstraintSprite");
		p_entity->setCastShadows(false);
		p_node->attachObject(p_entity);
		m_nodes.push_back(p_node);
		m_entities.push_back(p_entity);
	}
}

void ConstraintRenderObject::destroyNode()
{
	std::vector<Entity *>::iterator entIter = m_entities.begin();
	for(std::vector<SceneNode *>::iterator nodeIter = m_nodes.begin();
		nodeIter != m_nodes.end();
		nodeIter++) 
	{
		(*nodeIter)->detachAllObjects();
		(*nodeIter)->removeAllChildren();
		getSceneManager()->destroyMovableObject((*entIter));
		getSceneManager()->destroySceneNode((*nodeIter));
		entIter++;
	}

	m_nodes.clear();
	m_entities.clear();
}


// ========================================================================
// PhysicsRenderObject Implementation
// ========================================================================
PhysicsRenderObject::PhysicsRenderObject(PhysicsObject * object, SceneManager * mgr)
	: RenderObject(mgr), mp_object(object)
{
}

PhysicsObject * PhysicsRenderObject::getObject() {
	return mp_object;
}

// ========================================================================
// ShipRO Implementation
// ========================================================================
bool ShipRO::m_resourcesLoaded = false;

ShipRO::ShipRO(PhysicsObject * object, SceneManager * mgr)
	: PhysicsRenderObject(object, mgr), mp_shipNode(NULL), mp_shipRotateNode(NULL), mp_shipEntity(NULL),
	mp_spotLight(NULL), mp_pointLight(NULL)
{
}

/** Updates the node based on passed time and camera orientation (useful for sprites) */
void ShipRO::updateNode(Real elapsedTime, Quaternion camOrientation)
{
	mp_shipNode->setPosition(getObject()->getPosition());
	mp_shipNode->setOrientation(getObject()->getOrientation());
}

void ShipRO::loadSceneResources() 
{
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
}


// ========================================================================
// NpcShipRO Implementation
// ========================================================================
bool NpcShipRO::m_resourcesLoaded = false;

NpcShipRO::NpcShipRO(PhysicsObject * object, SceneManager * mgr)
	: ShipRO(object, mgr), mp_frameNode(NULL), mp_frameSprite(NULL)
{
}

void NpcShipRO::updateNode(Real elapsedTime, Quaternion camOrientation)
{
	ShipRO::updateNode(elapsedTime, camOrientation);
	mp_frameNode->setPosition(getObject()->getPosition());
	mp_frameNode->setOrientation(camOrientation);
}


void NpcShipRO::loadSceneResources()
{
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
}

void NpcShipRO::destroyNode()
{
	ShipRO::destroyNode();
	mp_frameNode->detachAllObjects();
	mp_frameNode->removeAllChildren();
	getSceneManager()->destroyMovableObject(mp_frameSprite);
	getSceneManager()->destroySceneNode(mp_frameNode);
}

// ========================================================================
// ProjectileRO Implementation
// ========================================================================
bool ProjectileRO::m_resourcesLoaded = false;

ProjectileRO::ProjectileRO(PhysicsObject * object, SceneManager * mgr)
	: PhysicsRenderObject(object, mgr), mp_projNode(NULL), mp_projSprite(NULL), mp_pointLight(NULL)
{
}

void ProjectileRO::updateNode(Real elapsedTime, Quaternion camOrientation)
{
	mp_projNode->setPosition(getObject()->getPosition());
	mp_projNode->setOrientation(camOrientation);
}


void ProjectileRO::loadSceneResources()
{
	if(!m_resourcesLoaded) {
		Plane planePlasma = Plane(Ogre::Vector3::UNIT_Z, 0);
		MeshManager::getSingleton().createPlane("projSprite", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			planePlasma, 100, 100, 1, 1, true, 1, 1, 1, Ogre::Vector3::UNIT_Y);
		m_resourcesLoaded = true;
	}
}

void ProjectileRO::buildNode()
{
	mp_projNode = getSceneManager()->getRootSceneNode()->createChildSceneNode();

	std::stringstream oss;
	oss << "Projectile" << getRenderIndex();
	mp_projSprite = getSceneManager()->createEntity(oss.str(), "projSprite");
	mp_projSprite->setMaterialName("Orewar/PlasmaSprite");
	mp_projSprite->setCastShadows(false);
	mp_projNode->attachObject(mp_projSprite);

	// Dynamic point lights on projectiles
	oss << "Light";
	mp_pointLight = getSceneManager()->createLight(oss.str());
	mp_pointLight->setType(Ogre::Light::LT_POINT);
	mp_pointLight->setPosition(Ogre::Vector3(0, 60, 0));
	mp_pointLight->setDiffuseColour(0.0, 1, 0.0);
	mp_pointLight->setSpecularColour(0.2, 0.7, 0.2);
	mp_pointLight->setAttenuation(3250, 1.0, 0.0014, 0.000007);
	mp_pointLight->setCastShadows(false);
	mp_projNode->attachObject(mp_pointLight);
}

void ProjectileRO::destroyNode()
{
	mp_projNode->detachAllObjects();
	mp_projNode->removeAllChildren();
	getSceneManager()->destroyMovableObject(mp_projSprite);
	getSceneManager()->destroyLight(mp_pointLight);
	getSceneManager()->destroySceneNode(mp_projNode);
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

void RenderModel::newPhysicsObject(PhysicsObject * object)
{
	PhysicsRenderObject * p_renderObj = NULL;
	if(object->getType() == ObjectType::SHIP) {
		p_renderObj = new ShipRO(object, mp_mgr);
	} else if (object->getType() == ObjectType::NPC_SHIP) {
		p_renderObj = new NpcShipRO(object, mp_mgr);
	} else if (object->getType() == ObjectType::PROJECTILE) {
		p_renderObj = new ProjectileRO(object, mp_mgr);
	}

	p_renderObj->loadSceneResources();
	p_renderObj->buildNode();
	m_physicsRenderList.push_back(p_renderObj);
}

void RenderModel::destroyedPhysicsObject(PhysicsObject * object)
{
	for(std::vector<PhysicsRenderObject *>::iterator renderIter =  m_physicsRenderList.begin(); 
		renderIter != m_physicsRenderList.end();
		renderIter++) {

		if((*(*renderIter)).getObject() == object) {
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
	ConstraintRenderObject * p_renderObj = new ConstraintRenderObject(constraint, mp_mgr, 10);

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