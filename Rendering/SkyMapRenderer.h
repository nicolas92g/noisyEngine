#pragma once
#include "Shader.h"
#include "Camera.h"

namespace ns {
	/**
	 * @brief allow to easily render a cubemap as a sky map
	 */
	class SkyMapRenderer
	{
	public:
		/**
		 * @brief create the renderer with a camera and a cubemap id
		 * the buffer at this id will not be modified
		 * \param cam
		 * \param cubeMapTexture
		 */
		SkyMapRenderer(Camera& cam, unsigned cubeMapTexture);
		/**
		 * @brief this do not destroy the cubemap texture
		 */
		~SkyMapRenderer();
		/**
		 * @brief change the cubemap texture
		 * \param cubeMapTexture
		 */
		void setCubeMapTexture(unsigned cubeMapTexture);
		/**
		 * @brief this draw the skymap
		 */
		void draw() const;

	protected:
		Shader shader_;
		Camera& cam_;

		unsigned cubeMapTexture_;
		unsigned VAO;
		unsigned VBO;
	};
}
