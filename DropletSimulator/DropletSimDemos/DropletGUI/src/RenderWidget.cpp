/**
* \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\src\RenderWidget.cpp
*
* \brief	Implements the main portion of the RenderWidget class.
*/

#include "RenderWidget.h"

RenderWidget::RenderWidget(const QGLFormat& format, QWidget *parent)
	: QGLWidget(format, parent)
{
	assets.setAssetDir(DEFAULT_ASSETDIR);
	assets.setShaderDir(DEFAULT_SHADERDIR);
	assets.setMeshDir(DEFAULT_MESHDIR);
	assets.setTextureDir(DEFAULT_TEXTUREDIR);

	_assetsLoaded = false;
	_renderLock = false;
	_simStateLock = false;
	_renderDebug = 0;
	_hud = true;
	_drawHelp = false;
	_hudInfo.paused = false;
	// initialize the camera
	_camera.x = 0;
	_camera.y = 0;
	_camera.z = 0;
	_camera.tilt = 0;
	_camera.pan = 0;
	_camera.radius = 100.0f;
	_camera.rotHoriz = 180;
	_camera.rotVert = 0;
	_camera.mode = 0;
	_camera.viewMatrix.setToIdentity();
	_camera.lightDir = glm::vec3(1,1,3);
	updateCamera();

	_timerID = 0;
	// start out not unpaused

	// intialize key status
	_keysDown.Q = false;
	_keysDown.W = false;
	_keysDown.E = false;
	_keysDown.A = false;
	_keysDown.S = false;
	_keysDown.D = false;
	_keysDown.Plus = false;
	_keysDown.Minus = false;

	_projectionTexture.width = 0;
	_projectionTexture.height = 0;
	_projectionTexture.handle = 0;
	_projectionTexture.valid = false;
	QTime time = QTime::currentTime();
	qsrand((uint)time.msec());
	_hudInfo.framesSinceLastUpdate = 99;
}

RenderWidget::~RenderWidget()
{
	//delete _sim;
	if (_timerID != 0)
	{
		killTimer(_timerID);
		_timerID = 0;
	}
	makeCurrent();

	if (_projectionTexture.valid)
	{
		glDeleteTextures(1,&_projectionTexture.handle);
		_projectionTexture.valid = false;
		_projectionTexture.handle = 0;
	}
	assets.clearAssets();
	_assetsLoaded = false;
	doneCurrent();
}

QSize RenderWidget::sizeHint() const
{
	return QSize(1280, 720);
}


void RenderWidget::initializeGL()
{

	ogl_LoadFunctions();

	// depth testing and culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glEnable(GL_TEXTURE_2D);   
	// set the clear color to black
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	qDebug() << "Initializing OpenGL";
	// load all assets
	_assetsLoaded = assets.loadAssets();

	_dropletStruct.color = glm::vec4(0,0,0,1);
	_dropletStruct.origin = glm::vec3(0,0,0);
	_dropletStruct.rotation = glm::vec3(0,0,0);
	_dropletStruct.scale = glm::vec3(1,1,1);
	_dropletStruct.mesh = assets.getMesh(assets.lookupAssetName(DROPLET_MESH_NAME));
	_dropletStruct.baseShader = assets.getShader(assets.lookupAssetName(DROPLET_SHADER_NAME));
	_dropletStruct.projShader = assets.getShader(assets.lookupAssetName(DROPLET_PROJECTION_SHADER_NAME));
	_dropletStruct.texture_0 = assets.getTexture(assets.lookupAssetName(DROPLET_TEXTURE_NAME));

	setupRenderStructs();
	_arenaObjects.clear();
	//setupArena();
	setFPS(60);
	//_runTime.start();
	_updateTimer.start();
	_timerID = startTimer(_targetFrameTime);
}	

void RenderWidget::setFPS(int FPS)
{	

	if (FPS > 0)
	{
		_targetFPS = FPS;
		_targetFrameTime = 1000.0/FPS;

		qDebug() << "Set render goal for" << (int) floor(1000.0/FPS) << "ms";
	} else
	{
		_targetFPS = 0;
		_targetFrameTime = 0;
	}
	_renderTimer.start();
}



void RenderWidget::resizeGL(int width, int height)
{
	if (height == 0) {
		height = 1;
	}	

	_camera.projectionMatrix.setToIdentity();
	_camera.projectionMatrix.perspective(60.0, (float) width / (float) height, 1, 1000);
	glViewport(0, 0, width, height);
}

void RenderWidget::updateCamera()
{
	// clamp camera values


	if (_camera.pan > 360)
	{
		_camera.pan -= 360;
	} else if (_camera.pan < 0)
	{
		_camera.pan += 360;
	}

	if (_camera.tilt > 90)
	{
		_camera.tilt = 90;
	} else if (_camera.tilt < -90)
	{
		_camera.tilt = -90;
	}

	/* Clamp camera rotations */
	if (_camera.rotHoriz > 360)
	{
	_camera.rotHoriz -= 360;
	} else if (_camera.rotHoriz < 0)
	{
	_camera.rotHoriz += 360;
	}

	if (_camera.rotVert > 90)
	{
	_camera.rotVert = 90;
	} else if (_camera.rotVert < 0)
	{
	_camera.rotVert = 0;
	}
	
	/* Clamp zooming in/out */
	if (_camera.radius < 5)
	{
		_camera.radius = 5;
	} else if (_camera.radius > 300)
	{
		_camera.radius = 300;
	}


	// If in the default camera mode
	// Convert the rotations around a sphere into xyz coordinate
	if (_camera.mode == 0)
	{	
		// generate x, y, z and pan/tilt angles off of rotation around a sphere

		_camera.y = _camera.radius * sin(_camera.rotVert * M_PI / 180.0) * cos(_camera.rotHoriz * M_PI / 180.0);
		_camera.x = _camera.radius * sin(_camera.rotVert * M_PI / 180.0) * sin(_camera.rotHoriz * M_PI / 180.0);
		_camera.z = _camera.radius * cos(_camera.rotVert * M_PI / 180.0);

		_camera.pan = 180 - _camera.rotHoriz;
		_camera.tilt = -90 +  _camera.rotVert;
	}


	_camera.viewMatrix.setToIdentity();
	_camera.viewMatrix.rotate(-90,1,0,0);
	_camera.viewMatrix.rotate(_camera.tilt,-1,0,0);
	_camera.viewMatrix.rotate(_camera.pan,0,0,-1);
	_camera.viewMatrix.translate(-_camera.x,-_camera.y,-_camera.z);
	//	_camera.viewMatrix.optimize();
}

