#include "FluidSimulator.h"

using namespace glm;

namespace basicgraphics {

	FluidSimulator::FluidSimulator(int xSize, int ySize, int zSize) : X_SIZE(xSize), Y_SIZE(ySize), Z_SIZE(zSize) {
		velocities = std::vector<glm::vec3>(X_SIZE * Y_SIZE * Z_SIZE);
		densities = std::vector<float>(X_SIZE * Y_SIZE * Z_SIZE);

		velocitySources = std::vector<glm::vec3>(X_SIZE * Y_SIZE * Z_SIZE);
		densitySources = std::vector<float>(X_SIZE * Y_SIZE * Z_SIZE);
	}

	bool FluidSimulator::isValidCoord(int x, int y, int z) {
		return x >= 0 && y >= 0 && z >= 0 && x < X_SIZE && y < Y_SIZE && z < Z_SIZE;
	}

	//Converts (x, y, z) to index in the vectors
	int FluidSimulator::index(int x, int y, int z)
	{
		if (!isValidCoord(x, y, z))
			throw std::exception("index out of range");

		return x + (y * X_SIZE) + (z * (X_SIZE + Y_SIZE));
	}

	int FluidSimulator::indexWithWrap(int x, int y, int z) {
		int xw = ((x % X_SIZE) + X_SIZE) % X_SIZE;
		int yw = ((y % Y_SIZE) + Y_SIZE) % Y_SIZE;
		int zw = ((z % Z_SIZE) + Z_SIZE) % Z_SIZE;
		return index(xw, yw, zw);
	}

	void FluidSimulator::addVelocitySource(int x, int y, int z, glm::vec3 dir) {
		velocitySources[index(x, y, z)] += dir;
	}
	void FluidSimulator::addDensitySource(int x, int y, int z, float amount) {
		densitySources[index(x, y, z)] += amount;
	}

	vec3 FluidSimulator::velocityAt(int x, int y, int z) {
		return velocities[index(x, y, z)];
	}

	glm::vec3 FluidSimulator::sampleVelocity(glm::vec3 coord) {
		return sampleBilinear(velocities, coord);
	}

	template<typename T>
	T FluidSimulator::sampleBilinear(std::vector<T> data, glm::vec3 pos) {
		int x0 = int(pos.x), x1 = x0 + 1;
		int y0 = int(pos.y), y1 = y0 + 1;
		int z0 = int(pos.z), z1 = z0 + 1;
		float xMix = x1 - pos.x;
		float yMix = y1 - pos.y;
		float zMix = z1 - pos.z;
		
		T interpolated =
			glm::mix(
				glm::mix(
					glm::mix(
						data[indexWithWrap(x0, y0, z0)],
						data[indexWithWrap(x0, y0, z1)],
						zMix
					),
					glm::mix(
						data[indexWithWrap(x0, y1, z0)],
						data[indexWithWrap(x0, y1, z1)],
						zMix
					),
					yMix
				),
				glm::mix(
					glm::mix(
						data[indexWithWrap(x1, y0, z0)],
						data[indexWithWrap(x1, y0, z1)],
						zMix
					),
					glm::mix(
						data[indexWithWrap(x1, y1, z0)],
						data[indexWithWrap(x1, y1, z1)],
						zMix
					),
					yMix
				),
				xMix
			);
		return interpolated;
	}

	void FluidSimulator::step(float dt) {
		//Velocity
		addSources(velocities, velocitySources, dt);
		std::vector<glm::vec3> outDiffusedVels(X_SIZE * Y_SIZE * Z_SIZE);
		diffuse(outDiffusedVels, velocities, dt);
		project(outDiffusedVels);
		advect(velocities, outDiffusedVels, outDiffusedVels, dt);
		project(velocities);

		//Density
		addSources(densities, densitySources, dt);
		std::vector<float> outDiffusedDensities(X_SIZE * Y_SIZE * Z_SIZE);
		diffuse(outDiffusedDensities, densities, dt);
		advect(densities, outDiffusedDensities, velocities, dt); 
	}

	template<typename T>
	void FluidSimulator::addSources(std::vector<T>& prev, std::vector<T>& sources, float dt)
	{
		for (int i = 0; i < prev.size(); i++) {
			prev[i] += dt * sources[i];
		}
	}

