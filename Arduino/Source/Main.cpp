#include "Audio/AudioDeviceUtils.hpp"
#include "Utility/Utility.hpp"
#include "Rendering/Rendering.hpp"
#include "Rendering/Interface.hpp"
#include "Threading/Threading.hpp"

int main()
{
    FreeConsole(); 
    g_Threading = new Threading(); 
    if (g_Threading)
    {
        g_Interface = new AudioMixerUI();
        VirtualCableManager* manager = new VirtualCableManager();
        manager->Initialize();
        g_Rendering = new Rendering();
        g_Interface->SetCableManager(manager);
        g_Rendering->Run();

        delete g_Rendering;
        delete manager;
        delete g_Interface;
    }
    delete g_Threading;
    return EXIT_SUCCESS;
}