void RenderWidget::paintGL()
{
	if (!_renderLock && _assetsLoaded)
	{
		_renderLock = true;
		_eventTimer.start();
		
		/* get new state */

		// block while resource is in use, then retain resource
		while(!_simStateLock.testAndSetOrdered(0,1)) {}

		// update the local state with a more recent one
		_renderState = _simState;

		// release resource
		_simStateLock.fetchAndStoreOrdered(0);


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (_renderDebug > 1)
		{
			glDisable(GL_CULL_FACE);
		}

		if (!_projectionTexture.valid && _arena.projecting)
		{
			if (_projectionTexture.handle != 0)
			{
				glDeleteTextures(1,&_projectionTexture.handle);
			}
			glGenTextures(1,&_projectionTexture.handle);
			_projectionTexture.valid = true;
		}

		if ( _arena.projecting && _projectionTexture.valid)
		{
			if (_projectionTexture.texture != _renderState.projTexture)
			{
				_projectionTexture.texture = _renderState.projTexture;
				glActiveTexture(GL_TEXTURE1);
				QImage image = QGLWidget::convertToGLFormat(_projectionTexture.texture);
				_projectionTexture.width = image.width();
				_projectionTexture.height = image.height();

				glBindTexture(GL_TEXTURE_2D, _projectionTexture.handle);
				glTexImage2D(GL_TEXTURE_2D, 0, 4, _projectionTexture.width, _projectionTexture.height, 0,  GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); // Enable Mipmapping
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
				glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps
				glActiveTexture(GL_TEXTURE0);

			} else {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, _projectionTexture.handle);
				glActiveTexture(GL_TEXTURE0);
			}
		}

		//drawArena();
		drawDroplets();
		drawObjects();

		if (_arena.projecting && _projectionTexture.valid)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D,0);
			glActiveTexture(GL_TEXTURE0);
		}
		if (_renderDebug > 1)
		{
			glEnable(GL_CULL_FACE);
			glFrontFace(GL_CW);
		}

		glFinish();

		qint64 nsElapsed = _eventTimer.nsecsElapsed();
		_msElapsedRenderer = nsElapsed / 1000000.0;

		_hudInfo.framesSinceLastUpdate++;


		if (_hud)
		{
			drawHUD();
		}
		_renderLock = false;
	}
}


void RenderWidget::drawArena()
{
	QGLShaderProgram *currentShader = NULL;
	MeshManager *currentMesh = NULL;
	TextureManager *currentTex0 = NULL;

	GLint lLoc, cLoc, mLoc;
	GLint t0Loc,t1Loc;
	GLint pLoc;

	float floorWidth = _arena.tileLength * _arena.numColTiles;
	float floorLength = _arena.tileLength * _arena.numRowTiles;
	glm::vec2 lengths = glm::vec2(floorWidth,floorLength);		

	// for every object in the arena 
	foreach(renderStruct_t object,_arenaObjects)
	{
		// new variables
		QGLShaderProgram *newShader;
		MeshManager *newMesh;
		TextureManager *newTex0;

		/* set current render object shader, mesh, and textures */
		// Check if visual debugging is on
		if (_renderDebug > 1)
		{
			// if debugging is on, use the debug assets instead of checking the object properties
			newShader = assets.getShader(DEBUG_SHADER);
			newMesh = assets.getMesh(DEBUG_MESH);
			newTex0 = NULL;
		} else
		{
			// if not debugging set new variables to object properties
			if (_arena.projecting)
			{
				newShader = object.projShader;

			} else {
				newShader = object.baseShader;
			}
			newMesh = object.mesh;
			newTex0 = object.texture_0;

		}


		/* compare existing state to new state */

		// rebind shaders
		if (currentShader != newShader)
		{
			// release the current shader if bound
			if (currentShader != NULL)
			{
				currentShader->release();
			}
			currentShader = newShader;
			// bind uniform attributes
			if (currentShader != NULL)
			{
				currentShader->bind();
				currentShader->setUniformValue("in_Projection",_camera.projectionMatrix);
				currentShader->setUniformValue("in_View",_camera.viewMatrix);
				lLoc = currentShader->uniformLocation("in_lightDir");
				glUniform3fv(lLoc,1,glm::value_ptr(_camera.lightDir));
				cLoc = currentShader->uniformLocation("in_Color");
				mLoc = currentShader->uniformLocation("in_Model");
				t0Loc = currentShader->uniformLocation("objectTexture");
				t1Loc = currentShader->uniformLocation("projectionTexture");
				pLoc = currentShader->uniformLocation("in_ProjOffsets");
				glUniform2fv(pLoc,1,glm::value_ptr(lengths));
				glUniform1i(t0Loc,0);
				glUniform1i(t1Loc,1);
			}
		}

		// test and rebind texture 0
		if (currentTex0 != newTex0)
		{
			glActiveTexture(GL_TEXTURE0);

			if (currentTex0 != NULL)
				currentTex0->unbindTexture();
			currentTex0 = newTex0;

			if (currentTex0 != NULL)
			{
				currentTex0->bindTexture();
			}
		}




		// test and rebind mesh
		if (currentMesh != newMesh)
		{
			if (currentMesh != NULL)
			{
				currentMesh->disableAttributeArrays();
				currentMesh->unbindBuffer();
			}

			currentMesh = newMesh;

			if (currentMesh != NULL)
			{
				currentMesh->bindBuffer();
				currentMesh->enableAttributeArrays();
			}
		}

		/* now that all properties are bound, draw */

		// error testing
		if (currentShader != NULL && currentMesh != NULL)
		{
			// bind color and model matrix uniforms
			glUniform4fv(cLoc,1,glm::value_ptr(object.color));
			glUniformMatrix4fv(mLoc,1,GL_FALSE,glm::value_ptr(object.modelMatrix));

			// draw
			currentMesh->draw();
		}

	} // end foreach

	/* all objects are rendered, so unbind all assets */

	if (currentShader != NULL)
	{
		currentShader->release();
	}

	if (currentMesh != NULL)
	{
		currentMesh->disableAttributeArrays();
		currentMesh->unbindBuffer();
	}


	if (currentTex0 != NULL)
	{
		glActiveTexture(GL_TEXTURE0);
		currentTex0->unbindTexture();
	}

}

