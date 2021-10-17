#include "DrawableObject3d.h"

#include "Model.h"

template<typename P, typename D>
ns::DrawableObject3d<P, D>::DrawableObject3d(Drawable& model, const vec3p& position, const vec3p& scale, const vec3d& axis, float angle)
	:
	GeometricObject3d<P, D>(position, scale, axis, angle ),
	model_(&model)
{
	const Model* ptr = dynamic_cast<const Model*>(model_);
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
		dynamic_cast<Object3d<P, D>*>(lights_[i].get())->setParent(this);
	}
}

template<typename P, typename D>
void ns::DrawableObject3d<P, D>::draw(const Shader& shader) const
{
	shader.set("model", this->modelMatrix_);
	model_->draw(shader);
}

template<typename P, typename D>
const std::vector<std::shared_ptr<ns::LightBase_>>& ns::DrawableObject3d<P, D>::getLights() const
{
	return lights_;
}

template<typename P, typename D>
void ns::DrawableObject3d<P, D>::setMesh(Drawable& mesh)
{
	model_ = &mesh;
}

template<typename P, typename D>
void templateLinkFixerFunction_drawableObject3d_(){
	_STL_REPORT_ERROR("the fix link function has been called");
	ns::Drawable* dr = nullptr;
	ns::Shader* sh = nullptr;
	ns::DrawableObject3d<P, D> obj(*dr);
	obj.setMesh(*dr);
	obj.draw(*sh);
}

void LinkFixerFunction_drawableObject3d_(){
	templateLinkFixerFunction_drawableObject3d_<float, float>();
	templateLinkFixerFunction_drawableObject3d_<double, float>();
	templateLinkFixerFunction_drawableObject3d_<float, double>();
	templateLinkFixerFunction_drawableObject3d_<double, double>();
}
