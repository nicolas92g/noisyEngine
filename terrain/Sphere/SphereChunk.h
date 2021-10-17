#pragma once
#include <array>
#include <terrain/Sphere/Sphere.h>
#include <Utils/BiArray.h>
#include <Rendering/Drawable.h>

namespace ns::Sphere 
{
	class SphereChunk : public Drawable
	{
	public:
		/**
		 * @brief create one chunk of a sphere based on a square that is the location of the sphere
		 * \param chunkLocation a square that locate the chunk on a sphere
		 * \param sphereRadius
		 * \param resolution
		 */
		SphereChunk(const std::array<glm::vec3, 4>& chunkLocation, float sphereRadius, uint16_t resolution);

		virtual void draw(const ns::Shader& shader) const override;

	protected:
		const float sphereRadius_;	//radius of the sphere that is an array of those chunks
		uint16_t resolution_; //the resolution is the sqrt of the number of squares that compound the chunk (the chunk is a grid with a size of res * res )
		std::unique_ptr<Mesh> mesh_;
	};
}
