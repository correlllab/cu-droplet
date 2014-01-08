/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\src\MeshManager.cpp
 *
 * \brief	Implements the MeshManager class.
 */

#include "meshmanager.h"

MeshManager::MeshManager()
{
	_hasIndex = false;
	_hasNormals = false;
	_hasTexCoords = false;
	_modelLoaded = false;
	_fileSet = false;
	setAttribLoc(0,1,2);
	_indexCount = 0;
	_indexedVertexCount = 0;
	_vertexCount = 0;

	_VBOID = 0;
	_indexBufferID = 0;
	ogl_LoadFunctions();
}

MeshManager::~MeshManager()
{
	freeModel();
}

bool MeshManager::setFile(QString fileName)
{

	QFile file(fileName);
	if (file.exists())
	{
		freeModel();
		_fileName = fileName;
		_fileSet = true;
		return true;
	} else 
	{
		return false;
	}
}
bool MeshManager::loadFile(QString fileName)
{
	if (setFile(fileName))
	{
		QString inLine;
		int n;
		QVector<int> vertexIndices, uvIndices, normalIndices;
		QVector<QVector3D> temp_vertices;
		QVector<QVector2D> temp_uvs;
		QVector<QVector3D> temp_normals;
		vertexData_t *vertexData = NULL;
		vertexData_t *indexedVertexData = NULL;
		GLuint *arrayIndex = NULL;

		QFile file(fileName);

		if (file.exists())
		{
			if (file.open(QIODevice::ReadOnly  |QIODevice::Text))
			{
				QTextStream in(&file);
				while (!in.atEnd())
				{
					inLine = in.readLine(0);
					if(inLine.length() == 0)
						continue;
					// qDebug() << inLine;

					QStringList list = inLine.split(" ",QString::SkipEmptyParts);

					//read in vertex texture coordinates
					if (list[0] == QString("vt") && list.size() == 3)
					{
						QVector2D uv;
						uv.setX(list[1].toFloat());
						uv.setY(list[2].toFloat());
						// qDebug() << uv;
						temp_uvs.push_back(uv);
					}
					//read in vertex normals
					else if(list[0] == QString("vn") && list.size() == 4)
					{
						QVector3D normal;
						normal.setX(list[1].toFloat());
						normal.setY(list[2].toFloat());
						normal.setZ(list[3].toFloat());
						// qDebug() << normal;
						temp_normals.push_back(normal);
					}
					//read in vertices
					else if(list[0] == QString("v") && list.size() == 4)
					{
						QVector3D vertex;
						vertex.setX(list[1].toFloat());
						vertex.setY(list[2].toFloat());
						vertex.setZ(list[3].toFloat());
						// qDebug() << vertex;
						temp_vertices.push_back(vertex);
					}
					//obj files include face information, the following parses through
					//three vertices at a time (for each triangle) storing indices
					else if(list[0] == QString("f"))
					{
						for (int i = 1 ; i < list.size() ; i++)
						{
							QStringList face = list[i].split("/");
							int vertexIndex, uvIndex, normalIndex;

							if (face.size() >= 1)
							{

								vertexIndex = face[0].toInt();
								vertexIndices.push_back(vertexIndex);

							}
							if (face.size() >= 2)
							{

								uvIndex = face[1].toInt();
								uvIndices.push_back(uvIndex);

							}
							if (face.size() == 3)
							{

								normalIndex = face[2].toInt();
								normalIndices.push_back(normalIndex);
							}
						}
					}
				}

				file.close();

				if (uvIndices.size() == vertexIndices.size())
				{
					_hasTexCoords = true;
					//qDebug() << "Mesh has texture coordinates";
				} 

				if (normalIndices.size() == vertexIndices.size())
				{
					_hasNormals = true;
					//qDebug() << "Mesh has normals";
				}


				_vertexCount = vertexIndices.size();

				vertexData = (vertexData_t *)malloc(_vertexCount * sizeof(vertexData_t));
				if (vertexData != NULL)
				{
					for(int i=0; i < _vertexCount; i++)
					{
						unsigned int vertexIndex = vertexIndices[i];

						//OBJ indexing starts at 1
						QVector3D vertex = temp_vertices[vertexIndex-1];

						vertexData[i].vert.x = vertex.x();
						vertexData[i].vert.y = vertex.z();
						vertexData[i].vert.z = vertex.y();
						vertexData[i].vert.w = 1;


						if (_hasNormals)
						{
							unsigned int normalIndex = normalIndices[i];
							QVector3D normal = temp_normals[normalIndex-1];
							vertexData[i].normal.x=normal.x();
							vertexData[i].normal.y=normal.z();
							vertexData[i].normal.z=normal.y();
						} else
						{
							vertexData[i].normal.x = 0;
							vertexData[i].normal.y = 0;
							vertexData[i].normal.z = 0;
						}

						if (_hasTexCoords)
						{
							unsigned int uvIndex = uvIndices[i];
							QVector2D uv = temp_uvs[uvIndex-1];
							vertexData[i].tex.s=uv.x();
							vertexData[i].tex.t=uv.y();
						} else
						{
							vertexData[i].tex.s = 0;
							vertexData[i].tex.t = 0;
						}

					}
					qDebug() << "Successfully parsed" << _vertexCount << "vertices";

					// build list of unique verticies
					QVector<vertexData_t> indexedVertices;
					QVector<GLuint> indices;
					for (int i = 0; i < _vertexCount ; i++)
					{
						int loc = 0;
						bool found = false;
						while (loc < indexedVertices.count() && found == false)
						{
							vertexData_t test = indexedVertices[loc];
							if (test.vert.x == vertexData[i].vert.x && test.vert.y == vertexData[i].vert.y && 
								test.vert.z == vertexData[i].vert.z && test.vert.w == vertexData[i].vert.w)
							{
								if (test.normal.x == vertexData[i].normal.x && test.normal.y == vertexData[i].normal.y && 
									test.normal.z == vertexData[i].normal.z)
								{
									if (test.tex.s == vertexData[i].tex.s && test.tex.t == vertexData[i].tex.t)
									{
										//			qDebug() << "Found duplicate vertex at" << loc;
										found = true;
									}
								}
							}

							if (!found)
								loc++;
						}

						if (!found)
							indexedVertices.append(vertexData[i]);

						indices.append(loc);

					}

					qDebug() << "Found" << indexedVertices.count() << "unique vertices";

					if (indices.count() > 0) 
					{
						//qDebug() << "Building indexed mesh...";


						arrayIndex = (GLuint *) malloc(indices.count() * sizeof(GLuint));
						if (arrayIndex != NULL)
							memcpy(arrayIndex,indices.constData(),indices.count() * sizeof(GLuint));

						indexedVertexData = (vertexData_t *) malloc(indexedVertices.count() * sizeof(vertexData_t));
						if (indexedVertexData != NULL)
							memcpy(indexedVertexData,indexedVertices.constData(),indexedVertices.count() * sizeof(vertexData_t));

						if (arrayIndex != NULL && indexedVertexData != NULL)
						{
							_hasIndex = true;
							_indexCount = indices.count();
							_indexedVertexCount = indexedVertices.count();
							qDebug() << "Indexed mesh generated with" << indices.count() << "indices and" << indexedVertices.count() << "vertices";
						} else
						{

							if (indexedVertexData != NULL)
								free(indexedVertexData);
							indexedVertexData = NULL;
							if (arrayIndex != NULL)
								free(arrayIndex);
							arrayIndex = NULL;
							_indexCount = 0;
							_indexedVertexCount = 0;
						}
					}
				}
			}
		} else 
		{
			qDebug() << "Error: file " << fileName << " does not exist";
			return false;
		}


		if (vertexData != NULL)
		{
			glGenBuffers(1,&_VBOID);

			if (_hasIndex)
			{
				glBindBuffer(GL_ARRAY_BUFFER,_VBOID);
				glBufferData(GL_ARRAY_BUFFER,_indexedVertexCount*sizeof(vertexData_t),indexedVertexData,GL_STATIC_DRAW);
				glGenBuffers(1,&_indexBufferID);
				qDebug() << "Created VBO ID " <<_VBOID << "and IB ID" << _indexBufferID;


				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_indexBufferID);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indexCount * sizeof(GLuint), arrayIndex, GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
				glBindBuffer(GL_ARRAY_BUFFER,0);

			} else {
				glBindBuffer(GL_ARRAY_BUFFER,_VBOID);
				glBufferData(GL_ARRAY_BUFFER,_vertexCount*sizeof(vertexData_t),vertexData,GL_STATIC_DRAW);
				qDebug() << "Created VBO ID " <<_VBOID;
				glBindBuffer(GL_ARRAY_BUFFER,0);

			}

			//	free(vertexData);
			_modelLoaded = true;

		}
		if (indexedVertexData != NULL)
			free(indexedVertexData);
		indexedVertexData = NULL;
		if (arrayIndex != NULL)
			free(arrayIndex);
		arrayIndex = NULL;
		if (vertexData != NULL)
			free(vertexData);
		vertexData = NULL;

		return true;
	} else {
		return false;
	}
}

