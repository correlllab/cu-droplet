/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\include\RenderWidget.h
 *
 * \brief	Declares the RenderWidget class.
 */

#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#define _USE_MATH_DEFINES

#include "gl_core_3_3.h"

#include <math.h>
#include <QGlobal.h>
#include <QTime>
#include <QDateTime>
#include <QVector>
#include <QFlags>
#include <QGLFormat>
#include <QTimer>
#include <QGLWidget>
#include <QGLShaderProgram>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QCoreApplication>
#include <QAtomicInt>

#include "DropletGUIDefaults.h"
#include "DropletGUIStructs.h"
#include "AssetManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
*  @defgroup render Renderer
*  \brief Items related to visualizing the state of the simulation
*/

/**
* @ingroup render
* \class	RenderWidget
*
* \brief	View controller for QGLWidget that renders the simulation state.
*/

class RenderWidget : public QGLWidget
{
	Q_OBJECT

public:

	/**
	* \fn	RenderWidget::RenderWidget(const QGLFormat& format, QWidget *parent = 0);
	*
	* \brief	Constructor.
	*
	* \param	format		  	Describes the format to use.
	* \param [in,out]	parent	(Optional) If non-null, (Optional) the parent.
	*/

	RenderWidget(const QGLFormat& format, QWidget *parent = 0);

	/**
	* \fn	RenderWidget::~RenderWidget();
	*
	* \brief	Destructor.
	*/

	~RenderWidget();

	/**
	* \fn	QSize RenderWidget::sizeHint() const;
	*
	* \brief	Gets the size hint.
	*
	* \return	.
	*/

	QSize sizeHint() const;

	public slots:
		/**
		* \fn	public slots: void RenderWidget::updateState(simState_t stateInfo);
		*
		* \brief	Updates the state described by stateInfo.
		*
		* \param	stateInfo	Information describing the state.
		*/


		void updateState(simState_t stateInfo);

		/**
		* \fn	void RenderWidget::updatePause(bool paused);
		*
		* \brief	Tells RenderWidget about the pause status of the simulator.
		*
		* \param	paused	true if paused.
		*/

		void updatePause(bool paused);

		/**
		* \fn	void RenderWidget::updateArena(simSetting_t settings);
		*
		* \brief	Updates the arena described by settings.
		*
		* \param	settings	Options for controlling the operation.
		*/

		void updateArena(simSetting_t settings);

		/**
		* \fn	void RenderWidget::updateRate(simRate_t rates);
		*
		* \brief	Updates the rate described by rates.
		*
		* \param	rates	The rates.
		*/

		void updateRate(simRate_t rates);


signals:
		/**
		* \fn	signals: void RenderWidget::togglePause(void);
		*
		* \brief	Toggle pause.
		*/

		void togglePause(void);

		/**
		* \fn	void RenderWidget::restart(void);
		*
		* \brief	Restarts this object.
		*/

		void restart(void);

		/**
		* \fn	void RenderWidget::requestNewDroplet(float x, float y, float a, droplet_t dType = CustomOne,
		* 		int dropletID = 0);
		*
		* \brief	Request new droplet.
		*
		* \param	x		 	The x coordinate.
		* \param	y		 	The y coordinate.
        * \param    a           The angle the droplet is initially rotated by.
		* \param	dType	 	(Optional) the type.
		* \param	dropletID	(Optional) identifier for the droplet.
		*/

		void requestNewDroplet(float x, float y, float a, droplet_t dType = CustomOne, int dropletID = 0);

		/**
		* \fn	void RenderWidget::toggleLimit(void);
		*
		* \brief	Toggle limit.
		*/

		void toggleLimit(void);

		/**
		* \fn	void RenderWidget::increaseRate(void);
		*
		* \brief	Increase rate.
		*/

		void increaseRate(void);

		/**
		* \fn	void RenderWidget::decreaseRate(void);
		*
		* \brief	Decrease rate.
		*/

		void decreaseRate(void);

protected:

	/**
	* \fn	void RenderWidget::initializeGL();
	*
	* \brief	Initializes the gl.
	*/

	void initializeGL();

	/**
	* \fn	void RenderWidget::resizeGL(int width, int height);
	*
	* \brief	Resize gl.
	*
	* \param	width 	The width.
	* \param	height	The height.
	*/

	void resizeGL(int width, int height);

