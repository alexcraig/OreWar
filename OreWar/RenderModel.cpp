#include "RenderModel.h"
#include <OgreMatrix4.h>

using namespace Ogre;

// ========================================================================
// RenderObject Implementation
// ========================================================================
RenderObject::RenderObject(PhysicsObject * object, SceneNode * node)
	: m_object(object), m_node(node)
{
}

void RenderObject::updateNode(Real elapsedTime)
{
	m_node->setPosition(m_object->getPosition());
	m_node->setOrientation(m_object->getOrientation());
	// m_node->lookAt(m_object->getPosition() + m_object->getHeading(), Node::TS_WORLD, Vector3::UNIT_Z);
}

PhysicsObject * RenderObject::getObject() {
	return m_object;
}

SceneNode * RenderObject::getRootNode() {
	return m_node;
}


// ========================================================================
// RenderModel Implementation
// ========================================================================
RenderModel::RenderModel(GameArena& model, SceneManager * mgr) : m_model(model), m_renderList(),
	m_mgr(mgr), m_entityIndex(0)
{
	m_model.addGameArenaListener(this);
}

void RenderModel::generateRenderObject(PhysicsObject * object)
{
	if(object->getType() == ObjectType::SHIP) {
		SceneNode * shipNode = m_mgr->getRootSceneNode()->createChildSceneNode();
		std::stringstream oss;
		oss << "Ship" << m_entityIndex;
		Entity * shipEntity = m_mgr->createEntity(oss.str(), "RZR-002.mesh");
		shipEntity->setCastShadows(true);
		shipNode->setScale(6, 6, 6);
		SceneNode * shipRotateNode = shipNode->createChildSceneNode();
		shipRotateNode->setDirection(Vector3(0, 0, 1));
		shipRotateNode->attachObject(shipEntity);

		// Add a spot light to the ship
		oss << "L1";
		Ogre::Light* spotLight = m_mgr->createLight(oss.str());
		spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
		spotLight->setDiffuseColour(0.8, 0.8, 1.0);
		spotLight->setSpecularColour(0.2, 0.2, 1.0);
		spotLight->setDirection(0, -1, -3);
		spotLight->setPosition(Vector3(0, 30, 0));
		spotLight->setSpotlightRange(Ogre::Degree(20), Ogre::Degree(45));
		shipNode->attachObject(spotLight);
		
		// Add a point light aboive the ship
		oss << "L2";
		Ogre::Light* pointLight = m_mgr->createLight(oss.str());
		pointLight->setType(Ogre::Light::LT_POINT);
		pointLight->setPosition(Ogre::Vector3(0, 30, 0));
		pointLight->setDiffuseColour(0.4, 0.1, 0.1);
		pointLight->setSpecularColour(0.4, 0.4, 0.4);
		shipNode->attachObject(pointLight);

		RenderObject renderShip = RenderObject(object, shipNode);
		m_renderList.push_back(renderShip);
		m_entityIndex++;
	} else if (object->getType() == ObjectType::PROJECTILE) {
		SceneNode * projNode = m_mgr->getRootSceneNode()->createChildSceneNode();

		std::stringstream oss;
		oss << "Projectile" << m_entityIndex;
		Ogre::Entity * projEntity = m_mgr->createEntity(oss.str(), "RZR-002.mesh");
		projEntity->setCastShadows(false);
		projNode->attachObject(projEntity);

		// Dynamic spot lights on projectiles
		oss << "Light";
		Ogre::Light* pointLight = m_mgr->createLight(oss.str());
		pointLight->setType(Ogre::Light::LT_POINT);
		pointLight->setPosition(Ogre::Vector3(0, 60, 0));
		pointLight->setDiffuseColour(0.0, 1, 0.0);
		pointLight->setSpecularColour(0.2, 0.7, 0.2);
		pointLight->setAttenuation(3250, 1.0, 0.0014, 0.000007);
		projNode->attachObject(pointLight);
		

		RenderObject newProj = RenderObject(object, projNode);
		m_renderList.push_back(newProj);
		m_entityIndex++;
	}
}

void RenderModel::destroyRenderObject(PhysicsObject * object)
{
	for(int i = 0; i < m_renderList.size(); i++) {
		if(m_renderList[i].getObject() == object) {
			m_mgr->destroySceneNode(m_renderList[i].getRootNode());
			m_renderList.erase(m_renderList.begin() + i);
		}
	}
}

void RenderModel::updateRenderList(Real elapsedTime)
{
	for(int i = 0; i < m_renderList.size(); i++) {
		m_renderList[i].updateNode(elapsedTime);
	}
}

void RenderModel::newPhysicsObject(PhysicsObject * object)
{
	generateRenderObject(object);
}

void RenderModel::destroyedPhysicsObject(PhysicsObject * object)
{
	destroyRenderObject(object);
}