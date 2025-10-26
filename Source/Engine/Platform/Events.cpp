#include "Events.h"
#include "Core/Log.h"

#include <SDL3/SDL.h>

#include <array>

static constexpr std::array kKeyCodeConversionTable =
{
	std::pair(AkKeyCode::ALPHA_0, SDLK_0),
	std::pair(AkKeyCode::ALPHA_1, SDLK_1),
	std::pair(AkKeyCode::ALPHA_2, SDLK_2),
	std::pair(AkKeyCode::ALPHA_3, SDLK_3),
	std::pair(AkKeyCode::ALPHA_4, SDLK_4),
	std::pair(AkKeyCode::ALPHA_5, SDLK_5),
	std::pair(AkKeyCode::ALPHA_6, SDLK_6),
	std::pair(AkKeyCode::ALPHA_7, SDLK_7),
	std::pair(AkKeyCode::ALPHA_8, SDLK_8),
	std::pair(AkKeyCode::ALPHA_9, SDLK_9),

	std::pair(AkKeyCode::A, SDLK_A),
	std::pair(AkKeyCode::B, SDLK_B),
	std::pair(AkKeyCode::C, SDLK_C),
	std::pair(AkKeyCode::D, SDLK_D),
	std::pair(AkKeyCode::E, SDLK_E),
	std::pair(AkKeyCode::F, SDLK_F),
	std::pair(AkKeyCode::G, SDLK_G),
	std::pair(AkKeyCode::H, SDLK_H),
	std::pair(AkKeyCode::I, SDLK_I),
	std::pair(AkKeyCode::J, SDLK_J),
	std::pair(AkKeyCode::K, SDLK_K),
	std::pair(AkKeyCode::L, SDLK_L),
	std::pair(AkKeyCode::M, SDLK_M),
	std::pair(AkKeyCode::N, SDLK_N),
	std::pair(AkKeyCode::O, SDLK_O),
	std::pair(AkKeyCode::P, SDLK_P),
	std::pair(AkKeyCode::Q, SDLK_Q),
	std::pair(AkKeyCode::R, SDLK_R),
	std::pair(AkKeyCode::S, SDLK_S),
	std::pair(AkKeyCode::T, SDLK_T),
	std::pair(AkKeyCode::U, SDLK_U),
	std::pair(AkKeyCode::V, SDLK_V),
	std::pair(AkKeyCode::W, SDLK_W),
	std::pair(AkKeyCode::X, SDLK_X),
	std::pair(AkKeyCode::Y, SDLK_Y),
	std::pair(AkKeyCode::Z, SDLK_Z),

	std::pair(AkKeyCode::F1, SDLK_F1),
	std::pair(AkKeyCode::F2, SDLK_F2),
	std::pair(AkKeyCode::F3, SDLK_F3),
	std::pair(AkKeyCode::F4, SDLK_F4),
	std::pair(AkKeyCode::F5, SDLK_F5),
	std::pair(AkKeyCode::F6, SDLK_F6),
	std::pair(AkKeyCode::F7, SDLK_F7),
	std::pair(AkKeyCode::F8, SDLK_F8),
	std::pair(AkKeyCode::F9, SDLK_F9),
	std::pair(AkKeyCode::F10, SDLK_F10),
	std::pair(AkKeyCode::F11, SDLK_F11),
	std::pair(AkKeyCode::F12, SDLK_F12),

	std::pair(AkKeyCode::KEYPAD_0, SDLK_KP_0),
	std::pair(AkKeyCode::KEYPAD_1, SDLK_KP_1),
	std::pair(AkKeyCode::KEYPAD_2, SDLK_KP_2),
	std::pair(AkKeyCode::KEYPAD_3, SDLK_KP_3),
	std::pair(AkKeyCode::KEYPAD_4, SDLK_KP_4),
	std::pair(AkKeyCode::KEYPAD_5, SDLK_KP_5),
	std::pair(AkKeyCode::KEYPAD_6, SDLK_KP_6),
	std::pair(AkKeyCode::KEYPAD_7, SDLK_KP_7),
	std::pair(AkKeyCode::KEYPAD_8, SDLK_KP_8),
	std::pair(AkKeyCode::KEYPAD_9, SDLK_KP_9),
	std::pair(AkKeyCode::KEYPAD_ENTER, SDLK_KP_ENTER),
	std::pair(AkKeyCode::KEYPAD_DIVIDE, SDLK_KP_DIVIDE),
	std::pair(AkKeyCode::KEYPAD_MULTIPLY, SDLK_KP_MULTIPLY),
	std::pair(AkKeyCode::KEYPAD_MINUS, SDLK_KP_MINUS),
	std::pair(AkKeyCode::KEYPAD_PLUS, SDLK_KP_PLUS),

	std::pair(AkKeyCode::ESCAPE, SDLK_ESCAPE),
	std::pair(AkKeyCode::GRAVE, SDLK_GRAVE),
	std::pair(AkKeyCode::TAB, SDLK_TAB),
	std::pair(AkKeyCode::CAPSLOCK, SDLK_CAPSLOCK),
	std::pair(AkKeyCode::LSHIFT, SDLK_LSHIFT),
	std::pair(AkKeyCode::LCTRL, SDLK_LCTRL),
	std::pair(AkKeyCode::LGUI, SDLK_LGUI),
	std::pair(AkKeyCode::LALT, SDLK_LALT),
	std::pair(AkKeyCode::SPACE, SDLK_SPACE),
	std::pair(AkKeyCode::RALT, SDLK_RALT),
	std::pair(AkKeyCode::RGUI, SDLK_RGUI),
	std::pair(AkKeyCode::CONTEXT_MENU, SDLK_APPLICATION),
	std::pair(AkKeyCode::RCTRL, SDLK_RCTRL),
	std::pair(AkKeyCode::RSHIFT, SDLK_RSHIFT),
	std::pair(AkKeyCode::ENTER, SDLK_RETURN),
	std::pair(AkKeyCode::BACKSPACE, SDLK_BACKSPACE),
	std::pair(AkKeyCode::DELETE, SDLK_DELETE),
	std::pair(AkKeyCode::INSERT, SDLK_INSERT),
	std::pair(AkKeyCode::COMMA, SDLK_COMMA),
	std::pair(AkKeyCode::PERIOD, SDLK_PERIOD),
	std::pair(AkKeyCode::SLASH, SDLK_SLASH),
	std::pair(AkKeyCode::SEMICOLON, SDLK_SEMICOLON),
	std::pair(AkKeyCode::APOSTROPHE, SDLK_APOSTROPHE),
	std::pair(AkKeyCode::LBRACKET, SDLK_LEFTBRACKET),
	std::pair(AkKeyCode::RBRACKET, SDLK_RIGHTBRACKET),
	std::pair(AkKeyCode::BACKSLASH, SDLK_BACKSLASH),
	std::pair(AkKeyCode::EQUALS, SDLK_EQUALS),
	std::pair(AkKeyCode::MINUS, SDLK_MINUS),

	std::pair(AkKeyCode::UP, SDLK_UP),
	std::pair(AkKeyCode::DOWN, SDLK_DOWN),
	std::pair(AkKeyCode::LEFT, SDLK_LEFT),
	std::pair(AkKeyCode::RIGHT, SDLK_RIGHT),

	std::pair(AkKeyCode::PAUSE, SDLK_PAUSE),
	std::pair(AkKeyCode::PRINTSCREEN, SDLK_PRINTSCREEN),
	std::pair(AkKeyCode::HOME, SDLK_HOME),
	std::pair(AkKeyCode::PAGEUP, SDLK_PAGEUP),
	std::pair(AkKeyCode::PAGEDOWN, SDLK_PAGEDOWN),
	std::pair(AkKeyCode::SCROLL_LOCK, SDLK_SCROLLLOCK),
	std::pair(AkKeyCode::END, SDLK_END),
	std::pair(AkKeyCode::NUM_LOCK, SDLK_NUMLOCKCLEAR)
};

