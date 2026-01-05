#include <Awki.h>
#include <print>

int main()
{
	std::shared_ptr<Awki> engine = nullptr;

	try
	{
		AkInstanceDescriptor descriptor =
		{
			.gameName = "Awesome Game",
			.gameVersion = {0, 0, 1}
		};

		engine = std::make_shared<Awki>(descriptor);
	}
	catch (const std::exception& exception)
	{
		std::println("Failed to initialize Awki engine: {}", exception.what());
		return EXIT_FAILURE;
	}

	engine->Run();
	return EXIT_SUCCESS;
}