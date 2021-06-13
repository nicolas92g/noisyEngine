#include "Texture.h"

//stl
#include <iostream>
#include <thread>

//stb_image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#ifndef NDEBUG
std::vector<std::string> ns::Texture::alreadyLoadedTextures;
#endif

ns::Texture::Texture(const char* textureFilePath) 
	: 
	loaded_(true),
	filePath_(textureFilePath)
{
	unsigned char* data = stbi_load(textureFilePath, &width_, &height_, &numberOfChannels_, 0);
	
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D, id_);
	
	if (!data) {
		std::cerr << "failed to load a texture at : " << textureFilePath << std::endl;
		loaded_ = false;
		return;
	}

#	ifndef NDEBUG

	if (std::find(alreadyLoadedTextures.begin(), alreadyLoadedTextures.end(), filePath_) != alreadyLoadedTextures.end()) {
		std::cerr << "PERFORMANCE WARNING !!! : same texture loaded twice : " << filePath_ << std::endl;
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
		std::cerr << "texture " << filePath_ << " has " << numberOfChannels_ << " number of channels !\n";
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

ns::Texture::Texture(aiTexture* const assimpTexture)
	:
	loaded_(true),
	filePath_(NS_EMBEMBEDDED_FILEPATH_NAME),
	numberOfChannels_(4)
{
	std::cerr << "embedded files are not supported yet !\n";
	return;
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D, id_);

	width_ = assimpTexture->mWidth;
	height_ = assimpTexture->mHeight;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (height_) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, assimpTexture->pcData);	
	}
	else if(assimpTexture->CheckFormat("png")){
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_R11_EAC,
			sqrt(width_), sqrt(width_), 0, width_, assimpTexture->pcData);
	}
	else {
		std::cerr << "can't load compressed texture under format :" << assimpTexture->achFormatHint << "\n";
	}

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

bool ns::Texture::isLoaded() const
{
	return loaded_;
}

bool ns::Texture::operator==(const TextureView& textureView) const
{
	return id_ == textureView.textureId_;
}

ns::Texture::~Texture()
{
#	ifndef NDEBUG

	if (filePath_ != NS_EMBEMBEDDED_FILEPATH_NAME) {
		auto it = std::find(alreadyLoadedTextures.begin(), alreadyLoadedTextures.end(), filePath_);
		if(it != alreadyLoadedTextures.end())
			alreadyLoadedTextures.erase(it);
	}
		
#	endif // !NDEBUG

	glDeleteTextures(1, &id_);
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

ns::TextureView::TextureView(const Texture& textureToSee)
{
	this->textureId_ = textureToSee.id_;
	this->width_ = textureToSee.width_;
	this->height_ = textureToSee.height_;
}

void ns::TextureView::operator=(const Texture& textureToSee)
{
	this->textureId_ = textureToSee.id_;
	this->width_ = textureToSee.width_;
	this->height_ = textureToSee.height_;
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
		std::cerr << "an OpenGL error " << errorCode << " occured while binding a texture view !\n";
	}
#	endif // NDEBUG
}

const int ns::TextureView::width() const
{
	return width_;
}

const int ns::TextureView::height() const
{
	return height_;
}
