#include "Billboard.h"

unsigned int ns::Billboard::planeVAO;
unsigned int ns::Billboard::planeVBO;

ns::Billboard::Billboard(const TextureView& texture, const glm::vec3& position)
	:
	GeometricObject3d(position, glm::vec3(1), glm::vec3(1), 0),
	texture_(texture)
{
	update();
}

void ns::Billboard::draw(const Shader& shader) const
{
	glActiveTexture(GL_TEXTURE0);
	texture_.bind();

	shader.set("model", modelMatrix_);

	glBindVertexArray(planeVAO);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void ns::Billboard::createPlane()
{
	std::vector<float> vert = {
		// positions			// texture Coords
		-1.0f,  1.0f,		0.0f, 1.0f,
		-1.0f, -1.0f,		0.0f, 0.0f,
		 1.0f,  1.0f,		1.0f, 1.0f,
		 1.0f, -1.0f,		1.0f, 0.0f,
	};

	glGenVertexArrays(1, &planeVAO);
	glBindVertexArray(planeVAO);

	glGenBuffers(1, &planeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vert.size(), vert.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);
}

void ns::Billboard::destroyPlane()
{
	glDeleteBuffers(1, &planeVBO);
	glDeleteVertexArrays(1, &planeVAO);
}

