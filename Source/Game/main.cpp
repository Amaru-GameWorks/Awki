#include <Awki.h>
#include <print>

int main(int /*argc*/, char** /*argv*/)
{
	std::shared_ptr<Awki> engine = nullptr;

	try
	{
		AkInstanceDescriptor descriptor =
		{
			.gameName = "Awesome Game",
			.gameVersion = {0, 0, 1},
			.windowDescriptor = {
				.name = descriptor.gameName,
				.flags = AkWindowFlag_RESIZABLE,
				.width = 1920,
				.height = 1080
			}
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