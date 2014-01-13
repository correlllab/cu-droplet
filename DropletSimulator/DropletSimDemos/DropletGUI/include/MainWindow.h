/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\include\mainwindow.h
 *
 * \brief	Declares the MainWindow class.
 */

#ifndef MAINWINDOW_H

/**
 * \def	MAINWINDOW_H
 *
 * \brief	A macro that defines mainwindow h.
 *
 */
#define MAINWINDOW_H

#include <QMainWindow>
#include "RenderWidget.h"
#include "SimInterface.h"
#include <QFileSystemWatcher>
#include <QStringList>
#include <QString>
#include <QSpinBox>
#include <QSlider>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QListView>
#include <QGroupBox>
#include <QLinkedList>
#include <QCloseEvent>
#include <QShowEvent>
#include <QCheckBox>
#include <QDesktopWidget>
#include <QMetaObject>
#include <QThread>
#include <QTableWidget>
#include <QTextEdit>

#include "SimInfoLogger.h"
#include "DropletGUIDefaults.h"

/**
*  @defgroup main Main Window
*  \brief Items related to the main GUI window.
*/

/**
 * @ingroup main
 * \class	MainWindow
 *
 * \brief	View controller for the main window.
 */

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:

	/**
	 * \fn	MainWindow::MainWindow(QWidget *parent = 0);
	 *
	 * \brief	Constructor. Basically initializes the GUI
	 *
	 * \param [in,out]	parent	(Optional) If non-null, (Optional) the parent.
	 */

	MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:

	/**
	 * \fn	void MainWindow::loadProjectionTextures(QString path);
	 *
	 * \brief	Populates the list of projection textures and adds them to the GUI:
	 * 			 _projectionTextures, projectorImageCombo.
	 *
	 * \param	path	Full pathname of the file.
	 */

	void loadProjectionTextures(QString path);

	/**
	 * \fn	void MainWindow::loadFloorFiles(QString path);
	 *
	 * \brief	Populates the list of floor files and adds them to the GUI: _floorFiles,
	 * 			arenaSelectionCombo.
	 *
	 * \param	path	Full pathname of the file.
	 */

	void loadFloorFiles(QString path);
	
	/**
	 * \fn	void MainWindow::loadSetupFiles();
	 *
	 * \brief	Populates the list of setup files and adds them to the GUI:
	 * 			_setupFiles, loadSetupFileCombo
	 *
	 */
	void loadSetupFiles(QString path);

	/**
	 * \fn	void MainWindow::updateParams(const QString & text);
	 *
	 * \brief	Parses a specified setup file and updates the params in the GUI
	 *
	 * \param	text	specifies a setup file
	 */
	void updateParams(const QString & text);

	/**
	 * \fn	void MainWindow::updateSetupFile(const QString &file);
	 *
	 * \brief	Checks if the file specified matches the currently selected setup file, and if it
	 * 			does it refreshes the GUI on change.
	 *
	 * \param	file	Path to the file.
	**/

	void updateSetupFile(const QString &file);

	/**
	 * \fn	bool MainWindow::loadFromFile(QString filename, simSetting_t &settings);
	 *
	 * \brief	Loads simulator settings from an input file.
	 *
	 * \param	filename			Filename of the file.
	 * \param [in,out]	settings	Contains simulator settings.
	 *
	 * \return	true if it succeeds, false if it fails.
	 */

	bool loadFromFile(QString filename, simSetting_t &settings);

	/**
	 * \fn	bool MainWindow::saveToFile(QString filename, simSetting_t settings);
	 *
	 * \brief	Saves current simulator settings to a file.
	 *
	 * \param	filename	Filename of the file.
	 * \param	settings	Contains simulator settings.
	 *
	 * \return	true if it succeeds, false if it fails.
	 */

	bool saveToFile(QString filename, simSetting_t settings);

	/**
	 * \fn	void MainWindow::setUI(simSetting_t settings);
	 *
	 * \brief	Sets GUI parameters values stored in simSetting_t settings.
	 *
	 * \param	settings	Contains simulator settings.
	 */

	void setUI(simSetting_t settings);

	/**
	 * \fn	void MainWindow::enableDisableRowColWidget(const QString & text);
	 *
	 * \brief	Toggles the rowColWidget between show and hide.
	 *
	 * \param	text	if 'Default (Rectangle)', widget is shown, otherwise hid.
	 */

	void enableDisableRowColWidget(const QString & text);

	/**
	 * \fn	void MainWindow::showHideLogWidget(int state);
	 *
	 * \brief	Toggles the logWidget between show and hide.
	 *
	 * \param	state	The state of the logCheckBox.
	 */

	void showHideLogWidget(int state);

	void enableDisableDropletTable(const QString &);
	
	void enableDisableResetTimerValue(int state);

