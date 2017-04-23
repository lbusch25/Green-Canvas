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
    class Grass {
    public:
        Grass(glm::vec3 position, glm::vec3 windDirection);
        ~Grass();
        
        void draw(GLSLProgram &shader);
        
    protected:
        std::shared_ptr<Mesh> _mesh;
        std::shared_ptr<glm::vec4> _color;
        
        glm::vec3 _edgeDirection;
        glm::vec3 _wDirection;
        const float _bladeLength = 1;
    };
}

#endif /* Grass_hpp */
