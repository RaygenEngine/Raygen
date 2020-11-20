#include <cmath>
extern "C" {
int Sum(int pLhs, int pRhs)
{
	float x = 2;
	// Garbage code that gets optimised on release mode, but costs on debug builds
	for (int i = 0; i < 1000000; ++i) {
		x = std::sqrt(x);
	}
	for (int i = 0; i < 1000000; ++i) {
		x = std::sqrt(x);
	}

	return pLhs + pRhs;
}
} /// extern C
