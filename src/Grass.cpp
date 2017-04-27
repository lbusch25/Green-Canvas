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
    
    //Takes in the position of the bottom point of the grass stalk and the direction of the wind
    //vector that will be acting upon the grass. The grass is then built up directly vertical as
    //a four point line strip from the input point.
    Grass::Grass(vec3 position, vec3 windDirection) {
        
        std::vector<Mesh::Vertex> cpuVertexArray;
        std::vector<int> cpuIndexArray;

        //This calculates the position of all four points in our grass mesh
        std::vector<vec3> pointPositions;
        pointPositions.push_back(position);
        
        vec3 position1 = position + vec3(0.25, 0.6, 0);
        pointPositions.push_back(position1);
        
        vec3 position2 = position + vec3(0.6, 0.85, 0);
        pointPositions.push_back(position2);
        
        vec3 position3 = position + vec3(0.75, 0.7, 0);
        pointPositions.push_back(position3);
        
        for (int i = 0; i < pointPositions.size() - 1; i++) {
            Mesh::Vertex currentVert; //The vertex we are working on
            Mesh::Vertex nextVert; //The next vertex up
            
            currentVert.position = pointPositions[i];
            nextVert.position = pointPositions[i + 1];
            
            //Calculates the vector from the current vertex to the next vertex up
            //Has to be normalized for the projection calculation below
            currentVert.edgeVector = normalize(nextVert.position - currentVert.position);
            
            //Calculates a normalized wind value by finding the amount of the wind that is in a
            //direction orthogonal to our edgeDirection (ie grass blade)
            currentVert.wVector = normalize(windDirection - dot(windDirection, currentVert.edgeVector)*windDirection);
            
            //Calculates the normal for our grass blade
            currentVert.normal = cross(currentVert.wVector, currentVert.edgeVector);
            
            cpuVertexArray.push_back(currentVert);
            cpuIndexArray.push_back(i);
        }
        
        Mesh::Vertex lastVert; //The end vertex of our grass blade
        Mesh::Vertex previousVert = cpuVertexArray[cpuVertexArray.size() - 1];
        
        lastVert.position = pointPositions[pointPositions.size() - 1];
        lastVert.edgeVector = vec3(0, 0, 0);
        lastVert.wVector = normalize(windDirection -
                                     dot(windDirection, previousVert.edgeVector)*windDirection);
        cpuVertexArray.push_back(lastVert);
        cpuIndexArray.push_back(cpuIndexArray.size());
        
        const int cpuVertexByteSize = sizeof(Mesh::Vertex) * cpuVertexArray.size();
        const int cpuIndexByteSize = sizeof(int) * cpuIndexArray.size();
        
        _mesh.reset(new Mesh(std::vector<std::shared_ptr<Texture>>(), GL_LINE_STRIP, GL_STATIC_DRAW,
                             cpuVertexByteSize, cpuIndexByteSize, 0, cpuVertexArray,
                             cpuIndexArray.size(), cpuIndexByteSize, &cpuIndexArray[0]));
    }
    
    Grass::~Grass() {};
    
    void Grass::draw(GLSLProgram &shader) {

		glBindVertexArray(_mesh->getVAOID());
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawElements(GL_PATCHES, _mesh->getNumIndices(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
    }
}
