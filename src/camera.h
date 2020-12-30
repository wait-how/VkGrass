#pragma once

#include <cmath>

#include <glm/vec3.hpp>

#include "glfw_wrapper.h"

namespace cam {
	constexpr bool keyboardLook = true;
	constexpr bool mouseLook = false;
	constexpr bool flipY = false; // flipping viewport, not needed here

	class camera {
	public:
		glm::vec3 pos;
		glm::vec3 front;

		float hangle; // angles to keep track of camera direction
		float vangle;
		
		camera(const float pos_x, const float pos_y, const float pos_z);
		camera();

		void update(GLFWwindow*);
	private:
		float rads(float deg) { return deg * float(M_PI) / 180.0f; }

		int skip;
		float prevX, prevY;
		float prevTime;
		float mscale, lscale; // movement and look speeds
	};
};
