#include "SkyMapRenderer.h"

#include <glad/glad.h>
#include <vector>

#include <configNoisy.hpp>

template<typename P, typename D>
ns::SkyMapRenderer<P, D>::SkyMapRenderer(Camera<P, D>& cam, unsigned cubeMapTexture)
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

template<typename P, typename D>
ns::SkyMapRenderer<P, D>::~SkyMapRenderer()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

template<typename P, typename D>
void ns::SkyMapRenderer<P, D>::setCubeMapTexture(unsigned tex)
{
    cubeMapTexture_ = tex;
}

template<typename P, typename D>
void ns::SkyMapRenderer<P, D>::draw() const
{
    glDepthFunc(GL_ALWAYS);

	shader_.set("hdr", true);
	shader_.set("view", glm::mat<4, 4, P>(glm::mat<3, 3, P>(cam_.view())));
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

template<typename P, typename D>
void templateLinkFixFunction_SkyMapRenderer_(){
    _STL_REPORT_ERROR("");
    using namespace ns;
    Camera<P, D>* c = 0;
    SkyMapRenderer<P, D> r(*c, 0);
    r.setCubeMapTexture(0);
    r.draw();
}

void LinkFixFunction_SkyMapRenderer_(){
    templateLinkFixFunction_SkyMapRenderer_<float, float>();
    templateLinkFixFunction_SkyMapRenderer_<float, double>();
    templateLinkFixFunction_SkyMapRenderer_<double, float>();
    templateLinkFixFunction_SkyMapRenderer_<double, double>();
}
