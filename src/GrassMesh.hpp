//
//  GrassMesh.hpp
//  GreenCanvas
//
//  Created by Lawson Busch on 4/26/17.
//
//

#ifndef GrassMesh_hpp
#define GrassMesh_hpp

#define GLM_FORCE_RADIANS
#include <glm/glm/glm.hpp>
#include <glad/glad.h>

#include "Texture.h"
#include "GLSLProgram.h"
#include <Vector>

namespace basicgraphics {
    
//    typedef std::shared_ptr<class GrassMesh> GrassMeshRef;
    
    class GrassMesh : std::enable_shared_from_this<GrassMesh> {
    public:
        //Note, I changed the structure of this vertex struct to include
        //our Grass' edgeVector and w vector
        //Lawson
        struct Vertex {
			vec3 position;
			vec3 wWithoutTwist;
			vec3 wWithTwist;
			float stiffness;

			float swingVel;
			float bendVel;
			float twistVel;
        };
        
        
        // Creates a vao and vbo. Usage should be GL_STATIC_DRAW, GL_DYNAMIC_DRAW, etc. Leave data empty to just allocate but not upload.
        GrassMesh(GLenum primitiveType, GLenum usage, int allocateVertexByteSize, int allocateIndexByteSize, int vertexOffset, const std::vector<Vertex> &data, int numIndices = 0, int indexByteSize = 0, int* index = nullptr);
        virtual ~GrassMesh();
        
        virtual void draw(GLSLProgram &shader);
        
        void setMaterialColor(const glm::vec4 &color);
        
        // Returns the number of bytes allocated in the vertexVBO
        int getAllocatedVertexByteSize() const;
        int getAllocatedIndexByteSize() const;
        // Returns the number of bytes actually filled with data in the vertexVBO
        int getFilledVertexByteSize() const;
        int getFilledIndexByteSize() const;
        int getNumIndices() const;
        
        GLuint getVAOID() const;
        
        // Update the vbos. startByteOffset+dataByteSize must be <= allocatedByteSize
        void updateVertexData(int startByteOffset, int vertexOffset, const GrassMesh::Vertex(&data)[4]);
        void updateIndexData(int totalNumIndices, int startByteOffset, int indexByteSize, int* index);
        
    private:
        GLuint _vaoID;
        GLuint _vertexVBO;
        GLuint _indexVBO;
        GLenum _primitiveType;
        
        int _allocatedVertexByteSize;
        int _allocatedIndexByteSize;
        int _filledVertexByteSize;
        int _filledIndexByteSize;
        int _numIndices;
        
        glm::vec4 _materialColor;
    };
    
}


#endif /* GrassMesh_hpp */
