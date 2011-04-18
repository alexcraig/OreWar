#include <Ogre.h>
#include <OIS.h>
#include "GameObjects.h"
 
using namespace Ogre;
 
class KeyboardTestListener : public FrameListener
{
public:
	KeyboardTestListener(OIS::Keyboard *keyboard, SceneManager *mgr, Camera *cam)
        : m_Keyboard(keyboard), m_testObject(20), m_testNode(mgr->getRootSceneNode()->createChildSceneNode()),
		m_cam(cam)
	{
		// Generate the keyboard testing entity and attach it to the listener's scene node
		Ogre::Entity* testEntity = mgr->createEntity("KeyboardListenerTest", "ogrehead.mesh");
		testEntity->setCastShadows(true);
		m_testNode->attachObject(testEntity);
    }
 
    bool frameStarted(const FrameEvent& evt)
    {
		// Capture the keyboard input
        m_Keyboard->capture();
		
		// Clear all existing forces, and add keyboard forces
		m_testObject.clearForces();
		if(m_Keyboard->isKeyDown(OIS::KC_W)) {
			m_testObject.applyForce(0, 40);
		}
		if(m_Keyboard->isKeyDown(OIS::KC_S)) {
			m_testObject.applyForce(0, -40);
		}
		if(m_Keyboard->isKeyDown(OIS::KC_A)) {
			m_testObject.applyForce(-40, 0);
		}
		if(m_Keyboard->isKeyDown(OIS::KC_D)) {
			m_testObject.applyForce(40, 0);
		}

		// Update the position of the physics object and move the scene node
		m_testObject.updatePosition(evt.timeSinceLastFrame);
		m_testNode->setPosition(m_testObject.getXPos(), 100, -m_testObject.getYPos());
		m_testNode->lookAt( Vector3(0, 100, 0) , Node::TS_WORLD, Vector3::UNIT_Z );

		// Move the camera
		m_cam->setPosition(m_testObject.getXPos(), 250,  -m_testObject.getYPos() + 500);
		m_cam->lookAt(m_testObject.getXPos(), 100, -m_testObject.getYPos());

        return !m_Keyboard->isKeyDown(OIS::KC_ESCAPE);
    }
 
private:
    OIS::Keyboard *m_Keyboard;
	PhysicsObject m_testObject;
	SceneNode *m_testNode;
	Camera *m_cam;
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

		// Add  point lights
		Ogre::Light* pointLight = mgr->createLight("pointLightRed");
		pointLight->setType(Ogre::Light::LT_POINT);
		pointLight->setPosition(Ogre::Vector3(350, 250, 350));
		pointLight->setDiffuseColour(1.0, 0.0, 0.0);
		pointLight->setSpecularColour(1.0, 0.0, 0.0);
		pointLight = mgr->createLight("pointLightBlue");
		pointLight->setType(Ogre::Light::LT_POINT);
		pointLight->setPosition(Ogre::Vector3(-350, 250, 350));
		pointLight->setDiffuseColour(0.0, 0.0, 1.0);
		pointLight->setSpecularColour(0.0, 0.0, 1.0);
		pointLight = mgr->createLight("pointLightGreen");
		pointLight->setType(Ogre::Light::LT_POINT);
		pointLight->setPosition(Ogre::Vector3(0, 250, -495));
		pointLight->setDiffuseColour(0.0, 1.0, 0.0);
		pointLight->setSpecularColour(0.0, 1.0, 0.0);

		// Add a spot light
		Ogre::Light* spotLight = mgr->createLight("spotLight");
		spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
		spotLight->setDiffuseColour(1.0, 1.0, 1.0);
		spotLight->setSpecularColour(1.0, 1.0, 1.0);
		spotLight->setDirection(0, -1, 0);
		spotLight->setPosition(Vector3(0, 500, 0));
		spotLight->setSpotlightRange(Ogre::Degree(15), Ogre::Degree(35));

		// Add a plane for the ground
		Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
		Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        plane, 6000, 6000, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
		Ogre::Entity* entGround = mgr->createEntity("GroundEntity", "ground");
		mgr->getRootSceneNode()->createChildSceneNode()->attachObject(entGround);
		entGround->setMaterialName("Examples/Rockwall");
		entGround->setCastShadows(false);

		// Add a skybox
		// mgr->setSkyBox(true, "Examples/SpaceSkyBox", 5000, false);

		// Create a ninja
		Ogre::Entity* entNinja = mgr->createEntity("Ninja", "ninja.mesh");
		entNinja->setCastShadows(true);
		SceneNode* ninjaNode = mgr->getRootSceneNode()->createChildSceneNode();
		ninjaNode->yaw( Ogre::Degree(180) );
		ninjaNode->attachObject(entNinja);
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