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
#include "GrassMesh.hpp"

#include <glm/glm/gtc/matrix_transform.hpp>


namespace basicgraphics {
    class Grass {
    public:
        Grass(glm::vec3 position);
        ~Grass();
        
        void draw(GLSLProgram &shader);
		void doPhysicsStuff(vec3 velocityAtTip, float dt);
        
        std::shared_ptr<GrassMesh> getMesh();
        
        std::vector<GrassMesh::Vertex> getVertArray();
        
    protected:
        std::shared_ptr<GrassMesh> _mesh;
        std::shared_ptr<glm::vec4> _color;
        
        std::vector<GrassMesh::Vertex> cpuVertexArray;
        std::vector<int> cpuIndexArray;

        //The control points to be used and updated when a wind force is applied
		GrassMesh::Vertex controlPoints[4];
        
        //The default status of the grass, use when no wind force applied
		GrassMesh::Vertex staticStateControlPoints[4];

		const float massOfABladeOfGrass = 0.01;

		void calcSwinging(vec3 windVelocity, float(&angularAcc)[3]);
		void calcBending(vec3 windVelocity, float(&acceleration)[3]);
		void calcBendingCustomTip(vec3 windVelocity, int tipEdge, int nextTipEdge, float(&acceleration)[3]);
		void calcTwisting(vec3 windVelocity, float(&angularAcc)[3]);
		
		float angularAccFromTorque(vec3 radiusVec, vec3 force);
    };
}

#endif /* Grass_hpp */