constexpr AkKeyCode ConvertKeyCode(unsigned int sdlKeyCode)
{
	const auto it = std::find_if(kKeyCodeConversionTable.begin(), kKeyCodeConversionTable.end(), [sdlKeyCode](auto pair) {return std::get<1>(pair) == sdlKeyCode; });
	return std::get<0>(*it);
}

static constexpr std::array kMouseButtonConversionTable =
{
	std::pair(AkMouseButton::LEFT, SDL_BUTTON_LEFT),
	std::pair(AkMouseButton::MIDDLE, SDL_BUTTON_MIDDLE),
	std::pair(AkMouseButton::RIGHT, SDL_BUTTON_RIGHT),
	std::pair(AkMouseButton::X1, SDL_BUTTON_X1),
	std::pair(AkMouseButton::X2, SDL_BUTTON_X2)
};

constexpr AkMouseButton ConvertMouseButton(int sdlMouseButtonIndex)
{
	const auto it = std::find_if(kMouseButtonConversionTable.begin(), kMouseButtonConversionTable.end(), [sdlMouseButtonIndex](auto pair) {return std::get<1>(pair) == sdlMouseButtonIndex; });
	return std::get<0>(*it);
}

bool AkEvents::Initialize()
{
	if (!SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC))
	{
		AkLogError("Failed to initialize platform events: {}", SDL_GetError());
		return false;
	}

	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_JOY_CONS, "1");
	SDL_SetHint(SDL_HINT_JOYSTICK_LINUX_DEADZONES, "1");
	SDL_SetHint(SDL_HINT_JOYSTICK_ENHANCED_REPORTS, "1");
	SDL_SetHint(SDL_HINT_GAMECONTROLLER_SENSOR_FUSION, "0");
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
			case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				AkInputState::AkButtonState& state = m_InputState.mouseButtonStates[ConvertMouseButton(event.button.button)];
				if (state.GetState() != AkInputState::State::HELD)
					state.SetState(AkInputState::State::PRESSED);
				break;
			}
			case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_UP:
			{
				AkInputState::AkButtonState& state = m_InputState.mouseButtonStates[ConvertMouseButton(event.button.button)];
				state.SetState(AkInputState::State::RELEASED);
				break;
			}
			case SDL_EventType::SDL_EVENT_KEY_DOWN:
			{
				AkInputState::AkButtonState& state = m_InputState.keyStates[ConvertKeyCode(event.key.key)];
				if (state.GetState() != AkInputState::State::HELD)
					state.SetState(AkInputState::State::PRESSED);
				break;
			}
			case SDL_EventType::SDL_EVENT_KEY_UP:
			{
				AkInputState::AkButtonState& state = m_InputState.keyStates[ConvertKeyCode(event.key.key)];
				state.SetState(AkInputState::State::RELEASED);
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

void AkEvents::AkInputState::BeginFrame()
{
	for (auto& [keyCode, state] : m_InputState.keyStates)
		state.Validate();

	for (auto& [mouseButton, state] : m_InputState.mouseButtonStates)
		state.Validate();
}

bool AkEvents::GetKey(AkKeyCode key)
{
	return m_InputState.keyStates[key].GetState() == AkInputState::State::HELD;
}

bool AkEvents::GetKeyUp(AkKeyCode key)
{
	return m_InputState.keyStates[key].GetState() == AkInputState::State::RELEASED;
}

bool AkEvents::GetKeyDown(AkKeyCode key)
{
	return m_InputState.keyStates[key].GetState() == AkInputState::State::PRESSED;
}

bool AkEvents::GetMouseButton(AkMouseButton mouseButton)
{
	return m_InputState.mouseButtonStates[mouseButton].GetState() == AkInputState::State::HELD;
}

bool AkEvents::GetMouseButtonUp(AkMouseButton mouseButton)
{
	return m_InputState.mouseButtonStates[mouseButton].GetState() == AkInputState::State::RELEASED;
}

bool AkEvents::GetMouseButtonDown(AkMouseButton mouseButton)
{
	return m_InputState.mouseButtonStates[mouseButton].GetState() == AkInputState::State::PRESSED;
}