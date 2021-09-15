#include "Texture.h"

//stl
#include <thread>

//stb_image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//ns
#include <Utils/DebugLayer.h>

#ifndef NDEBUG
std::vector<std::string> ns::Texture::alreadyLoadedTextures;
#endif

ns::Texture::Texture(const char* textureFilePath) 
	: 
	loaded_(true),
	filePath_(textureFilePath)
{
	load();
}

bool ns::Texture::isLoaded() const
{
	return loaded_;
}

void ns::Texture::reload(const std::string& textureFilePath)
{
	destroy();
	filePath_ = textureFilePath;
	load();
}

bool ns::Texture::operator==(const TextureView& textureView) const
{
	return id_ == textureView.textureId_;
}

ns::Texture::~Texture()
{
	destroy();
}

const int ns::Texture::width() const
{
	return width_;
}

const int ns::Texture::height() const
{
	return height_;
}

const int ns::Texture::numberOfChannels() const
{
	return numberOfChannels_;
}

const std::string& ns::Texture::filePath() const
{
	return filePath_;
}

void ns::Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, id_);
}

void ns::Texture::load()
{
	unsigned char* data = stbi_load(filePath_.c_str(), &width_, &height_, &numberOfChannels_, 0);

	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D, id_);

	if (!data) {
		dout << "failed to load a texture at : " << filePath_ << std::endl;
		loaded_ = false;
		return;
	}

#	ifndef NDEBUG

	if (std::find(alreadyLoadedTextures.begin(), alreadyLoadedTextures.end(), filePath_) != alreadyLoadedTextures.end()) {
		dout << "PERFORMANCE WARNING !!! : same texture loaded twice : " << filePath_ << std::endl;
	}
	else {
		alreadyLoadedTextures.push_back(filePath_);
	}

#	endif

	GLuint format;
	switch (numberOfChannels_) {
	case 1:
		format = GL_RED;
		break;
	case 2:
		format = GL_RG;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		Debug::get() << "texture " << filePath_ << " has " << numberOfChannels_ << " number of channels !\n";
		return;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width_, height_, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);


	glBindTexture(GL_TEXTURE_2D, 0);
}

void ns::Texture::destroy()
{
	glDeleteTextures(1, &id_);
}

ns::TextureView::TextureView(Texture& textureToSee) : 
	textureId_(textureToSee.id_)
#ifdef NS_TEXTURE_VIEW_STORE_POINTER
	, ptr_(&textureToSee)
#endif
{}

void ns::TextureView::operator=(Texture& textureToSee)
{
	this->textureId_ = textureToSee.id_;

#	ifdef NS_TEXTURE_VIEW_STORE_POINTER
	ptr_ = &textureToSee;
#	endif
}

bool ns::TextureView::operator==(const Texture& texture)
{
	return textureId_ == texture.id_;
}

void ns::TextureView::bind() const
{
#	ifndef NDEBUG
	glGetError();
#	endif // NDEBUG

	glBindTexture(GL_TEXTURE_2D, textureId_);

#	ifndef NDEBUG
	const GLenum errorCode = glGetError();
	if (errorCode) {
		dout << "an OpenGL error " << errorCode << " occured while binding a texture view !\n";
	}
#	endif // NDEBUG
}
