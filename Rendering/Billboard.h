#pragma once

#include "Object3d.h"
#include "Texture.h"
#include "Shader.h"

namespace ns {
	class Billboard : public GeometricObject3d
	{
	public:
		Billboard(const TextureView& texture, const glm::vec3& position);

		static void createPlane();
		static void destroyPlane();


		void draw(const Shader& shader) const;

	protected:
		TextureView texture_;

		static unsigned int planeVAO;
		static unsigned int planeVBO;

	private:
		using GeometricObject3d::setRotation;
		using GeometricObject3d::rotationAxe;
		using GeometricObject3d::rotationAngle;
	};
}