void RenderWidget::drawDroplets()
{
	if (_renderState.dropletData.count() > 0)
	{
		QGLShaderProgram *currentShader = NULL;
		MeshManager *currentMesh = NULL;
		TextureManager *currentTex0 = NULL;

		GLint lLoc, cLoc, mLoc;
		GLint t0Loc,t1Loc;
		GLint pLoc;
		glActiveTexture(GL_TEXTURE0);

		// check if rendering debugging is on
		if (_renderDebug > 1)
		{
			currentShader = assets.getShader(DEBUG_SHADER);
			//currentMesh = assets.getMesh(DEBUG_DROPLET_MESH);

			currentTex0 = NULL;

		} else
		{
			if (_arena.projecting)
			{
				currentShader = _dropletStruct.projShader;
			} else {
				currentShader = _dropletStruct.baseShader;

			}

			currentTex0 = _dropletStruct.texture_0;
		}

		currentMesh = _dropletStruct.mesh;
		if (currentShader != NULL && currentMesh != NULL)
		{

			// bind and set up droplet shader and mesh
			currentShader->bind();
			currentShader->setUniformValue("in_Projection",_camera.projectionMatrix);
			currentShader->setUniformValue("in_View",_camera.viewMatrix);
			//_droplet->setUniformValue("in_lightDir",0.5,1,1);
			lLoc = currentShader->uniformLocation("in_lightDir");
			glUniform3fv(lLoc,1,glm::value_ptr(_camera.lightDir));
			t0Loc = currentShader->uniformLocation("objectTexture");
			t1Loc = currentShader->uniformLocation("projectionTexture");
			glUniform1i(t0Loc,0);
			glUniform1i(t1Loc,1);
			currentMesh->bindBuffer();
			currentMesh->enableAttributeArrays();

			cLoc = currentShader->uniformLocation("in_Color");
			mLoc = currentShader->uniformLocation("in_Model");
			pLoc = currentShader->uniformLocation("in_ProjOffsets");
			float floorWidth = _arena.tileLength * _arena.numColTiles;
			float floorLength = _arena.tileLength * _arena.numRowTiles;
			glm::vec2 lengths = glm::vec2(floorWidth,floorLength);		
			// draw each droplet
			glUniform2fv(pLoc,1,glm::value_ptr(lengths));

			if (currentTex0 != NULL)
			{
				glActiveTexture(GL_TEXTURE0);
				currentTex0->bindTexture();

			}


			foreach(dropletStruct_t droplet,_renderState.dropletData)
			{
				// make the model matrix
				glm::vec3 origin = glm::vec3( droplet.origin.x, droplet.origin.y, droplet.origin.z);

				glm::quat quaternion = glm::quat(droplet.quaternion.w,droplet.quaternion.x,
					droplet.quaternion.y,droplet.quaternion.z);

				glm::mat4 model =  glm::translate(glm::mat4(1.0f),origin);
				model = model * glm::mat4_cast(quaternion);
				model = glm::scale(model,glm::vec3(_arena.dropletRadius));
				model = glm::translate(model,glm::vec3(0,0,_arena.dropletOffset));


				vec4 color = {
					droplet.color.r / 255.0f,
					droplet.color.g / 255.0f,
					droplet.color.b / 255.0f,
					1.0f
				};

				// bind droplet uniforms
				glUniform4fv(cLoc,1,color.v);
				glUniformMatrix4fv(mLoc,1,GL_FALSE,glm::value_ptr(model));
				//	glDrawArrays(GL_TRIANGLES,0,vertCount);
				currentMesh->draw();

			}
			currentMesh->disableAttributeArrays();
			currentMesh->unbindBuffer();
			currentShader->release();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,0);

		}
		glActiveTexture(GL_TEXTURE0);
	}
}
void RenderWidget::drawObjects()
{
	QVector<objectStruct_t> objects = _renderState.dynamicObjectData + _renderState.staticObjectData;
	if (objects.count() > 0){
		QGLShaderProgram *currentShader = NULL;
		MeshManager *currentMesh = NULL;
		TextureManager *currentTex0 = NULL;

		GLint lLoc, cLoc, mLoc;
		GLint t0Loc,t1Loc;
		GLint pLoc;


		glActiveTexture(GL_TEXTURE0);

		float floorWidth = _arena.tileLength * _arena.numColTiles;
		float floorLength = _arena.tileLength * _arena.numRowTiles;
		glm::vec2 lengths = glm::vec2(floorWidth,floorLength);

		// draw each object

		// for every object in the arena
		foreach(objectStruct_t object,objects)
		{
			// new variables
			QGLShaderProgram *newShader;
			MeshManager *newMesh;
			TextureManager *newTex0;

			/* set current render object shader, mesh, and textures */
			// check if debuggin is on
			if (_renderDebug > 1)
			{
				// if debugging is on, use the debug assets instead of checking the object properties
				newShader = assets.getShader(DEBUG_SHADER);
				newTex0 = NULL;

			} else
			{
				// if not debugging set new variables to object properties
				if (_arena.projecting)
				{
					newShader = _objectStructs[object.oType].projShader;
				} else {
					newShader = _objectStructs[object.oType].baseShader;

				}
				newTex0 = _objectStructs[object.oType].texture_0;
			}

			// render the proper mesh regardless of debug mode
			newMesh = _objectStructs[object.oType].mesh;

			/* compare existing state to new state */

			// rebind shaders
			if (currentShader != newShader)
			{
				// release the current shader if bound
				if (currentShader != NULL)
				{
					currentShader->release();
				}
				currentShader = newShader;
				// bind uniform attributes
				if (currentShader != NULL)
				{
					currentShader->bind();
					currentShader->setUniformValue("in_Projection",_camera.projectionMatrix);
					currentShader->setUniformValue("in_View",_camera.viewMatrix);
					lLoc = currentShader->uniformLocation("in_lightDir");
					glUniform3fv(lLoc,1,glm::value_ptr(_camera.lightDir));
					cLoc = currentShader->uniformLocation("in_Color");
					mLoc = currentShader->uniformLocation("in_Model");
					t0Loc = currentShader->uniformLocation("objectTexture");
					t1Loc = currentShader->uniformLocation("projectionTexture");
					pLoc = currentShader->uniformLocation("in_ProjOffsets");
					glUniform2fv(pLoc,1,glm::value_ptr(lengths));
					glUniform1i(t0Loc,0);
					glUniform1i(t1Loc,1);
				}
			}

			// test and rebind texture 0
			if (currentTex0 != newTex0)
			{
				glActiveTexture(GL_TEXTURE0);

				if (currentTex0 != NULL)
					currentTex0->unbindTexture();
				currentTex0 = newTex0;

				if (currentTex0 != NULL)
				{
					currentTex0->bindTexture();
				}
			}

			// test and rebind mesh
			if (currentMesh != newMesh)
			{
				if (currentMesh != NULL)
				{
					currentMesh->disableAttributeArrays();
					currentMesh->unbindBuffer();
				}

				currentMesh = newMesh;

				if (currentMesh != NULL)
				{
					currentMesh->bindBuffer();
					currentMesh->enableAttributeArrays();
				}
			}

			/* now that all properties are bound, draw */

			// error testing
			if (currentShader != NULL && currentMesh != NULL)
			{
				glm::vec3 origin = glm::vec3( object.origin.x, object.origin.y, object.origin.z);

				glm::quat quaternion = glm::quat(object.quaternion.w,object.quaternion.x,
					object.quaternion.y,object.quaternion.z);

				glm::mat4 model =  glm::translate(glm::mat4(1.0f),origin);

				model = model * glm::mat4_cast(quaternion);
				model = glm::scale(model,glm::vec3(object.objectRadius*object.scale.x,object.objectRadius*object.scale.y,object.objectRadius*object.scale.z));

				vec4 color = {
					object.color.r / 255.0f,
					object.color.g / 255.0f,
					object.color.b / 255.0f,
					1.0f
				};

				// bind color and model matrix uniforms
				glUniform4fv(cLoc,1,color.v);
				glUniformMatrix4fv(mLoc,1,GL_FALSE,glm::value_ptr(model));

				// draw
				currentMesh->draw();
			}


		} // end foreach

		/* all objects are rendered, so unbind all assets */

		if (currentShader != NULL)
		{
			currentShader->release();
		}

		if (currentMesh != NULL)
		{
			currentMesh->disableAttributeArrays();
			currentMesh->unbindBuffer();
		}

		if (currentTex0 != NULL)
		{
			glActiveTexture(GL_TEXTURE0);
			currentTex0->unbindTexture();
		}

	}
}

