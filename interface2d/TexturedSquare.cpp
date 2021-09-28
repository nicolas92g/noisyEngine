#include "TexturedSquare.h"

GLuint ns::TexturedSquare::squareVAO;
GLuint ns::TexturedSquare::squareVBO;

ns::TexturedSquare::TexturedSquare(const TextureView& texture) : 
	texture_(texture)
{}

void ns::TexturedSquare::draw(const ns::Shader& shader) const
{
	texture_.bind();
	glBindVertexArray(squareVAO);
	shader.use();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void ns::TexturedSquare::createSquare()
{
	float vert[20] = {
		// positions			// texture Coords 
		-.5f,  .5f, 0.f,		0.0f, 1.0f,
		-.5f, -.5f, 0.f,		0.0f, 0.0f,
		 .5f,  .5f, 0.f,		1.0f, 1.0f,
		 .5f, -.5f, 0.f,		1.0f, 0.0f
	};

	glGenVertexArrays(1, &squareVAO);
	glBindVertexArray(squareVAO);

	glGenBuffers(1, &squareVBO);
	glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20, vert, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
}
	 
void ns::TexturedSquare::destroySquare()
{
	glDeleteBuffers(1, &squareVBO);
	glDeleteVertexArrays(1, &squareVAO);
}
