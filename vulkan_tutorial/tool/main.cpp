#include "vulkan_tutorial.h"

int main(int argc, char* argv[])
{
	using VulkanTutorial::TutorialEngine;

	TutorialEngine engine(1200, 800);

	engine.setup();

	engine.run();

	engine.cleanup();

	return 0;
};