protected:

	/**
	 * \fn	void MainWindow::closeEvent(QCloseEvent *event);
	 *
	 * \brief	Uses the closing signal to signal all child objects to close/quit.
	 *
	 * \param [in,out]	event	If non-null, the event.
	 */

	void closeEvent(QCloseEvent *event);

	/**
	 * \fn	bool MainWindow::eventFilter(QObject *watched, QEvent *event);
	 *
	 * \brief	Determines if the application is in focus. If application
	 * 			regains focus, projector image and floor file lists are 
	 * 			refreshed.
	 *
	 * \param [in,out]	watched	If non-null, the watched.
	 * \param [in,out]	event  	If non-null, the event.
	 *
	 * \return	true if it succeeds, false if it fails.
	 */

	bool eventFilter(QObject *watched, QEvent *event);

private:
	//Ui::MainWindow ui;
	
	/**
	 * \property SimInterface _siminterface
	 * \brief	 Instance of the simulator interface.
	 */
	SimInterface _simInterface;

	/**
	 * \property QLinkedList<RenderWidget *> _renderWidgets 
	 * \brief	 List of render widgets.
	 */
	QLinkedList<RenderWidget *> _renderWidgets;

	/**
	 * \property QStringList _projectionTextures, QStringList _floorFiles
	 * \brief	 List of filenames for the projection textures and custom floor files.
	 */

	QStringList _projectionTextures;
	QStringList _floorFiles;
	QStringList _setupFiles;


	/**
	 * \property bool _simulatorRunning
	 * \brief	 true to simulator running.
	 */

	bool _simulatorRunning;
	QThread _simThread;

	/**
	 * \property SimInfoLogger _logger 
	 * \brief	 Instance of the logging class, used to log simulation data.
	 */

	SimInfoLogger _logger;

	/**
	 * \property QString fileName
	 * \brief	 Initialized to the assest directory.
	 */
	QString fileName;

	/**
	 * \property QSpinBox *numDropBox, QLabel *numDropLabel, QHBoxLayout *numDropLayout
	 * \brief	 Qt widgets for selecting the number of droplets parameter.
	 */

	QSpinBox *numDropBox;
	QLabel *numDropLabel;
	QHBoxLayout *numDropLayout;

	/**
	 * \property QSpinBox *numColBox, QLabel *numColLabel
	 * \brief	Qt widgets for selecting the number of arena columns parameter.
	 */

	QSpinBox *numColBox;
	QLabel *numColLabel;


	/**
	 * \property QSpinBox *numRowBox, QLabel *numRowLabel, QHBoxLayout *numRowLayout 
	 * \brief	Qt widgets for selecting the number of arena rows parameter.
	 */

	QSpinBox *numRowBox;
	QLabel *numRowLabel;

	/**
	 * \property QLabel *loadSetupFileLabel, QComboBox *loadSetupFileCombo, QListView *loadSetupFileList, QVBoxLayout *loadSetupFileLayout 
	 * \brief	Qt widgets for selecting the default or a custom floor.
	 */

	QComboBox *loadSetupFileCombo;
	QListView *loadSetupFileList;
	QTextEdit *loadSetupFileDescription;
	QPushButton *setupFileSaveButton;
	QHBoxLayout *selectSetupFileLayout;
	bool setupFileDescriptionLoaded;

	/**
	 * \property QLabel *arenaSelectionLabel, QComboBox *arenaSelectionCombo, QListView *arenaSelectionList, QVBoxLayout *arenaSelectionLayout 
	 * \brief	Qt widgets for selecting the default or a custom floor.
	 */

	QLabel *arenaSelectionLabel;
	QComboBox *arenaSelectionCombo;
	QListView *arenaSelectionList;
	QHBoxLayout *arenaSelectionLayout;

	/**
	 * \property QVBoxLayout *rowColLayout, QWidget *rowColWidget
	 * \brief	The row widgets and column widgets are shown or hid, so they are within a single rowColWid.
	 */

	QHBoxLayout *rowColLayout;
	QWidget *rowColWidget;

	/**
	 * \property QLabel *dropProgramsLabel, QComboBox *dropProgramsCombo
	 * \brief	Qt widgets for selecting the droplet program.
	 */

	QLabel *dropProgramsLabel;
	QComboBox *dropProgramsCombo;
	QListView *dropProgramsList;
	QTextEdit *dropProgramsDescription;
	QVBoxLayout *dropProgramsLayout;
	QTableWidget *dropletTableWidget;
	bool dropProgramDescriptionLoaded;

	/**
	 * \property QLabel *projectorImageLabel, QComboBox *projectorImageCombo, QListView *projectorImageList, QVBoxLayout *projectorImagesLayout
	 * \brief	 Qt widgets for selecting the projected image.
	 */

	QLabel *projectorImageLabel;
	QComboBox *projectorImageCombo;
	QListView *projectorImageList;
	QHBoxLayout *projectorImagesLayout;

	/**
	 * \property QPushButton *launchSimulation, QPushButton *launchRenderWidget 
	 * \brief	 Qt buttons for launching the simulator or renderer.
	 */

	QPushButton *launchSimulation;
	QPushButton *launchRenderWidget;

	/**
	 * \property QCheckBox *logCheckBox, QCheckBox *posCheckBox, QCheckBox *colCheckBox, QCheckBox *rotCheckBox, QCheckBox *commSACheckBox, QCheckBox *macroRedCheckBox, QCheckBox *macroSACheckBox
	 * \brief	 Qt checkboxes for selecting if the user wants to log data, and what data he/she wants to log.
	 */

	QCheckBox *logCheckBox;
	QCheckBox *posCheckBox;
	QCheckBox *colCheckBox;
	QCheckBox *rotCheckBox;
	QCheckBox *commSACheckBox;
	QCheckBox *macroRedCheckBox;
	QCheckBox *macroSACheckBox;

	/**
	 * \property QVBoxLayout *logLayout, QHBoxLayout *microLayout, QHBoxLayout *macroLayout, QWidget *logWidget 
	 * \brief	 Qt widgets for combine all of the logging widgets, for show hide capability.
	 */

	QVBoxLayout *logLayout;
	QHBoxLayout *microLayout;
	QHBoxLayout *macroLayout;
	QWidget *logWidget;

	/**
	 * \property QPushButton *pauseButton, QPushButton *resumeButton, QPushButton *resetButton
	 * \brief	 Qt buttons for playback options.
	 */

	QPushButton *pauseButton;
	QPushButton *resumeButton;
	QPushButton *resetButton;

	/**
	* \property QCheckBox *resetTimerCheckBox, QLabel *resetTimerLabel, QSpinBox *resetTimerValue, QHBoxLayout *resetTimerLayout,
	* \brief Qt control values and buttons for setting simulator reset time.
	*/

	QCheckBox *resetTimerCheckBox;
	QLineEdit *resetTimerValue;
	QHBoxLayout *resetTimerLayout;


	/**
	 * \property QFrame *loadSetupFile, QFrame *dropletParams,	QFrame *arenaParams, QFrame *buttons 
	 * \brief	 Qt frames to separate parameter types.
	 */
	QFrame *loadSetupFile;
	QFrame *dropletParams;
	QFrame *arenaParams;
	QFrame *buttons;

	QFileSystemWatcher projectionWatcher;
	QFileSystemWatcher arenaWatcher;
	QFileSystemWatcher setupWatcher;
	
	/**
	 * \fn	void MainWindow::addLoadSetupFileWidgets();
	 *
	 * \brief	Sets up the setup file widgets.
	 * 			
	 */
	void addLoadSetupFileWidgets();
	/**
	 * \fn	void MainWindow::addDropletWidgets();
	 *
	 * \brief	Sets up the droplet parameter widgets.
	 * 			
	 */

	void addDropletWidgets();

	/**
	 * \fn	void MainWindow::addArenaWidgets();
	 *
	 * \brief	Sets up the arena parameter widgets.
	 *
	 */

	void addArenaWidgets();

	/**
	 * \fn	void MainWindow::addButtonWidgets();
	 *
	 * \brief	Sets up the launch button widgets.
	 */

	void addButtonWidgets();

	/**
	 * \fn	void MainWindow::setUpGUI();
	 *
	 * \brief	Sets up the graphical user interface. Calls addDropletWidgets(), 
	 * 			addArenaWidgets(), and addButtonWidgets()
	 */

	void setUpGUI();


	// temporary fix
	// TODO: make thsi better
	struct {
		QVector<QStringList> droplets;
		QVector<QStringList> objects; 
	} _arenaObjects;