void MeshManager::bindBuffer()
{
	if (!_modelLoaded)
	{
		loadFile(_fileName);
	}
	if (_modelLoaded)
	{
		glBindBuffer(GL_ARRAY_BUFFER,_VBOID);

		if (_hasIndex)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_indexBufferID);
		}
	} 

}



void MeshManager::unbindBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

// these are, for the time being, basically hard coded values
// the intent is that this class could be modified to use structs that are more memory efficient and these values might change

GLuint MeshManager::getVertexStride()
{
	return (sizeof(vertexData_t));
}
GLuint MeshManager::getTexStride()
{
	return (sizeof(vertexData_t));

}
GLuint MeshManager::getNormalStride()
{
	return (sizeof(vertexData_t));
}
GLuint MeshManager::getVertexOffset()
{
	return (0);
}
GLuint MeshManager::getTexOffset()
{
	return (7 * sizeof(float));
}
GLuint MeshManager::getNormalOffset()
{
	return (4 * sizeof(float));
}

GLuint MeshManager::getNumVertices()
{
	return _vertexCount;
}

void MeshManager::enableAttributeArrays()
{
	if (!_modelLoaded && _fileSet)
	{
		loadFile(_fileName);
	}
	if (_modelLoaded)
	{
		if (_shaderAttribLocs.vertex > -1)
		{
			glVertexAttribPointer(_shaderAttribLocs.vertex,4,GL_FLOAT,GL_FALSE,getVertexStride(),BUFFER_OFFSET(getVertexOffset()));
			glEnableVertexAttribArray(_shaderAttribLocs.vertex);
		}
		if (_hasNormals && _shaderAttribLocs.normal > -1)
		{
			glVertexAttribPointer(_shaderAttribLocs.normal,3,GL_FLOAT,GL_FALSE,getNormalStride(),BUFFER_OFFSET(getNormalOffset()));
			glEnableVertexAttribArray(_shaderAttribLocs.normal);
		}
		if (_hasTexCoords && _shaderAttribLocs.texCoords > -1)
		{
			glVertexAttribPointer(_shaderAttribLocs.texCoords,2,GL_FLOAT,GL_FALSE,getTexStride(),BUFFER_OFFSET(getTexOffset()));
			glEnableVertexAttribArray(_shaderAttribLocs.texCoords);
		}

	}
}

