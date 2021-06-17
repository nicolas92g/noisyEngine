#include "BillboardRenderer.h"
#include <configNoisy.hpp>

ns::BillboardRenderer::BillboardRenderer(Camera& cam)
	:
	cam_(cam),
	shader_(NS_PATH"ns/shaders/billboard.vert", NS_PATH"ns/shaders/billboard.frag", nullptr, {}, true)
{
	Billboard::createPlane();
}

ns::BillboardRenderer::~BillboardRenderer()
{
	Billboard::destroyPlane();
}

void ns::BillboardRenderer::update()
{
	for (Billboard* b : billboards_)
		b->update();
}

void ns::BillboardRenderer::draw()
{
	shader_.set("projView", cam_.projectionView());
	shader_.set("cameraRight", cam_.rightDirection());
	shader_.set("cameraUp", cam_.upDirection());

	for (const Billboard* b : billboards_)
		b->draw(shader_);
}

void ns::BillboardRenderer::addBillboard(Billboard& billboard)
{
	for (Billboard* b : billboards_) if (b == &billboard) return;

	billboards_.push_back(&billboard);
}

void ns::BillboardRenderer::removeBillboard(Billboard& billboard)
{
	for (auto it = billboards_.begin(); it != billboards_.end(); it++) {
		if (*it == &billboard)
			billboards_.erase(it);
	}
}

void ns::BillboardRenderer::clearBillboards()
{
	billboards_.clear();
}
