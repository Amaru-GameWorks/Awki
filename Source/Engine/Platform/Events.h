#pragma once
#include <unordered_map>

enum class AkKeyCode
{
	ALPHA_0,
	ALPHA_1,
	ALPHA_2,
	ALPHA_3,
	ALPHA_4,
	ALPHA_5,
	ALPHA_6,
	ALPHA_7,
	ALPHA_8,
	ALPHA_9,

	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,

	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,

	KEYPAD_0,
	KEYPAD_1,
	KEYPAD_2,
	KEYPAD_3,
	KEYPAD_4,
	KEYPAD_5,
	KEYPAD_6,
	KEYPAD_7,
	KEYPAD_8,
	KEYPAD_9,
	KEYPAD_ENTER,
	KEYPAD_DIVIDE,
	KEYPAD_MULTIPLY,
	KEYPAD_MINUS,
	KEYPAD_PLUS,

	ESCAPE,
	GRAVE,
	TAB,
	CAPSLOCK,
	LSHIFT,
	LCTRL,
	LGUI,
	LALT,
	SPACE,
	RALT,
	RGUI,
	CONTEXT_MENU,
	RCTRL,
	RSHIFT,
	ENTER,
	BACKSPACE,
	DELETE,
	INSERT,
	COMMA,
	PERIOD,
	SLASH,
	SEMICOLON,
	APOSTROPHE,
	LBRACKET,
	RBRACKET,
	BACKSLASH,
	EQUALS,
	MINUS,
	HASH,

	UP,
	DOWN,
	LEFT,
	RIGHT,

	PAUSE,
	PRINTSCREEN,
	HOME,
	PAGEUP,
	PAGEDOWN,
	SCROLL_LOCK,
	END,
	NUM_LOCK
};

enum class AkMouseButton
{
	LEFT,
	MIDDLE,
	RIGHT,
	X1,
	X2
};

class AkEvents
{
public:
	static bool Initialize();
	static void Deinitialize();

	static void PollEvents();
	static void TriggerQuit();
	static bool ShouldClose();

	static bool GetKey(AkKeyCode key);
	static bool GetKeyUp(AkKeyCode key);
	static bool GetKeyDown(AkKeyCode key);

	static bool GetMouseButton(AkMouseButton mouseButton);
	static bool GetMouseButtonUp(AkMouseButton mouseButton);
	static bool GetMouseButtonDown(AkMouseButton mouseButton);

private:
	struct AkInputState
	{
		enum class State
		{
			PRESSED,
			HELD,
			RELEASED,
			IDLE
		};

		class AkButtonState
		{
		public:
			State GetState() const { return m_Current; }
			void SetState(const State newState) { m_Current = newState; }

			void Validate()
			{
				if (m_Current == State::PRESSED)
					m_Current = State::HELD;
				else if (m_Current == State::RELEASED)
					m_Current = State::IDLE;
			}

		private:
			State m_Current = State::IDLE;
		};

		std::unordered_map<AkKeyCode, AkButtonState> keyStates;
		std::unordered_map<AkMouseButton, AkButtonState> mouseButtonStates;

		void BeginFrame();
	};

	static inline bool m_ShouldClose = false;
	static inline AkInputState m_InputState = {};
};