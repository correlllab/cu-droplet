/**
* \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\src\mainwindow.cpp
*
* \brief	Implements the MainWindow class.
*/
#include "MainWindow.h"
#include <QtGui>


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	QStringList argsList = qApp->arguments();
	fileName = QString(DEFAULT_ASSETDIR).append(DEFAULT_SETUPDIR).append(DEFAULT_SETTINGS);
	if (argsList.count() > 1)
	{
		// read the first item as the arena file, otherwise use the hard coded default
		fileName = argsList[1];
	}
	if (qApp)
		qApp->installEventFilter(this);

	setupFileDescriptionLoaded = false;
	dropProgramDescriptionLoaded = false;
	setUpGUI();

	simSetting_t settings = _simInterface.getDefaultSettings();
	if (loadFromFile(fileName,settings))
	{
		qDebug() << "Loaded settings out of file" << fileName;
	}

	// TODO: Update UI so these are propagated without a hack
	_arenaObjects.droplets.clear();
	_arenaObjects.objects.clear();
	_arenaObjects.droplets = settings.startingDroplets;
	_arenaObjects.objects = settings.startingObjects;

	setUI(settings);
	_simulatorRunning = false;

	
	// connect the file system monitors to the appropriate functions
	connect(&projectionWatcher,SIGNAL(directoryChanged(QString)),this,SLOT(loadProjectionTextures(QString)));
	connect(&arenaWatcher,SIGNAL(directoryChanged(QString)),this,SLOT(loadFloorFiles(QString)));
	connect(&setupWatcher,SIGNAL(directoryChanged(QString)),this,SLOT(loadSetupFiles(QString)));
	connect(&setupWatcher,SIGNAL(fileChanged(QString)),this,SLOT(updateSetupFile(QString)));

	projectionWatcher.addPath((QString(DEFAULT_ASSETDIR).append(DEFAULT_PROJECTDIR)));
	arenaWatcher.addPath(QString(DEFAULT_ASSETDIR).append(DEFAULT_FLOORDIR));
	setupWatcher.addPath(QString(DEFAULT_ASSETDIR).append(DEFAULT_SETUPDIR));

	qRegisterMetaType<simState_t>("simState_t");
	qRegisterMetaType<simRate_t>("simRate_t");
	qRegisterMetaType<simSetting_t>("simSetting_t");
	qRegisterMetaType<droplet_t>("droplet_t");

	_simInterface.moveToThread(&_simThread);
	_simThread.start();
	connect(this,SIGNAL(launchSim(simSetting_t)),&_simInterface,SLOT(loadArena(simSetting_t)));
	connect(this,SIGNAL(closing(void)),&_simThread,SLOT(quit(void)));
	connect(this,SIGNAL(closing(void)),&_logger,SLOT(close(void)));


	connect(this,SIGNAL(enableLimit(void)),&_simInterface,SLOT(enableUpdateLimit(void)));
	connect(this,SIGNAL(disableLimit(void)),&_simInterface,SLOT(disableUpdateLimit(void)));
	connect(this,SIGNAL(setUpdateRate(float)),&_simInterface,SLOT(setUpdateRate(float)));


}

MainWindow::~MainWindow() { }

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == qApp)
	{
		if (event->type() == QEvent::ApplicationActivate)
		{
			qDebug() << "Application gained focus";
		} else if (event->type() == QEvent::ApplicationDeactivate)
		{
			qDebug() << "Application lost focus";

		}
	}
	return QMainWindow::eventFilter(watched,event);
}

