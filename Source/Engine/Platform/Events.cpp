#include "Events.h"
#include "Core/Log.h"

#include <SDL3/SDL.h>

#include <array>

static constexpr std::array kKeyCodeLookupTable = std::to_array<uint32_t>
({
	SDLK_0,
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_4,
	SDLK_5,
	SDLK_6,
	SDLK_7,
	SDLK_8,
	SDLK_9,

	SDLK_A,
	SDLK_B,
	SDLK_C,
	SDLK_D,
	SDLK_E,
	SDLK_F,
	SDLK_G,
	SDLK_H,
	SDLK_I,
	SDLK_J,
	SDLK_K,
	SDLK_L,
	SDLK_M,
	SDLK_N,
	SDLK_O,
	SDLK_P,
	SDLK_Q,
	SDLK_R,
	SDLK_S,
	SDLK_T,
	SDLK_U,
	SDLK_V,
	SDLK_W,
	SDLK_X,
	SDLK_Y,
	SDLK_Z,

	SDLK_F1,
	SDLK_F2,
	SDLK_F3,
	SDLK_F4,
	SDLK_F5,
	SDLK_F6,
	SDLK_F7,
	SDLK_F8,
	SDLK_F9,
	SDLK_F10,
	SDLK_F11,
	SDLK_F12,

	SDLK_KP_0,
	SDLK_KP_1,
	SDLK_KP_2,
	SDLK_KP_3,
	SDLK_KP_4,
	SDLK_KP_5,
	SDLK_KP_6,
	SDLK_KP_7,
	SDLK_KP_8,
	SDLK_KP_9,
	SDLK_KP_ENTER,
	SDLK_KP_DIVIDE,
	SDLK_KP_MULTIPLY,
	SDLK_KP_MINUS,
	SDLK_KP_PLUS,

	SDLK_ESCAPE,
	SDLK_GRAVE,
	SDLK_TAB,
	SDLK_CAPSLOCK,
	SDLK_LSHIFT,
	SDLK_LCTRL,
	SDLK_LGUI,
	SDLK_LALT,
	SDLK_SPACE,
	SDLK_RALT,
	SDLK_RGUI,
	SDLK_APPLICATION,
	SDLK_RCTRL,
	SDLK_RSHIFT,
	SDLK_RETURN,
	SDLK_BACKSPACE,
	SDLK_DELETE,
	SDLK_INSERT,
	SDLK_COMMA,
	SDLK_PERIOD,
	SDLK_SLASH,
	SDLK_SEMICOLON,
	SDLK_APOSTROPHE,
	SDLK_LEFTBRACKET,
	SDLK_RIGHTBRACKET,
	SDLK_BACKSLASH,
	SDLK_EQUALS,
	SDLK_MINUS,

	SDLK_UP,
	SDLK_DOWN,
	SDLK_LEFT,
	SDLK_RIGHT,

	SDLK_PAUSE,
	SDLK_PRINTSCREEN,
	SDLK_HOME,
	SDLK_PAGEUP,
	SDLK_PAGEDOWN,
	SDLK_SCROLLLOCK,
	SDLK_END,
	SDLK_NUMLOCKCLEAR
});

static constexpr std::array kMouseButtonLookupTable = std::to_array<uint8_t>
({
	SDL_BUTTON_LEFT,
	SDL_BUTTON_MIDDLE,
	SDL_BUTTON_RIGHT,
	SDL_BUTTON_X1,
	SDL_BUTTON_X2
});

bool AkEvents::Initialize()
{
	if (!SDL_InitSubSystem(SDL_INIT_EVENTS))
	{
		AkLogError("Failed to initialize platform events: {}", SDL_GetError());
		return false;
	}

	return true;
}

void AkEvents::Deinitialize()
{
	SDL_Quit();
}

