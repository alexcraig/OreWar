#include "RenderModel.h"
#include <OgreMatrix4.h>

using namespace Ogre;

// ========================================================================
// RenderObject Implementation
// ========================================================================
RenderObject::RenderObject(PhysicsObject * object, SceneNode * node)
	: mp_object(object), mp_node(node)
{
}

void RenderObject::updateNode(Real elapsedTime, Quaternion camOrientation)
{
	mp_node->setPosition(mp_object->getPosition());
	if(mp_object->getType() == ObjectType::PROJECTILE) {
		mp_node->setOrientation(camOrientation);
	} else {
		mp_node->setOrientation(mp_object->getOrientation());
	}
}

PhysicsObject * RenderObject::getObject() {
	return mp_object;
}

SceneNode * RenderObject::getRootNode() {
	return mp_node;
}


// ========================================================================
// RenderModel Implementation
// ========================================================================
RenderModel::RenderModel(GameArena& model, SceneManager * mgr) : m_model(model), m_renderList(),
	mp_mgr(mgr), m_entityIndex(0)
{
	m_model.addGameArenaListener(this);

	// Set up required textures
	Plane plane = Plane(Ogre::Vector3::UNIT_Z, 0);
	MeshManager::getSingleton().createPlane("projSprite", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		plane, 200, 200, 1, 1, true, 1, 1, 1, Ogre::Vector3::UNIT_Y);
}

void RenderModel::generateRenderObject(PhysicsObject * object)
{
	if(object->getType() == ObjectType::SHIP) {
		SceneNode * shipNode = mp_mgr->getRootSceneNode()->createChildSceneNode();
		std::stringstream oss;
		oss << "Ship" << m_entityIndex;
		Entity * shipEntity = mp_mgr->createEntity(oss.str(), "RZR-002.mesh");
		shipEntity->setCastShadows(true);
		shipNode->setScale(6, 6, 6);
		SceneNode * shipRotateNode = shipNode->createChildSceneNode();
		shipRotateNode->setDirection(Vector3(0, 0, 1));
		shipRotateNode->attachObject(shipEntity);

		// Add a spot light to the ship
		oss << "L1";
		Ogre::Light* spotLight = mp_mgr->createLight(oss.str());
		spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
		spotLight->setDiffuseColour(0.8, 0.8, 1.0);
		spotLight->setSpecularColour(0.2, 0.2, 1.0);
		spotLight->setDirection(0, -1, -3);
		spotLight->setPosition(Vector3(0, 30, 0));
		spotLight->setSpotlightRange(Ogre::Degree(20), Ogre::Degree(45));
		shipNode->attachObject(spotLight);
		
		// Add a point light aboive the ship
		oss << "L2";
		Ogre::Light* pointLight = mp_mgr->createLight(oss.str());
		pointLight->setType(Ogre::Light::LT_POINT);
		pointLight->setPosition(Ogre::Vector3(0, 30, 0));
		pointLight->setDiffuseColour(0.4, 0.1, 0.1);
		pointLight->setSpecularColour(0.4, 0.4, 0.4);
		shipNode->attachObject(pointLight);

		RenderObject renderShip = RenderObject(object, shipNode);
		m_renderList.push_back(renderShip);
		m_entityIndex++;
	} else if (object->getType() == ObjectType::PROJECTILE) {
		SceneNode * projNode = mp_mgr->getRootSceneNode()->createChildSceneNode();

		std::stringstream oss;
		oss << "Projectile" << m_entityIndex;
		Ogre::Entity* plasmaSprite = mp_mgr->createEntity(oss.str(), "projSprite");
		plasmaSprite->setMaterialName("Orewar/PlasmaSprite");
		plasmaSprite->setCastShadows(false);
		projNode->attachObject(plasmaSprite);

		// Dynamic spot lights on projectiles
		oss << "Light";
		Ogre::Light* pointLight = mp_mgr->createLight(oss.str());
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
			mp_mgr->destroySceneNode(m_renderList[i].getRootNode());
			m_renderList.erase(m_renderList.begin() + i);
		}
	}
}

void RenderModel::updateRenderList(Real elapsedTime, Quaternion camOrientation)
{
	for(int i = 0; i < m_renderList.size(); i++) {
		m_renderList[i].updateNode(elapsedTime, camOrientation);
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