void MainWindow::launchSimulator()
{
	simSetting_t newSettings = _simInterface.getDefaultSettings();
	if(arenaSelectionCombo->currentText() != QString("Default (Rectangle)"))
	{
		newSettings.floorFile = arenaSelectionCombo->currentText();
		newSettings.floorFile.append(".txt");
		simSetting_t currentSimSettings = _simInterface.getCurrentSettings();
		newSettings.numColTiles = currentSimSettings.numColTiles;
		newSettings.numRowTiles = currentSimSettings.numRowTiles;
	}
	else
	{
		newSettings.numColTiles = numColBox->value();
		newSettings.numRowTiles = numRowBox->value();
	}
	if (projectorImageCombo->currentText() != QString("None"))
	{
		newSettings.projecting = true;
		newSettings.projTexture = projectorImageCombo->currentText();
		newSettings.projTexture.append(".bmp");
	}

	newSettings.startingDroplets = _arenaObjects.droplets;
	newSettings.startingObjects = _arenaObjects.objects;
	
	if(loadSetupFileCombo->currentText() == QString("None"))
	{
		
		newSettings.startingDroplets.clear();
		for(int i = 0; i < dropletTableWidget->rowCount(); i++)
		{
			QStringList droplets;
			qDebug() << "header" << dropletTableWidget->verticalHeaderItem(i)->text();
			qDebug() << "header row" << dropletTableWidget->verticalHeaderItem(i)->row();
			qDebug() << "item to text" << dropletTableWidget->item(i,0)->text();
			qDebug() << "item to text to int" << dropletTableWidget->item(i,0)->text().toInt();
			int entry = dropletTableWidget->item(i,0)->text().toInt();
			qDebug() << "entry" << entry;
			if(entry != 0)
			{
				droplets.append(QString(dropletTableWidget->verticalHeaderItem(i)->text()));
				droplets.append(QString("%1").arg(entry));
				newSettings.startingDroplets.append(droplets);
			}
		}
	}
	
	qDebug() << "checkbox: " << logCheckBox->isChecked();
	if(logCheckBox->isChecked())
	{
		qDebug() << "logbox checked";
		_logger.setPosFlag(posCheckBox->isChecked());
		_logger.setColorFlag(colCheckBox->isChecked());
		_logger.setRotationFlag(rotCheckBox->isChecked());
		_logger.setCommSAFlag(commSACheckBox->isChecked());
		_logger.setMacroRedFlag(macroRedCheckBox->isChecked());
		_logger.setMacroSAFlag(macroSACheckBox->isChecked());
		_logger.Init();
		connect(&_simInterface, SIGNAL(simulationUpdated(simState_t)), &_logger, SLOT(timeCheck(simState_t)));
        connect(&_simInterface, SIGNAL(resetLogger(void)), &_logger, SLOT(Init()));
	}

	emit launchSim(newSettings);

	if (!_simulatorRunning)
	{
		emit setUpdateRate(1.0f);
		emit enableLimit();

		_simulatorRunning = true;
	}
}

void MainWindow::launchRenderer()
{

	// set the format for the OpenGL window
	QGLFormat glFormat = QGLFormat(QGL::DoubleBuffer | QGL::DirectRendering);
	glFormat.setVersion( 3, 3 );
	glFormat.setDepthBufferSize(24);
	glFormat.setProfile( QGLFormat::CompatibilityProfile ); // Requires >=Qt-4.8.0

	// create the window and pass it the simulator as a parameter
	RenderWidget *renderer =  new RenderWidget(glFormat);
	if (_renderWidgets.count() == 0)
	{
		emit enableLimit();
	}
	_renderWidgets.append(renderer);

	connect(this,SIGNAL(closing(void)),renderer,SLOT(close(void)));

	connect(&_simInterface,SIGNAL(simulationUpdated(simState_t)),renderer,SLOT(updateState(simState_t)),Qt::DirectConnection);
	connect(&_simInterface,SIGNAL(arenaChanged(simSetting_t)),renderer,SLOT(updateArena(simSetting_t)));
	connect(&_simInterface,SIGNAL(pauseChanged(bool)),renderer,SLOT(updatePause(bool)));
	connect(&_simInterface,SIGNAL(ratesChanged(simRate_t)),renderer,SLOT(updateRate(simRate_t)));

	connect(renderer,SIGNAL(togglePause(void)),&_simInterface,SLOT(togglePause(void)));
	connect(renderer,SIGNAL(restart(void)),&_simInterface,SLOT(reset(void)));
	connect(renderer,SIGNAL(requestNewDroplet(float,float,float,droplet_t)),&_simInterface,SLOT(addDroplet(float,float,float,droplet_t)));
	connect(renderer,SIGNAL(toggleLimit(void)),&_simInterface,SLOT(toggleUpdateLimit(void)));
	connect(renderer,SIGNAL(increaseRate(void)),&_simInterface,SLOT(increaseUpdateRate(void)));
	connect(renderer,SIGNAL(decreaseRate(void)),&_simInterface,SLOT(decreaseUpdateRate(void)));

	connect(renderer,SIGNAL(destroyed(QObject*)),this,SLOT(removeRenderer(QObject*)));

	renderer->updateArena(_simInterface.getSimulatorSettings());
	renderer->updateState(_simInterface.getSimulatorState());
	renderer->updateRate(_simInterface.getSimulatorRate());
	// position the window
	renderer->setWindowTitle(QString(DEFAULT_WINDOW_NAME));
	renderer->move(0,0);
	renderer->resize(DEFAULT_WINDOW_WIDTH,DEFAULT_WINDOW_HEIGHT);


	// show the window
	renderer->show();

	if (!_simulatorRunning)
		launchSimulator();

}

