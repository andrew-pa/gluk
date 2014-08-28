#include "cmmn.h"
#include "app.h"
using namespace gluk;

class test_app : public app
{
public:
	test_app() 
		: app("test", vec2(640, 480), 16)
	{
	}

	void update(float t, float dt) override
	{
	}

	void render(float t, float dt) override
	{
	}
};

int main()
{
	test_app a;
	a.run();
}