void MeshManager::disableAttributeArrays()
{
	if (!_modelLoaded && _fileSet)
	{
		loadFile(_fileName);
	}
	if (_modelLoaded)
	{
		if (_shaderAttribLocs.vertex > -1)
			glDisableVertexAttribArray(_shaderAttribLocs.vertex);
		if (_hasNormals && _shaderAttribLocs.normal > -1)
		{
			glDisableVertexAttribArray(_shaderAttribLocs.normal);
		}
		if (_hasTexCoords && _shaderAttribLocs.texCoords > -1)
		{
			glDisableVertexAttribArray(_shaderAttribLocs.texCoords);
		}

	}
}
void MeshManager::draw()
{
	if (!_modelLoaded && _fileSet)
	{
		loadFile(_fileName);
	}
	if (_modelLoaded)
	{

		if (_hasIndex)
		{

			glDrawElements(GL_TRIANGLES,_indexCount,GL_UNSIGNED_INT,0);

		} else {
			glDrawArrays(GL_TRIANGLES,0,_vertexCount);
		}
	}
}

void MeshManager::setAttribLoc(int locVertex, int locNormal, int locTexCoords)
{
	_shaderAttribLocs.vertex = locVertex;
	_shaderAttribLocs.normal = locNormal;
	_shaderAttribLocs.texCoords = locTexCoords;
}

void MeshManager::freeModel()
{
	if (_modelLoaded)
	{
		glDeleteBuffers(1,&_VBOID);
		_VBOID = 0;
		_modelLoaded = false;
		_hasIndex = false;
		_hasTexCoords = false;
		_hasNormals = false;
		_indexCount = 0;
		_indexedVertexCount = 0;
		_vertexCount = 0;
		_indexBufferID = 0;
	}
}
