#pragma once
#include <glad/glad.h>
#include <string>
#include <assimp/scene.h>
#include <vector>

#define NS_EMBEMBEDDED_FILEPATH_NAME "<embedded>"

namespace ns {
	class TextureView;
	class Texture
	{
	public:
		Texture(const char* textureFilePath);
		Texture(aiTexture* const assimpTexture);

		bool isLoaded() const;
		
		Texture(Texture&) = delete;
		Texture& operator=(Texture&) = delete;
		bool operator==(const TextureView& textureView) const;

		~Texture();

		const int width() const;
		const int height() const;
		const int numberOfChannels() const;
		const std::string& filePath() const;

		void bind() const;

	protected:
		unsigned int id_;
		int width_;
		int height_;
		int numberOfChannels_;
		std::string filePath_;
		bool loaded_;

		friend class TextureView;

	protected:
#		ifndef NDEBUG
		static std::vector<std::string> alreadyLoadedTextures;
#		endif // !NDEBUG
	};

	class TextureView {
	public:
		TextureView(const Texture& textureToSee);
		void operator=(const Texture& textureToSee);
		bool operator==(const Texture& texture);

		void bind() const;
		const int width() const;
		const int height() const;

	protected:
		unsigned int textureId_;
		int width_;
		int height_;

		friend class Texture;
		friend class Debug;
	};
}