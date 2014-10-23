/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\include\TextureManager.h
 *
 * \brief	Declares the TextureManager class.
 */

#ifndef TEXTUREMANAGER_H

/**
 * \def	TEXTUREMANAGER_H
 *
 * \brief	A macro that defines texturemanager h.
 */

#define TEXTUREMANAGER_H

#include "gl_core_3_3.h"
#include <QImage>
#include <QGLWidget>
#include <QDebug>
#include <QFileInfo>

/**
 * @ingroup assets
 * \class	TextureManager
 *
 * \brief	Container class for a texture.
 */

class TextureManager
{


public:

	/**
	 * \fn	TextureManager::TextureManager();
	 *
	 * \brief	Default constructor.
	 */

	TextureManager();
	~TextureManager();
	bool loadFile(QString fileName);

	/**
	 * \fn	void TextureManager::bindTexture();
	 *
	 * \brief	Bind texture.
	 */

	void bindTexture();

	/**
	 * \fn	void TextureManager::unbindTexture();
	 *
	 * \brief	Unbind texture.
	 */

	void unbindTexture();

	/**
	 * \fn	bool TextureManager::setFile(QString fileName);
	 *
	 * \brief	Sets a file.
	 *
	 * \param	fileName	Filename of the file.
	 *
	 * \return	true if it succeeds, false if it fails.
	 */

	bool setFile(QString fileName);
	void freeTexture();
private:
	struct {
		GLuint width, height;
		GLuint handle;
		bool valid;
	} _textureInfo;
	bool _hasLoaded;
	bool _fileSet;
	QString _fileName;

};

#endif // TEXTUREMANAGER_H
