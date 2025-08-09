#include "Audio/AudioDeviceUtils.hpp"
#include "Rendering/Rendering.hpp"
#include "Interface/Interface.hpp"
#include "Utility/Utility.hpp"
#include "Config/Configuration.hpp"

int main()
{
	FreeConsole(); 

	try
	{
		g_VirtualCableManager = new VirtualCableManager(); 
		if (FAILED(g_VirtualCableManager->Initialize()))
		{
			delete g_VirtualCableManager; 
			return EXIT_FAILURE; 
		}

		g_Configuration = new Configuration(); 
		g_Interface = new AudioMixerUI(); 
		g_Rendering = new Rendering(); 
		g_Rendering->Run(); 
		delete g_Rendering; 
		delete g_Interface;
		delete g_Configuration;
		delete g_VirtualCableManager;
	}
	catch (const std::exception ex)
	{
		std::cout << ex.what() << std::endl; 
	}

	return EXIT_SUCCESS;
}