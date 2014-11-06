/**
* \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\src\RW_input.cpp
*
* \brief	Implements the input handling for the RenderWidget class.
*/

#include "RenderWidget.h"

// handle mouse movement
void RenderWidget::mouseMoveEvent ( QMouseEvent * event )
{
	if (event->buttons().testFlag(Qt::RightButton) && _mouseStatus.rightButtonHeldDown)
	{
		if (_camera.mode == 0)
		{
			_camera.rotHoriz = _mouseStatus.origHoriz + (event->x() - _mouseStatus.startX) / 2.0;
			_camera.rotVert = _mouseStatus.origVert - (event->y() - _mouseStatus.startY) / 2.0;
		} else if (_camera.mode == 1) {
			_camera.pan = _mouseStatus.origPan - (event->x() - _mouseStatus.startX) / 2.0;
			_camera.tilt =_mouseStatus.origTilt - (event->y() - _mouseStatus.startY) / 2.0;
		}
	}
	event->accept();
}

// handle mouse button pressed
void RenderWidget::mousePressEvent ( QMouseEvent * event )
{


	if (event->buttons().testFlag(Qt::RightButton) || event->buttons().testFlag(Qt::LeftButton))
	{
		if (event->buttons().testFlag(Qt::LeftButton))
		{
			_mouseStatus.leftButtonHeldDown = true;
		}
		else 
		{
			_mouseStatus.rightButtonHeldDown = true;
			_mouseStatus.startX = event->x();
			_mouseStatus.startY = event->y();
			_mouseStatus.origHoriz = _camera.rotHoriz;
			_mouseStatus.origVert = _camera.rotVert;
			_mouseStatus.origPan = _camera.pan;
			_mouseStatus.origTilt = _camera.tilt;
			setCursor(QCursor(Qt::BlankCursor));
		}
		
	}
	event->accept();
}

// handle releasing the mouse button
void RenderWidget::mouseReleaseEvent ( QMouseEvent * event )
{
	if (!event->buttons().testFlag(Qt::RightButton))
	{
		_mouseStatus.rightButtonHeldDown = false;
		setCursor(QCursor(Qt::ArrowCursor));
	}

	if (!event->buttons().testFlag(Qt::LeftButton)) 
	{
		_mouseStatus.leftButtonHeldDown = false;
	}
	event->accept();
}

// event to handle scrolling with scroll wheel
void RenderWidget::wheelEvent(QWheelEvent *event)
{

	float numDegrees = event->delta() / 8;
	float numSteps = numDegrees / 15;

	if (event->orientation() == Qt::Vertical && _camera.mode == 0) {
#ifdef __APPLE__
		// compensate for apple's inverted scrolling
		_camera.radius += numSteps * 2.0;
#else
		// everyone else behaves correctly
		_camera.radius -= numSteps * 2.0;
#endif
	}
	event->accept();
}


void RenderWidget::keyPressEvent(QKeyEvent *event)
{
	if (!event->isAutoRepeat())
	{
		if(event->modifiers().testFlag(Qt::ControlModifier))
		{
			if(event->key() == Qt::Key_H)
			{
				_hud = !_hud;
			} 
			else if(event->key() == Qt::Key_R)
			{
				emit restart();
			} 
			else if(event->key() == Qt::Key_L)
			{
				_renderLock = true;
				assets.reloadAssets();
				setupRenderStructs();
				_renderLock = false;
			} 
			else if(event->key() == Qt::Key_B)
			{
				if (_renderDebug == 2)
				{
					_renderDebug = 0;
				} else {
					_renderDebug++;
				}
			}
		} else {
			if(event->key() == Qt::Key_Escape)
			{
				qApp->processEvents();
				close();
				makeCurrent();
				assets.clearAssets();
				doneCurrent();
			} else if(event->key() == Qt::Key_P)
			{
				emit togglePause();
			} else if(event->key() == Qt::Key_L)
			{
				emit toggleLimit();
			} else if(event->key() == Qt::Key_H)
			{
				_drawHelp = !_drawHelp;
			} else if(event->key() == Qt::Key_Q)
			{
				_keysDown.Q = true;
			} else if(event->key() == Qt::Key_W)
			{
				_keysDown.W = true;
			} else if(event->key() == Qt::Key_E)
			{
				_keysDown.E = true;
			} else if(event->key() == Qt::Key_A)
			{
				_keysDown.A = true;
			} else if(event->key() == Qt::Key_S)
			{
				_keysDown.S = true;
			} else if(event->key() == Qt::Key_D)
			{
				_keysDown.D = true;
			} else if(event->key() == Qt::Key_Plus)
			{
				_keysDown.Plus = true;
			} else if(event->key() == Qt::Key_Minus)
			{
				_keysDown.Minus = true;
			} else if(event->key() == Qt::Key_Space)
			{
				if (_camera.mode == 1)
				{
					_camera.mode = 0;
					_mouseStatus.origHoriz = _camera.rotHoriz;
					_mouseStatus.origVert = _camera.rotVert;

				} else
				{
					_camera.mode++;
					_mouseStatus.origPan = _camera.pan;
					_mouseStatus.origTilt = _camera.tilt;
				}
			} else if(event->key() == Qt::Key_Backslash)
			{
				makeCurrent();
				saveScreenShot(SCREENSHOT_WIDTH,SCREENSHOT_HEIGHT);
				assets.reloadAssets();
				setupRenderStructs();
				doneCurrent();
			}
		}
	}

	if(event->key() == Qt::Key_BracketRight)
	{
		if (_simRates.limitRate)
			emit decreaseRate();
	}
	if(event->key() == Qt::Key_BracketLeft)
	{
		if (_simRates.limitRate)
			emit increaseRate();
	}
}

