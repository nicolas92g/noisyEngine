#include "SkyMapRenderer.h"

#include <glad/glad.h>
#include <vector>

#include <configNoisy.hpp>

ns::SkyMapRenderer::SkyMapRenderer(Camera& cam, unsigned cubeMapTexture)
    :
    cam_(cam),
    shader_(NS_PATH"assets/shaders/main/skyBox.vert", NS_PATH"assets/shaders/main/skyBox.frag"),
    cubeMapTexture_(cubeMapTexture)
{
    //cube vertices positions
    std::vector<float> vertices = {
        -1.0f, -1.0f, -1.0f,  
         1.0f,  1.0f, -1.0f,  
         1.0f, -1.0f, -1.0f,  
        -1.0f,  1.0f, -1.0f,  
         1.0f,  1.0f, -1.0f,  
        -1.0f, -1.0f, -1.0f,  

        -1.0f, -1.0f,  1.0f,  
         1.0f, -1.0f,  1.0f,  
         1.0f,  1.0f,  1.0f,  
         1.0f,  1.0f,  1.0f,  
        -1.0f,  1.0f,  1.0f,  
        -1.0f, -1.0f,  1.0f,  

        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,

         1.0f,  1.0f,  1.0f,  
         1.0f, -1.0f, -1.0f,  
         1.0f,  1.0f, -1.0f,  
         1.0f, -1.0f,  1.0f,  
         1.0f, -1.0f, -1.0f,  
         1.0f,  1.0f,  1.0f,  

        -1.0f, -1.0f, -1.0f,  
         1.0f, -1.0f, -1.0f,  
         1.0f, -1.0f,  1.0f,  
         1.0f, -1.0f,  1.0f,  
        -1.0f, -1.0f,  1.0f,  
        -1.0f, -1.0f, -1.0f,  

        -1.0f,  1.0f, -1.0f,  
         1.0f,  1.0f,  1.0f,  
         1.0f,  1.0f, -1.0f,  
        -1.0f,  1.0f,  1.0f,  
         1.0f,  1.0f,  1.0f, 
        -1.0f,  1.0f, -1.0f
    };

    //create VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);

    shader_.set("rotation", glm::rotate(0.0f, glm::vec3(0, 1, 0)));
}

ns::SkyMapRenderer::~SkyMapRenderer()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void ns::SkyMapRenderer::setCubeMapTexture(unsigned tex)
{
    cubeMapTexture_ = tex;
}

void ns::SkyMapRenderer::draw() const
{
    glDepthFunc(GL_ALWAYS);

	shader_.set("hdr", true);
	shader_.set("view", glm::mat4(glm::mat3(cam_.view())));
	shader_.set("projection", cam_.projection());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture_);

	bool cull = glIsEnabled(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES, 0, 3 * 6 * 6);

	if (cull)
		glEnable(GL_CULL_FACE);
}
