#include <Awki.h>

int main()
{
	AkInstanceDescriptor descriptor =
	{
		.name = "New Game",
		.version = {0, 1, 0}
	};

	Awki engine;
	if (engine.Initialize(descriptor))
		engine.Run();

	engine.Deinitialize();
	return 0;
}