// TIMER
void RenderWidget::drawHUD()
{

	//QTime time = _renderState.realTime;
	double time = _renderState.realTime;
	if (_hudInfo.framesSinceLastUpdate > 20)
	{
		_hudInfo.framesSinceLastUpdate = 0;

		// TIMER
		/*
		int deltaRT = time.msecsTo(_hudInfo.lastRunTime);
		int deltaSim = _renderState.simTime.msecsTo(_hudInfo.lastSimTime);


		_hudInfo.lastRunTime = time;
		_hudInfo.lastSimTime = _renderState.simTime;
		float timeRatio = (float) deltaSim / (float) deltaRT;
		*/
		double timeRatio = _renderState.timeRatio;
		_hudInfo.simRealTimeRatio = floor(timeRatio*10000.0 + 0.5) / 10000.0;


	}
	// render the current screen size into the window


	int baseOffset = 20;
	QString text = QString("Number of droplets: %1").arg(_renderState.dropletData.count(),0);
	renderText(10,baseOffset,text,this->font());
	baseOffset += 20;

	text = QString("Simulator step size: %1 ms").arg(_hudInfo.simStepSize,0);
	renderText(10,baseOffset,text,this->font());
	baseOffset += 20;

	int secs = _renderState.simTime;
	int mSecs = (_renderState.simTime - secs) * 1000.0;
	QTime iTime = QTime(0,0,0,0).addSecs(secs).addMSecs(mSecs);

	text = QString("Simulator time elapsed: %1").arg(iTime.toString(QString("H : mm : ss.zzz")));
	//text = QString("Simulator time elapsed: %1").arg(floor(_renderState.simTime*100.0 + 0.5) / 100.0,0);
	renderText(10,baseOffset,text,this->font());
	baseOffset += 20;


	secs = floor(_renderState.realTime);
	mSecs = (_renderState.realTime - secs) * 1000.0;
	iTime = QTime(0,0,0,0).addSecs(secs).addMSecs(mSecs);
	text = QString("Real time elapsed: %1").arg(iTime.toString(QString("H : mm : ss.zzz")));
	//text = QString("Real time elapsed: %1").arg(floor(_renderState.realTime*100.0 + 0.5) / 100.0,0);
	renderText(10,baseOffset,text,this->font());
	baseOffset += 20;


	if (_simRates.limitRate)
	{
		text = QString("Requested simulator/real time ratio: %1x").arg(_simRates.timeScale,2);
		renderText(10,baseOffset,text,this->font());
		baseOffset += 20;
	}

	if (_hudInfo.paused)
	{
		text = QString("PAUSED");
		renderText(10,baseOffset,text,this->font());
		baseOffset += 20;
	} else {

		// If you display this while the display is paused it will be invalid data
		text = QString("Estimated simulator/real time ratio: %1x").arg(_hudInfo.simRealTimeRatio,2);
		renderText(10,baseOffset,text,this->font());
		baseOffset += 20;
	}

	// if *any* level of debugging is enabled
	if (_renderDebug > 0)
	{

		// print off a whole slew of debug information related to rendering
		QString width;
		QString height;
		width.setNum(this->width());
		height.setNum(this->height());

		QString text = QString("Res: ").append(width).append(" x ").append(height);
		renderText(10,baseOffset,text,this->font());
		baseOffset += 20;


		int numObjects = _arenaObjects.count() + _renderState.dropletData.count() + _renderState.dynamicObjectData.count() + _renderState.staticObjectData.count();
		text = QString("Number of render objects: %1").arg(numObjects,0);
		renderText(10,baseOffset,text,this->font());
		baseOffset += 20;

		// draw the frame time
		QString frameTime = QString("%1").arg(_msElapsedRenderer,0,'g',4);
		text = QString("Frame Time: ").append(frameTime).append(" ms");
		renderText(10,baseOffset,text,this->font());
		baseOffset += 20;

		if (_camera.mode == 0)
		{
			text = QString("Fixed camera - Elevation: %1  Rotation: %2 Radius: %3").arg(90 - _camera.rotVert,0,'g',3).arg(_camera.rotHoriz,0,'g',3).arg(_camera.radius,0,'g',3);
			renderText(10,baseOffset,text,this->font());
		} else if (_camera.mode == 1)
		{
			text = QString("Free camera - X: %1  Y: %2 Z: %3 Pan: %4 Tilt: %5").arg(_camera.x,0,'g',4).arg(_camera.y,0,'g',4).arg(_camera.z,0,'g',2).arg(_camera.pan,0,'g',3).arg(_camera.tilt,0,'g',3);
			renderText(10,baseOffset,text,this->font());
		}
		baseOffset += 20;

		// if debug rendering is enabled
		if (_renderDebug > 1)
		{
			text = QString("DEBUG RENDERING");
			renderText(10,baseOffset,text,this->font());
			baseOffset += 20;
		}
	}
	if (_camera.mode)
	{
		text = QString("Free camera");
		renderText(10,baseOffset,text,this->font());
		baseOffset += 20;

	}


	if (_drawHelp)
	{
		baseOffset = height() - 10;
		text = QString("Control-R: Reset simulator to starting state");
		renderText(10,baseOffset,text,this->font());
		baseOffset -= 20;
		text = QString("Control-L: Force reloading of rendering assets");
		renderText(10,baseOffset,text,this->font());
		baseOffset -= 20;
		text = QString("Control-B: Toggle debug rendering on/off");
		renderText(10,baseOffset,text,this->font());
		baseOffset -= 20;
		text = QString("Control-H: Toggle HUD on/off");
		renderText(10,baseOffset,text,this->font());
		baseOffset -= 20;
		text = QString("Escape: Close this window");
		renderText(10,baseOffset,text,this->font());
		baseOffset -= 20;
		text = QString("Spacebar: Change camera modes");
		renderText(10,baseOffset,text,this->font());
		baseOffset -= 20;
		text = QString("H: Toggle this help on/off");
		renderText(10,baseOffset,text,this->font());
		baseOffset -= 20;

		if (_hudInfo.paused)
		{
			text = QString("P: Unpause the simulator");
			renderText(10,baseOffset,text,this->font());
			baseOffset -= 20;
		} else {
			text = QString("P: Pause the simulator");
			renderText(10,baseOffset,text,this->font());
			baseOffset -= 20;
		}
		if (_simRates.limitRate)
		{
			text = QString("L: Disable simulation speed limiting");
			renderText(10,baseOffset,text,this->font());
			baseOffset -= 20;
			text = QString("[/]: Lower/Raise speed limit");
			renderText(10,baseOffset,text,this->font());
			baseOffset -= 20;
		} else {
			text = QString("L: Enable simulation speed limiting");
			renderText(10,baseOffset,text,this->font());
			baseOffset -= 20;
		}
		/*
		text = QString("Z: Create StickPullers droplet");
		renderText(10,baseOffset,text,this->font());
		baseOffset -= 20;
		text = QString("X: Create RGBSense droplet");
		renderText(10,baseOffset,text,this->font());
		baseOffset -= 20;
		*/

		if (_camera.mode == 0)
		{
			text = QString("Q/E and -/+: Zoom out/in");
			renderText(10,baseOffset,text,this->font());
			baseOffset -= 20;
			text = QString("A/D: Rotate right/left");
			renderText(10,baseOffset,text,this->font());
			baseOffset -= 20;
			text = QString("W/S: Rotate up/down");
			renderText(10,baseOffset,text,this->font());
			baseOffset -= 20;

		} else if (_camera.mode == 1)
		{
			text = QString("Q/E: Lower/raise camera height");
			renderText(10,baseOffset,text,this->font());
			baseOffset -= 20;
			text = QString("A/D: Strafe camera left/right");
			renderText(10,baseOffset,text,this->font());
			baseOffset -= 20;
			text = QString("W/S: Move forward/back");
			renderText(10,baseOffset,text,this->font());
			baseOffset -= 20;

		}
	}

}
void RenderWidget::saveScreenShot(int width, int height)
{
	GLint currentFBO = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,&currentFBO);

	GLuint offscreenFBO = 0;
	GLuint renderedTexture = 0;
	GLuint renderBuffer = 0;
	glGenFramebuffers(1,&offscreenFBO);
	glGenTextures(1,&renderedTexture);
	glGenRenderbuffers(1,&renderBuffer);
	qDebug() << "Created texture" << renderedTexture;
	//	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D,renderedTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
	glBindTexture(GL_TEXTURE_2D,0);

	glBindFramebuffer(GL_FRAMEBUFFER,offscreenFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,renderedTexture,0);

	glBindRenderbuffer(GL_RENDERBUFFER,renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,width,height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,renderBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		QMatrix4x4 oldCamera = _camera.projectionMatrix;

		_camera.projectionMatrix.setToIdentity();
		_camera.projectionMatrix.perspective(60., (float) width / (float) height, 1, 1000);
		glViewport(0, 0, width, height);

		bool wasHudEnabled = _hud;
		_hud = false;
		paintGL();
		_hud = wasHudEnabled;

		_camera.projectionMatrix = oldCamera;
		glViewport(0,0,this->width(),this->height());

		glBindFramebuffer(GL_FRAMEBUFFER,currentFBO);

		glBindTexture(GL_TEXTURE_2D,renderedTexture);
		uchar* image = (uchar *) malloc(width * height * 4 * sizeof(uchar));
		glGetTexImage(GL_TEXTURE_2D,0,GL_RGB,GL_UNSIGNED_BYTE,image);

		glBindTexture(GL_TEXTURE_2D,0);
		/*
		uchar* newImage = (uchar *) malloc(width * height * 3 * sizeof(uchar));

		for (int i = 0; i < width * height ; i++)
		{
		int j = i * 3;
		int k = i * 4;
		newImage[j] = image[k];
		newImage[j+1] = image[k+1];
		newImage[j+2] = image[k+2];
		}
		*/
		QImage screenShot(image,width,height,QImage::Format_RGB888);
		screenShot = screenShot.mirrored(false,true);
		QString fileName = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
		qDebug() << fileName;
		screenShot.save(QString("%1.png").arg(fileName));
		free(image);
		//		free(newImage);
	}

	glBindFramebuffer(GL_FRAMEBUFFER,currentFBO);

	glDeleteBuffers(1, &renderBuffer);
	glDeleteTextures(1, &renderedTexture);
	glDeleteFramebuffers(1, &offscreenFBO);

}
void RenderWidget::setupRenderStructs()
{
	//	setUpdateRate(_arena.fps);
	_dropletStruct.mesh = assets.getMesh(assets.lookupAssetName(DROPLET_MESH_NAME));
	_dropletStruct.baseShader = assets.getShader(assets.lookupAssetName(DROPLET_SHADER_NAME));
	_dropletStruct.projShader = assets.getShader(assets.lookupAssetName(DROPLET_PROJECTION_SHADER_NAME));
	_dropletStruct.texture_0 = assets.getTexture(assets.lookupAssetName(DROPLET_TEXTURE_NAME));

	_objectStructs.clear();
	renderStruct_t sphereStruct;
	_objectStructs.append(sphereStruct);

	_objectStructs[Sphere].mesh = assets.getMesh(assets.lookupAssetName(OBJECT_MESH_NAME));
	_objectStructs[Sphere].baseShader = assets.getShader(assets.lookupAssetName(OBJECT_SHADER_NAME));
	_objectStructs[Sphere].projShader = assets.getShader(assets.lookupAssetName(OBJECT_PROJECTION_SHADER_NAME));
	_objectStructs[Sphere].texture_0 = assets.getTexture(assets.lookupAssetName(OBJECT_TEXTURE_NAME));

	renderStruct_t cubeStruct;
	_objectStructs.append(cubeStruct);

	_objectStructs[Cube].mesh = assets.getMesh(assets.lookupAssetName(OBJECT_CUBE_MESH_NAME));
	_objectStructs[Cube].baseShader = assets.getShader(assets.lookupAssetName(OBJECT_CUBE_SHADER_NAME));
	_objectStructs[Cube].projShader = assets.getShader(assets.lookupAssetName(OBJECT_CUBE_PROJECTION_SHADER_NAME));
	_objectStructs[Cube].texture_0 = assets.getTexture(assets.lookupAssetName(OBJECT_CUBE_TEXTURE_NAME));

	renderStruct_t wallStruct;
	_objectStructs.append(wallStruct);

	_objectStructs[Wall].mesh = assets.getMesh(assets.lookupAssetName(OBJECT_CUBE_MESH_NAME));
	_objectStructs[Wall].baseShader = assets.getShader(assets.lookupAssetName(WALL_SHADER_NAME));
	_objectStructs[Wall].projShader = assets.getShader(assets.lookupAssetName(OBJECT_CUBE_PROJECTION_SHADER_NAME));
	_objectStructs[Wall].texture_0 = assets.getTexture(assets.lookupAssetName(WALL_TEXTURE_NAME));

	renderStruct_t floorStruct;
	_objectStructs.append(floorStruct);

	_objectStructs[Floor].mesh = assets.getMesh(assets.lookupAssetName(OBJECT_CUBE_MESH_NAME));
	_objectStructs[Floor].baseShader = assets.getShader(assets.lookupAssetName(FLOOR_SHADER_NAME));
	_objectStructs[Floor].projShader = assets.getShader(assets.lookupAssetName(OBJECT_CUBE_PROJECTION_SHADER_NAME));
	_objectStructs[Floor].texture_0 = assets.getTexture(assets.lookupAssetName(FLOOR_TEXTURE_NAME));

}

