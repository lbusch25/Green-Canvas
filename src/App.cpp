#include "App.h"
#include <iostream>

namespace basicgraphics{

using namespace std;
using namespace glm;

App::App(int argc, char** argv, std::string windowName, int windowWidth, int windowHeight) : BaseApp(argc, argv, windowName, windowWidth, windowHeight) {

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

	mouseDown = false;

	//sphere.reset(new Sphere(vec3(0), 1.0, vec4(1.0)));
    //grass.reset(new Grass(vec3(0)));
	//windSim.reset(new FluidSimulator(10, 10, 10));
	//windSim->addVelocitySource(0, 0, 0, vec3(1, 1, 1));

	lastTime = glfwGetTime();
    
    diffuseRamp = Texture::create2DTextureFromFile("lightingToon.jpg");
    diffuseRamp->setTexParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    diffuseRamp->setTexParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
//    specularRamp = Texture::create2DTextureFromFile("lightingToon.jpg");
//    specularRamp->setTexParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    specularRamp->setTexParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
  //  //Generates a grass square centered around the origin
  //  for (int i = 0; i <= GRASS_PER_SQUARE_EDGE; i ++) {
		//for (int j = 0; j <= GRASS_PER_SQUARE_EDGE; j++) {
		//	vec3 grassPos = vec3(-1, 0, -1);
		//	grassPos.z += (i * 1.0/GRASS_PER_SQUARE_EDGE);       
		//	grassPos.x += (j * 1.0/GRASS_PER_SQUARE_EDGE); //Do every x for z so its stored in row major order
  //      
		//	Grass *grassBlade = new Grass(grassPos); 
		//	grassBlades.emplace_back(std::move(grassBlade)); //std::move moves the ptr to this new vector
		//}
  //  }

	//Grass *grassBlade = new Grass(vec3(0));
	//grassBlades.emplace_back(std::move(grassBlade)); //std::move moves the ptr to this new vector

	generatePoissonPts();
}

void App::generatePoissonPts() {
	//Initialize grid
	for (int x = 0; x < GRID_WIDTH; x++) {
		poissonGrid.push_back(std::vector<vec2>());
		for (int y = 0; y < GRID_HEIGHT; y++) {
			poissonGrid[x].push_back(NO_PT);
		}
	}

	//Based on the implmentation by Mike Bostock https://bl.ocks.org/mbostock/19168c663618b7f07158
	float RADIUS_SQUARED = POISSON_RADIUS * POISSON_RADIUS;
	//Initial sample
	std::vector<vec2> activeSamples;
	activeSamples.push_back(vec2(0));
	poissonGrid[0][0] = activeSamples[0];

	while (activeSamples.size() > 0) {
		//Pick a random sample
		int randIndex = rand() % activeSamples.size();
		vec2 sample = activeSamples[randIndex];

		vec2 newSample = NO_PT;
		for (int c = 0; c < MAX_CANDIDATES; c++) {
			float angle = glm::linearRand(0.0, 2 * 3.14159265358979323846);
			float radius = sqrt(glm::linearRand(0.0f, 3 * RADIUS_SQUARED) + RADIUS_SQUARED);
			vec2 cand = sample + (radius * vec2(cos(angle), sin(angle)));

			//Reject if outside bounds
			if (cand.x < 0 || cand.y < 0 || cand.x >= POISSON_WIDTH || cand.y >= POISSON_HEIGHT) {
				continue;
			}

			//Reject if too close to other points
			i32vec2 gridPos = floor(cand / CELL_SIZE);
			int minX = std::max(gridPos.x - 2, 0);
			int minY = std::max(gridPos.y - 2, 0);
			int maxX = std::min(gridPos.x + 3, GRID_WIDTH);
			int maxY = std::min(gridPos.y + 3, GRID_HEIGHT);

			bool reject = false;
			for (int y = minY; y < maxY; y++) {
				for (int x = minX; x < maxX; x++) {
					vec2 checkPt = poissonGrid[x][y];
					if (checkPt != NO_PT) {
						if (length(checkPt - cand) < POISSON_RADIUS) {
							reject = true;
							break;
						}
					}
				}
				if (reject)
					break;
			}
			if (reject)
				continue;

			//We're good - add the point 
			newSample = cand;
			break;
		}
		if (newSample == NO_PT) {
			activeSamples.erase(activeSamples.begin() + randIndex);
		}
		else {
			activeSamples.push_back(newSample);
			i32vec2 gridPos = floor(newSample / CELL_SIZE);
			poissonGrid[gridPos.x][gridPos.y] = newSample;
		}
	}
}

void App::onEvent(shared_ptr<Event> event) {
	if (event->getName() == "mouse_pointer") {
		mousePos = vec2(event->get2DData());
		if (mouseDown) {
			int mouseGridX = floor((pt.x + (POISSON_WIDTH / 2)) / CELL_SIZE);
			int mouseGridY = floor((pt.z + (POISSON_HEIGHT / 2)) / CELL_SIZE);
			int gridRadius = ceil(BRUSH_RADIUS / CELL_SIZE);
			int minX = std::max(mouseGridX - gridRadius, 0);
			int minY = std::max(mouseGridY - gridRadius, 0);
			int maxX = std::min(mouseGridX + gridRadius, GRID_WIDTH - 1);
			int maxY = std::min(mouseGridY + gridRadius, GRID_HEIGHT - 1);
			for (int x = minX; x <= maxX; x++) {
				for (int y = minY; y <= maxY; y++) {
					if (poissonGrid[x][y] != NO_PT) {
						vec2 shiftedPos = poissonGrid[x][y] - vec2(POISSON_WIDTH / 2, POISSON_HEIGHT / 2);
						vec3 actualPos = vec3(shiftedPos.x, 0.0, shiftedPos.y);
						if (length(actualPos - pt) < BRUSH_RADIUS) {
							Grass *userBlade = new Grass(actualPos, glm::linearRand(0.0f, 2 * 3.14159265f));
							userGrass.emplace_back(std::move(userBlade)); //std::move moves the ptr to this new vector
							poissonGrid[x][y] = NO_PT;
						}
					}
				}
			}
		}
    }
	if (event-> getName() == "mouse_btn_left_down") {
		mouseDown = true;
    }
	if (event->getName() == "mouse_btn_left_up") {
		mouseDown = false;
	}
}

vec3 App::mousePosToRay(mat4 view, mat4 projection) {
	vec2 normalizedDeviceCoords((2 * mousePos.x) / (float)_windowWidth - 1,
							  -((2 * mousePos.y) / (float)_windowHeight - 1));
	vec4 rayPt = vec4(normalizedDeviceCoords, -1.0, 1.0);
	vec4 camCoords = inverse(projection) * rayPt;
	camCoords = vec4(camCoords.x, camCoords.y, -1.0, 0.0);

	vec3 worldDir = vec3(inverse(view) * camCoords);
	return normalize(worldDir);
}

void App::onRenderGraphics() {
	double curTime = glfwGetTime();
	double dt = curTime - lastTime;
	lastTime = curTime;

	//windSim->step(dt); 
	
	vec3 eye_world(0,8,8);
    // Setup the camera with a good initial position and view direction to see the table
    glm::mat4 view = glm::lookAt(eye_world, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    
    // Setup the projection matrix so that things are rendered in perspective
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)_windowWidth / (GLfloat)_windowHeight, 0.1f, 100.0f);
    
