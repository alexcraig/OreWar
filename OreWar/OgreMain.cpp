#include <Ogre.h>
#include <OIS.h>
#include <OgreMath.h>
#include <sstream>
#include <vector>
#include "PhysicsEngine.h"
#include "GameObjects.h"
#include "RenderModel.h"
#include "OgreTextAreaOverlayElement.h"
#include "OgreFontManager.h"
#include "Gorilla.h"
 
using namespace Ogre;
 
class KeyboardTestListener : public FrameListener
{
public:
	KeyboardTestListener(OIS::Keyboard *keyboard, SceneManager *mgr, Camera *cam, RenderWindow * renderWindow)
        : m_Keyboard(keyboard), m_rotateNode(mgr->getRootSceneNode()->createChildSceneNode()), m_cam(cam), 
		m_camHeight(0), m_camOffset(0), m_arena(5000), m_mgr(mgr),
		m_thirdPersonCam(true), m_renderModel(m_arena, m_mgr), mp_vp(cam->getViewport()), mp_fps(NULL), m_timer(0),
		mp_renderWindow(renderWindow)
	{
		m_cam->setFarClipDistance(0);

		// Generate the keyboard testing entity and attach it to the listener's scene node
		PlayerShip playerShip = PlayerShip(1, Vector3(0, -2000, 0));
		PlayerShip * p_playerShip = m_arena.addShip(playerShip);


		// Generate GUI elements
		Gorilla::Silverback * mGorilla = new Gorilla::Silverback();
		mGorilla->loadAtlas("dejavu");
		Gorilla::Screen * mScreen = mGorilla->createScreen(mp_vp, "dejavu");
		Gorilla::Layer * layer = mScreen->createLayer(10);

		layer->createCaption(14, 5, 5, "OreWar Beta v0.01");
		Gorilla::Rectangle * crosshair = layer->createRectangle(Vector2(mp_vp->getActualWidth() / 2.0f - 6, mp_vp->getActualHeight() / 2.0f - 6),
			Vector2(12, 12));
		crosshair->background_image("crosshair");

		mp_fps = layer->createCaption(14, 5, mp_vp->getActualHeight() - 24, "FPS Counter");
    }
 
    bool frameStarted(const FrameEvent& evt)
    {
		// Capture the keyboard input
        m_Keyboard->capture();
		PlayerShip * playerShip = m_arena.getShips()->front();

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
			playerShip->pitch(Radian(2 * evt.timeSinceLastFrame));
		}
		if(m_Keyboard->isKeyDown(OIS::KC_S)) {
			playerShip->pitch(Radian(-2 * evt.timeSinceLastFrame));
		}
		if(m_Keyboard->isKeyDown(OIS::KC_A)) {
			playerShip->yaw(Radian(2 * evt.timeSinceLastFrame));
		}
		if(m_Keyboard->isKeyDown(OIS::KC_D)) {
			playerShip->yaw(Radian(-2 * evt.timeSinceLastFrame));
		}
		if(m_Keyboard->isKeyDown(OIS::KC_Q)) {
			playerShip->roll(Radian(2 * evt.timeSinceLastFrame));
		}
		if(m_Keyboard->isKeyDown(OIS::KC_E)) {
			playerShip->roll(Radian(-2 * evt.timeSinceLastFrame));
		}

		if(m_Keyboard->isKeyDown(OIS::KC_LCONTROL)) {
			playerShip->applyForce(playerShip->getVelocity().normalisedCopy() * (-1) * Vector3(1000, 1000, 1000));
		}

		if(m_Keyboard->isKeyDown(OIS::KC_UP)) {
			playerShip->applyForce(playerShip->getHeading() * Vector3(1000, 1000, 1000));
		}
		if(m_Keyboard->isKeyDown(OIS::KC_DOWN)) {
			playerShip->applyForce(playerShip->getHeading() * Vector3(-1000, -1000, -1000));
		}

		if(m_Keyboard->isKeyDown(OIS::KC_C)) {
			m_thirdPersonCam = !m_thirdPersonCam; 
		}

		// Update FPS counter
		m_timer += evt.timeSinceLastFrame;
		if (m_timer > 1.0f / 60.0f)
		{
			m_timer = 0;
			mp_fps->text("FPS: " + Ogre::StringConverter::toString(mp_renderWindow->getLastFPS()));
		}
		

		// Generate projectile if required
		if(m_Keyboard->isKeyDown(OIS::KC_SPACE)) {
			if(playerShip->canShoot()) {
				m_arena.fireProjectileFromShip(playerShip);
			}
		}

		// Update the position of the physics object and move the scene node
		m_arena.updatePhysics(evt.timeSinceLastFrame);
		m_renderModel.updateRenderList(evt.timeSinceLastFrame, m_cam->getOrientation());

