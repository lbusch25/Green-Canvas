//
//  Grass.cpp
//  GreenCanvas
//
//  Created by Lawson Busch on 4/22/17.
//
//

#include "Grass.hpp"
#include <stdio.h>
#include "GLSLProgram.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "Mesh.h"

namespace basicgraphics {
    using namespace glm;
    using namespace std;
    
    Grass::Grass(vec3 position, vec3 windDirection) {
        std::shared_ptr<Texture> tex = Texture::create2DTextureFromFile("lightingToon.jpg");
        
        std::vector<Mesh::Vertex> cpuVertexArray;
        std::vector<int> cpuIndexArray;
        std::vector<std::shared_ptr<Texture>> textures;
        
        textures.push_back(tex);
        
        std::vector<vec3> pointPositions;
        pointPositions.push_back(position);
        for(int i = 1; i < 4; i++) {
            vec3 pointPosition = vec3(position.x, position.y + (i * 1/3 * _bladeLength), position.z);
            pointPositions.push_back(pointPosition);
        }
        
        for (int i = 0; i < pointPositions.size() - 1; i++) {
            Mesh::Vertex currentVert;
            Mesh::Vertex nextVert;
            
            currentVert.position = pointPositions[i];
            nextVert.position = pointPositions[i + 1];
            
            currentVert.edgeVector = normalize(nextVert.position - currentVert.position);
            currentVert.wVector = normalize(windDirection - dot(windDirection, currentVert.edgeVector)*windDirection);
            currentVert.normal = cross(currentVert.wVector, currentVert.edgeVector);
            
            cpuVertexArray.push_back(currentVert);
            cpuIndexArray.push_back(i);
        }
        
        Mesh::Vertex lastVert;
        Mesh::Vertex previousVert = cpuVertexArray[cpuVertexArray.size() - 1];
        
        lastVert.position = pointPositions[pointPositions.size() - 1];
        lastVert.edgeVector = vec3(0, 0, 0);
        lastVert.wVector = normalize(windDirection -
                                     dot(windDirection, previousVert.edgeVector)*windDirection);
        cpuVertexArray.push_back(lastVert);
        cpuIndexArray.push_back(cpuIndexArray.size());
        
        const int cpuVertexByteSize = sizeof(Mesh::Vertex) * cpuVertexArray.size();
        const int cpuIndexByteSize = sizeof(int) * cpuIndexArray.size();
        
        _mesh.reset(new Mesh(textures, GL_LINE_STRIP, GL_STATIC_DRAW,
                             cpuVertexByteSize, cpuIndexByteSize, 0, cpuVertexArray,
                             cpuIndexArray.size(), cpuIndexByteSize, &cpuIndexArray[0]));
    }
    
    Grass::~Grass() {};
    
    void Grass::draw(GLSLProgram &shader) {
        _mesh->draw(shader);
    }
}
