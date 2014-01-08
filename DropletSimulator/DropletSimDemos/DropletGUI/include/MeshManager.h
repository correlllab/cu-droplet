/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\include\MeshManager.h
 *
 * \brief	Declares the MeshManager class.
 */

#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include "gl_core_3_3.h"

#include <QString>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QDebug>
#include <QGLBuffer>
#include "DropletGUIStructs.h"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

/**
* @ingroup assets
* \class	MeshManager
*
* \brief	Container class for meshes.
*/

class MeshManager
{


public:

	/**
	* \fn	MeshManager::MeshManager();
	*
	* \brief	Default constructor.
	*/

	MeshManager();

	/**
	* \fn	MeshManager::~MeshManager();
	*
	* \brief	Destructor.
	*/

	~MeshManager();

	/**
	* \fn	bool MeshManager::loadFile(QString fileName);
	*
	* \brief	Loads a file.
	*
	* \param	fileName	Filename of the file.
	*
	* \return	true if it succeeds, false if it fails.
	*/

	bool loadFile(QString fileName);

	/**
	* \fn	bool MeshManager::setFile(QString fileName);
	*
	* \brief	Assigns a filename, but defers the actual loading of the file until first use.
	*
	* \param	fileName	Filename of the file.
	*
	* \return	true if it succeeds, false if it fails.
	*/

	bool setFile(QString fileName);

	/**
	* \fn	void MeshManager::freeModel();
	*
	* \brief	Free model.
	*/

	void freeModel();

	/**
	* \fn	void MeshManager::bindBuffer();
	*
	* \brief	Bind buffer.
	*/

	void bindBuffer();

	/**
	* \fn	void MeshManager::unbindBuffer();
	*
	* \brief	Unbind buffer.
	*/

	void unbindBuffer();

	/**
	* \fn	GLuint MeshManager::getVertexStride();
	*
	* \brief	Gets vertex stride.
	*
	* \return	The vertex stride.
	*/

	GLuint getVertexStride();

	/**
	* \fn	GLuint MeshManager::getTexStride();
	*
	* \brief	Gets tex stride.
	*
	* \return	The tex stride.
	*/

	GLuint getTexStride();

	/**
	* \fn	GLuint MeshManager::getNormalStride();
	*
	* \brief	Gets normal stride.
	*
	* \return	The normal stride.
	*/

	GLuint getNormalStride();

	/**
	* \fn	GLuint MeshManager::getVertexOffset();
	*
	* \brief	Gets vertex offset.
	*
	* \return	The vertex offset.
	*/

	GLuint getVertexOffset();

	/**
	* \fn	GLuint MeshManager::getTexOffset();
	*
	* \brief	Gets tex offset.
	*
	* \return	The tex offset.
	*/

	GLuint getTexOffset();

	/**
	* \fn	GLuint MeshManager::getNormalOffset();
	*
	* \brief	Gets normal offset.
	*
	* \return	The normal offset.
	*/

	GLuint getNormalOffset();

	/**
	* \fn	GLuint MeshManager::getNumVertices();
	*
	* \brief	Gets number vertices.
	*
	* \return	The number vertices.
	*/

	GLuint getNumVertices();

	/**
	* \fn	void MeshManager::setAttribLoc(int locVertex, int locNormal, int locTexCoords);
	*
	* \brief	Sets attribute location.
	*
	* \param	locVertex   	The location vertex.
	* \param	locNormal   	The location normal.
	* \param	locTexCoords	The location tex coords.
	*/

	void setAttribLoc(int locVertex, int locNormal, int locTexCoords);

	/**
	* \fn	void MeshManager::enableAttributeArrays();
	*
	* \brief	Enables the attribute arrays.
	*/

	void enableAttributeArrays();

	/**
	* \fn	void MeshManager::disableAttributeArrays();
	*
	* \brief	Disables the attribute arrays.
	*/

	void disableAttributeArrays();

	/**
	* \fn	void MeshManager::draw();
	*
	* \brief	Draws this object.
	*/

	void draw();

private:

	/**
	 * \struct	vertexData_t
	 *
	 * \brief	Structure type to contain packed vertex attributes.
	 */

	struct vertexData_t
	{
		/**
		* \brief	The vertex coordinates.
		*/

		vec4 vert;

		/**
		* \brief	The normal vector.
		*/

		vec3 normal;

		/**
		* \brief	The texure coordates.
		*/

		vec2 tex;
	};


	struct {
		GLint vertex;
		GLint normal;
		GLint texCoords;

		/**
		* \brief	The shader attribute locs.
		*/

	}  _shaderAttribLocs;

	/**
	* \brief	true if this object has index.
	*/

	bool _hasIndex;

	/**
	* \brief	true if this object has tex coords.
	*/

	bool _hasTexCoords;

	/**
	* \brief	true if this object has normals.
	*/

	bool _hasNormals;

	/**
	* \brief	true if model loaded.
	*/

	bool _modelLoaded;

	/**
	* \brief	Filename of the file.
	*/

	QString _fileName;

	/**
	* \brief	true to file set.
	*/

	bool _fileSet;

	/**
	* \brief	Number of indexes.
	*/

	int _indexCount;

	/**
	* \brief	Number of vertices.
	*/

	int _indexedVertexCount;

	/**
	* \brief	Number of vertices.
	*/

	int _vertexCount;

	/**
	* \brief	The vbo.
	*/

	QGLBuffer _VBO;

	/**
	* \brief	Identifier for the index buffer.
	*/

	GLuint _indexBufferID;

	/**
	* \brief	The vboid.
	*/

	GLuint _VBOID;

};

#endif // MeshManager_H