	template<typename T>
	void FluidSimulator::diffuse(std::vector<T>& initialGuess, std::vector<T>& prev, float dt)
	{
		float a = dt * DIFFUSION_RATE * X_SIZE * Y_SIZE * Z_SIZE;
		float div = 1 + (6 * a);

		for (int k = 0; k < 20; k++) {
			for (int x = 0; x < X_SIZE; x++) {
				for (int y = 0; y < Y_SIZE; y++) {
					for (int z = 0; z < Z_SIZE; z++) {

						T neighborSum =
							initialGuess[indexWithWrap(x + 1, y, z)] +
							initialGuess[indexWithWrap(x - 1, y, z)] +
							initialGuess[indexWithWrap(x, y + 1, z)] +
							initialGuess[indexWithWrap(x, y - 1, z)] +
							initialGuess[indexWithWrap(x, y, z + 1)] +
							initialGuess[indexWithWrap(x, y, z - 1)];

						initialGuess[index(x, y, z)] = (prev[index(x, y, z)] + a*neighborSum) / div;
					}
				}
			}
		}
	}

	template<typename T>
	void FluidSimulator::advect(std::vector<T>& outAdvected, std::vector<T>& prev, std::vector<glm::vec3>& vels, float dt) {

		vec3 dtAdj = dt * vec3(max(X_SIZE, max(Y_SIZE, Z_SIZE)));

		for (int x = 0; x < X_SIZE; x++) {
			for (int y = 0; y < Y_SIZE; y++) {
				for (int z = 0; z < Z_SIZE; z++) {
					vec3 prevPos = vec3(x, y, z) - (dtAdj*vels[index(x, y, z)]);

					outAdvected[index(x, y, z)] = sampleBilinear(prev, prevPos);
				}
			}
		}
	}

	void FluidSimulator::project(std::vector<glm::vec3>& vels) {
		std::vector<float> p(X_SIZE*Y_SIZE*Z_SIZE);
		std::vector<float> div(X_SIZE*Y_SIZE*Z_SIZE);
		
		for (int x = 0; x < X_SIZE; x++) {
			for (int y = 0; y < Y_SIZE; y++) {
				for (int z = 0; z < Z_SIZE; z++) {
					float thingx = (vels[indexWithWrap(x + 1, y, z)].x - vels[indexWithWrap(x - 1, y, z)].x) / X_SIZE;
					float thingy = (vels[indexWithWrap(x, y + 1, z)].y - vels[indexWithWrap(x, y - 1, z)].y) / Y_SIZE;
					float thingz = (vels[indexWithWrap(x, y, z + 1)].z - vels[indexWithWrap(x, y, z - 1)].z) / Z_SIZE;

					div[index(x, y, z)] = -0.5 * thingx * thingy * thingz;
				}
			}
		}

		for (int k = 0; k < 20; k++) {
			for (int x = 0; x < X_SIZE; x++) {
				for (int y = 0; y < Y_SIZE; y++) {
					for (int z = 0; z < Z_SIZE; z++) {

						float neighborSum =
							p[indexWithWrap(x + 1, y, z)] +
							p[indexWithWrap(x - 1, y, z)] +
							p[indexWithWrap(x, y + 1, z)] +
							p[indexWithWrap(x, y - 1, z)] +
							p[indexWithWrap(x, y, z + 1)] +
							p[indexWithWrap(x, y, z - 1)];

							p[index(x, y, z)] = (div[index(x, y, z)] + neighborSum) / 6;
					}
				}
			}
		}

		for (int x = 0; x < X_SIZE; x++) {
			for (int y = 0; y < Y_SIZE; y++) {
				for (int z = 0; z < Z_SIZE; z++) {
					vels[index(x, y, z)].x -= 0.5*(p[indexWithWrap(x + 1, y, z)] - p[indexWithWrap(x - 1, y, z)]) * X_SIZE;
					vels[index(x, y, z)].y -= 0.5*(p[indexWithWrap(x, y + 1, z)] - p[indexWithWrap(x, y - 1, z)]) * Y_SIZE;
					vels[index(x, y, z)].z -= 0.5*(p[indexWithWrap(x, y, z + 1)] - p[indexWithWrap(x, y, z - 1)]) * X_SIZE;
				}
			}
		}
	}
}
