#pragma once
#include "Camera.h"
#include <glm/glm.hpp>

#include "Texture.h"

namespace ns {
	/**
	 * @brief struct that describe a billboard with a position and a scale
	 */
	struct Billboard {
		glm::vec3 position;
		float scale;
	};

	/**
	 * @brief class that is able to render an array of billboards
	 */
	class BillboardRenderer
	{
	public:
		/**
		 * @brief initialize the renderer
		 * \param cam
		 * \param texture
		 * \param billboards
		 */
		BillboardRenderer(Camera<>& cam, const TextureView& texture, const std::vector<ns::Billboard>& billboards);
		~BillboardRenderer();
		/**
		 * @brief replace the array
		 * \param billboards
		 */
		void setBillboards(const std::vector<ns::Billboard>& billboards);
		/**
		 * @brief draw the billboard without binding any FBO
		 */
		void draw() const;

	protected:
		Shader shader_;
		Camera<>& cam_;
		TextureView texture_;

		uint32_t numberOfBillboards_;

		GLuint VAO;
		GLuint VBO;
	};
}
