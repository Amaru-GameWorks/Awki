#include "Assert.h"

#include <SDL3/SDL_messagebox.h>

void AkAssertMessageBox::ShowError(const std::string_view message)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Awki Assert", message.data(), nullptr);
}

void AkAssertMessageBox::ShowWarning(const std::string_view message)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Awki SoftAssert", message.data(), nullptr);
}