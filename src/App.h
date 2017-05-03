#ifndef App_h
#define App_h

#include "BaseApp.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "Grass.hpp"
#include "FluidSimulator.h"
#include <glm/glm/gtc/random.hpp>

namespace basicgraphics {
class App : public BaseApp {
public:
  
    App(int argc, char** argv, std::string windowName, int windowWidth, int windowHeight);
    ~App(){};
  
    void onRenderGraphics() override;
    void onEvent(std::shared_ptr<Event> event) override;

	void generatePoissonPts();
protected:
	vec3 mousePosToRay(mat4 view, mat4 projection);

	vec2 mousePos;   
    vec3 brushPos;
	const float BRUSH_RADIUS = 0.8;
    
    const vec4 lightPosition = vec4(-20, 20, -20, 1.0);
    
    std::vector<std::unique_ptr<Grass>> userGrass;
    
    std::shared_ptr<Texture> diffuseRamp;

	double lastTime;

	bool mouseDown;

	const float POISSON_RADIUS = 0.2;
	const float POISSON_WIDTH = 25;
	const float POISSON_HEIGHT = 25;

	const float CELL_SIZE = POISSON_RADIUS / sqrt(2);
	const int GRID_WIDTH = ceil(POISSON_WIDTH / CELL_SIZE);
	const int GRID_HEIGHT = ceil(POISSON_WIDTH / CELL_SIZE);

	const float MAX_CANDIDATES = 30;
	const vec2 NO_PT = vec2(INFINITE, INFINITE);

	std::vector<std::vector<vec2>> poissonGrid;
};
}
#endif
