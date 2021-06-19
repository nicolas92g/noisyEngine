#pragma once
#include "Shader.h"
#include "Camera.h"

namespace ns {
	class SkyMapRenderer
	{
	public:
		SkyMapRenderer(Camera& cam, unsigned cubeMapTexture);
		~SkyMapRenderer();

		void setCubeMapTexture(unsigned cubeMapTexture);

		void draw() const;

	protected:
		Shader shader_;
		Camera& cam_;

		unsigned cubeMapTexture_;
		unsigned VAO;
		unsigned VBO;
	};
}