void MainWindow::addLoadSetupFileWidgets()
{
	loadSetupFileCombo = new QComboBox;
	loadSetupFileCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	loadSetupFileList = new QListView(loadSetupFileCombo);
	loadSetupFileList->setStyleSheet("QListView::item {  \
									  border-bottom: 5px solid white; margin:3px; } \
									  QListView::item:selected { \
									  border-bottom: 5px solid black; margin:3px; \
									  color:black; \
									  } \
									  ");
	loadSetupFileCombo->setView(loadSetupFileList);

	setupFileSaveButton = new QPushButton("Save");
	setupFileSaveButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

	loadSetupFileDescription = new QTextEdit;
	loadSetupFileDescription->setReadOnly(true);
	loadSetupFileDescription->setPlainText("Setup File Description");

	selectSetupFileLayout = new QHBoxLayout;
	selectSetupFileLayout->addWidget(loadSetupFileCombo);
	selectSetupFileLayout->addWidget(setupFileSaveButton);
	
	QObject::connect(loadSetupFileCombo, SIGNAL(currentIndexChanged(const QString &)),this,SLOT(updateParams(const QString &)));
	QObject::connect(loadSetupFileCombo, SIGNAL(currentIndexChanged(const QString &)),this,SLOT(enableDisableDropletTable(const QString &)));

	loadSetupFile = new QFrame();
	loadSetupFile->setWindowTitle(tr("Load Parameters from Setup File"));
	
	QVBoxLayout *setupFileLayout = new QVBoxLayout();
	setupFileLayout->addLayout(selectSetupFileLayout);
	setupFileLayout->addWidget(loadSetupFileDescription);

	loadSetupFile->setLayout(setupFileLayout);
	loadSetupFile->setFrameStyle(QFrame::StyledPanel);
	loadSetupFile->setLineWidth(1);
}

void MainWindow::addDropletWidgets()
{
	dropletTableWidget = new QTableWidget(10,1);
	QStringList vertHeaders;
	vertHeaders.append("CustomOne");
	vertHeaders.append("CustomTwo");
	vertHeaders.append("CustomThree");
	vertHeaders.append("CustomFour");
	vertHeaders.append("CustomFive");
	vertHeaders.append("CustomSix");
	vertHeaders.append("CustomSeven");
	vertHeaders.append("CustomEight");
	vertHeaders.append("CustomNine");
	vertHeaders.append("CustomTen");
	dropletTableWidget->setVerticalHeaderLabels(vertHeaders);
	QStringList horHeaders;
	horHeaders.append("Number");
	dropletTableWidget->setHorizontalHeaderLabels(horHeaders);

	for(int i = 0; i < dropletTableWidget->rowCount(); i++) 
		dropletTableWidget->setItem(i, 0, new QTableWidgetItem("0"));		

	dropProgramsDescription = new QTextEdit;
	dropProgramsDescription->setReadOnly(true);
	dropProgramsDescription->insertPlainText("Droplet Program Description");

	dropletParams = new QFrame;
	dropletParams->setWindowTitle(tr("Droplet Programs"));
	QHBoxLayout *dropletParamsLayout = new QHBoxLayout;
	dropletParamsLayout->addWidget(dropletTableWidget);
	dropletParamsLayout->addWidget(dropProgramsDescription);
	dropletParams->setLayout(dropletParamsLayout);
	dropletParams->setFrameStyle(QFrame::StyledPanel);
	dropletParams->setLineWidth(1);
}

