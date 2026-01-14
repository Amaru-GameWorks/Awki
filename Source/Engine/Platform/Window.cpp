#include "Window.h"
#include "Core/Log.h"

#include <SDL3/SDL.h>

AkWindow::AkWindow(const AkWindowDescriptor& descriptor)
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		AkLogError("Couldn't initialize SDL: {}", SDL_GetError());
		throw std::runtime_error("AkWindow could not initialize");
	}

	Uint32 flags = SDL_WINDOW_VULKAN;
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
		throw std::runtime_error("AkWindow could be created!");
	}
}

AkWindow::~AkWindow()
{
	if (m_WindowHandle)
		SDL_DestroyWindow(m_WindowHandle);
}

void AkWindow::SetTitle(std::string_view title)
{
	if (!SDL_SetWindowTitle(m_WindowHandle, title.data()))
		AkLogError("Failed to set window title: {}", SDL_GetError());
}

void AkWindow::SetSize(const glm::uvec2& size)
{

	if (!SDL_SetWindowSize(m_WindowHandle, static_cast<int>(size.x), static_cast<int>(size.y)))
		AkLogError("Failed to set window size: {}", SDL_GetError());
}

glm::uvec2 AkWindow::GetSize()
{
	int width = 1, height = 1;
	if (!SDL_GetWindowSizeInPixels(m_WindowHandle, &width, &height))
		AkLogError("Failed to get window size: {}", SDL_GetError());

	return { width, height };
}

void AkWindow::SetPosition(const glm::uvec2& position)
{
	if (!SDL_SetWindowPosition(m_WindowHandle, static_cast<int>(position.x), static_cast<int>(position.y)))
		AkLogError("Failed to set window position: {}", SDL_GetError());
}

glm::uvec2 AkWindow::GetPosition()
{
	int x = 0, y = 0;
	if (!SDL_GetWindowPosition(m_WindowHandle, &x, &y))
		AkLogError("Failed to get window position: {}", SDL_GetError());

	return { x, y };
}

void AkWindow::SetBorderlessFullScreen(bool state)
{
	if (!SDL_SetWindowFullscreenMode(m_WindowHandle, nullptr))
	{
		AkLogError("Failed to reset fullscreen display mode: {}", SDL_GetError());
		return;
	}

	if (!SDL_SetWindowFullscreen(m_WindowHandle, state))
		AkLogError("Failed to change window screen mode: {}", SDL_GetError());
}

void AkWindow::SetExclusiveFullscreen(const AkDisplayMode& displayMode)
{
	SDL_DisplayMode modeToUse = {};
	if (!SDL_GetClosestFullscreenDisplayMode(SDL_GetPrimaryDisplay(), static_cast<int>(displayMode.width), static_cast<int>(displayMode.height), displayMode.refreshRate, true, &modeToUse))
	{
		AkLogError("Failed to match display mode to available display modes: {}", SDL_GetError());
		return;
	}

	if (!SDL_SetWindowFullscreenMode(m_WindowHandle, &modeToUse))
	{
		AkLogError("Failed to set fullscreen display mode: {}", SDL_GetError());
		return;
	}

	if (!SDL_SetWindowFullscreen(m_WindowHandle, true))
	{
		AkLogError("Failed to change window screen mode: {}", SDL_GetError());
		return;
	}

	SDL_SyncWindow(m_WindowHandle);
}

void AkWindow::SetBorderless(bool state)
{
	if (!SDL_SetWindowBordered(m_WindowHandle, !state))
		AkLogError("Failed to set window borderless: {}", SDL_GetError());
}

void AkWindow::SetResizable(bool state)
{
	if (!SDL_SetWindowResizable(m_WindowHandle, state))
		AkLogError("Failed to set window resizable: {}", SDL_GetError());
}

void AkWindow::SetAlwaysOnTop(bool state)
{
	if (!SDL_SetWindowAlwaysOnTop(m_WindowHandle, state))
		AkLogError("Failed to set window always on top: {}", SDL_GetError());
}

void AkWindow::SetHidden(bool state)
{
	if (state)
	{
		if (!SDL_HideWindow(m_WindowHandle))
			AkLogError("Failed to hide window: {}", SDL_GetError());
	}
	else
	{
		if (!SDL_ShowWindow(m_WindowHandle))
			AkLogError("Failed to show window: {}", SDL_GetError());
	}
}

void AkWindow::Maximize()
{
	if (!SDL_MaximizeWindow(m_WindowHandle))
		AkLogError("Failed to maximize window: {}", SDL_GetError());
}

bool AkWindow::IsMaximized() const
{
	return SDL_GetWindowFlags(m_WindowHandle) & SDL_WINDOW_MAXIMIZED;
}

void AkWindow::Minimize()
{
	if (!SDL_MinimizeWindow(m_WindowHandle))
		AkLogError("Failed to minimize window: {}", SDL_GetError());
}

bool AkWindow::IsMinimized() const
{
	return SDL_GetWindowFlags(m_WindowHandle) & SDL_WINDOW_MINIMIZED;
}

void AkWindow::Focus()
{
	if (!SDL_RaiseWindow(m_WindowHandle))
		AkLogError("Failed to set window focus: {}", SDL_GetError());
}

bool AkWindow::HasFocus() const
{
	return HasInputFocus() || HasMouseFocus();
}

bool AkWindow::HasInputFocus() const
{
	return SDL_GetWindowFlags(m_WindowHandle) & SDL_WINDOW_INPUT_FOCUS;
}

bool AkWindow::HasMouseFocus() const
{
	return SDL_GetWindowFlags(m_WindowHandle) & SDL_WINDOW_MOUSE_FOCUS;
}

std::vector<AkDisplayMode> AkWindow::GetAvailableDisplayModes()
{
	int displayModesCount = 0;
	if (SDL_DisplayMode** displayModes = SDL_GetFullscreenDisplayModes(SDL_GetPrimaryDisplay(), &displayModesCount))
	{
		std::vector<AkDisplayMode> outDisplayModes;
		outDisplayModes.reserve(displayModesCount);

		for (int i = 0; i < displayModesCount; ++i)
		{
			SDL_DisplayMode* mode = displayModes[i];
			outDisplayModes.push_back({ static_cast<uint32_t>(mode->w), static_cast<uint32_t>(mode->h), mode->refresh_rate });
		}

		return outDisplayModes;
	}
		
	AkLogError("Failed to get window display modes: {}", SDL_GetError());
	return {};
}