void AkEvents::PollEvents()
{
	m_InputState.BeginFrame();

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_EventType::SDL_EVENT_MOUSE_MOTION:
			{
				m_InputState.mousePosition = { event.motion.x, event.motion.y };
				m_InputState.mouseDelta = { event.motion.xrel, -event.motion.yrel };
				break;
			}
			case SDL_EventType::SDL_EVENT_MOUSE_WHEEL:
			{
				const float directionMultiplier = event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? 1.f : -1.f;
				m_InputState.mouseWheel.x = directionMultiplier * event.wheel.x;
				m_InputState.mouseWheel.y = directionMultiplier * event.wheel.y;
				break;
			}
			case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				// http://www-graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
				AkInputState::AkButtonState& state = m_InputState.mouseButtonStates[event.button.button];
				const bool shouldSet = state & AkInputState::IDLE;
				state = state ^ ((-static_cast<uint8_t>(shouldSet) ^ state) & AkInputState::PRESSED);
				break;
			}
			case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_UP:
			{
				AkInputState::AkButtonState& state = m_InputState.mouseButtonStates[event.button.button];
				state = AkInputState::RELEASED;
				break;
			}
			case SDL_EventType::SDL_EVENT_KEY_DOWN:
			{
				AkInputState::AkButtonState& state = m_InputState.keyStates[event.key.key];
				const bool shouldSet = state & AkInputState::IDLE;
				state = state ^ ((-static_cast<uint8_t>(shouldSet) ^ state) & AkInputState::PRESSED);
				break;
			}
			case SDL_EventType::SDL_EVENT_KEY_UP:
			{
				AkInputState::AkButtonState& state = m_InputState.keyStates[event.key.key];
				state = AkInputState::RELEASED;
				break;
			}
			case SDL_EventType::SDL_EVENT_QUIT:
			case SDL_EventType::SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			{
				m_ShouldClose = true;
				break;
			}
		}
	}
}

void AkEvents::TriggerQuit()
{
	SDL_Event event;
	event.type = SDL_EVENT_QUIT;
	SDL_PushEvent(&event);
}

bool AkEvents::ShouldClose()
{
	return m_ShouldClose;
}

const glm::vec2& AkEvents::GetMouseWheel()
{
	return m_InputState.mouseWheel;
}

const glm::vec2& AkEvents::GetMouseDelta()
{
	return m_InputState.mouseDelta;
}

const glm::vec2& AkEvents::GetMousePosition()
{
	return m_InputState.mousePosition;
}

bool AkEvents::GetKey(AkKeyCode key)
{
	const uint32_t storageKey = kKeyCodeLookupTable[static_cast<uint32_t>(key)];
	return m_InputState.keyStates[storageKey] & AkInputState::HELD;
}

bool AkEvents::GetKeyUp(AkKeyCode key)
{
	const uint32_t storageKey = kKeyCodeLookupTable[static_cast<uint32_t>(key)];
	return m_InputState.keyStates[storageKey] & AkInputState::RELEASED;
}

bool AkEvents::GetKeyDown(AkKeyCode key)
{
	const uint32_t storageKey = kKeyCodeLookupTable[static_cast<uint32_t>(key)];
	return m_InputState.keyStates[storageKey] & AkInputState::PRESSED;
}

bool AkEvents::GetMouseButton(AkMouseButton mouseButton)
{
	const uint8_t storageKey = kMouseButtonLookupTable[static_cast<uint32_t>(mouseButton)];
	return m_InputState.mouseButtonStates[storageKey] & AkInputState::HELD;
}

bool AkEvents::GetMouseButtonUp(AkMouseButton mouseButton)
{
	const uint8_t storageKey = kMouseButtonLookupTable[static_cast<uint32_t>(mouseButton)];
	return m_InputState.mouseButtonStates[storageKey] & AkInputState::RELEASED;
}

bool AkEvents::GetMouseButtonDown(AkMouseButton mouseButton)
{
	const uint8_t storageKey = kMouseButtonLookupTable[static_cast<uint32_t>(mouseButton)];
	return m_InputState.mouseButtonStates[storageKey] & AkInputState::PRESSED;
}

void AkEvents::AkInputState::BeginFrame()
{
	mouseWheel = glm::vec2(0.f);
	mouseDelta = glm::vec2(0.f);

	for (auto& [keyCode, state] : m_InputState.keyStates)
		state.Validate();

	for (auto& [mouseButton, state] : m_InputState.mouseButtonStates)
		state.Validate();
}

void AkEvents::AkInputState::AkButtonState::Validate()
{
	const bool shift = m_State & (AkInputState::PRESSED | AkInputState::RELEASED);
	m_State <<= static_cast<uint8_t>(shift);
}