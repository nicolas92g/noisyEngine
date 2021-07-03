#include "BillboardRenderer.h"
#include <configNoisy.hpp>

unsigned int ns::BillboardRenderer::textureId = 0;

ns::BillboardRenderer::BillboardRenderer(Camera& cam, const TextureView& texture, const std::vector<ns::Billboard>& billboards)
	:
	cam_(cam),
	texture_(texture),
	shader_(NS_PATH"assets/shaders/main/billboard.vert", NS_PATH"assets/shaders/main/billboard.frag", NS_PATH"assets/shaders/main/billboard.geom", {}, true)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	setBillboards(billboards);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Billboard), (void*)0);
}

ns::BillboardRenderer::~BillboardRenderer()
{
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void ns::BillboardRenderer::setBillboards(const std::vector<ns::Billboard>& billboards)
{

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Billboard) * billboards.size(), billboards.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	numberOfBillboards_ = billboards.size();
}

void ns::BillboardRenderer::draw() const
{
	glBindVertexArray(VAO);

	shader_.set("cameraPos", cam_.position());
	shader_.set("projView", cam_.projectionView());
	shader_.set("cameraUp", cam_.upDirection());

	glActiveTexture(GL_TEXTURE0);
	texture_.bind();
	//glBindTexture(GL_TEXTURE_2D, textureId);

	glDrawArrays(GL_POINTS, 0, numberOfBillboards_);
}
