#include "Terrain.h"

#include <cmath>
#include <cstdlib>

#undef M_PI
#define M_PI 3.141592653589793238462643f

static float rfloat() {
	return (float)rand() / (float)RAND_MAX;
}

namespace Terrain {

	Terrain::Terrain() {
		for (unsigned i = 0; i < _T_NUM_WEIGHTS; i++) {
			_weights[i][0] = rfloat() * 7.0f - 3.5f;
			_weights[i][1] = rfloat() * 7.0f - 3.5f;
			_weights[i][2] = rfloat() * 3.0f;
			_weights[i][3] = rfloat() * M_PI * 2.0f + 0.5f;
		}
	}

	Terrain::~Terrain() {
	}

	float Terrain::height(float x, float y) {
		float r = 0.0f;
		float sw = 0.0f;
		for (int i = 0; i < _T_NUM_WEIGHTS; i++) {
			r += _weights[i][2] * sin(_weights[i][0] * x + _weights[i][1] * y + _weights[i][3]);
			sw += _weights[i][2];
		}
		return 2.0f * r / sw;
	}

}