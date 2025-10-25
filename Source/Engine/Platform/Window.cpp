#include "Window.h"
#include "Core/Log.h"
#include <SDL3/SDL.h>

AkWindow::AkWindow(const AkWindowDescriptor& descriptor)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) 
	{
		AkLogError("Couldn't initialize SDL: {}", SDL_GetError());
		throw std::exception("AkWindow could not initialize");
	}

	Uint32 flags = 0;
	flags |= SDL_WINDOW_VULKAN;
	if (descriptor.flags & AkWindowFlag_FULLSCREEN)			flags |= SDL_WINDOW_FULLSCREEN;
	if (descriptor.flags & AkWindowFlag_HIDDEN)				flags |= SDL_WINDOW_HIDDEN;
	if (descriptor.flags & AkWindowFlag_BORDERLESS)			flags |= SDL_WINDOW_BORDERLESS;
	if (descriptor.flags & AkWindowFlag_RESIZABLE)			flags |= SDL_WINDOW_RESIZABLE;
	if (descriptor.flags & AkWindowFlag_MINIMIZED)			flags |= SDL_WINDOW_MINIMIZED;
	if (descriptor.flags & AkWindowFlag_MAXIMIZED)			flags |= SDL_WINDOW_MAXIMIZED;
	if (descriptor.flags & AkWindowFlag_HIGH_PIXEL_DENSITY)	flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
	if (descriptor.flags & AkWindowFlag_ALWAYS_ON_TOP)		flags |= SDL_WINDOW_ALWAYS_ON_TOP;
	if (descriptor.flags & AkWindowFlag_UTILITY_WINDOW)		flags |= SDL_WINDOW_UTILITY;

	m_WindowHandle = SDL_CreateWindow(descriptor.name.data(), static_cast<int>(descriptor.width), static_cast<int>(descriptor.height), flags);
	if (m_WindowHandle == nullptr)
	{
		AkLogError("Failed to create SDL window: {}", SDL_GetError());
		throw std::exception("AkWindow could not create window");
	}
}

AkWindow::~AkWindow()
{
	if(m_WindowHandle)
		SDL_DestroyWindow(m_WindowHandle);
}
