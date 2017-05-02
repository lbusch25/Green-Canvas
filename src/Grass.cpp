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
#include "GrassMesh.hpp"

namespace basicgraphics {
    using namespace glm;
    using namespace std;
    
    //Takes in the position of the bottom point of the grass stalk and the direction of the wind
    //vector that will be acting upon the grass. The grass is then built up directly vertical as
    //a four point line strip from the input point.
    Grass::Grass(vec3 position) {
        
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
            GrassMesh::Vertex currentVert; //The vertex we are working on
            
            currentVert.position = pointPositions[i];
            currentVert.wWithoutTwist = vec3(0, 0, 1);
			currentVert.wWithTwist = currentVert.wWithoutTwist;
			currentVert.velocity = vec3(0);
			currentVert.stiffness = 0.5;
            
            cpuVertexArray.push_back(currentVert);
            cpuIndexArray.push_back(i);
        }
        
        GrassMesh::Vertex lastVert; //The end vertex of our grass blade
        
        lastVert.position = pointPositions[pointPositions.size() - 1];

		lastVert.wWithoutTwist = vec3(0, 0, 1);
		lastVert.wWithTwist = lastVert.wWithoutTwist;
		lastVert.velocity = vec3(0);
		lastVert.stiffness = 0.5;

        cpuVertexArray.push_back(lastVert);
        cpuIndexArray.push_back(cpuIndexArray.size());
        
        const int cpuVertexByteSize = sizeof(GrassMesh::Vertex) * cpuVertexArray.size();
        const int cpuIndexByteSize = sizeof(int) * cpuIndexArray.size();
        
        //Going to want to use GL_DYNAMIC_DRAW for updating since we are
        //regularly updated the mesh on the GPU
        _mesh.reset(new GrassMesh(GL_LINE_STRIP, GL_DYNAMIC_DRAW, cpuVertexByteSize, cpuIndexByteSize, 0, cpuVertexArray, cpuIndexArray.size(), cpuIndexByteSize, &cpuIndexArray[0]));
        
		std::copy(cpuVertexArray.begin(), cpuVertexArray.end(), controlPoints);
		std::copy(cpuVertexArray.begin(), cpuVertexArray.end(), staticStateControlPoints);
    }
    
    Grass::~Grass() {};

	void Grass::doPhysicsStuff(vec3 velocityAtTip) {
		float areaOfThrustSurfaceOfWind = 1;
		float dragCoefficient = 1;
		GrassMesh::Vertex tip = controlPoints[3];

		vec3 staticGrowthVector = staticStateControlPoints[3].position - staticStateControlPoints[0].position;
		vec3 growthVec = controlPoints[3].position - controlPoints[0].position;
		growthVec.y = 0; //project
		float growthVecAngularDisp = acos(dot(normalize(growthVec), normalize(staticGrowthVector)));

		//Bending angular displacement
		float staticBendAngularDisp = asin(normalize(staticStateControlPoints[3].position - staticStateControlPoints[2].position).y);
		float currentBendAngularDisp = asin(normalize(controlPoints[3].position - controlPoints[2].position).y);

		float currentBendAngularTwist = acos(dot(normalize(staticStateControlPoints[3].wWithTwist), normalize(controlPoints[3].wWithTwist)));
		
		//Start at 1 because the root point can't swing
		for (int edge = 0; edge < 3; edge++) {
			GrassMesh::Vertex edgePtLower = controlPoints[edge];
			GrassMesh::Vertex edgePtHigher = controlPoints[edge + 1];
			GrassMesh::Vertex edgePtLowerStatic = staticStateControlPoints[edge];
			GrassMesh::Vertex edgePtHigherStatic = staticStateControlPoints[edge + 1];

			////////// Swinging //////////
			// Wind force
			vec3 projectedSwingVel = dot(velocityAtTip, edgePtHigher.wWithoutTwist) * edgePtHigher.wWithoutTwist;
			vec3 windForceSwinging = areaOfThrustSurfaceOfWind * dragCoefficient * projectedSwingVel;

			// Restoration force
			float growthVecAnglularDispAdj = (tip.stiffness / edgePtHigher.stiffness) * growthVecAngularDisp;
			vec3 restorationForceSwing = tip.stiffness * growthVecAnglularDispAdj * normalize(staticGrowthVector - growthVec);

			// Total
			vec3 totalSwingForce = windForceSwinging + restorationForceSwing;

			////////// Bending //////////
			vec3 staticEdgeVec = edgePtLowerStatic.position - edgePtHigherStatic.position;
			vec3 edgeVec = edgePtLower.position - edgePtHigher.position;
			vec3 edgeNormalWithoutTwist = cross(edgePtLower.wWithoutTwist, edgeVec);
			bool windIsTowardsNormal = dot(velocityAtTip, edgeNormalWithoutTwist) > 0;

			// Wind force
			vec3 projectedBendVel = dot(velocityAtTip, edgeNormalWithoutTwist) * edgeNormalWithoutTwist;
			vec3 windForceBending = areaOfThrustSurfaceOfWind * dragCoefficient * projectedBendVel;

			// Restoration force
			float currentBendAngularDispAdj = (tip.stiffness / edgePtHigher.stiffness) * currentBendAngularDisp;		
			vec3 restorationForceBending = tip.stiffness * growthVecAnglularDispAdj * normalize(staticEdgeVec - edgeVec);

			// Total
			vec3 totalBendForce = windForceBending + restorationForceBending;

			////////// Twisting //////////
			vec3 edgeNormalWithTwist = cross(edgePtLower.wWithTwist, edgeVec);

			// Wind force
			vec3 projectedTwistVel = dot(velocityAtTip, edgeNormalWithTwist) * edgeNormalWithTwist;
			vec3 windForceTwist = areaOfThrustSurfaceOfWind * dragCoefficient * projectedTwistVel;
			
			// Restoration force
			float currentTwistAngularDispAdj = (tip.stiffness / edgePtHigher.stiffness) * currentBendAngularTwist;
			vec3 restorationForceTwist = tip.stiffness * currentTwistAngularDispAdj * normalize(edgePtHigher.wWithoutTwist - edgePtHigher.wWithTwist);

			// Total
            vec3 totalTwistForce = windForceTwist + restorationForceTwist;
			////////// Finally... //////////
			//do total force
			//update vel
			//update pos
			//update normal, w
		}
	}
    
    void Grass::draw(GLSLProgram &shader) {

		glBindVertexArray(_mesh->getVAOID());
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawElements(GL_PATCHES, _mesh->getNumIndices(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
    }
    
    std::shared_ptr<GrassMesh> Grass::getMesh() {
        return _mesh;
    }
    
    std::vector<GrassMesh::Vertex> Grass::getVertArray() {
        return cpuVertexArray;
    }
}
