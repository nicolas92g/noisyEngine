#pragma once
#include <glad/glad.h>
#include <string>
#include <assimp/scene.h>
#include <vector>

#define NS_EMBEMBEDDED_FILEPATH_NAME "<embedded>"

#define NS_TEXTURE_VIEW_STORE_POINTER

namespace ns {
	class TextureView;
	class Texture
	{
	public:
		Texture(const char* textureFilePath);
		Texture(aiTexture* const assimpTexture);

		bool isLoaded() const;
		void reload(const std::string& textureFilePath);
		
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
		void load();
		void destroy();

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
		TextureView(Texture& textureToSee);
		void operator=(Texture& textureToSee);
		bool operator==(const Texture& texture);

		void bind() const;

#		ifdef NS_TEXTURE_VIEW_STORE_POINTER
		const std::string& filepath() const { return ptr_->filePath_; }
#		endif

	protected:
		unsigned int textureId_;
#		ifdef NS_TEXTURE_VIEW_STORE_POINTER
		ns::Texture* ptr_;
#		endif

		friend class Texture;
		friend class Debug;
	};
}