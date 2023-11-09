#include "VulkanEngine.h"

int main()
{
	VAP::VulkanEngine engine;

	engine.Init();
	engine.Run();
	engine.Cleanup();

	return 0;
}