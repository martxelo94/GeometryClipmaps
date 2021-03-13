#pragma once

#include "math_utils.h"
#include <SDL.h>
#include <bitset>

#define SDL_CHARCODE_HIGH SDLK_DELETE
#define SDL_SCANCODE_LOW SDLK_CAPSLOCK
#define SDL_SCANCODE_HIGH SDLK_SLEEP
#define SDL_SCANCODE_MASK (1 << 30)
#define MAP_SCANCODE(key) ((SDL_SCANCODE_MASK & key) == SDL_SCANCODE_MASK? ((~SDL_SCANCODE_MASK & key) - (~SDL_SCANCODE_MASK & SDL_SCANCODE_LOW) + SDL_CHARCODE_HIGH + 1) : key)
#define MAP_SCANCODE_RANGE ((SDL_CHARCODE_HIGH) + (SDL_SCANCODE_HIGH - SDL_SCANCODE_LOW))
#define INPUT_MOUSE_NUM (SDL_BUTTON_RIGHT - SDL_BUTTON_LEFT)
#define MAP_MOUSECODE(button) (button - SDL_BUTTON_LEFT)
#define INPUT_JOYSTICK_NUM 0

//WARNING: Some special keys do not work becouse they are wrong mapped to bitset
enum KeyCode : unsigned int
{
	//Mouse buttons
	LMB = SDL_BUTTON_LEFT - 1, MMB = SDL_BUTTON_MIDDLE - 1, RMB = SDL_BUTTON_RIGHT - 1,
	//Special keys
	Tab = SDLK_TAB, Alt = MAP_SCANCODE(SDLK_LALT), Shift = MAP_SCANCODE(SDLK_LSHIFT), Ctrl = MAP_SCANCODE(SDLK_LCTRL),
	Escape = SDLK_ESCAPE, Space = SDLK_SPACE, Delete = SDLK_DELETE,
	Enter = SDLK_RETURN, EnterNumpad = MAP_SCANCODE(SDLK_KP_ENTER),
	//Arrows
	ArrowUp = MAP_SCANCODE(SDLK_UP), ArrowDown = MAP_SCANCODE(SDLK_DOWN), ArrowLeft = MAP_SCANCODE(SDLK_LEFT), ArrowRight = MAP_SCANCODE(SDLK_RIGHT),
	//Alphabet
	A = SDLK_a, B = SDLK_b, C = SDLK_c, D = SDLK_d, E = SDLK_e, F = SDLK_f, G = SDLK_g, H = SDLK_h,
	I = SDLK_i, J = SDLK_j, K = SDLK_k, L = SDLK_l, M = SDLK_m, N = SDLK_n, O = SDLK_o, P = SDLK_p,
	Q = SDLK_q, R = SDLK_r, S = SDLK_s, T = SDLK_t, U = SDLK_u, V = SDLK_v, W = SDLK_w, X = SDLK_x, Y = SDLK_y, Z = SDLK_z,
	//Numbers
	n0 = SDLK_0, n1 = SDLK_1, n2 = SDLK_2, n3 = SDLK_3, n4 = SDLK_4, n5 = SDLK_5, n6 = SDLK_6, n7 = SDLK_7, n8 = SDLK_8, n9 = SDLK_9

};

enum class KeyState : bool { up = false, triggered = true };


template<unsigned RANGE>
struct InputDevice
{
protected:
	std::bitset<RANGE> devicePrev;
	std::bitset<RANGE> deviceCurr;
	inline void set(unsigned vr_keycode, KeyState val)
	{
		deviceCurr.set(vr_keycode, (bool)val);
	}
public:
	void init()
	{
		devicePrev.reset();
		deviceCurr.reset();
	}
	void free() {}
	void update()
	{
		devicePrev = deviceCurr;
		assert(devicePrev == deviceCurr);
	}

	bool triggered(unsigned keycode) const
	{
		assert(keycode < RANGE);
		if (deviceCurr[keycode] == true && devicePrev[keycode] == false)
			return true;
		return false;
	}
	bool pressed(unsigned keycode) const
	{
		assert(keycode < RANGE);
		if (deviceCurr[keycode] == true && devicePrev[keycode] == true)
			return true;
		return false;
	}
	bool released(unsigned keycode) const
	{
		assert(keycode < LAST + 1);
		if (deviceCurr[keycode] == false && devicePrev[keycode] == true)
			return true;
		return false;
	}

	void handle(const SDL_Event& e) {
		switch (e.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			keyboard.set(MAP_SCANCODE(e.key.keysym.sym), e.type == SDL_KEYDOWN ? KeyState::triggered : KeyState::up);
			break;

		}
	}
};

struct MouseDevice : public InputDevice<SDL_BUTTON_RIGHT - SDL_BUTTON_LEFT + 1>
{
private:
	ivec2 wheel;		// Specifies the value of the mouse wheel.
	ivec2 pos;		// Specifies the mouse position in centered window coordinates.
	ivec2 move;		// Specifies the mouse movement from the previous frame to the current frame.
	bool over_window = false;	// Specifies whether the mouse is inside the window or not.
public:
	void init();
	void free() {}
	void update();
	void handle(const SDL_Event& e);


	__forceinline const ivec2& getPos() const { return pos; }
	__forceinline const ivec2& getMove() const { return move; }
	__forceinline const ivec2& getWheel() const { return wheel; }
	__forceinline bool isOverWindow() const { return over_window; }
};
typedef InputDevice<MAP_SCANCODE_RANGE> KeyboardDevice;
typedef InputDevice<1> JoystickDevice;
typedef MouseDevice MouseDevice;

extern JoystickDevice joystick;
extern KeyboardDevice keyboard;
extern MouseDevice mouse;