void MainWindow::addArenaWidgets()
{
	// Floor Files
	arenaSelectionCombo = new QComboBox;
	arenaSelectionList = new QListView(arenaSelectionCombo);
	arenaSelectionCombo->addItem("Default (Rectangle)");

	arenaSelectionList->setStyleSheet("QListView::item {  \
									  border-bottom: 5px solid white; margin:3px; } \
									  QListView::item:selected { \
									  border-bottom: 5px solid black; margin:3px; \
									  color:black; \
									  } \
									  ");
	arenaSelectionCombo->setView(arenaSelectionList);
	arenaSelectionLabel = new QLabel(tr("Arena Selection"));
	arenaSelectionLabel->setBuddy(arenaSelectionCombo);
	arenaSelectionLayout = new QHBoxLayout;
	arenaSelectionLayout->addWidget(arenaSelectionLabel);
	arenaSelectionLayout->addWidget(arenaSelectionCombo);

	QObject::connect(arenaSelectionCombo, SIGNAL(currentIndexChanged(const QString &)),this,SLOT(enableDisableRowColWidget(const QString &)));
	QObject::connect(arenaSelectionCombo, SIGNAL(currentIndexChanged(const QString &)),&_simInterface,SLOT(loadTilePositions(const QString &)));

	rowColLayout = new QHBoxLayout;
	rowColWidget = new QWidget;

	// Number of Rows
	numRowBox = new QSpinBox;
	numRowBox->setRange(1,10000);
	numRowBox->setValue(DEFAULT_ROW_TILES);
	numRowLabel = new QLabel(tr("Rows "));
	numRowLabel->setBuddy(numRowBox);

	// Number of Columns
	numColBox = new QSpinBox;
	numColBox->setRange(1,10000);
	numColBox->setValue(DEFAULT_COL_TILES);
	numColLabel = new QLabel(tr("x Columns "));
	numColLabel->setBuddy(numColBox);

	rowColLayout->addWidget(numRowLabel);
	rowColLayout->addWidget(numRowBox);
	rowColLayout->addWidget(numColLabel);
	rowColLayout->addWidget(numColBox);
	rowColWidget->setLayout(rowColLayout);

	// Projector Images
	projectorImageCombo = new QComboBox;
	projectorImageList = new QListView(projectorImageCombo);
	projectorImageCombo->addItem("None");

	projectorImageList->setStyleSheet("QListView::item {  \
									  border-bottom: 5px solid white; margin:3px; } \
									  QListView::item:selected { \
									  border-bottom: 5px solid black; margin:3px; \
									  color:black; \
									  } \
									  ");
	projectorImageCombo->setView(projectorImageList);
	projectorImageLabel = new QLabel(tr("Projection Images"));
	projectorImageLabel->setBuddy(projectorImageCombo);
	projectorImagesLayout = new QHBoxLayout;
	projectorImagesLayout->addWidget(projectorImageLabel);
	projectorImagesLayout->addWidget(projectorImageCombo);

	arenaParams = new QFrame();
	arenaParams->setWindowTitle(tr("Arena Parameters"));
	QVBoxLayout *arenaParamsLayout = new QVBoxLayout();
	arenaParamsLayout->addLayout(arenaSelectionLayout);
	arenaParamsLayout->addWidget(rowColWidget);
	arenaParamsLayout->addLayout(projectorImagesLayout);
	arenaParams->setLayout(arenaParamsLayout);
	arenaParams->setFrameStyle(QFrame::Panel);
	arenaParams->setLineWidth(1);
}

