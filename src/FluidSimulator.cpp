#include "FluidSimulator.h"

using namespace glm;

namespace basicgraphics {

	FluidSimulator::FluidSimulator(int xSize, int ySize, int zSize) : X_SIZE(xSize), Y_SIZE(ySize), Z_SIZE(zSize) {
		velocities = std::vector<glm::vec3>(X_SIZE * Y_SIZE * Z_SIZE);
		densities = std::vector<float>(X_SIZE * Y_SIZE * Z_SIZE);

		velocitySources = std::vector<glm::vec3>(X_SIZE * Y_SIZE * Z_SIZE);
		densitySources = std::vector<float>(X_SIZE * Y_SIZE * Z_SIZE);
	}

	//bool FluidSimulator::isEdge(int x, int y, int z) {
	//	return x == 0 || y == 0 || z == 0 || x == X_SIZE-1 || y == Y_SIZE-1 || z < Z_SIZE-1;
	//}

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
		//if (isEdge(x, y, z))
		//	throw std::exception();
		velocitySources[index(x, y, z)] += dir;
	}
	void FluidSimulator::addDensitySource(int x, int y, int z, float amount) {
		//if (isEdge(x, y, z))
		//	throw std::exception();
		densitySources[index(x, y, z)] += amount;
	}

	vec3 FluidSimulator::velocityAt(int x, int y, int z) {
		return velocities[index(x, y, z)];
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
			//if (typeid(T) == typeid(glm::vec3)) {
			//	setBounds(initialGuess);
			//}
		}
	}

	template<typename T>
	void FluidSimulator::advect(std::vector<T>& outAdvected, std::vector<T>& prev, std::vector<glm::vec3>& vels, float dt) {

		vec3 dtAdj = dt * vec3(max(X_SIZE, max(Y_SIZE, Z_SIZE)));

		for (int x = 0; x < X_SIZE; x++) {
			for (int y = 0; y < Y_SIZE; y++) {
				for (int z = 0; z < Z_SIZE; z++) {
					vec3 prevPos = vec3(x, y, z) - (dtAdj*vels[index(x, y, z)]);

					int x0 = int(prevPos.x), x1 = x0 + 1;
					int y0 = int(prevPos.y), y1 = y0 + 1;
					int z0 = int(prevPos.z), z1 = z0 + 1;
					float xMix = x1 - prevPos.x;
					float yMix = y1 - prevPos.y;
					float zMix = z1 - prevPos.z;
					
					//Mix together the 8 values
					T interpolated =
						glm::mix(
							glm::mix(
								glm::mix(
									prev[indexWithWrap(x0, y0, z0)],
									prev[indexWithWrap(x0, y0, z1)],
									zMix
								),
								glm::mix(
									prev[indexWithWrap(x0, y1, z0)],
									prev[indexWithWrap(x0, y1, z1)],
									zMix
								),
								yMix
							),
							glm::mix(
								glm::mix(
									prev[indexWithWrap(x1, y0, z0)],
									prev[indexWithWrap(x1, y0, z1)],
									zMix
								),
								glm::mix(
									prev[indexWithWrap(x1, y1, z0)],
									prev[indexWithWrap(x1, y1, z1)],
									zMix
								),
								yMix
							),
							xMix
						);
					outAdvected[index(x, y, z)] = interpolated;
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
			//if (typeid(T) == typeid(glm::vec3)) {
			//	setBounds(initialGuess);
			//}
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

	//void FluidSimulator::setBounds(std::vector<glm::vec3> velocityData) {
	//	//any edge parts not set will be 0
	//	
	//	for (int x = 1; x < X_SIZE - 1; x++) {
	//		for (int y = 1; y < Y_SIZE - 1; y++) {
	//			velocityData[index(x, y, 0)].x = velocityData[index(x, y, 1)].x;
	//			velocityData[index(x, y, 0)].y = velocityData[index(x, y, 1)].y;

	//			velocityData[index(x, y, Z_SIZE - 1)].x = velocityData[index(x, y, Z_SIZE - 2)].x;
	//			velocityData[index(x, y, Z_SIZE - 1)].y = velocityData[index(x, y, Z_SIZE - 2)].y;
	//		}
	//	}

	//	for (int x = 1; x < X_SIZE - 1; x++) {
	//		for (int z = 1; z < Z_SIZE - 1; z++) {
	//			velocityData[index(x, 0, z)].x = velocityData[index(x, 1, z)].x;
	//			velocityData[index(x, 0, z)].z = velocityData[index(x, 1, z)].z;

	//			velocityData[index(x, Y_SIZE - 1, z)].x = velocityData[index(x, Y_SIZE - 2, z)].x;
	//			velocityData[index(x, Y_SIZE - 1, z)].z = velocityData[index(x, Y_SIZE - 2, z)].z;
	//		}
	//	}

	//	for (int y = 1; y < Y_SIZE - 1; y++) {
	//		for (int z = 1; z < Z_SIZE - 1; z++) {
	//			velocityData[index(0, y, z)].y = velocityData[index(1, y, z)].y;
	//			velocityData[index(0, y, z)].z = velocityData[index(1, y, z)].z;

	//			velocityData[index(X_SIZE - 1, y, z)].y = velocityData[index(X_SIZE - 2, y, z)].y;
	//			velocityData[index(X_SIZE - 1, y, z)].z = velocityData[index(X_SIZE - 2, y, z)].z;
	//		}
	//	}

	//	for (int x = 1; x < X_SIZE - 1; x++) {
	//		velocityData[index(x, 0, 0)].x = velocityData[index(x, 1, 1)].x;
	//		velocityData[index(x, Y_SIZE - 1, 0)].x = velocityData[index(x, Y_SIZE - 2, 1)].x;
	//		velocityData[index(x, 0, Z_SIZE - 1)].x = velocityData[index(x, 0, Z_SIZE - 2)].x;
	//		velocityData[index(x, Y_SIZE - 1, Z_SIZE - 1)].x = velocityData[index(x, Y_SIZE - 2, Z_SIZE - 2)].x;
	//	}

	//	for (int y = 1; y < Y_SIZE - 1; y++) {
	//		velocityData[index(0, y, 0)].y = velocityData[index(1, y, 1)].y;
	//		velocityData[index(X_SIZE - 1, y, 0)].y = velocityData[index(X_SIZE - 2, y, 1)].y;
	//		velocityData[index(0, y, Z_SIZE - 1)].y = velocityData[index(1, y, Z_SIZE - 2)].y;
	//		velocityData[index(X_SIZE - 1, y, Z_SIZE - 1)].y = velocityData[index(X_SIZE - 2, y, Z_SIZE - 2)].y;
	//	}

	//	for (int z = 1; z < Z_SIZE - 1; z++) {
	//		velocityData[index(0, 0, z)].z = velocityData[index(1, 1, z)].z;
	//		velocityData[index(x, Y_SIZE - 1, 0)].z = velocityData[index(x, Y_SIZE - 2, 1)].z;
	//		velocityData[index(x, 0, Z_SIZE - 1)].z = velocityData[index(x, 0, Z_SIZE - 2)].z;
	//		velocityData[index(x, Y_SIZE - 1, Z_SIZE - 1)].z = velocityData[index(x, Y_SIZE - 2, Z_SIZE - 2)].z;
	//	}
	//}
}