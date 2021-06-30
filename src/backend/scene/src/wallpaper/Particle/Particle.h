#pragma once

namespace wallpaper {

struct Particle {
	float position[3] {0.0f};
	float color[3] {0.0f};
	float alpha {1.0f};
	float size {20};
	float lifetime {1.0f};

	float rotation[3] {0.0f}; // radian
	float velocity[3] {0.0f};
	float angularVelocity[3] {0.0f};
};
}