void MainWindow::addButtonWidgets()
{
	// Launch Buttons
	launchSimulation = new QPushButton(tr("Update"));
	connect(launchSimulation,SIGNAL(clicked()),this,SLOT(launchSimulator()));
	launchRenderWidget = new QPushButton(tr("Render"));
	QObject::connect(launchRenderWidget,SIGNAL(clicked()),this,SLOT(launchRenderer()));
	QHBoxLayout	*launchButtons = new QHBoxLayout;
	launchButtons->addWidget(launchSimulation);
	launchButtons->addWidget(launchRenderWidget);

	// Simulation Interation Buttons
	pauseButton = new QPushButton("Pause");
	resumeButton = new QPushButton("Resume");
	resetButton = new QPushButton("Reset");
	QHBoxLayout *playbackButtons = new QHBoxLayout;
	playbackButtons->addWidget(pauseButton);
	playbackButtons->addWidget(resumeButton);
	playbackButtons->addWidget(resetButton);

	logCheckBox = new QCheckBox("Log Droplet Infomation");
	posCheckBox = new QCheckBox("Position");
	colCheckBox = new QCheckBox("Color");
	rotCheckBox = new QCheckBox("Rotation");
	commSACheckBox = new QCheckBox("CommSA");
	macroRedCheckBox = new QCheckBox("MacroRed");
	macroSACheckBox = new QCheckBox("macroSA");
	logLayout = new QVBoxLayout;
	microLayout = new QHBoxLayout;
	macroLayout = new QHBoxLayout;
	microLayout->addWidget(posCheckBox);
	microLayout->addWidget(colCheckBox);
	microLayout->addWidget(rotCheckBox);
	microLayout->addWidget(commSACheckBox);
	macroLayout->addWidget(macroRedCheckBox);
	macroLayout->addWidget(macroSACheckBox);
	logLayout->addLayout(microLayout);
	logLayout->addLayout(macroLayout);
	logWidget = new QWidget;
	logWidget->setLayout(logLayout);
	connect(logCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showHideLogWidget(int)));

	resetTimerCheckBox = new QCheckBox(tr("Simulation Reset Timer (sec)"));
	resetTimerValue = new QLineEdit;
	QDoubleValidator *v = new QDoubleValidator(this);
	v->setBottom(1.0);
	resetTimerValue->setValidator(v);
	connect(resetTimerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableDisableResetTimerValue(int)));
	connect(resetTimerCheckBox, SIGNAL(stateChanged(int)), &_simInterface, SLOT(useResetTimer(int)));
	connect(resetTimerValue, SIGNAL(textChanged(const QString&)), &_simInterface, SLOT(updateResetTimer(const QString&)));

	resetTimerValue->setText(QString::number(DEFAULT_RESET_TIME));
	resetTimerValue->setEnabled(false);
	resetTimerCheckBox->setChecked(false);

	resetTimerLayout = new QHBoxLayout;
	resetTimerLayout->addWidget(resetTimerCheckBox);
	resetTimerLayout->addWidget(resetTimerValue);


	buttons = new QFrame();
	buttons->setWindowTitle(tr("Playback and Logging Buttons"));

	QVBoxLayout *buttonsLayout = new QVBoxLayout();
	buttonsLayout->addWidget(logCheckBox);
	buttonsLayout->addWidget(logWidget);
	logWidget->hide();
	buttonsLayout->addLayout(resetTimerLayout);
	buttonsLayout->addLayout(launchButtons);
	buttonsLayout->addLayout(playbackButtons);
	buttons->setLayout(buttonsLayout);
	buttons->setFrameStyle(QFrame::Panel);
	buttons->setLineWidth(1);
}

void MainWindow::setUpGUI()
{	
	addLoadSetupFileWidgets();
	addDropletWidgets();
	addArenaWidgets();
	addButtonWidgets();

	// Combine to main window
	QWidget *window = new QWidget;
	QVBoxLayout *mainLayout = new QVBoxLayout(window);
	QLabel *loadSetupFileLabel = new QLabel("Load Setup File");
	mainLayout->addWidget(loadSetupFileLabel);
	mainLayout->addWidget(loadSetupFile);
	QLabel *dropletParamsLabel = new QLabel("Droplet Programs");
	mainLayout->addWidget(dropletParamsLabel);
	mainLayout->addWidget(dropletParams);
	QLabel *arenaParamsLabel = new QLabel("Arena Parameters");
	mainLayout->addWidget(arenaParamsLabel);
	mainLayout->addWidget(arenaParams);
	QLabel *buttonsLabel = new QLabel("Launch and Playback");
	mainLayout->addWidget(buttonsLabel);
	mainLayout->addWidget(buttons);
	window->setLayout(mainLayout);
	setCentralWidget(window);
	setWindowTitle(tr("Droplet GUI"));

	connect(pauseButton,SIGNAL(released()),&_simInterface,SLOT(pause()));
	connect(resumeButton,SIGNAL(released()),&_simInterface,SLOT(resume()));
	connect(resetButton,SIGNAL(released()),&_simInterface,SLOT(reset()));

	// only load these after the GUI is set up
	loadSetupFiles(QString(DEFAULT_ASSETDIR).append(DEFAULT_SETUPDIR));
	loadFloorFiles(QString(DEFAULT_ASSETDIR).append(DEFAULT_FLOORDIR));
	loadProjectionTextures(QString(DEFAULT_ASSETDIR).append(DEFAULT_PROJECTDIR));
}


