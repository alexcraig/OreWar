#include <Ogre.h>
#include <OIS.h>
#include <OgreMath.h>
#include <sstream>
#include <vector>
#include "GameObjects.h"
#include "RenderModel.h"
 
using namespace Ogre;
 
class KeyboardTestListener : public FrameListener
{
public:
	KeyboardTestListener(OIS::Keyboard *keyboard, SceneManager *mgr, Camera *cam)
        : m_Keyboard(keyboard), m_rotateNode(mgr->getRootSceneNode()->createChildSceneNode()), m_cam(cam), 
		m_camHeight(0), m_camOffset(0), m_rotationFactor(0), m_arena(3000), m_entityIndex(0), m_renderList(), m_mgr(mgr)
	{
		// Generate the keyboard testing entity and attach it to the listener's scene node
		PhysicsObject playerShip = PhysicsObject(1, Vector3(0, 200, 0));
		SceneNode * shipNode = mgr->getRootSceneNode()->createChildSceneNode();
		Ogre::Entity * shipEntity = mgr->createEntity("TestShip", "RZR-002.mesh");
		shipEntity->setCastShadows(true);
		shipNode->attachObject(shipEntity);
		shipNode->setScale(4, 4, 4);
		m_arena.addShip(playerShip);
		RenderObject renderShip = RenderObject(&(*m_arena.getShips()).front(), shipNode);
		m_renderList.push_back(renderShip);

		// Add another ship and point light which will circle the keyboard entity
		Ogre::Entity * circleShipEntity = mgr->createEntity("CirclingShip", "RZR-002.mesh");
		circleShipEntity->setCastShadows(true);
		m_rotateNode->attachObject(circleShipEntity);

		Ogre::Light* pointLight = mgr->createLight("pointLightWhite");
		pointLight->setType(Ogre::Light::LT_POINT);
		pointLight->setPosition(Ogre::Vector3(0, 60, 0));
		pointLight->setDiffuseColour(1.0, 1.0, 1.0);
		pointLight->setSpecularColour(1.0, 1.0, 1.0);
		m_rotateNode->attachObject(pointLight);
		m_rotateNode->setScale(3, 3, 3);
    }
 
