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


namespace basicgraphics {
    class Grass {
    public:
        Grass(glm::vec3 position);
        ~Grass();
        
        void draw(GLSLProgram &shader);
		void doPhysicsStuff(vec3 velocityAtTip);
        
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

        const float _bladeLength = 1;
    };
}

#endif /* Grass_hpp */
