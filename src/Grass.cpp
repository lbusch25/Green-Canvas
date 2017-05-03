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

#include <glm/glm/gtc/random.hpp>

namespace basicgraphics {
    using namespace glm;
    using namespace std;
    
    //Takes in the position of the bottom point of the grass stalk and the direction of the wind
    //vector that will be acting upon the grass. The grass is then built up directly vertical as
    //a four point line strip from the input point.
    Grass::Grass(vec3 position, float angle) {
        
        //This calculates the position of all four points in our grass mesh
        std::vector<vec3> pointPositions;
        pointPositions.push_back(vec3(0));
		
		float curAngle = glm::linearRand(radians(70.0f), radians(85.0f));
		float sectionLen = 0.4;

		for (int i = 1; i < 4; i++) {
			vec3 prevPt = pointPositions[i - 1];

			vec3 newVec = sectionLen * vec3(cos(curAngle), sin(curAngle), 0);

			pointPositions.push_back(prevPt + newVec);

			curAngle -= glm::linearRand(radians(5.0f), radians(30.0f));
		}

		mat4 rotMatrix = glm::rotate(mat4(1.0), angle, vec3(0, 1, 0));
        
        for (int i = 0; i < pointPositions.size(); i++) {
            GrassMesh::Vertex currentVert; //The vertex we are working on
            
            currentVert.position = position + vec3(rotMatrix * vec4(pointPositions[i], 1.0));
            //currentVert.wWithoutTwist = vec3(0, 0, 1);
			currentVert.wWithTwist = vec3(rotMatrix * vec4(0, 0, 1, 0));
			/*currentVert.swingVel = 0;
			currentVert.bendVel = 0;
			currentVert.twistVel = 0;
			currentVert.stiffness = 0.5;*/
            
            cpuVertexArray.push_back(currentVert);
            cpuIndexArray.push_back(i);
        }
        
        const int cpuVertexByteSize = sizeof(GrassMesh::Vertex) * cpuVertexArray.size();
        const int cpuIndexByteSize = sizeof(int) * cpuIndexArray.size();
        
        //Going to want to use GL_DYNAMIC_DRAW for updating since we are
        //regularly updated the mesh on the GPU
        _mesh.reset(new GrassMesh(GL_LINE_STRIP, GL_DYNAMIC_DRAW, cpuVertexByteSize, cpuIndexByteSize, 0, cpuVertexArray, cpuIndexArray.size(), cpuIndexByteSize, &cpuIndexArray[0]));
        
		std::copy(cpuVertexArray.begin(), cpuVertexArray.end(), controlPoints);
		std::copy(cpuVertexArray.begin(), cpuVertexArray.end(), staticStateControlPoints);
    }
    
    Grass::~Grass() {};

	void Grass::doPhysicsStuff(vec3 velocityAtTip, float dt) {

		float swingAngularAcc[3];
		calcSwinging(velocityAtTip, swingAngularAcc);
		float bendAngularAcc[3];
		calcBending(velocityAtTip, bendAngularAcc);
		float twistAngularAcc[3];
		calcTwisting(velocityAtTip, twistAngularAcc);

		vec3 base = controlPoints[0].position;
		//Don't need to modify the base
		for (int i = 3; i >= 1; i--) {
			controlPoints[i].swingVel += swingAngularAcc[i - 1] * dt;
			controlPoints[i].bendVel  +=  bendAngularAcc[i - 1] * dt;
			controlPoints[i].twistVel += twistAngularAcc[i - 1] * dt;

			//Swing
			mat4 swingTransform = glm::translate(mat4(1.0), base) *
							   	  glm::rotate(mat4(1.0), controlPoints[i].swingVel * dt, vec3(0, 1, 0)) *
								  glm::translate(mat4(1.0), -base);

			controlPoints[i].position = vec3(swingTransform * vec4(controlPoints[i].position, 1.0));
			controlPoints[i].wWithoutTwist = vec3(swingTransform * vec4(controlPoints[i].wWithoutTwist, 0.0));
			controlPoints[i].wWithTwist = vec3(swingTransform * vec4(controlPoints[i].wWithTwist, 0.0));

			//Bend
			vec3 lowerPt = controlPoints[i - 1].position;
			mat4 bendTransform = glm::translate(mat4(1.0), lowerPt) *
								 glm::rotate(mat4(1.0), controlPoints[i].bendVel * dt, controlPoints[i].wWithoutTwist) *
								 glm::translate(mat4(1.0), -lowerPt);

			controlPoints[i].position = vec3(bendTransform * vec4(controlPoints[i].position, 1.0));
			controlPoints[i].wWithTwist = vec3(bendTransform * vec4(controlPoints[i].wWithTwist, 0.0));

			//Twist
			vec3 edgeVec = controlPoints[i].position - lowerPt;
			mat4 twistTransform = glm::rotate(mat4(1.0), controlPoints[i].twistVel * dt, edgeVec);
			controlPoints[i].wWithTwist = vec3(twistTransform * vec4(controlPoints[i].wWithTwist, 0.0));

			//Should still be normal but better safe than sorry
			controlPoints[i].wWithTwist = normalize(controlPoints[i].wWithTwist);
			controlPoints[i].wWithoutTwist = normalize(controlPoints[i].wWithoutTwist);
		}
		_mesh->updateVertexData(0, 0, controlPoints);
	}

