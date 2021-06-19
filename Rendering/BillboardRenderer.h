#pragma once
#include "Camera.h"
#include <glm/glm.hpp>

#include "Texture.h"

namespace ns {
	struct Billboard {
		glm::vec3 position;
		float scale;
	};

	class BillboardRenderer
	{
	public:
		BillboardRenderer(Camera& cam, const TextureView& texture, const std::vector<ns::Billboard>& billboards);
		~BillboardRenderer();

		void setBillboards(const std::vector<ns::Billboard>& billboards);

		void draw() const;

		static unsigned int textureId;

	protected:
		Shader shader_;
		Camera& cam_;
		TextureView texture_;

		uint32_t numberOfBillboards_;

		GLuint VAO;
		GLuint VBO;
	};
}
