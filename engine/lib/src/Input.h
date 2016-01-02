#ifndef __Input_h__
#define __Input_h__

#include "Vector2.h"
#include "Vector3.h"
#include <vector>

namespace Galaxy3D
{
	struct KeyCode
	{
		enum Enum
		{
			None = 0,
			Backspace = 8,
			Tab = 9,
			Clear = 12,
			Return = 13,
			Pause = 19,
			Escape = 27,
			Space = 32,
			Exclaim = 33,
			DoubleQuote = 34,
			Hash = 35,
			Dollar = 36,
			Ampersand = 38,
			Quote = 39,
			LeftParen = 40,
			RightParen = 41,
			Asterisk = 42,
			Plus = 43,
			Comma = 44,
			Minus = 45,
			Period = 46,
			Slash = 47,
			Alpha0 = 48,
			Alpha1 = 49,
			Alpha2 = 50,
			Alpha3 = 51,
			Alpha4 = 52,
			Alpha5 = 53,
			Alpha6 = 54,
			Alpha7 = 55,
			Alpha8 = 56,
			Alpha9 = 57,
			Colon = 58,
			Semicolon = 59,
			Less = 60,
			Equals = 61,
			Greater = 62,
			Question = 63,
			At = 64,
			LeftBracket = 91,
			Backslash = 92,
			RightBracket = 93,
			Caret = 94,
			Underscore = 95,
			BackQuote = 96,
			A = 97,
			B = 98,
			C = 99,
			D = 100,
			E = 101,
			F = 102,
			G = 103,
			H = 104,
			I = 105,
			J = 106,
			K = 107,
			L = 108,
			M = 109,
			N = 110,
			O = 111,
			P = 112,
			Q = 113,
			R = 114,
			S = 115,
			T = 116,
			U = 117,
			V = 118,
			W = 119,
			X = 120,
			Y = 121,
			Z = 122,
			Delete = 127,
			Keypad0 = 256,
			Keypad1 = 257,
			Keypad2 = 258,
			Keypad3 = 259,
			Keypad4 = 260,
			Keypad5 = 261,
			Keypad6 = 262,
			Keypad7 = 263,
			Keypad8 = 264,
			Keypad9 = 265,
			KeypadPeriod = 266,
			KeypadDivide = 267,
			KeypadMultiply = 268,
			KeypadMinus = 269,
			KeypadPlus = 270,
			KeypadEnter = 271,
			KeypadEquals = 272,
			UpArrow = 273,
			DownArrow = 274,
			RightArrow = 275,
			LeftArrow = 276,
			Insert = 277,
			Home = 278,
			End = 279,
			PageUp = 280,
			PageDown = 281,
			F1 = 282,
			F2 = 283,
			F3 = 284,
			F4 = 285,
			F5 = 286,
			F6 = 287,
			F7 = 288,
			F8 = 289,
			F9 = 290,
			F10 = 291,
			F11 = 292,
			F12 = 293,
			F13 = 294,
			F14 = 295,
			F15 = 296,
			Numlock = 300,
			CapsLock = 301,
			ScrollLock = 302,
			RightShift = 303,
			LeftShift = 304,
			RightControl = 305,
			LeftControl = 306,
			RightAlt = 307,
			LeftAlt = 308,
			RightApple = 309,
			RightCommand = 309,
			LeftApple = 310,
			LeftCommand = 310,
			LeftWindows = 311,
			RightWindows = 312,
			AltGr = 313,
			Help = 315,
			Print = 316,
			SysReq = 317,
			Break = 318,
			Menu = 319,
			Mouse0 = 323,
			Mouse1 = 324,
			Mouse2 = 325,
			Mouse3 = 326,
			Mouse4 = 327,
			Mouse5 = 328,
			Mouse6 = 329,
			JoystickButton0 = 330,
			JoystickButton1 = 331,
			JoystickButton2 = 332,
			JoystickButton3 = 333,
			JoystickButton4 = 334,
			JoystickButton5 = 335,
			JoystickButton6 = 336,
			JoystickButton7 = 337,
			JoystickButton8 = 338,
			JoystickButton9 = 339,
			JoystickButton10 = 340,
			JoystickButton11 = 341,
			JoystickButton12 = 342,
			JoystickButton13 = 343,
			JoystickButton14 = 344,
			JoystickButton15 = 345,
			JoystickButton16 = 346,
			JoystickButton17 = 347,
			JoystickButton18 = 348,
			JoystickButton19 = 349,
			Joystick1Button0 = 350,
			Joystick1Button1 = 351,
			Joystick1Button2 = 352,
			Joystick1Button3 = 353,
			Joystick1Button4 = 354,
			Joystick1Button5 = 355,
			Joystick1Button6 = 356,
			Joystick1Button7 = 357,
			Joystick1Button8 = 358,
			Joystick1Button9 = 359,
			Joystick1Button10 = 360,
			Joystick1Button11 = 361,
			Joystick1Button12 = 362,
			Joystick1Button13 = 363,
			Joystick1Button14 = 364,
			Joystick1Button15 = 365,
			Joystick1Button16 = 366,
			Joystick1Button17 = 367,
			Joystick1Button18 = 368,
			Joystick1Button19 = 369,
			Joystick2Button0 = 370,
			Joystick2Button1 = 371,
			Joystick2Button2 = 372,
			Joystick2Button3 = 373,
			Joystick2Button4 = 374,
			Joystick2Button5 = 375,
			Joystick2Button6 = 376,
			Joystick2Button7 = 377,
			Joystick2Button8 = 378,
			Joystick2Button9 = 379,
			Joystick2Button10 = 380,
			Joystick2Button11 = 381,
			Joystick2Button12 = 382,
			Joystick2Button13 = 383,
			Joystick2Button14 = 384,
			Joystick2Button15 = 385,
			Joystick2Button16 = 386,
			Joystick2Button17 = 387,
			Joystick2Button18 = 388,
			Joystick2Button19 = 389,
			Joystick3Button0 = 390,
			Joystick3Button1 = 391,
			Joystick3Button2 = 392,
			Joystick3Button3 = 393,
			Joystick3Button4 = 394,
			Joystick3Button5 = 395,
			Joystick3Button6 = 396,
			Joystick3Button7 = 397,
			Joystick3Button8 = 398,
			Joystick3Button9 = 399,
			Joystick3Button10 = 400,
			Joystick3Button11 = 401,
			Joystick3Button12 = 402,
			Joystick3Button13 = 403,
			Joystick3Button14 = 404,
			Joystick3Button15 = 405,
			Joystick3Button16 = 406,
			Joystick3Button17 = 407,
			Joystick3Button18 = 408,
			Joystick3Button19 = 409,
			Joystick4Button0 = 410,
			Joystick4Button1 = 411,
			Joystick4Button2 = 412,
			Joystick4Button3 = 413,
			Joystick4Button4 = 414,
			Joystick4Button5 = 415,
			Joystick4Button6 = 416,
			Joystick4Button7 = 417,
			Joystick4Button8 = 418,
			Joystick4Button9 = 419,
			Joystick4Button10 = 420,
			Joystick4Button11 = 421,
			Joystick4Button12 = 422,
			Joystick4Button13 = 423,
			Joystick4Button14 = 424,
			Joystick4Button15 = 425,
			Joystick4Button16 = 426,
			Joystick4Button17 = 427,
			Joystick4Button18 = 428,
			Joystick4Button19 = 429,
		};

