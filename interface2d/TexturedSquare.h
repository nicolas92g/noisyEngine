#pragma once

//gl
#include <glad/glad.h>

//ns
#include <Rendering/Texture.h>
#include <Rendering/Drawable.h>

namespace ns {
	/**
	 * @brief create a drawable Object that is a plane with a texture rendered on it
	 * need to call the createSquare() static function when the opengl is created to be able to instanciate this class
	 * you can also call the destroySquare() static function just before deleting the opengl context
	 */
	class TexturedSquare : public Drawable
	{
	public:
		/**
		 * @brief initialize the texture of the square
		 * \param texture
		 */
		TexturedSquare(const TextureView& texture);
		/**
		 * @brief draw the square by binding the right texture
		 * \param shader
		 */
		virtual void draw(const ns::Shader& shader) const override;
		/**
		 * @brief send the square data into the GPU
		 */
		static void createSquare();
		/**
		 * @brief delete the square data from the GPU
		 */
		static void destroySquare();
	protected:
		TextureView texture_;

	protected:
		static GLuint squareVAO;
		static GLuint squareVBO;
	};
}
