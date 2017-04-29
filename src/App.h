#ifndef App_h
#define App_h

#include "BaseApp.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "Grass.hpp"
#include "FluidSimulator.h"

namespace basicgraphics {
class App : public BaseApp {
public:
  
    App(int argc, char** argv, std::string windowName, int windowWidth, int windowHeight);
    ~App(){};
  
    void onRenderGraphics() override;
    void onEvent(std::shared_ptr<Event> event) override;

  
protected:
    const int GRASS_PER_SQUARE_EDGE = 1;
    
	vec3 mousePosToRay(mat4 view, mat4 projection);

	vec2 mousePos;
    
    vec3 pt;
    
    const vec4 lightPosition = vec4(-20, 20, -20, 1.0);
    
    std::vector<std::unique_ptr<Grass>> grassBlades;
    
    std::vector<std::unique_ptr<Grass>> userGrass;
    
    std::shared_ptr<Texture> diffuseRamp;
    
    std::shared_ptr<Texture> specularRamp;

	std::unique_ptr<Sphere> sphere;
    
    std::unique_ptr<Grass> grass;

	std::unique_ptr<FluidSimulator> windSim;

	double lastTime;

	bool mouseDown;
};
}
#endif
