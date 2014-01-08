/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\src\TextureManager.cpp
 *
 * \brief	Implements the TextureManager class.
 */

#include "texturemanager.h"

TextureManager::TextureManager()
{
	_textureInfo.width = 0;
	_textureInfo.height = 0;
	_textureInfo.handle = 0;
	_hasLoaded = false;
	_fileSet = false;
	_fileName = "";
	ogl_LoadFunctions();
}

TextureManager::~TextureManager()
{
	freeTexture();
}

bool TextureManager::setFile(QString fileName)
{
	QFileInfo inf(fileName);
	if (inf.exists())
	{
		freeTexture();
		_fileName = fileName;
		_fileSet = true;
		return true;
	} else
	{
		return false;
	}
}

bool TextureManager::loadFile(QString fileName)
{
	if (setFile(fileName))
	{
		QImage temp;
		bool result = temp.load(fileName);
		if (!result)
		{
			qDebug() << "Could not load file" << fileName;
			return false;
		}



		QImage image = QGLWidget::convertToGLFormat(temp);
		_textureInfo.width = image.width();
		_textureInfo.height = image.height();

		glGenTextures(1,&_textureInfo.handle);
		glBindTexture(GL_TEXTURE_2D, _textureInfo.handle);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, _textureInfo.width, _textureInfo.height, 0,  GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); // Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,0);
		qDebug() << "Bound texture to ID" << _textureInfo.handle;
		_hasLoaded = true;

		return true;
	} else {
		return false;
	}
}
void TextureManager::bindTexture()
{
	if (!_hasLoaded && _fileSet)
	{
		loadFile(_fileName);
	} 


	if (_hasLoaded)
	{
		glBindTexture(GL_TEXTURE_2D,_textureInfo.handle);
	}
}
void TextureManager::unbindTexture()
{
	glBindTexture(GL_TEXTURE_2D,0);
}

void TextureManager::freeTexture()
{
	if (_hasLoaded)
	{
		glDeleteTextures(1,&_textureInfo.handle);
		_hasLoaded = false;
		_textureInfo.handle = 0;
		_textureInfo.height = 0;
		_textureInfo.width = 0;
	}
}