    bool frameStarted(const FrameEvent& evt)
    {
		// Capture the keyboard input
        m_Keyboard->capture();
		PhysicsObject * playerShip = &(*m_arena.getShips()).front();

		// Adjust or reset the camera modifiers
		if(m_Keyboard->isKeyDown(OIS::KC_Z)) {
			m_camHeight = 0;
			m_camOffset = 0;
		}
		if(m_Keyboard->isKeyDown(OIS::KC_UP)) {
			m_camHeight += 2;
		}
		if(m_Keyboard->isKeyDown(OIS::KC_DOWN)) {
			m_camHeight -= 2;
		}
		if(m_Keyboard->isKeyDown(OIS::KC_RIGHT)) {
			m_camOffset += 2;
		}
		if(m_Keyboard->isKeyDown(OIS::KC_LEFT)) {
			m_camOffset -= 2;
		}
		
		// Clear all existing forces, and add keyboard forces
		playerShip->clearForces();
		if(m_Keyboard->isKeyDown(OIS::KC_W)) {
			playerShip->applyForce(playerShip->getHeading() * Vector3(1000, 1000, 1000));
		}
		if(m_Keyboard->isKeyDown(OIS::KC_S)) {
			playerShip->applyForce(playerShip->getHeading() * Vector3(-1000, -1000, -1000));
		}
		if(m_Keyboard->isKeyDown(OIS::KC_A)) {
			playerShip->rotateHeadingAboutY(Math::DegreesToRadians(-150 * evt.timeSinceLastFrame));
		}
		if(m_Keyboard->isKeyDown(OIS::KC_D)) {
			playerShip->rotateHeadingAboutY(Math::DegreesToRadians(150 * evt.timeSinceLastFrame));
		}

		if(m_Keyboard->isKeyDown(OIS::KC_Q)) {
			playerShip->applyForce(Vector3(0, 1000, 0));
		}
		if(m_Keyboard->isKeyDown(OIS::KC_E)) {
			playerShip->applyForce(Vector3(0, -1000, 0));
		}

		// Generate projectile if required
		if(m_Keyboard->isKeyDown(OIS::KC_SPACE)) {
			m_arena.fireProjectileFromShip(*playerShip);
			std::vector<PhysicsObject> * projectiles = m_arena.getProjectiles();

			SceneNode * projNode = m_mgr->getRootSceneNode()->createChildSceneNode();
			std::stringstream oss;
			oss << "Projectile" << m_entityIndex;
			std::cout << oss.str();

			Ogre::Entity * projEntity = m_mgr->createEntity(oss.str(), "RZR-002.mesh");
			projEntity->setCastShadows(false);
			projNode->attachObject(projEntity);

			oss << "Light";
			Ogre::Light* spotLight = m_mgr->createLight(oss.str());
			spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
			spotLight->setDiffuseColour(1.0, 0.0, 0.0);
			spotLight->setSpecularColour(1.0, 5.0, 5.0);
			spotLight->setDirection(0, -1, 0);
			spotLight->setPosition(Vector3(0, 0, 0));
			spotLight->setSpotlightRange(Ogre::Degree(15), Ogre::Degree(35));
			projNode->attachObject(spotLight);

			RenderObject newProj = RenderObject(& projectiles->back(), projNode);
			m_renderList.push_back(newProj);

			m_entityIndex++;
		}

		// Update the position of the physics object and move the scene node
		m_arena.updatePhysics(evt.timeSinceLastFrame);
		for(int i = 0; i < m_renderList.size(); i++) {
			m_renderList[i].updateNode();
		}

		// Update the position of the circling ball
		m_rotationFactor = (m_rotationFactor + 1) % 360;
		m_rotateNode->setPosition( playerShip->getPosition() + 
			Vector3( Math::Sin( Math::DegreesToRadians( m_rotationFactor ) ) * 300,
				Math::Sin( Math::DegreesToRadians( m_rotationFactor) ) * 100,
				Math::Cos( Math::DegreesToRadians( m_rotationFactor) ) * 300 ));
		m_rotateNode->lookAt(playerShip->getPosition(), Node::TS_WORLD, Vector3::UNIT_Z );

		// Move the camera
		m_cam->setPosition(playerShip->getPosition() + Vector3(0, 1500 + m_camHeight, 1000 + m_camOffset));
		m_cam->lookAt(playerShip->getPosition());

        return !m_Keyboard->isKeyDown(OIS::KC_ESCAPE);
    }
 
private:
    OIS::Keyboard *m_Keyboard;
	SceneNode *m_rotateNode;
	Camera *m_cam;
	int m_camHeight;
	int m_camOffset;
	int m_rotationFactor;
	GameArena m_arena;
	int m_entityIndex;

	// Note: All references stored by the RenderObjects will becomes
	// invalid when the arenas vectors resize. Need to implement a 
	// much, much better way to do this.
	std::vector<RenderObject> m_renderList;
	SceneManager * m_mgr;
};
 
class Application
{
public:
    void go()
    {
        createRoot();
        defineResources();
        setupRenderSystem();
        createRenderWindow();
        initializeResourceGroups();
        setupScene();
        setupInputSystem();
        createFrameListener();
        startRenderLoop();
    }
 
    ~Application()
    {
		mInputManager->destroyInputObject(mKeyboard);
		OIS::InputManager::destroyInputSystem(mInputManager);
		delete mListener;
        delete mRoot;
    }
 
private:
    Root *mRoot;
	SceneManager* mMgr;
    OIS::Keyboard *mKeyboard;
    OIS::InputManager *mInputManager;
    KeyboardTestListener *mListener;
 
    void createRoot()
    {
		// Generate the OGRE Root object
		#ifdef _DEBUG
			mRoot = new Root("plugins_d.cfg");
		#else
			mRoot = new Root("plugins.cfg");
		#endif
    }
 