private slots:

	/**
	 * \fn	void MainWindow::launchRenderer();
	 *
	 * \brief	Creates an instance of the renderer. Connects signals/slots between
	 * 			the simulator interface and the renderer. If no simulator is currently
	 * 			running, also launches the simulator. 
	 */

	void launchRenderer();

	/**
	 * \fn	void MainWindow::launchSimulator();
	 *
	 * \brief	Creates an instance of the simulator. If a simulator is already running when 
	 * 			this launchSimulator() is called, that simulator is first destroyed so there
	 * 			are not multiple intances of a simulator. Uses GUI parameters. If logCheckBox
	 * 			is checked, also creates an instance of the logger class.
	 */

	void launchSimulator();

	/**
	 * \fn	void MainWindow::removeRenderer(QObject *obj = 0);
	 *
	 * \brief	Removes the renderer.
	 *
	 * \param [in,out]	obj	(Optional) If non-null, (Optional) the object.
	 */

	void removeRenderer(QObject *obj = 0);

signals:

	/**
	 * \fn	void MainWindow::launchSim(simSetting_t);
	 *
	 * \brief	Signal emmited when a simulator is launched.
	 *
	 */

	void launchSim(simSetting_t);

	/**
	 * \fn	void MainWindow::closing();
	 *
	 * \brief	Signal emitted in MainWindow::closeEvent(QCloseEvent *event).
	 */

	void closing();

	/**
	 * \fn	void MainWindow::enableLimit(void);
	 *
	 * \brief	Signal to enable real/simulator time limiting.
	 */

	void enableLimit(void);

	/**
	 * \fn	void MainWindow::disableLimit(void);
	 *
	 * \brief	Signal to disable real/simulator time limiting.
	 */

	void disableLimit(void);

	/**
	 * \fn	void MainWindow::setUpdateRate(float);
	 *
	 * \brief	Sets update rate.
	 *
	 */

	void setUpdateRate(float);


};

#endif // MAINWINDOW_H
