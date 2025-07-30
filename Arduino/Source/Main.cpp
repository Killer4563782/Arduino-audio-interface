#include "Audio/AudioDeviceUtils.hpp"
#include "Rendering/Rendering.hpp"
#include "Rendering/Interface.hpp"
#include "Utility/Utility.hpp"

int main()
{
	FreeConsole(); 

	try
	{
		g_Interface = new AudioMixerUI(); 
		g_Rendering = new Rendering(); 
		g_VirtualCableManager = new VirtualCableManager(); 
		g_VirtualCableManager->Initialize(); 
		g_Rendering->Run(); 
		delete g_VirtualCableManager;
		delete g_Rendering; 
		delete g_Interface;
	}
	catch (const std::exception ex)
	{
		std::cout << ex.what() << std::endl; 
	}

	return EXIT_SUCCESS;
}