	/**
	* \fn	void RenderWidget::paintGL();
	*
	* \brief	Paints the gl.
	*/

	void paintGL();

	/**
	* \fn	void RenderWidget::updateCamera();
	*
	* \brief	Updates the camera.
	*/

	void updateCamera();

	/**
	* \fn	void RenderWidget::drawHUD();
	*
	* \brief	Draw HUD.
	*/

	void drawHUD();

	/**
	* \fn	void RenderWidget::drawArena();
	*
	* \brief	Draw arena.
	*/

	void drawArena();

	/**
	* \fn	void RenderWidget::drawDroplets();
	*
	* \brief	Draw droplets.
	*/

	void drawDroplets();

	/**
	* \fn	void RenderWidget::drawObjects();
	*
	* \brief	Draw objects.
	*/

	void drawObjects();

	/**
	* \fn	void RenderWidget::timerEvent(QTimerEvent *event);
	*
	* \brief	timing control.
	*
	* \param [in,out]	event	If non-null, the event.
	*/

	void timerEvent(QTimerEvent *event);

	/**
	* \fn	void RenderWidget::setFPS(int FPS);
	*
	* \brief	Sets the FPS.
	*
	* \param	FPS	The FPS.
	*/

	void setFPS(int FPS);

	/**
	* \fn	void RenderWidget::setupRenderStructs();
	*
	* \brief	Sets up the render structs.
	*/

	void setupRenderStructs();
	//void setupArena();

	/**
	* \fn	void RenderWidget::wheelEvent(QWheelEvent *event);
	*
	* \brief	input handling.
	*
	* \param [in,out]	event	If non-null, the event.
	*/

	void wheelEvent(QWheelEvent *event);

	/**
	* \fn	void RenderWidget::keyPressEvent(QKeyEvent *event);
	*
	* \brief	Key press event.
	*
	* \param [in,out]	event	If non-null, the event.
	*/

	void keyPressEvent(QKeyEvent *event);

	/**
	* \fn	void RenderWidget::keyReleaseEvent(QKeyEvent *event);
	*
	* \brief	Key release event.
	*
	* \param [in,out]	event	If non-null, the event.
	*/

	void keyReleaseEvent(QKeyEvent *event);

	/**
	* \fn	void RenderWidget::processInput(float timeSinceLastUpdate);
	*
	* \brief	Process the input described by timeSinceLastUpdate.
	*
	* \param	timeSinceLastUpdate	The time since last update.
	*/

	void processInput(float timeSinceLastUpdate);

	/**
	* \fn	void RenderWidget::mouseMoveEvent ( QMouseEvent * event );
	*
	* \brief	Mouse move event.
	*
	* \param [in,out]	event	If non-null, the event.
	*/

	void mouseMoveEvent ( QMouseEvent * event );

	/**
	* \fn	void RenderWidget::mousePressEvent ( QMouseEvent * event );
	*
	* \brief	Mouse press event.
	*
	* \param [in,out]	event	If non-null, the event.
	*/

	void mousePressEvent ( QMouseEvent * event );

	/**
	* \fn	void RenderWidget::mouseReleaseEvent ( QMouseEvent * event );
	*
	* \brief	Mouse release event.
	*
	* \param [in,out]	event	If non-null, the event.
	*/

	void mouseReleaseEvent ( QMouseEvent * event );

	/**
	* \fn	void RenderWidget::closeEvent(QCloseEvent *event);
	*
	* \brief	Event invoked when instance is closed.
	*
	* \param [in,out]	event	If non-null, the event initiating the closing.
	*/

	void closeEvent(QCloseEvent *event);

	/**
	* \fn	float RenderWidget::getRandomf(float min, float max);
	*
	* \brief	Gets a random float.
	*
	* \param	min	The minimum.
	* \param	max	The maximum.
	*
	* \return	The random number.
	*/

	float getRandomf(float min, float max);


private:
	/**
	* \struct	renderStruct_t
	*
	* \brief	Defines a structure that represents a renderable object.
	*/

	struct renderStruct_t {
		GLuint vertCount;
		GLuint vaoID;
		QGLShaderProgram *baseShader;
		QGLShaderProgram *projShader;
		MeshManager *mesh;
		TextureManager *texture_0;
		//	TextureManager *texture_1;

		glm::vec4 color;
		glm::vec3 scale;
		glm::vec3 rotation;
		glm::vec3 origin;
		glm::mat4 modelMatrix;
	};