void MainWindow::removeRenderer(QObject *obj)
{
	if (obj != NULL)
	{
		RenderWidget *renderer = (RenderWidget *) obj;

		if (_renderWidgets.contains(renderer))
		{
			_renderWidgets.removeOne(renderer);
			qDebug() << QString("Removed RenderWidget");
		}
	}

	if (_renderWidgets.count() == 0)
		emit disableLimit();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
	// use the closing signal to signal all child objects to close/quit
	emit closing();
	qApp->processEvents();

	// spinlock until child thread is done
	while(_simThread.isRunning()) {};
	event->accept();
}


void MainWindow::loadProjectionTextures(QString path)
{
	qDebug() << "Refreshing texture list";
	QString currentTexture = projectorImageCombo->currentText();

	QDir inDir(path);
	QStringList filters;
	filters << "*.bmp";
	inDir.setNameFilters(filters);

	_projectionTextures = inDir.entryList();
	for(QStringList::iterator i = _projectionTextures.begin() ; i != _projectionTextures.end();i++)
	{
		i->remove(".bmp");
	}
	projectorImageCombo->clear();
	projectorImageCombo->addItem("None");
	projectorImageCombo->addItems(_projectionTextures);
	int index = projectorImageCombo->findText(currentTexture,Qt::MatchFixedString);
	if (index == -1)
		index = 0;
	projectorImageCombo->setCurrentIndex(index);
}

void MainWindow::loadFloorFiles(QString path)
{
	qDebug() << "Refreshing file list";
	QString currentArena = arenaSelectionCombo->currentText();

	QDir inDir(path);
	QStringList filters;
	filters << "*.txt";
	inDir.setNameFilters(filters);

	_floorFiles = inDir.entryList();
	for(QStringList::iterator i = _floorFiles.begin() ; i != _floorFiles.end();i++)
	{
		i->remove(".txt");
	}

	arenaSelectionCombo->clear();
	arenaSelectionCombo->addItem("Default (Rectangle)");
	arenaSelectionCombo->addItems(_floorFiles);
	int index = arenaSelectionCombo->findText(currentArena,Qt::MatchFixedString);
	if (index < 0)
		index = 0;
	arenaSelectionCombo->setCurrentIndex(index);
}

void MainWindow::loadSetupFiles(QString path)
{
	QString currentSetup = loadSetupFileCombo->currentText();

	QDir inDir(path);
	QStringList filters;
	filters << "*.txt";
	inDir.setNameFilters(filters);

	_setupFiles = inDir.entryList();
	for(QStringList::iterator i = _setupFiles.begin() ; i != _setupFiles.end();i++)
	{
		QString temp = path;
		temp.append(*i);
		qDebug() << "Added file" << temp << "to watch list";
		setupWatcher.addPath(temp);
		i->remove(".txt");
	}

	loadSetupFileCombo->clear();
	loadSetupFileCombo->addItem(QString("None"));
	loadSetupFileCombo->addItems(_setupFiles);
	int index = loadSetupFileCombo->findText(currentSetup,Qt::MatchFixedString);
	if (index < 0)
		index = 0;
	loadSetupFileCombo->setCurrentIndex(index);
	updateParams(currentSetup);
}

