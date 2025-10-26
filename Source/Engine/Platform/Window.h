#pragma once
#include <string_view>

enum AkWindowFlagBits : uint16_t
{
	AkWindowFlag_FULLSCREEN			= 1 << 0,
	AkWindowFlag_HIDDEN				= 1 << 1,
	AkWindowFlag_BORDERLESS			= 1 << 2,
	AkWindowFlag_RESIZABLE			= 1 << 3,
	AkWindowFlag_MINIMIZED			= 1 << 4,
	AkWindowFlag_MAXIMIZED			= 1 << 5,
	AkWindowFlag_HIGH_PIXEL_DENSITY	= 1 << 6,
	AkWindowFlag_ALWAYS_ON_TOP		= 1 << 7,
	AkWindowFlag_UTILITY_WINDOW		= 1 << 8
};
using AkWindowFlags = std::underlying_type<AkWindowFlagBits>::type;

struct AkWindowDescriptor
{
	std::string_view name;
	AkWindowFlags flags = 1;
	uint16_t width = 1280;
	uint16_t height = 720;
};

class AkWindow
{
public:
	AkWindow(const AkWindowDescriptor& descriptor);
	~AkWindow();

	void SetTitle(std::string_view title);
	
	void SetSize(uint32_t width, uint32_t height);
	void GetSize(uint32_t& width, uint32_t& height);

	void SetPosition(uint32_t x, uint32_t y);
	void GetPosition(uint32_t& x, uint32_t& y);

	void SetFullScreen(bool state);
	void SetBorderless(bool state);
	void SetResizable(bool state);
	void SetAlwaysOnTop(bool state);
	void SetHidden(bool state);

	void Maximize();
	bool IsMaximized() const;
	
	void Minimize();
	bool IsMinimized() const;
	
	void Focus();
	bool HasFocus() const;
	bool HasInputFocus() const;
	bool HasMouseFocus() const;

private:
	struct SDL_Window* m_WindowHandle = nullptr;
};