    void defineResources()
    {
		String secName, typeName, archName;
        
		// Load the resources.cfg resource definition file
		ConfigFile cf;

		#ifdef _DEBUG
			cf.load("resources_d.cfg");
		#else
			cf.load("resources.cfg");
		#endif

		// Loop through all the provided resource folders and add them
		// to the ResourceGroupManager
		ConfigFile::SectionIterator seci = cf.getSectionIterator();
        while (seci.hasMoreElements())	{
			// Loop through the provided sections
			secName = seci.peekNextKey();
            ConfigFile::SettingsMultiMap *settings = seci.getNext();
            ConfigFile::SettingsMultiMap::iterator i;
	
			for (i = settings->begin(); i != settings->end(); ++i) {
				// Add each provided directory path to the GroupResourceManager
				typeName = i->first;
				archName = i->second;
				ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
            }
        }
    }
 
    void setupRenderSystem()
    {
		// Restore the last configuration settings, or provide a dialogue if no prior
		// settings are stored
		//if (!mRoot->restoreConfig() && !mRoot->showConfigDialog()) // Only show config if no existing config is found
		if (!mRoot->showConfigDialog()) // Always show config
			throw Exception(52, "User canceled the config dialog!", "Application::setupRenderSystem()");
    }
 
    void createRenderWindow()
    {
		// Create the render window (first parameter determines whether OGRE should generate
		// the render window)
		mRoot->initialise(true, "Ore War");
    }
 
    void initializeResourceGroups()
    {
		// Mipmaps setting must be set before resource groups are initialized
		TextureManager::getSingleton().setDefaultNumMipmaps(5);
        ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    }
 
    void setupScene()
    {
		// Create the SceneManager
		SceneManager *mgr = mRoot->createSceneManager(ST_GENERIC, "Default SceneManager");
        
		// Create a camera
		Camera *cam = mgr->createCamera("Camera");
		cam->setPosition(Ogre::Vector3(0,0,1000));
		cam->lookAt(Ogre::Vector3(0,0,0));

		// Create a viewport
        Viewport *vp = mRoot->getAutoCreatedWindow()->addViewport(cam);

		// Setup ambient light and shadows
		mgr->setAmbientLight(Ogre::ColourValue(0, 0, 0));
		mgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

		// Add a plane for the ground
		Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
		Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        plane, 6000, 6000, 100, 100, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
		Ogre::Entity* entGround = mgr->createEntity("GroundEntity", "ground");
		SceneNode * groundNode = mgr->getRootSceneNode()->createChildSceneNode();
		groundNode->attachObject(entGround);
		groundNode->setPosition(Vector3(0, -200, 0));
		entGround->setMaterialName("Orewar/Starfield");
		entGround->setCastShadows(false);

		// Add a skybox
		mgr->setSkyBox(true, "Orewar/SpaceSkyBox", 5000, false);
    }
 
    void setupInputSystem()
    {
		// Initialize OIS
		size_t windowHnd = 0;
        std::ostringstream windowHndStr;
        OIS::ParamList pl;
        RenderWindow *win = mRoot->getAutoCreatedWindow();
 
        win->getCustomAttribute("WINDOW", &windowHnd);
        windowHndStr << windowHnd;
        pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
        mInputManager = OIS::InputManager::createInputSystem(pl);


		// Setup (Unbuffered in this case) input objects
		try {
            mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, false));
            //mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, false));
            //mJoy = static_cast<OIS::JoyStick*>(mInputManager->createInputObject(OIS::OISJoyStick, false));
        }
        catch (const OIS::Exception &e) {
            throw Exception(42, e.eText, "Application::setupInputSystem");
        }
    }
 
    void createFrameListener()
    {
		// Create and add a listener for the keyboard
		// Note: Input devices can have only one listener
		mListener = new KeyboardTestListener(mKeyboard, mRoot->getSceneManager("Default SceneManager"), 
			mRoot->getSceneManager("Default SceneManager")->getCamera("Camera"));
        mRoot->addFrameListener(mListener);
    }
 
    void startRenderLoop()
    {
		mRoot->startRendering();
    }
};

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
		int main(int argc, char *argv[])
#endif
		{
			// Create application object
			Application app;

			try {
				app.go();
			} catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
				MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
				std::cerr << "An exception has occured: " <<
					e.getFullDescription().c_str() << std::endl;
#endif
			}

			return 0;
		}

#ifdef __cplusplus
	}
#endif