void MainWindow::setUI(simSetting_t settings)
{
	numColBox->setValue(settings.numColTiles);
	numRowBox->setValue(settings.numRowTiles);

	int index = 0;
	if (settings.projecting == true)
	{
		QString temp = settings.projTexture;
		temp.remove(".bmp");
		index = projectorImageCombo->findText(temp,Qt::MatchFixedString);
		if (index == -1)
			index = 0;
	}	
	projectorImageCombo->setCurrentIndex(index);

	// if an arena was specified set the UI for it
	QString arenaName = settings.arenaName.remove(".txt");
	index = arenaSelectionCombo->findText(arenaName,Qt::MatchFixedString);
	if (index == -1)
		index = 0;

	arenaSelectionCombo->setCurrentIndex(index);
	index = 0;
	int numDroplets = 0;
	foreach(QStringList list, settings.startingDroplets)
	{
		int program_id = -1;
		int addition;
		if(list.count() == 2)
		{
			addition = list[1].toInt();
		}
		if(list.count() == 3)
		{
			addition = 1;
		}
		QString type = list[0].toLower();

		// TODO : If this works put the strings in an array and set program_id by comparing strings in a loop
		if (type == QString("customone"))
		{
			program_id = 0;
		} else if (type == QString("customtwo"))
		{
			program_id = 1;
		} else if (type == QString("customthree"))
		{
			program_id = 2;
		} else if (type == QString("customfour"))
		{
			program_id = 3;
		} else if (type == QString("customfive"))
		{
			program_id = 4;
		} else if (type == QString("customsix"))
		{
			program_id = 5;
		} else if (type == QString("customseven"))
		{
			program_id = 6;
		} else if (type == QString("customeight"))
		{
			program_id = 7;
		} else if (type == QString("customnine"))
		{
			program_id = 8;
		} else if (type == QString("customten"))
		{
			program_id = 9;
		}
		if(program_id > -1)
		{
			int temp = dropletTableWidget->item(program_id,0)->text().toInt() + addition;
			QString string = QString::number(temp);
			dropletTableWidget->setItem(program_id, 0, new QTableWidgetItem(string));
		}
	}
}

bool MainWindow::saveToFile(QString filename, simSetting_t settings)
{

	QFile file(filename);
	if (file.open(QIODevice ::WriteOnly | QIODevice ::Text | QIODevice::Truncate))
	{
		simSetting_t defaults = _simInterface.getDefaultSettings();
		QTextStream out(&file);
		out << "col_tiles " << settings.numColTiles << "\n";
		out << "row_tiles " << settings.numRowTiles << "\n";
		out << "droplet_radius " << settings.dropletRadius << "\n";
		if (settings.projecting == true && settings.projTexture != QString("None"))
		{
			out << "projecting " << settings.projTexture << "\n";

		}
		foreach(QStringList droplet,settings.startingDroplets)
		{
			out << "droplets";
			foreach(QString string,droplet)
			{
				out << " " << string;
			}
			out << "\n";
		}
		foreach(QStringList object,settings.startingObjects)
		{
			foreach(QString string,object)
			{
				out << string << " ";
			}
		}
		file.close();
		return true;
	}
	return false;
}