	void Grass::calcSwinging(vec3 windVelocity, float (&angularAcc)[3]) {
		float areaOfThrustSurfaceOfWind = 0.1;
		float dragCoefficient = 1;

		GrassMesh::Vertex tip = controlPoints[3];

		vec3 staticGrowthVec = staticStateControlPoints[3].position - staticStateControlPoints[0].position;
		vec3 growthVec = tip.position - controlPoints[0].position;
		staticGrowthVec.y = 0; //project
		growthVec.y = 0; //project
		float growthVecAngularDisp = acos(dot(normalize(growthVec), normalize(staticGrowthVec)));
		
		for (int edge = 0; edge < 3; edge++) {
			GrassMesh::Vertex lowVert = controlPoints[edge];
			GrassMesh::Vertex hiVert = controlPoints[edge + 1];

			// Wind force
			vec3 projectedSwingVel = dot(windVelocity, hiVert.wWithoutTwist) * hiVert.wWithoutTwist;
			vec3 windForceSwinging = areaOfThrustSurfaceOfWind * dragCoefficient * projectedSwingVel;

			// Restoration force
			vec3 restorationForceSwing;
			if (growthVecAngularDisp == 0) {
				restorationForceSwing = vec3(0);
			}
			else {
				float growthVecAnglularDispAdj = (tip.stiffness / hiVert.stiffness) * growthVecAngularDisp;
				restorationForceSwing = tip.stiffness * growthVecAnglularDispAdj * normalize(staticGrowthVec - growthVec);
			}

			// Total
			vec3 totalSwingForce = windForceSwinging + restorationForceSwing;

			vec3 edgeVec = hiVert.position - lowVert.position;

			vec3 torqueDir;
			angularAcc[edge] = angularAccFromTorque(edgeVec, totalSwingForce, torqueDir);
		}
	}

	void Grass::calcBending(vec3 windVelocity, float (&angularAcc)[3]) {
		
		//Figure out where the second tip is
		int secondTipEdge = -1;
		bool prevENTW;
		for (int edge = 2; edge >= 0; edge--) {
			GrassMesh::Vertex lowVert = controlPoints[edge];
			GrassMesh::Vertex hiVert = controlPoints[edge + 1];

			vec3 edgeVec = normalize(lowVert.position - hiVert.position);
			vec3 edgeNormalWithoutTwist = cross(hiVert.wWithoutTwist, edgeVec);
			bool edgeNormalTowardsWind = dot(windVelocity, edgeNormalWithoutTwist) > 0;
			if (edge == 2) {
				prevENTW = edgeNormalTowardsWind;
			}
			else if (edgeNormalTowardsWind != prevENTW) {
				secondTipEdge = edge;
				break;
			}
		}
		if (secondTipEdge == -1) {
			calcBendingCustomTip(windVelocity, 2, 0, angularAcc);
		}
		else {
			calcBendingCustomTip(windVelocity, 2, secondTipEdge+1, angularAcc);
			calcBendingCustomTip(windVelocity, secondTipEdge, 0, angularAcc);
		}
	}

