// Local Includes
//---------------------------------------------------
#include <easylogging++.cc>
INITIALIZE_NULL_EASYLOGGINGPP

#include <sge/configmanager/ConfigManager.hpp>
#include <sge/display/DisplayManager.hpp>
#include <sge/scene/scene.hpp>
#include <sge/scene/sceneimporter.hpp>
#include <sge/graphics/ShaderManager.hpp>
#include <sge/graphics/OGLGraphicsManager.hpp>


using namespace SGE;

SGE::IDisplay* dm;
GraphicsManager::IGraphicsManager* gm;
Scene* scene;

bool quit = false;

bool main_loop()
{
	dm->handleEvents();
	if(dm->wasQuitRequested())
		return false;

	dm->setAsTarget();

	gm->clearBuffer();

	scene->update();

	scene->draw();

	return true;
}

void init()
{
	/* Initialise the display manager */
	dm = SGE::DisplayManager::getDisplayInstance();

	/* Initialise the graphics manager */
	gm = new GraphicsManager::OGLGraphicsManager(dm);

	/* Instantiate the scene object */
	SceneImporter sceneImporter;
	scene = sceneImporter.importSceneFromFile("resources/scenes/test_rt_scene.xml");

	ShaderManager::loadShader("normals");
	ShaderManager::loadShader("depth");
}

void exit()
{
	dm->exit();
}

int main( int argc, char* args[] )
{
	el::Helpers::setStorage(SGE::Utils::getELStorage());
	el::Configurations conf("resources/logger.conf");
	el::Loggers::reconfigureLogger("default", conf);
	el::Loggers::reconfigureAllLoggers(conf);

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = args[i];
		if (arg == "-c")
		{
			std::string configFile = args[i + 1];
			ConfigManager::setConfigFile(configFile);
		}
	}
    ConfigManager::init();

	init();
	while(main_loop());

	exit();

	return 0;
}
