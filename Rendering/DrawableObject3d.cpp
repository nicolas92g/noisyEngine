#include "DrawableObject3d.h"

#include "Model.h"


ns::DrawableObject3d::DrawableObject3d(Drawable& model, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& axis, float angle)
	:
	GeometricObject3d(position, scale, axis, angle ),
	model_(model)
{
	const Model* ptr = dynamic_cast<const Model*>(&model_);
	if (!ptr) return;

	lights_.resize(ptr->getLights().size());
	for (size_t i = 0; i < ptr->getLights().size(); i++)
	{
		auto dLight = dynamic_cast<DirectionalLight*>(ptr->getLights()[i].get());
		auto pLight = dynamic_cast<PointLight*>(ptr->getLights()[i].get());
		auto sLight = dynamic_cast<SpotLight*>(ptr->getLights()[i].get());

		if (dLight) { 
			lights_[i] = std::make_shared<DirectionalLight>(*dLight);
		}
		else if (sLight) {
			lights_[i] = std::make_shared<SpotLight>(*sLight);
		}
		else if (pLight) {
			lights_[i] = std::make_shared<PointLight>(*pLight);
		}
		dynamic_cast<Object3d*>(lights_[i].get())->setParent(this);
	}
}

void ns::DrawableObject3d::draw(const Shader& shader) const
{
	shader.set("model", modelMatrix_);
	model_.draw(shader);
}

const std::vector<std::shared_ptr<ns::LightBase_>>& ns::DrawableObject3d::getLights() const
{
	return lights_;
}