	void Grass::calcBendingCustomTip(vec3 windVelocity, int tipEdge, int lastEdge, float(&angularAcc)[3]) {
		float areaOfThrustSurfaceOfWind = 0.1;
		float dragCoefficient = 1;
		
		GrassMesh::Vertex localTip = controlPoints[tipEdge + 1];

		float staticBendAngle = asin(normalize(staticStateControlPoints[tipEdge + 1].position - staticStateControlPoints[tipEdge].position).y);
		float currentBendAngle = asin(normalize(controlPoints[tipEdge + 1].position - controlPoints[tipEdge].position).y);
		float tipBendAngularDisp = currentBendAngle - staticBendAngle;
		
		for (int edge = tipEdge; edge >= lastEdge; edge--) {
			GrassMesh::Vertex lowVert = controlPoints[edge];
			GrassMesh::Vertex hiVert = controlPoints[edge + 1];
			GrassMesh::Vertex staticLowVert = staticStateControlPoints[edge];
			GrassMesh::Vertex staticHiVert = staticStateControlPoints[edge + 1];

			vec3 staticEdgeVec = staticHiVert.position - staticLowVert.position;
			vec3 edgeVec = hiVert.position - lowVert.position;
			vec3 edgeNormalWithoutTwist = normalize(cross(lowVert.wWithoutTwist, edgeVec));

			// Wind force
			vec3 projectedBendVel = dot(windVelocity, edgeNormalWithoutTwist) * edgeNormalWithoutTwist;
			vec3 windForceBending = areaOfThrustSurfaceOfWind * dragCoefficient * projectedBendVel;

			// Restoration force
			vec3 restorationForceBending;
			if (tipBendAngularDisp == 0) {
				restorationForceBending = vec3(0);
			}
			else {
				float currentBendAngularDispAdj = (localTip.stiffness / hiVert.stiffness) * tipBendAngularDisp;
				restorationForceBending = localTip.stiffness * currentBendAngularDispAdj * normalize(staticEdgeVec - edgeVec);
			}

			//Most of this was me trying to get bending to work - that's why it's different from the other functions
			vec3 torqueDir;
			float ra = angularAccFromTorque(edgeVec, restorationForceBending, torqueDir);
			if (dot(torqueDir, hiVert.wWithoutTwist) < 0) {
				ra *= -1;
			}
			/*if (sign(tipBendAngularDisp) == sign(ra)) {
				restorationForceBending = -restorationForceBending;
			}*/

			// Total
			vec3 totalBendForce = windForceBending + restorationForceBending;
			
			//vec3 torqueDir;
			float absAcc = angularAccFromTorque(edgeVec, totalBendForce, torqueDir);
			if (dot(torqueDir, hiVert.wWithoutTwist) < 0) {
				absAcc *= -1;
			}
			angularAcc[edge] = absAcc;
			//float wa = angularAccFromTorque(edgeVec, windForceBending, torqueDir);
			//float ra = angularAccFromTorque(edgeVec, restorationForceBending, torqueDir);
			//float ta = angularAccFromTorque(edgeVec, totalBendForce, torqueDir);
		}
	}
	void Grass::calcTwisting(vec3 windVelocity, float (&angularAcc)[3]) {
		float areaOfThrustSurfaceOfWind = 0.1;
		float dragCoefficient = 1;

		GrassMesh::Vertex tip = controlPoints[3];

		float topEdgeTwistDisp = acos(dot(staticStateControlPoints[3].wWithTwist, tip.wWithTwist));
		
		for (int edge = 0; edge < 3; edge++) {
			GrassMesh::Vertex lowVert = controlPoints[edge];
			GrassMesh::Vertex hiVert = controlPoints[edge + 1];
			vec3 edgeVec = lowVert.position - hiVert.position;

			vec3 edgeNormalWithTwist = normalize(cross(lowVert.wWithTwist, edgeVec));

			// Wind force
			vec3 projectedTwistVel = dot(windVelocity, edgeNormalWithTwist) * edgeNormalWithTwist;
			vec3 windForceTwist = areaOfThrustSurfaceOfWind * dragCoefficient * projectedTwistVel;

			// Restoration force
			vec3 restorationForceTwist;
			if (topEdgeTwistDisp == 0) {
				restorationForceTwist = vec3(0);
			}
			else {
				float currentTwistAngularDispAdj = (tip.stiffness / hiVert.stiffness) * topEdgeTwistDisp;
				restorationForceTwist = tip.stiffness * currentTwistAngularDispAdj * normalize(hiVert.wWithoutTwist - hiVert.wWithTwist);
			}

			// Total
			vec3 totalTwistForce = windForceTwist + restorationForceTwist;

			vec3 torqueDir;
			angularAcc[edge] = angularAccFromTorque(hiVert.wWithTwist, totalTwistForce, torqueDir);
		}
	}

	float Grass::angularAccFromTorque(vec3 radiusVec, vec3 force, vec3& outTorqueDir) {
		vec3 crossed = cross(radiusVec, force);
		float torque = length(crossed);
		outTorqueDir = normalize(crossed);
		float momentOfInertia = pow(length(radiusVec), 2) * massOfABladeOfGrass;
		return torque / momentOfInertia;
	}
    
    void Grass::draw(GLSLProgram &shader) {

		shader.use();
		glBindVertexArray(_mesh->getVAOID());
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawElements(GL_PATCHES, _mesh->getNumIndices(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
    }
}
