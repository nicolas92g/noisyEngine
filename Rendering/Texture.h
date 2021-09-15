#pragma once
#include <glad/glad.h>
#include <string>
#include <assimp/scene.h>
#include <vector>
#include <configNoisy.hpp>

namespace ns {
	class TextureView;
	/**
	 * @brief allow to easily create a 2d texture by loading an image file
	 */
	class Texture
	{
	public:
		/**
		 * @brief load a texture from an image file thanks to stb_image 
		 * \param textureFilePath
		 */
		Texture(const char* textureFilePath);
		/**
		 * @brief allow to know if the loading succeed 
		 * \return 
		 */
		bool isLoaded() const;
		/**
		 * @brief reload the texture from another file (or the same but it's stupid men)
		 * \param textureFilePath
		 */
		void reload(const std::string& textureFilePath);
		Texture(Texture&) = delete;
		Texture& operator=(Texture&) = delete;
		/**
		 * @brief allow to know if a texture view point to this texture
		 * \param textureView
		 * \return 
		 */
		bool operator==(const TextureView& textureView) const;
		~Texture();
		/**
		 * @brief return the width of the loaded 2D texture
		 * \return 
		 */
		const int width() const;
		/**
		 * @brief return the height of the loaded 2D texture
		 * \return
		 */
		const int height() const;
		/**
		 * @brief return the number of channels in the loaded texture (1, 2, 3 or 4)
		 * \return 
		 */
		const int numberOfChannels() const;
		/**
		 * @brief return the filepath of the file thanks to which this texture is loaded
		 * \return 
		 */
		const std::string& filePath() const;
		/**
		 * @brief use this texture in opengl
		 */
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

	/**
	 * @brief allow to safely reference a Texture with its uint id
	 */
	class TextureView {
	public:
		/**
		 * @brief create the texture view with a real texture
		 * \param textureToSee
		 */
		TextureView(Texture& textureToSee);
		/**
		 * @brief reset the value of the texture view thanks to a real texture
		 * \param textureToSee
		 */
		void operator=(Texture& textureToSee);
		/**
		 * @brief compare this with a real texture to know if this point to the texture
		 * \param texture
		 * \return 
		 */
		bool operator==(const Texture& texture);
		/**
		 * @brief use the texture in opengl
		 */
		void bind() const;

#		ifdef NS_TEXTURE_VIEW_STORE_POINTER
		const std::string& filepath() const { return ptr_->filePath_; }
#		endif

	protected:
		GLuint textureId_;

#		ifdef NS_TEXTURE_VIEW_STORE_POINTER
		ns::Texture* ptr_;
#		endif

		//allow the access of a texture's id
		friend class Texture;
		friend class Debug;
	};
}
