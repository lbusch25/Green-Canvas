//
//  Grass.hpp
//  GreenCanvas
//
//  Created by Lawson Busch on 4/22/17.
//
//

#ifndef Grass_hpp
#define Grass_hpp

#include <stdio.h>
#include "GLSLProgram.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "Mesh.h"


namespace basicgraphics {
    //Holds the data necessary to compute physics calculations on grass control points
	struct GrassControlPoint {
		vec3 pos;
		vec3 vel;
		vec3 normal;
		vec3 wVec;
		float stiffness;
	};
    class Grass {
    public:
        Grass(glm::vec3 position, glm::vec3 windDirection);
        ~Grass();
        
        void draw(GLSLProgram &shader);
		void doPhysicsStuff(vec3 velocityAtTip);
        
    protected:
        std::shared_ptr<Mesh> _mesh;
        std::shared_ptr<glm::vec4> _color;

        //The control points to be used and updated when a wind force is applied
		GrassControlPoint controlPoints[4];
        
        //The default status of the grass, use when no wind force applied
		GrassControlPoint staticStateControlPoints[4];
		vec3 staticGrowthVector;
        
        glm::vec3 _edgeDirection;
        const float _bladeLength = 1;
    };
}

#endif /* Grass_hpp */
