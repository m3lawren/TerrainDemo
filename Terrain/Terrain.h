#ifndef __TERRAIN_TERRAIN_H__
#define __TERRAIN_TERRAIN_H__

namespace Terrain {

	class Terrain {
	public:
		Terrain();
		~Terrain();

		float height(float x, float y);

	private:
		Terrain(const Terrain&);
		Terrain& operator=(const Terrain&);
	};

};

#endif //__TERRAIN_TERRAIN_H__