void RenderWidget::keyReleaseEvent(QKeyEvent *event)
{
	if (!event->isAutoRepeat())
	{
		if(!event->modifiers().testFlag(Qt::ControlModifier))
		{
			if(event->key() == Qt::Key_Q)
			{
				_keysDown.Q = false;
			}
			if(event->key() == Qt::Key_W)
			{
				_keysDown.W = false;
			}
			if(event->key() == Qt::Key_E)
			{
				_keysDown.E = false;
			}
			if(event->key() == Qt::Key_A)
			{
				_keysDown.A = false;
			}
			if(event->key() == Qt::Key_S)
			{
				_keysDown.S = false;
			}
			if(event->key() == Qt::Key_D)
			{
				_keysDown.D = false;
			}
			if(event->key() == Qt::Key_Plus)
			{
				_keysDown.Plus = false;
			}
			if(event->key() == Qt::Key_Minus)
			{
				_keysDown.Minus = false;
			}
		}
	}
}

void RenderWidget::processInput(float timeSinceLastUpdate)
{

	float perFrame = timeSinceLastUpdate / 1000.0;
	if (_camera.mode == 0)
	{
		if ((_keysDown.Q || _keysDown.Minus) && (!_keysDown.E && !_keysDown.Plus))
		{
			_camera.radius += 100 * perFrame;
		} else if ((_keysDown.E || _keysDown.Plus) && (!_keysDown.Q && !_keysDown.Minus))
		{
			_camera.radius -= 100 * perFrame;
		}
		if (_keysDown.W && !_keysDown.S)
		{
			_camera.rotVert -= 45  * perFrame;
		} else if (_keysDown.S && !_keysDown.W)
		{
			_camera.rotVert += 45 * perFrame;
		}
		if (_keysDown.A && !_keysDown.D)
		{
			_camera.rotHoriz += 90 * perFrame;
		} else if (_keysDown.D && !_keysDown.A)
		{
			_camera.rotHoriz -= 90 * perFrame;
		}
	} else 	if (_camera.mode == 1)
	{
		if (_keysDown.Q && !_keysDown.E)
		{
            _camera.z -= 50 * perFrame;
		} else if (_keysDown.E && !_keysDown.Q)
		{
            _camera.z += 50 * perFrame;
		}
		if (_keysDown.W && !_keysDown.S)
		{
			_camera.x -= sin(_camera.pan * M_PI / 180) * 50 * perFrame;
			_camera.y -= -cos(_camera.pan * M_PI / 180) * 50 * perFrame;


		} else if (_keysDown.S && !_keysDown.W)
		{
			_camera.x += sin(_camera.pan * M_PI / 180) * 50 * perFrame;
			_camera.y += -cos(_camera.pan * M_PI / 180) * 50 * perFrame;
		}
        if (_keysDown.A && !_keysDown.D)
        {
            _camera.x -= cos(_camera.pan * M_PI / 180) * 50 * perFrame;
            _camera.y -= sinf(_camera.pan * M_PI / 180) * 50 * perFrame;


        } else if (_keysDown.D && !_keysDown.A)
        {
            _camera.x += cos(_camera.pan * M_PI / 180) * 50 * perFrame;
            _camera.y += sinf(_camera.pan * M_PI / 180) * 50 * perFrame;

        }
	}

}
