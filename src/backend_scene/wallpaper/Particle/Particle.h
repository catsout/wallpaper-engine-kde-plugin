#pragma once

namespace wallpaper {

struct Particle {
	float position[3] {0.0f};

	float color[3] {1.0f, 1.0f, 1.0f};
	float colorInit[3] {1.0f, 1.0f, 1.0f};

	float alpha {1.0f};
	float alphaInit {1.0f};

	float size {20};
	float sizeInit {20};

	float lifetime {1.0f};
	float lifetimeInit {1.0f};

	float rotation[3] {0.0f, 0.0f, 0.0f}; // radian  z x y
	float velocity[3] {0.0f, 0.0f, 0.0f};
	float acceleration[3] {0.0f, 0.0f, 0.0f};
	float angularVelocity[3] {0.0f, 0.0f, 0.0f};
	float angularAcceleration[3] {0.0f, 0.0f, 0.0f};
};
}