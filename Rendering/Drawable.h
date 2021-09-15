#pragma once
#include "Shader.h"

namespace ns {
	/**
	 * @brief abstract class to describe an object that can be draw
	 */
	struct Drawable {
		/**
		 * @brief draw calls needs a shader
		 * \param shader
		 */
		virtual void draw(const ns::Shader& shader) const = 0;
	};
}
