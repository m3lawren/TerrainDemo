#ifndef __TERRAIN_TERRAIN_H__
#define __TERRAIN_TERRAIN_H__

#define _T_NUM_WEIGHTS 55

namespace Terrain {

	class Terrain {
	public:
		Terrain();
		~Terrain();

		float height(float x, float y);

	private:
		Terrain(const Terrain&);
		Terrain& operator=(const Terrain&);

		// xsc, ysc, osc, off
		float _weights[_T_NUM_WEIGHTS][4];
	};

};

#endif //__TERRAIN_TERRAIN_H__