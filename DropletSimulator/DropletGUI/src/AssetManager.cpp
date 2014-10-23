/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\src\AssetManager.cpp
 *
 * \brief	Implements the AssetManager class.
 */

#include "assetmanager.h"

AssetManager::AssetManager()
{
	_assetDir = "./";
	_assetManifest = "manifest.txt";
	_textureDir = "./";
	_shaderDir = "./";
	_shaderManifest = "shaders.txt";
	_meshDir = "./";
	_unused = true;
}

AssetManager::~AssetManager()
{
	if (!_unused)
	{
		clearAssets();
	}
}

bool AssetManager::loadAssets()
{
	qDebug() << "Loading assets...";
	loadManifest();
	loadShaders();
	loadMeshes();
	loadTextures();
	return true;
}

void AssetManager::loadMeshes()
{
	qDebug() << "Loading Meshes...";
	QString meshPath = _assetDir;
	meshPath.append(_meshDir);
	QDir inDir(meshPath,"*.obj");
	if (inDir.exists())
	{
		QStringList files = inDir.entryList();
		QList<QString>::const_iterator i = files.begin();
		for (i = files.begin() ; i != files.end() ; i++)
		{
			qDebug() << *i;
			MeshManager *mesh = new MeshManager();
			if (mesh->setFile(QString(meshPath).append(*i)))
			{
				QString name(*i);
				name.remove(".obj",Qt::CaseInsensitive);
				_meshes[name.toLower()] = mesh;
			} else {
				delete mesh;
			}
		}

	} else {
		qDebug() << "Error: directory not found " << _meshDir;
	}	
}

void AssetManager::setShaderDir(QString dir)
{
	_shaderDir = dir;
}
void AssetManager::setMeshDir(QString dir)
{
	_meshDir = dir;
}
void AssetManager::setTextureDir(QString dir)
{
	_textureDir = dir;
}

void AssetManager::setAssetDir(QString dir)
{
	_assetDir = dir;
}
void AssetManager::setManifest(QString file)
{
	_assetManifest = file;
}
void AssetManager::setShaderManifest(QString file)
{
	_shaderManifest = file;
}
void AssetManager::loadShaders()
{
	qDebug() << "Loading Shaders...";
	QString shaderDir(_assetDir);

	shaderDir.append(_shaderDir);
	QString manifestPath(shaderDir);
	manifestPath.append(_shaderManifest);

	qDebug() << "Reading in shader manifest " << manifestPath;

	QFile file(manifestPath);

	if (file.exists())
	{
		if (file.open(QIODevice::ReadOnly  |QIODevice::Text))
		{
			QTextStream in(&file);
			while (!in.atEnd())
			{
				QString line = in.readLine(0);
				if (line.count() > 0)
				{
					if (!line.startsWith("#"))
					{
						QStringList list = line.split(" ",QString::SkipEmptyParts);
						if (list.size() == 3)
						{
							QGLShaderProgram *prog = new QGLShaderProgram();

							if (prog->addShaderFromSourceFile(QGLShader::Vertex,QString(shaderDir).append(list[1])))
							{
								if (prog->addShaderFromSourceFile(QGLShader::Fragment,QString(shaderDir).append(list[2])))
								{
									if(prog->link())
									{
										qDebug() << "Linked program " << list[0];
										_shaders[list[0].toLower()] = prog;
										_unused = false;
									} else {
										qDebug() << "Error:" << prog->log();
									}
								} else {
									qDebug() << "Error:" << prog->log();
								}
							} else {
								qDebug() << "Error:" << prog->log();
							}
						}  else {
							qDebug() << "Error: malformed line" << line;
						}
					} else {
						// push the comment to the debug console
						qDebug() << line;
					}
				}
			}
		} else
		{
			qDebug() << "Error: " << file.error();

		}
	} else
	{
		qDebug() << "Error: file does not exist";
	}
}

