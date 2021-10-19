#pragma once
#include <functional>
#include <cassert>
#include <cmath>
#include <Eigen/Dense>

namespace  wallpaper {
namespace  algorism {
	float CalculatePersperctiveDistance(float fov, float height);
	float CalculatePersperctiveFov(float distence, float height);

	template<typename TInt>
	TInt PowOfTwo(TInt x) {
		TInt pow2 {8};
		while(pow2 < x) pow2*=2;	
		return pow2;
	}

	inline std::array<double, 3> GenSphere(const std::function<float()>& random) {
		double x1,x2;
		double m;
		while(true) {
			x1 = 1.0f - 2.0f*random();
			x2 = 1.0f - 2.0f*random();
			m = x1*x1 + x2*x2;
			if(m < 1.0f) break;
		}
	
		return {
			2.0f*x1*std::sqrt(1.0f-m),
			2.0f*x2*std::sqrt(1.0f-m),
			1.0f-2.0f*m
		};
	}

	inline double DragForce(double speed, double strength, double density) {
		//return -0.5f * speed*speed * strength * density;
		return -speed * strength * density;
	}
	inline Eigen::Vector3d DragForce(Eigen::Vector3d v, double strength, double density=1.0f) {
		return v.normalized()*DragForce(v.norm(), strength, density);
	}
}
}