		// Move the camera
		if(m_thirdPersonCam) {
			m_cam->setPosition(playerShip->getPosition() + Vector3(0, 1000, 1000));
			m_cam->lookAt(playerShip->getPosition());
		} else {
			m_cam->setPosition(playerShip->getPosition() + playerShip->getNormal() * 60 - playerShip->getHeading() * 160);
			m_cam->setOrientation(playerShip->getOrientation());
		}

        return !m_Keyboard->isKeyDown(OIS::KC_ESCAPE);
    }
 
private:
    OIS::Keyboard *m_Keyboard;
	SceneNode *m_rotateNode;
	Camera *m_cam;
	int m_camHeight;
	int m_camOffset;
	GameArena m_arena;
	bool m_thirdPersonCam;

	SceneManager * m_mgr;
	RenderModel m_renderModel;
	Viewport * mp_vp;
	Gorilla::Caption * mp_fps;
	Real m_timer;
	RenderWindow * mp_renderWindow;
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
		cam->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));

		// Setup ambient light and shadows
		mgr->setAmbientLight(Ogre::ColourValue(0.15, 0.15, 0.3));
		mgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_MODULATIVE);

		// Add planes for the arena boundaries
		Plane plane(Ogre::Vector3::UNIT_Y, 0);
		MeshManager::getSingleton().createPlane("starwall", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        plane, 10000, 10000, 100, 100, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);

		Ogre::Entity* wallEnt = mgr->createEntity("BottomWall", "starwall");
		SceneNode * wallNode = mgr->getRootSceneNode()->createChildSceneNode();
		wallNode->attachObject(wallEnt);
		wallNode->setPosition(Vector3(0, -5000, 0));
		wallEnt->setMaterialName("Orewar/Starfield");
		wallEnt->setCastShadows(false);

		wallEnt = mgr->createEntity("TopWall", "starwall");
		wallNode = mgr->getRootSceneNode()->createChildSceneNode();
		wallNode->attachObject(wallEnt);
		wallNode->setPosition(Vector3(0, 5000, 0));
		wallNode->setOrientation(Quaternion(Degree(180), Vector3::UNIT_X));
		wallEnt->setMaterialName("Orewar/Starfield");
		wallEnt->setCastShadows(false);

		wallEnt = mgr->createEntity("LeftWall", "starwall");
		wallNode = mgr->getRootSceneNode()->createChildSceneNode();
		wallNode->attachObject(wallEnt);
		wallNode->setPosition(Vector3(-5000, 0, 0));
		wallNode->setOrientation(Quaternion(Degree(-90), Vector3::UNIT_Z));
		wallEnt->setMaterialName("Orewar/Starfield");
		wallEnt->setCastShadows(false);

		wallEnt = mgr->createEntity("RightWall", "starwall");
		wallNode = mgr->getRootSceneNode()->createChildSceneNode();
		wallNode->attachObject(wallEnt);
		wallNode->setOrientation(Quaternion(Degree(90), Vector3::UNIT_Z));
		wallNode->setPosition(Vector3(5000, 0, 0));
		wallEnt->setMaterialName("Orewar/Starfield");
		wallEnt->setCastShadows(false);

		wallEnt = mgr->createEntity("FrontWall", "starwall");
		wallNode = mgr->getRootSceneNode()->createChildSceneNode();
		wallNode->attachObject(wallEnt);
		wallNode->setPosition(Vector3(0, 0, 5000));
		wallNode->setOrientation(Quaternion(Degree(-90), Vector3::UNIT_X));
		wallEnt->setMaterialName("Orewar/Starfield");
		wallEnt->setCastShadows(false);

		wallEnt = mgr->createEntity("BackWall", "starwall");
		wallNode = mgr->getRootSceneNode()->createChildSceneNode();
		wallNode->attachObject(wallEnt);
		wallNode->setOrientation(Quaternion(Degree(90), Vector3::UNIT_X));
		wallNode->setPosition(Vector3(0, 0, -5000));
		wallEnt->setMaterialName("Orewar/Starfield");
		wallEnt->setCastShadows(false);


		// Put a giant ship in the middle of the arena
		SceneNode * shipNode = mgr->getRootSceneNode()->createChildSceneNode();
		Entity * shipEntity = mgr->createEntity("GiantShip", "RZR-002.mesh");
		shipEntity->setCastShadows(true);
		shipNode->attachObject(shipEntity);
		shipNode->setScale(100, 100, 100);

		// Add a skybox
		// mgr->setSkyBox(true, "Orewar/SpaceSkyBox", 20000, false);
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
			mRoot->getSceneManager("Default SceneManager")->getCamera("Camera"),
			mRoot->getAutoCreatedWindow());
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