void AssetManager::loadTextures()
{
	qDebug() << "Loading Textures...";
	QString texDir(_assetDir);
	texDir.append(_textureDir);
	QDir inDir(texDir);
	QStringList filters;
	filters << "*.jpg" << "*.bmp" << "*.png";
	inDir.setNameFilters(filters);
	if (inDir.exists())
	{
		QStringList files = inDir.entryList();
		QList<QString>::const_iterator i = files.begin();
		for (i = files.begin() ; i != files.end() ; i++)
		{
			qDebug() << *i;

			TextureManager *texture = new TextureManager();

			if (texture->setFile(QString(texDir).append(*i)))
			{

				QString name(*i);
				name = name.toLower();
				name.remove(".jpg");
				name.remove(".bmp");
				name.remove(".png");
				if (!_textures.contains(name))
				{
					_unused = false;
					_textures[name] = texture;
				} else {
					qDebug() << "Error: namespace collision when loading texture" << *i;
					delete texture;
				}
			} else {
				delete texture;
			}
		}

	} else {
		qDebug() << "Error: directory not found " << _textureDir;
	}
}
QGLShaderProgram* AssetManager::getShader(QString name)
{
	if (_shaders.contains(name.toLower()))
	{
		return _shaders[name.toLower()];
	} else
	{
		return NULL;
	}

}

MeshManager* AssetManager::getMesh(QString name)
{
	if (_meshes.contains(name.toLower()))
	{
		return _meshes[name.toLower()];
	} else
	{
		return NULL;
	}

}

TextureManager* AssetManager::getTexture(QString name)
{
	name = name.toLower();
	name.remove(".jpg");
	name.remove(".bmp");
	name.remove(".png");
	if (_textures.contains(name))
	{
		return _textures[name];
	} else
	{
		return NULL;
	}

}

void AssetManager::clearShaders()
{
	qDebug() << "Tearing down shaders...";
	QHash<QString,QGLShaderProgram *>::iterator i = _shaders.begin();
	while (i != _shaders.end())
	{
		QGLShaderProgram *prog = i.value();
		qDebug() << "Deleting shader" << i.key();
		QHash<QString,QGLShaderProgram *>::iterator prev = i;
		++i;
		_shaders.erase(prev);
		delete prog;

	}
}

void AssetManager::clearTextures()
{
	qDebug() << "Tearing down textures...";
	QHash<QString,TextureManager *>::iterator i = _textures.begin();
	while (i != _textures.end())
	{
		TextureManager *prog = i.value();
		QHash<QString,TextureManager *>::iterator prev = i;
		++i;
		qDebug() << "Deleting texture" << prev.key();
		delete prog;
		_textures.erase(prev);
	}
}

void AssetManager::clearMeshes()
{
	qDebug() << "Tearing down meshes...";

	QHash<QString,MeshManager *>::iterator i = _meshes.begin();
	while (i != _meshes.end())
	{
		MeshManager *prog = i.value();
		QHash<QString,MeshManager *>::iterator prev = i;
		++i;
		qDebug() << "Deleting mesh" << prev.key();
		delete prog;
		_meshes.erase(prev);
	}
}

void AssetManager::clearAssets()
{
	clearManifest();
	clearShaders();
	clearMeshes();
	clearTextures();
	_unused = true;
}

void AssetManager::reloadAssets()
{
	clearAssets();
	loadAssets();
}


void AssetManager::loadManifest()
{
	qDebug() << "Loading asset names...";
	QString manifestPath(_assetDir) ;
	manifestPath.append(_assetManifest);

	qDebug() << "Reading in asset manifest " << manifestPath;

	QFile file(manifestPath);

	if (file.exists())
	{
		if (file.open(QIODevice::ReadOnly  |QIODevice::Text))
		{
			QTextStream in(&file);
			while (!in.atEnd())
			{
				QString line = in.readLine(0);
				if (line.count() > 0)
				{
					if (!line.startsWith("#"))
					{
						QStringList list = line.split(" ",QString::SkipEmptyParts);
						if (list.size() == 2)
						{
							if (!_manifest.contains(list[0]))
							{
								_manifest[list[0]] = list[1];
							} else {
								qDebug() << "Error: namespace collision on manifest item" << list[0];
							}

						}  else {
							qDebug() << "Error: malformed line" << line;
						}
					} else {
						// push the comment to the debug console
						qDebug() << line;
					}
				}
			}
		} else
		{
			qDebug() << "Error: " << file.error();

		}
	} else
	{
		qDebug() << "Error: file does not exist";
	}
}

void AssetManager::clearManifest()
{
	_manifest.clear();
}

QString AssetManager::lookupAssetName(QString name)
{
	if (_manifest.contains(name))
	{
		return _manifest[name];
	} else {
		return QString("null");	
	}
}
