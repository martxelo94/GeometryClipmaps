#include "input.h"

JoystickDevice joystick;
KeyboardDevice keyboard;
MouseDevice mouse;

void MouseDevice::init()
{
	InputDevice::init();
	pos = move = wheel = ivec2{};
	over_window = false;
}

void MouseDevice::update()
{
	InputDevice::update();
	move = wheel = ivec2{};
}

void MouseDevice::handle(const SDL_Event &e)
{
	switch (e.type) {
	case SDL_MOUSEMOTION:
		mouse.pos = ivec2{ e.motion.x, e.motion.y };
		mouse.move = ivec2{ e.motion.xrel, e.motion.yrel };
		mouse.over_window = e.motion.windowID != 0 ? true : false;
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		mouse.set(MAP_MOUSECODE(e.button.button), e.type == SDL_MOUSEBUTTONDOWN ? KeyState::triggered : KeyState::up);
		break;
	case SDL_MOUSEWHEEL:
		mouse.wheel = ivec2{ e.wheel.x, e.wheel.y };
		break;

	}
}