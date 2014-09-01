#pragma once
#include "cmmn.h"
#include "device.h"
#include "timer.h"

namespace gluk
{
	enum class key_action
	{
		press = GLFW_PRESS,
		release = GLFW_RELEASE,
		repeat = GLFW_REPEAT,
	};

	enum class key_mod
	{
		alt = GLFW_MOD_ALT,
		ctrl = GLFW_MOD_CONTROL,
		shift = GLFW_MOD_SHIFT,
		super = GLFW_MOD_SUPER,
	};

  class app
  {
  protected:
    GLFWwindow* wnd;
	device* dev;
	timer tm;
  public:
    app(const string& title, vec2 winsize, uint aa_samples,
        uvec4 colorbits = uvec4(8), uvec2 dsbits = uvec2(32,0), function<void()> apply_window_hints = function<void()>([&]{}));
    ~app();
    void run();

    virtual void render(float t, float dt) = 0;
    virtual void update(float t, float dt) = 0;
    virtual void resize() { }
	virtual void key_down(uint keycode, key_action action, key_mod mods) {}
  };
}