	/**
	* \fn	glm::mat4 RenderWidget::makeModelMatrix(glm::vec3 scale, glm::vec3 rotate,
	* 		glm::vec3 translation);
	*
	* \brief	Makes model matrix.
	*
	* \param	scale	   	The scale.
	* \param	rotate	   	The rotate.
	* \param	translation	The translation.
	*
	* \return	.
	*/

	glm::mat4 makeModelMatrix(glm::vec3 scale, glm::vec3 rotate, glm::vec3 translation);


	void saveScreenShot(int width, int height);
	/**
	* \brief	The assets.
	*/

	AssetManager assets;

	/**
	* \brief	timer related objects/variables.
	*/

	QElapsedTimer _renderTimer;

	/**
	* \brief	one off timer, only declared here to keep from needing to create/destroy timers
	* 			inside functions.
	*/

	QElapsedTimer _eventTimer;

	/**
	* \property	double _msElapsedRenderer, _msElapsedUpdate
	*
	* \brief	QTime _runTime;
	*
	* \return	The milliseconds elapsed update.
	*/

	double _msElapsedRenderer, _msElapsedUpdate;

	/**
	* \brief	FPS variables.
	*/

	int _targetFPS;

	/**
	* \brief	Time of the target frame.
	*/

	float _targetFrameTime;

	/**
	* \brief	Flag to enforce reentrancy of rendering.
	*/

	bool _renderLock;

	/**
	* \brief	true if assets loaded.
	*/

	bool _assetsLoaded;

	/**
	* \brief	Enables or disables the HUD.
	*/

	bool _hud;

	/**
	 * \brief	Enables or disables debug rendering.
	 * 			
	 * _renderDebug has three different levels:
	 * 0 - Disabled
	 * 1 - Display additional information on the HUD  
	 * 2 - Enable debug rendering (Falls back to debug shaders)
	**/


	int _renderDebug;

	/**
	* \brief	true to draw help.
	*/

	bool _drawHelp;

	/**
	* \brief	The update timer.
	*/

	QElapsedTimer _updateTimer;

	/**
	* \brief	Identifier for the timer.
	*/

	int _timerID;

	/**
	* \brief	QVector containing local objects to render.
	*/

	QVector<renderStruct_t> _arenaObjects;

	/**
	* \brief	renderStruct_t that contains necessary information about how to render droplets.
	*/

	renderStruct_t _dropletStruct;

	/**
	* \brief	QVector containing information on how to render physics objects.
	*/

	QVector<renderStruct_t> _objectStructs;

	/**
	* \brief	Struct that caches information used by the HUD.
	*/
	struct {
		int framesSinceLastUpdate;
		float simRealTimeRatio;
		float simStepSize;
		bool paused;
	} _hudInfo;

	/**
	* \brief	Struct that contains state information for the camera.
	*/
	struct {
		QMatrix4x4 projectionMatrix;
		QMatrix4x4 viewMatrix;
		GLfloat x,y,z;
		GLfloat pan, tilt;
		GLfloat rotHoriz,rotVert;
		GLfloat radius;
		unsigned int mode;
		glm::vec3 lightDir;
	} _camera;

	/**
	* \brief	Struct that keeps track of mouse status.
	*/
	struct {
		bool leftButtonHeldDown;
		bool rightButtonHeldDown;
		int startX, startY;
		GLfloat origHoriz;
		GLfloat origVert;
		GLfloat origPan;
		GLfloat origTilt;
	} _mouseStatus;


	/**
	* \brief	Struct that keeps track of which keys are held down.
	*/
	struct {
		bool Q;
		bool W;
		bool E;
		bool A;
		bool S;
		bool D;
		bool Minus;
		bool Plus;
	} _keysDown;

	struct {
		GLuint width, height;
		GLuint handle;
		bool valid;
		QImage texture;
		/**
		* \brief	The projection texture.
		*/

	} _projectionTexture;

	/**
	* \brief	The arena.
	*/

	simSetting_t _arena;

	/**
	 * \brief	State of the simulation.
	 */

	simState_t _simState;
	simState_t _renderState;

	/**
	 * \brief	true to lock, false to unlock the simulation state.
	 */

	QAtomicInt _simStateLock;
	/**
	* \brief	Rate of the simulation.
	*/

	simRate_t _simRates;
};

#endif // RENDERWIDGET_H