/*
void RenderWidget::setupArena()
{

_arenaObjects.clear();

glm::vec4 grey = glm::vec4(0.5,0.5,0.5,1);
float xTilePos, yTilePos;

// make floor tiles first
xTilePos = (-_arena.tileLength * _arena.numColTiles / 2.0f) + (_arena.tileLength/2);
yTilePos = (_arena.tileLength * _arena.numRowTiles / 2.0f) - (_arena.tileLength/2);

float xTexWidth,yTexWidth;
xTexWidth = (1.0 / _arena.numColTiles);
yTexWidth = (1.0 / _arena.numRowTiles);

// CREATE FLOOR TILES
for(int i = 0; i < _arena.numRowTiles * _arena.numColTiles; i++)
{
float xTexOffset, yTexOffset; 
if(xTilePos >= (_arena.tileLength * _arena.numColTiles / 2.0f))
{
xTilePos = (-_arena.tileLength * _arena.numColTiles / 2.0f) + (_arena.tileLength/2);
yTilePos -= _arena.tileLength;
}

xTexOffset = (xTilePos - (_arena.tileLength / 2.0) + (_arena.tileLength * _arena.numColTiles / 2.0f)) / (_arena.tileLength * _arena.numColTiles);
yTexOffset = (yTilePos - (_arena.tileLength / 2.0) + (_arena.tileLength * _arena.numRowTiles / 2.0f)) / (_arena.tileLength * _arena.numRowTiles);
//	qDebug() << xTexOffset << yTexOffset;
renderStruct_t floor;
floor.color = grey;
//floor.color = glm::vec4(qrand()/(float) RAND_MAX,qrand()/(float) RAND_MAX,qrand()/(float) RAND_MAX,1);
floor.origin = glm::vec3(xTilePos, yTilePos, -0.05f);
floor.scale = glm::vec3(_arena.tileLength, _arena.tileLength,  0.1f);
floor.rotation = glm::vec3(0,0,0);
floor.modelMatrix = makeModelMatrix(floor.scale,floor.rotation,floor.origin);
floor.mesh = assets.getMesh(assets.lookupAssetName(FLOOR_MESH_NAME));
floor.baseShader = assets.getShader(assets.lookupAssetName(FLOOR_SHADER_NAME));
floor.projShader = assets.getShader(assets.lookupAssetName(FLOOR_PROJECTION_SHADER_NAME));
floor.texture_0 = assets.getTexture(assets.lookupAssetName(FLOOR_TEXTURE_NAME));
floor.texture_1 = assets.getProjection(_arena.projTexture);
_arenaObjects.push_back(floor);
xTilePos += _arena.tileLength;

}

//walls
for(int j = 0; j < 2; j++)	
{
if(j < 1)
{
yTilePos = -_arena.tileLength * _arena.numRowTiles / 2.0f + _arena.tileLength/2.0f;
xTilePos = -_arena.tileLength * _arena.numColTiles / 2.0f + _arena.tileLength/2.0f;
}
else
{
yTilePos = _arena.tileLength * _arena.numRowTiles / 2.0f - _arena.tileLength/2.0f;
xTilePos = _arena.tileLength * _arena.numColTiles / 2.0f - _arena.tileLength/2.0f;
}

for(int i = 0; i < _arena.numRowTiles; i++)
{
float xPos; 
if (j < 1)
{
xPos = -_arena.tileLength * _arena.numColTiles/2.0f - _arena.wallWidth/2.0f;
}
else{
xPos = _arena.tileLength * _arena.numColTiles/2.0f + _arena.wallWidth/2.0f;
}
float zpos = (.5 * _arena.wallHeight) - (.5 * 1) - .5;

renderStruct_t wall;
//	wall.color = glm::vec4(qrand()/(float) RAND_MAX,qrand()/(float) RAND_MAX,qrand()/(float) RAND_MAX,1);
wall.color = grey;
wall.origin = glm::vec3(xPos,yTilePos,zpos);
wall.scale = glm::vec3(_arena.wallWidth,_arena.tileLength,_arena.wallHeight);
wall.rotation = glm::vec3(0,0,0);
wall.modelMatrix = makeModelMatrix(wall.scale,wall.rotation,wall.origin);

wall.mesh = assets.getMesh(assets.lookupAssetName(WALL_MESH_NAME));
wall.baseShader = assets.getShader(assets.lookupAssetName(WALL_SHADER_NAME));
wall.projShader = assets.getShader(assets.lookupAssetName(WALL_SHADER_NAME));
wall.texture_0 = assets.getTexture(assets.lookupAssetName(WALL_TEXTURE_NAME));
wall.texture_1 = NULL;



_arenaObjects.push_back(wall);


if(j < 1) yTilePos += _arena.tileLength; else yTilePos -= _arena.tileLength;
}
for(int i = 0; i < _arena.numColTiles; i++)
{
float yPos; 
if (j < 1)
{
yPos = _arena.tileLength * _arena.numRowTiles/2.0f + _arena.wallWidth/2.0f;
}
else{
yPos = -_arena.tileLength * _arena.numRowTiles/2.0f - _arena.wallWidth/2.0f;
}
float zpos = (.5 * _arena.wallHeight) - (.5 * 1) -.5;

renderStruct_t wall;
//wall.color = glm::vec4(qrand()/(float) RAND_MAX,qrand()/(float) RAND_MAX,qrand()/(float) RAND_MAX,1);
wall.color = grey;
wall.origin = glm::vec3(xTilePos,yPos,zpos);
wall.scale = glm::vec3(_arena.tileLength,_arena.wallWidth,_arena.wallHeight);
wall.rotation = glm::vec3(0,0,0);
wall.modelMatrix = makeModelMatrix(wall.scale,wall.rotation,wall.origin);
wall.mesh = assets.getMesh(assets.lookupAssetName(WALL_MESH_NAME));
wall.baseShader = assets.getShader(assets.lookupAssetName(WALL_SHADER_NAME));
wall.projShader = assets.getShader(assets.lookupAssetName(WALL_SHADER_NAME));
wall.texture_0 = assets.getTexture(assets.lookupAssetName(WALL_TEXTURE_NAME));
wall.texture_1 = NULL;

_arenaObjects.push_back(wall);


if(j < 1) xTilePos += _arena.tileLength; else xTilePos -= _arena.tileLength;
}
}






float xPos;
float yPos;
float yRot;
float mult;
// Create four IR Towers
for(int i = 0; i < 4; i++)
{
yRot = 0;
mult = -1;
if(i % 2 == 0)
{
xPos = -_arena.tileLength * _arena.numColTiles/2.0f - 6;
yRot += 180;
}
else
{
xPos = _arena.tileLength * _arena.numColTiles/2.0f + 6;
mult = 1;
}
if(i / 2 == 0)
{
yPos = -_arena.tileLength * _arena.numRowTiles/2.0f - 6;
yRot -= mult * 45;
}
else
{
yPos = _arena.tileLength * _arena.numRowTiles/2.0f + 6;
yRot += mult * 45;
}

renderStruct_t tower;
tower.color = grey;
//			tower.color = glm::vec4(qrand()/(float) RAND_MAX,qrand()/(float) RAND_MAX,qrand()/(float) RAND_MAX,1);
tower.origin = glm::vec3(xPos, yPos, 0.0);
tower.scale = glm::vec3(2.0f,2.0f,2.0f);
tower.rotation = glm::vec3(0,0,yRot);
tower.modelMatrix = makeModelMatrix(tower.scale,tower.rotation,tower.origin);
tower.mesh = assets.getMesh(assets.lookupAssetName(TOWER_MESH_NAME));
tower.baseShader = assets.getShader(assets.lookupAssetName(TOWER_SHADER_NAME));
tower.projShader = assets.getShader(assets.lookupAssetName(TOWER_SHADER_NAME));
tower.texture_0 = assets.getTexture(assets.lookupAssetName(TOWER_TEXTURE_NAME));
tower.texture_1 = NULL;
_arenaObjects.push_back(tower);

}

}

*/

