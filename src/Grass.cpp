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

	void Grass::doPhysicsStuff(vec3 velocityAtTip) {
		float areaOfThrustSurfaceOfWind = 1;
		float dragCoefficient = 1;
		GrassControlPoint tip = controlPoints[3];

		vec3 staticGrowthVector = staticStateControlPoints[3].pos - staticStateControlPoints[0].vel;
		vec3 growthVec = controlPoints[3].pos - controlPoints[0].pos;
		growthVec.y = 0; //project
		float growthVecAngularDisp = acos(dot(normalize(growthVec), normalize(staticGrowthVector)));

		//Bending angular displacement
		float staticBendAngularDisp = asin(normalize(staticStateControlPoints[3].pos - staticStateControlPoints[2].pos).y);
		float currentBendAngularDisp = asin(normalize(controlPoints[3].pos - controlPoints[2].pos).y);

		float currentBendAngularTwist = acos(dot(normalize(staticStateControlPoints[3].wWithTwist), normalize(controlPoints[3].wWithTwist)));
		
		//Start at 1 because the root point can't swing
		for (int edge = 0; edge < 3; edge++) {
			GrassControlPoint edgePtLower = controlPoints[edge];
			GrassControlPoint edgePtHigher = controlPoints[edge + 1];
			GrassControlPoint edgePtLowerStatic = staticStateControlPoints[edge];
			GrassControlPoint edgePtHigherStatic = staticStateControlPoints[edge + 1];

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
			vec3 staticEdgeVec = edgePtLowerStatic.pos - edgePtHigherStatic.pos;
			vec3 edgeVec = edgePtLower.pos - edgePtHigher.pos;
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
}
