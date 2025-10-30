#include <Awki.h>

int main(int /*argc*/, char** /*argv*/)
{
	AkInstanceDescriptor descriptor =
	{
		.name = "New Game",
		.version = {0, 1, 0},
		.window = 
		{
			.name = descriptor.name,
			.flags = AkWindowFlag_RESIZABLE,
			.width = 1920,
			.height = 1080,
		}
	};

	Awki engine;
	if (engine.Initialize(descriptor))
		engine.Run();

	engine.Deinitialize();
	return 0;
}