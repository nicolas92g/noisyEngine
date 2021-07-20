#include "Mesh.h"

#include <iostream>
#include <limits>
#include <Utils/DebugLayer.h>

ns::Mesh::Mesh(
    const std::vector<Vertex>& vertices,
	const std::vector<unsigned int>& indices,
    const ns::Material& material,
    const ns::MeshConfigInfo& info)
    :
    numberOfVertices_((info.indexedVertices) ? static_cast<int>(indices.size()) : static_cast<int>(vertices.size())),
    material_(material),
    info_(info)
{
    //create vertex array
    glGenVertexArrays(1, &vertexArrayObject_);
    glBindVertexArray(vertexArrayObject_);

    //create vertex buffer
    glGenBuffers(1, &vertexBufferObject_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    if (info_.indexedVertices) {
        //create index buffer
        glGenBuffers(1, &indexBufferObject_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject_);
        std::vector<unsigned char> buf1; std::vector<unsigned short> buf2;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * getIndexTypeSize(), getIndices(indices, buf1, buf2), GL_STATIC_DRAW);
    }

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    // vertex tangents
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    // vertex bitangents
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

    //unbind vertex array 
    glBindVertexArray(0);
}

ns::Mesh::Mesh(const std::vector<Vertex>& vertices, 
    const std::vector<VertexBoneData>& animData, 
    const std::vector<unsigned int>& indices, 
    const ns::Material& material, 
    const MeshConfigInfo& info)
    :
    Mesh(vertices, indices, material, info)
{
    glBindVertexArray(vertexArrayObject_);

    glGenBuffers(1, &bonesBufferObject_);
    glBindBuffer(GL_ARRAY_BUFFER, bonesBufferObject_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexBoneData) * animData.size(), animData.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(VertexBoneData), (const void*)offsetof(VertexBoneData, ids));

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const void*)offsetof(VertexBoneData, weights));

    glBindVertexArray(0);
}

ns::Mesh::~Mesh()
{
    //destroy buffers and vertex array
	glDeleteBuffers(1, &vertexBufferObject_);
	if(info_.indexedVertices) 
        glDeleteBuffers(1, &indexBufferObject_);
	glDeleteBuffers(1, &bonesBufferObject_);
	glDeleteVertexArrays(1, &vertexArrayObject_);
}

void ns::Mesh::draw(const Shader& shader) const
{
    //bind shader, texture and vertex array to draw
    shader.use();

    material_.bind(shader);
    shader.set("computeBitangents", !info_.hasBitangents);

    glBindVertexArray(vertexArrayObject_);

    //draw call
    if (info_.indexedVertices)
        glDrawElements(info_.primitive, numberOfVertices_, info_.indexType, (void*)0);
    else
        glDrawArrays(info_.primitive, 0, numberOfVertices_);
}

const void* ns::Mesh::getIndices(const std::vector<unsigned int>& indices,
    std::vector<unsigned char>& indicesBytes,
    std::vector<unsigned short>& indicesShorts
    ) const
{

    //put the data in the vertex buffer in the format wanted 
    switch (info_.indexType)
    {
    case GL_UNSIGNED_BYTE:

        indicesBytes.resize(indices.size());

        for (size_t i = 0; i < indices.size(); i++) {
#               ifndef NDEBUG
            //check for invalid values
            if (indices[i] > std::numeric_limits<unsigned char>::max()) {
                Debug::get() << "error in the mesh  " << info_.name <<
                    " : invalid indexType unsigned char, indices provided are too big !\n";
                return indices.data();
            }
#               endif // NDEBUG
            indicesBytes[i] = static_cast<unsigned char>(indices[i]);
        }

        return indicesBytes.data();
        break;

    case GL_UNSIGNED_SHORT:

        indicesShorts.resize(indices.size());

        for (size_t i = 0; i < indices.size(); i++) {
#               ifndef NDEBUG
            //check for invalid values
            if (indices[i] > std::numeric_limits<unsigned short>::max()) {
                Debug::get() << "error in the mesh  " << info_.name <<
                    " : invalid indexType unsigned short, indices provided are too big !\n";
                return indices.data();
            }
#               endif // NDEBUG
            indicesShorts[i] = static_cast<unsigned short>(indices[i]);
        }

        return indicesShorts.data();
        break;

        //default format is unsigned int 
    default:
        return indices.data();
        break;
    }
}

const short ns::Mesh::getIndexTypeSize() const
{
    switch (info_.indexType) {
    case GL_UNSIGNED_BYTE:
        return sizeof(unsigned char);
    case GL_UNSIGNED_SHORT:
        return sizeof(unsigned short);
    default:
        return sizeof(unsigned int);
    }
}

void ns::Vertex::genTangent()
{
    using namespace glm;

    vec3 c1 = cross(normal, vec3(0.0, 0.0, 1.0));
    vec3 c2 = cross(normal, vec3(0.0, 1.0, 0.0));

    if (length(c1) > length(c2))
        tangent = c1;
    else 
        tangent = c2;

    tangent = normalize(tangent);
}

void ns::Vertex::genBitangent()
{
    using namespace glm;

    bitangent = normalize(cross(normal, tangent));
}