    // Setup the model matrix
    glm::mat4 model(1.0);
    
    // Update shader variables
    _shader.setUniform("view_mat", view);
    _shader.setUniform("projection_mat", projection);
    _shader.setUniform("model_mat", model);
    _shader.setUniform("eye_world", eye_world);
    
    diffuseRamp->bind(0); //Binds our diffuse texture to our first texture
    _shader.setUniform("diffuseRamp", 0);
//    specularRamp->bind(1);
//    _shader.setUniform("specularRamp", 1);
    
    //_shader.setUniform("normal_mat", mat3(transpose(inverse(model)))); //Gives the normal in world coords

    //Properties of our grass surface, we can adjust these for a more realistic grass later
    vec3 ambientReflectionCoeff = vec3(1,1,1);
    vec3 diffuseReflectionCoeff = vec3(1,1,1);
//    vec3 specularReflectionCoeff = vec3(1,1,1);
//    float specularExponent = 50.0;
    
    //Intensity for our white light
    vec3 ambientLightIntensity = vec3(0.4, 0.4, 0.4);
    vec3 diffuseLightIntensity = vec3(0.6, 0.6, 0.6);
//    vec3 specularLightIntensity = vec3(1.0, 1.0, 1.0);
    
    //Light position is defined in App.h as a constant vec4
    _shader.setUniform("lightPosition", lightPosition);
    
    _shader.setUniform("ambientReflection", ambientReflectionCoeff);
    _shader.setUniform("diffuseReflection", diffuseReflectionCoeff);
//    _shader.setUniform("specularReflection", specularReflectionCoeff);
//    _shader.setUniform("specularExp", specularExponent);
    
    _shader.setUniform("ambientIntensity", ambientLightIntensity);
    _shader.setUniform("diffuseIntensity", diffuseLightIntensity);
//    _shader.setUniform("specularIntensity", specularLightIntensity);
    
    
	//Draw a sphere where the user's mouse is.
	//Credit: http://antongerdelan.net/opengl/raycasting.html

	//Generate ray
	vec3 worldRay = mousePosToRay(view, projection);

	//Calculate ray intesection with y=0 plane
	float t = -eye_world.y / worldRay.y;
	pt = eye_world + (worldRay * t);
	pt.y = 0;

	//Draw sphere at point
	//sphere->draw(_shader, translate(mat4(1.0), pt) * scale(mat4(1.0), vec3(0.1)));
    //grass.reset(new Grass(pt, vec3(0)));
    //grass->draw(_shader);
    
    //Draws our grass square 
    //for(int i = 0; i < grassBlades.size(); i++) {
    //    //grassBlades[i]->doPhysicsStuff(vec3(1, 0, 0), dt);
    //    grassBlades[i]->draw(_shader);
    //}
    
    for(int i = 0; i < userGrass.size(); i++) {
        //userGrass[i]->doPhysicsStuff(vec3(0.1, 0, 0.1), dt);
        userGrass[i]->draw(_shader);
    }
}
}//namespace




