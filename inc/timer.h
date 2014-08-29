#pragma once
#include "cmmn.h"

namespace gluk
{
	class timer
	{
		float last_time;
		float curr_time;
		float _ctime;
		float _deltat;
	public:
		timer()
		{
			last_time = curr_time = glfwGetTime();
			_deltat = 0;
			_ctime = 0;
		}

		void reset()
		{
			last_time = curr_time = glfwGetTime();
			_ctime = 0;
			_deltat = 0;
		}

		void update()
		{
			curr_time = glfwGetTime();

			_deltat = curr_time - last_time;
			_ctime += _deltat;

			last_time = curr_time;
		}

		inline float time() { return _ctime; }
		inline float delta_time() { return _deltat; }
	};
}