// generate a model matrix
glm::mat4 RenderWidget::makeModelMatrix(glm::vec3 scale, glm::vec3 rotate, glm::vec3 translation)
{
	glm::mat4 model =  glm::translate(glm::mat4(1.0f),translation);
	model = glm::rotate(model,rotate.x,glm::vec3(1,0,0));
	model = glm::rotate(model,rotate.y,glm::vec3(0,1,0));
	model = glm::rotate(model,rotate.z,glm::vec3(0,0,1));
	model = glm::scale(model,scale);
	return model;
}




void RenderWidget::timerEvent ( QTimerEvent * event ) 
{
	qint64 nsElapsed = _renderTimer.nsecsElapsed();
	float msElapsedSinceRender = nsElapsed / 1000000.0;

	if (isVisible())
	{
		// restart the timer
		_renderTimer.start();

		//	qApp->processEvents();
		_eventTimer.start();

		processInput(msElapsedSinceRender);

		// event updating goes here

		// update the simulator at a fixed step
		updateCamera();

		nsElapsed = _eventTimer.nsecsElapsed();
		_msElapsedUpdate = nsElapsed / 1000000.0;

		//	qApp->processEvents();
		if (!_renderLock )
		{
			updateGL();
		}
		//	qApp->processEvents();

	}

}

void RenderWidget::closeEvent(QCloseEvent *event)
{
	if (_timerID != 0)
	{
		killTimer(_timerID);
		_timerID = 0;
	}
	_renderLock = true;
	deleteLater();
	event->accept();
}

float RenderWidget::getRandomf(float min, float max)
{
	uint randNum = rand();
	float range = max - min;
	return (min + ((range * randNum) / RAND_MAX));
}
