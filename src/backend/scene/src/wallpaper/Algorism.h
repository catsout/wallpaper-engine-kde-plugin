#pragma once

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
}
}