bool MainWindow::loadFromFile(QString filename, simSetting_t &settings)
{
	for(int i = 0; i < dropletTableWidget->rowCount(); i++) 
	{
		dropletTableWidget->setItem(i, 0, new QTableWidgetItem("0"));		
	}
	QFile file(filename);
	simSetting_t newSettings;
	newSettings = _simInterface.getDefaultSettings();

	if (file.exists())
	{
		if (file.open(QIODevice::ReadOnly  |QIODevice::Text))
		{
			QStringList objects;
			objects << "cube" << "cubes" << "sphere" << "spheres";
			QTextStream in(&file);
			while (!in.atEnd())
			{
				QString line = in.readLine(0);
				if (line.count() > 0)
				{
					// Parse description
					if(line.startsWith('#') && !setupFileDescriptionLoaded)
					{
						QString descriptionText = line.remove('#');
						while(!in.atEnd())
						{
							line = in.readLine(0);
							if(!line.startsWith('#'))
								break;
							descriptionText.append(line.remove('#'));
						}
						loadSetupFileDescription->clear();
						loadSetupFileDescription->insertPlainText(descriptionText);
					}
					if (!line.startsWith("#"))
					{
						setupFileDescriptionLoaded = true;
						QStringList list = line.split(" ",QString::SkipEmptyParts);
						if (list.count() > 0)
						{
							list[0] = list[0].toLower();
							if (objects.contains(list[0]))
							{
								newSettings.startingObjects.append(list);
							}  else if (list.size() == 2)
							{

								if (list[0] == QString("droplets") || list[0] == QString("droplet"))
								{
									QStringList dropletsToAdd;								
									dropletsToAdd.append(list[1]);
									dropletsToAdd.append(QString("1"));
									newSettings.startingDroplets.append(dropletsToAdd);
								} else if (list[0] == QString("col_tiles"))
								{
									int numColTiles = list[1].toInt();
									newSettings.numColTiles = numColTiles;
								} else if (list[0] == QString("row_tiles"))
								{
									int numRowTiles = list[1].toInt();
									newSettings.numRowTiles = numRowTiles;
								} else if (list[0] == QString("fps"))
								{
									int newFPS = list[1].toInt();
									newSettings.fps = newFPS;
								} else if (list[0] == QString("droplet_radius"))
								{
									float newRadius = list[1].toFloat();
									newSettings.dropletRadius = newRadius;
								} else if (list[0] == QString("wall_height"))
								{
									float newHeight = list[1].toFloat();
									newSettings.wallHeight = newHeight;
								} else if (list[0] == QString("wall_width"))
								{
									float newWidth = list[1].toFloat();
									newSettings.wallWidth = newWidth;
								} else if (list[0] == QString("projecting"))
								{
									QString filePath = QString(DEFAULT_ASSETDIR).append(DEFAULT_PROJECTDIR).append(list[1]);
									QFileInfo file(filePath);
									if (file.exists())
									{
										newSettings.projecting = true;
										newSettings.projTexture = QString(list[1]);
									} else {
										newSettings.projecting = false;
										newSettings.projTexture = QString("none");
									}
								} else if (list[0] == QString("arena"))
								{
									newSettings.arenaName = list[1];
								} else {
									qDebug() << "Error: unrecognized line" << line;
								}
							}  else if (list.count() >= 3) 
							{
								if (list[0] == QString("droplets"))
								{
									QStringList dropletsToAdd;	
									for(int i = 1; i < list.count(); i++)
									{
										dropletsToAdd.append(list[i]);

									}
									qDebug() << dropletsToAdd;
									newSettings.startingDroplets.append(dropletsToAdd);
								}
							} else
							{
								qDebug() << "Error: malformed line" << line;
							}
						} else {
							// push the comment to the debug console
							qDebug() << line;
						}
					}
				}
			}
		} else
		{
			qDebug() << "Error: " << file.error();	
			file.close();
			return false;
		}
		file.close();
		settings = newSettings;
		return true;
	} else
	{
		qDebug() << "Error: file does not exist - using compiled defaults";
	}
	return false;
}

void MainWindow::showHideLogWidget(int state)
{
	if (state == 2)
		logWidget->show();
	else
		logWidget->hide();
}

void MainWindow::enableDisableRowColWidget(const QString &text)
{
	if(text.operator==("Default (Rectangle)"))
		rowColWidget->setEnabled(true);
	else
		rowColWidget->setEnabled(false);
}

void MainWindow::updateSetupFile(const QString &file)
{
	QString fileName = file.section('\\', -1);
	if (fileName == file)
		fileName = file.section('/', -1);
		
	fileName = fileName.remove(".txt");
	qDebug() << "File" << fileName << "changed";
	if (loadSetupFileCombo->currentText() == fileName)
		updateParams(fileName);
}
void MainWindow::updateParams(const QString & text)
{
	fileName = QString(DEFAULT_ASSETDIR).append(DEFAULT_SETUPDIR).append(text).append(".txt");
	simSetting_t settings = _simInterface.getDefaultSettings();
	setupFileDescriptionLoaded = false;
	if (loadFromFile(fileName,settings))
	{
		qDebug() << "Loaded settings out of file" << fileName;
	}

	// TODO: update UI so these are propagated without need for hack
	_arenaObjects.droplets.clear();
	_arenaObjects.objects.clear();
	_arenaObjects.droplets = settings.startingDroplets;
	_arenaObjects.objects = settings.startingObjects;

	setUI(settings);
}

void MainWindow::enableDisableDropletTable(const QString & text)
{
	if(text.operator!=("None"))
		dropletTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	else
		dropletTableWidget->setEditTriggers(QAbstractItemView::AllEditTriggers);
}

void MainWindow::enableDisableResetTimerValue(int state)
{
	if (state == 2)
		resetTimerValue->setEnabled(true);
	else
		resetTimerValue->setEnabled(false);
}