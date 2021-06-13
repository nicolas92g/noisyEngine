#pragma once
#include "Shader.h"

namespace ns {
	struct Drawable {
		virtual void draw(const ns::Shader& shader) const = 0;
	};
}
