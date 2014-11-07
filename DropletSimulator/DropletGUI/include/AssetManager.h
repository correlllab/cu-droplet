/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\include\AssetManager.h
 *
 * \brief	Declares the AssetManager class.
 */

#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H


#include "TextureManager.h"
#include "MeshManager.h"

#include <QObject>
#include <QHash>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QGLShaderProgram>
#include <QStringList>
#include <QDir>

 /**
  *  @defgroup assets Assets
  *  @ingroup render
  *  \brief Items related to managing assets required for rendering.
  */

 /**
  * @ingroup assets
  * \class	AssetManager
  *
  * \brief	Items related to managing assets required for rendering.
  */
class AssetManager
{

public:

	/**
	 * \fn	AssetManager::AssetManager();
	 *
	 * \brief	Default constructor.
	 */

	AssetManager();

	/**
	 * \fn	AssetManager::~AssetManager();
	 *
	 * \brief	Destructor.
	 */

	~AssetManager();

	/**
	 * \fn	bool AssetManager::loadAssets();
	 *
	 * \brief	Loads the assets.
	 *
	 * \return	true if it succeeds, false if it fails.
	 */

	bool loadAssets();

	/**
	 * \fn	void AssetManager::clearAssets();
	 *
	 * \brief	Clears the assets.
	 */

	void clearAssets();

	/**
	 * \fn	void AssetManager::reloadAssets();
	 *
	 * \brief	Reload assets.
	 */

	void reloadAssets();

	/**
	 * \fn	void AssetManager::setShaderDir(QString dir);
	 *
	 * \brief	Sets shader dir.
	 *
	 * \param	dir	The dir.
	 */

	void setShaderDir(QString dir);

	/**
	 * \fn	void AssetManager::setMeshDir(QString dir);
	 *
	 * \brief	Sets mesh dir.
	 *
	 * \param	dir	The dir.
	 */

	void setMeshDir(QString dir);

	/**
	 * \fn	void AssetManager::setTextureDir(QString dir);
	 *
	 * \brief	Sets texture dir.
	 *
	 * \param	dir	The dir.
	 */

	void setTextureDir(QString dir);

	/**
	 * \fn	void AssetManager::setAssetDir(QString dir);
	 *
	 * \brief	Sets asset dir.
	 *
	 * \param	dir	The dir.
	 */

	void setAssetDir(QString dir);

	/**
	 * \fn	void AssetManager::setManifest(QString file);
	 *
	 * \brief	Sets a manifest.
	 *
	 * \param	file	The file.
	 */

	void setManifest(QString file);

	/**
	 * \fn	void AssetManager::setShaderManifest(QString file);
	 *
	 * \brief	Sets shader manifest.
	 *
	 * \param	file	The file.
	 */

	void setShaderManifest(QString file);

	/**
	 * \fn	QString AssetManager::lookupAssetName(QString name);
	 *
	 * \brief	Looks up a given key to find its associated asset name.
	 *
	 * \param	name	The name.
	 *
	 * \return	.
	 */

	QString lookupAssetName(QString name);

	/**
	 * \fn	QGLShaderProgram* AssetManager::getShader(QString name);
	 *
	 * \brief	Gets a shader.
	 *
	 * \param	name	The name.
	 *
	 * \return	null if it fails, else the shader.
	 */

	QGLShaderProgram* getShader(QString name);

	/**
	 * \fn	MeshManager* AssetManager::getMesh(QString name);
	 *
	 * \brief	Gets a mesh.
	 *
	 * \param	name	The name.
	 *
	 * \return	null if it fails, else the mesh.
	 */

	MeshManager* getMesh(QString name);

	/**
	 * \fn	TextureManager* AssetManager::getTexture(QString name);
	 *
	 * \brief	Gets a texture.
	 *
	 * \param	name	The name.
	 *
	 * \return	null if it fails, else the texture.
	 */

	TextureManager* getTexture(QString name);
private:

	/**
	 * \brief	The shaders.
	 */

	QHash<QString,QGLShaderProgram *> _shaders;

	/**
	 * \brief	The textures.
	 */

	QHash<QString,TextureManager *> _textures;

	/**
	 * \brief	The meshes.
	 */

	QHash<QString,MeshManager *> _meshes;

	/**
	 * \brief	The shader dir.
	 */

	QString _shaderDir;

	/**
	 * \brief	The mesh dir.
	 */

	QString _meshDir;

	/**
	 * \brief	The texture dir.
	 */

	QString _textureDir;

	/**
	 * \brief	The projection dir.
	 */

	QString _projectionDir;

	/**
	 * \brief	A list of shaders.
	 */

	QString _shaderManifest;

	/**
	 * \brief	The asset dir.
	 */

	QString _assetDir;

	/**
	 * \brief	A list of assets.
	 */

	QString _assetManifest;

	/**
	 * \brief	The manifest.
	 */

	QHash<QString,QString> _manifest;

	/**
	 * \brief	true if unused.
	 */

	bool _unused;

	/**
	 * \fn	void AssetManager::loadShaders();
	 *
	 * \brief	Loads the shaders.
	 */

	void loadShaders();

	/**
	 * \fn	void AssetManager::clearShaders();
	 *
	 * \brief	Clears the shaders.
	 */

	void clearShaders();

	/**
	 * \fn	void AssetManager::loadTextures();
	 *
	 * \brief	Loads the textures.
	 */

	void loadTextures();

	/**
	 * \fn	void AssetManager::clearTextures();
	 *
	 * \brief	Clears the textures.
	 */

	void clearTextures();

	/**
	 * \fn	void AssetManager::loadMeshes();
	 *
	 * \brief	Loads the meshes.
	 */

	void loadMeshes();

	/**
	 * \fn	void AssetManager::clearMeshes();
	 *
	 * \brief	Clears the meshes.
	 */

	void clearMeshes();

	/**
	 * \fn	void AssetManager::loadManifest();
	 *
	 * \brief	Loads the manifest.
	 */

	void loadManifest();

	/**
	 * \fn	void AssetManager::clearManifest();
	 *
	 * \brief	Clears the manifest.
	 */

	void clearManifest();
};

#endif // ASSETMANAGER_H
