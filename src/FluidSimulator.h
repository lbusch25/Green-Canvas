#ifndef FluidSimulator_h
#define FluidSimulator_h

#include <glm/glm/glm.hpp>
#include <vector>

namespace basicgraphics {
	class FluidSimulator {
	public:
		FluidSimulator(int xSize, int ySize, int zSize);
		void step(float dt);
		void addVelocitySource(int x, int y, int z, glm::vec3 dir);
		void addDensitySource(int x, int y, int z, float amount);

		glm::vec3 velocityAt(int x, int y, int z);
		glm::vec3 sampleVelocity(glm::vec3 coord);

	private:
		int X_SIZE;
		int Y_SIZE;
		int Z_SIZE;

		const float DIFFUSION_RATE = 0.5f;

		std::vector<glm::vec3> velocities;
		std::vector<float> densities;
		
		std::vector<glm::vec3> velocitySources;
		std::vector<float> densitySources;

		template<typename T>
		T sampleBilinear(std::vector<T> data, glm::vec3 pos);

		bool isValidCoord(int x, int y, int z);
		int index(int x, int y, int z);
		int indexWithWrap(int x, int y, int z);
		
		template<typename T>
		void addSources(std::vector<T>& prev, std::vector<T>& sources, float dt);

		template<typename T>
		void diffuse(std::vector<T>& base, std::vector<T>& prev, float dt);

		template<typename T>
		void advect(std::vector<T>& outAdvected, std::vector<T>& prev, std::vector<glm::vec3>& vels, float dt);

		void project(std::vector<glm::vec3>& vels);
	};
}

#endif