		static const int COUNT = 430;
	};
    
    struct TouchPhase
    {
        enum Enum
        {
            Began,          //A finger touched the screen.
            Moved,          //A finger moved on the screen.
            Stationary,     //A finger is touching the screen but hasn't moved.
            Ended,          //A finger was lifted from the screen. This is the final phase of a touch.
            Canceled,       //The system cancelled tracking for the touch.
        };
    };
    
    struct Touch
    {
        Vector2 deltaPosition;
        float time;
        float deltaTime;
        int fingerId;
        TouchPhase::Enum phase;
        Vector2 position;
        int tapCount;
    };

	class Input
	{
	public:
        static bool IsMultiTouchEnabled() { return m_multi_touch_enabled; }
        static void EnableMultiTouch(bool value);
        static int GetTouchCount();
        static const Touch *GetTouch(int index);
        static void Update();
        static bool GetKeyDown(KeyCode::Enum key);
        static bool GetKey(KeyCode::Enum key);
        static bool GetKeyUp(KeyCode::Enum key);
        static bool GetMouseButtonDown(int index);
        static bool GetMouseButton(int index);
        static bool GetMouseButtonUp(int index);
        static Vector3 GetMousePosition();
		static void ResetInputAxes();
        
	private:
        static bool m_multi_touch_enabled;
	};
}

#endif