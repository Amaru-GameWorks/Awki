#pragma once
#include <glm/vec2.hpp>
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

	static const glm::vec2& GetMouseWheel();
	static const glm::vec2& GetMouseDelta();
	static const glm::vec2& GetMousePosition();

	static bool GetKey(AkKeyCode key);
	static bool GetKeyUp(AkKeyCode key);
	static bool GetKeyDown(AkKeyCode key);

	static bool GetMouseButton(AkMouseButton mouseButton);
	static bool GetMouseButtonUp(AkMouseButton mouseButton);
	static bool GetMouseButtonDown(AkMouseButton mouseButton);

private:
	struct AkInputState
	{
		enum : uint8_t
		{
			PRESSED		= 1 << 0,
			HELD		= 1 << 1,
			RELEASED	= 1 << 2,
			IDLE		= 1 << 3
		};

		class AkButtonState
		{
		public:
			AkButtonState() = default;
			AkButtonState(uint8_t newState) { m_State = newState; }
			operator uint8_t() const { return m_State; }
			
			void Validate();
			
		private:
			uint8_t m_State = IDLE;
		};

		glm::vec2 mouseDelta;
		glm::vec2 mouseWheel;
		glm::vec2 mousePosition;
		std::unordered_map<uint32_t, AkButtonState> keyStates;
		std::unordered_map<uint8_t, AkButtonState> mouseButtonStates;

		void BeginFrame();
	};

	static inline bool m_ShouldClose = false;
	static inline AkInputState m_InputState = {};
};