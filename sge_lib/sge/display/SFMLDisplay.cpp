#include "SFMLDisplay.hpp"

namespace SGE
{
	SFMLDisplay::SFMLDisplay(int w, int h, int x, int y, bool fullscreen)
	{
		LOG(INFO) << "Initiliasing...";

		//Create window
		sf::ContextSettings settings;
		settings.depthBits = 24;
		settings.stencilBits = 8;


		window = new sf::RenderWindow(sf::VideoMode(w, h), "Window", fullscreen ? sf::Style::Fullscreen : sf::Style::Close, settings);
		if( window == NULL )
			LOG(FATAL) << "Window could not be created";

		//window->setMouseCursorVisible(false);
		//window->setMouseCursorGrabbed(true);
		hasCursor = true;

		mWindowSize.width = w;
		mWindowSize.height = h;
		quit = false;
	}

	void SFMLDisplay::toggleGrabCursor()
	{
		hasCursor = !hasCursor;
		//window->setMouseCursorVisible(!hasCursor);
		//window->setMouseCursorGrabbed(hasCursor);
	}

	void SFMLDisplay::setAsTarget()
	{
	}

	void SFMLDisplay::handleEvents()
	{
		sf::Event e;
		while(window->pollEvent(e))
		{
			if(e.type == sf::Event::Closed)
			{
				quit = true;
			}
		}
	}

	void SFMLDisplay::swapBuffers()
	{
		window->display();
	}

	void SFMLDisplay::exit()
	{
		LOG(INFO) << "Quitting";
		window->close();
		delete window;
	}

	bool SFMLDisplay::wasQuitRequested()
	{
		return this->quit;
	}

	// Callback functions
	// ----------------------------------------------------------------
	void SFMLDisplay::onResize(void (*func)(Size))
	{
	}
}
