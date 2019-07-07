#include "include/SponzaDemo.h"

VkGen::VkGenerator g_VkGenerator(1280, 720);
Logger             g_Logger;

int main()
{
#ifdef _DEBUG
	const HANDLE cmd_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	g_Logger.Create(cmd_handle);
	g_Logger.Info("Logger Created");
	g_VkGenerator.RequireValidation(true);
	g_VkGenerator.AddValidationLayerCallback(VkSponzaDemo::DebugCallback);
#endif

	g_VkGenerator.LogStateOnInitisation(true);
	g_VkGenerator.LogDeviceInfo(true);

	g_VkGenerator.Init();

	VkSponzaDemo cube_demo;
	cube_demo.SetShaderDirectory("../assets/shaders/");
	cube_demo.SetModelDirectory("../assets/models/");
	cube_demo.SetTextureDirectory("../assets/textures/");
	cube_demo.Setup();
	cube_demo.Run();
	cube_demo.Shutdown();

	g_VkGenerator.Destroy();

	return 0;
}
