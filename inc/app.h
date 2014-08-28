#pragma once
#include "cmmn.h"
#include "device.h"

namespace gluk
{
  class app
  {
  protected:
    GLFWwindow* wnd;
	device* dev;
  public:
    app(const string& title, vec2 winsize, uint aa_samples,
        uvec4 colorbits = uvec4(8), uvec2 dsbits = uvec2(32,0), function<void()> apply_window_hints = function<void()>([&]{}));
    ~app();
    void run();

    virtual void render(float t, float dt) = 0;
    virtual void update(float t, float dt) = 0;
    virtual void resize() { }
  };
}
