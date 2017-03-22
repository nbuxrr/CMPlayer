//*************************************************************************
//	filename: cmplayer.cpp    create-time: 2015-10-27 16:44:52
//	author: xurr
//	note: 		
//*************************************************************************
#include "cmplayer.h"
#include <string>
using namespace std;

#define qtu(i) ((i).toUtf8().constData())

CMPlayer::CMPlayer(QWidget *parent, Qt::WFlags flags)
: QMainWindow(parent, flags)
, m_vlcPlayer(NULL)
{	
	m_pvlcInstance = libvlc_new(0, NULL);	// Initialize libVLC

	/* Complain in case of broken installation */
	if (m_pvlcInstance == NULL)
	{
		QMessageBox::critical(this, "Qt libVLC player", "Could not init libVLC");
	}

	/* Interface initialization */
	initUI();
}

CMPlayer::~CMPlayer()
{
	/* Release libVLC instance on quit */
	if (m_pvlcInstance)
	{
		libvlc_release(m_pvlcInstance);
	}
}

void CMPlayer::initUI() 
{
	/* Menu */
	QMenu *fileMenu = menuBar()->addMenu("&File");
	QMenu *editMenu = menuBar()->addMenu("&Edit");

	QAction *Open    = new QAction("&Open", this);
	QAction *Quit    = new QAction("&Quit", this);
	QAction *playAc  = new QAction("&Play/Pause", this);
	QAction *fsAc  = new QAction("&Fullscreen", this);
	QAction *aboutAc = new QAction("&About", this);

	Open->setShortcut(QKeySequence("Ctrl+O"));
	Quit->setShortcut(QKeySequence("Ctrl+Q"));

	fileMenu->addAction(Open);
	fileMenu->addAction(aboutAc);
	fileMenu->addAction(Quit);
	editMenu->addAction(playAc);
	editMenu->addAction(fsAc);

	connect(Open,    SIGNAL(triggered()), this, SLOT(SlotOpenFile()));
	connect(playAc,  SIGNAL(triggered()), this, SLOT(SlotPlay()));
	connect(aboutAc, SIGNAL(triggered()), this, SLOT(SlotAbout()));
	connect(fsAc,    SIGNAL(triggered()), this, SLOT(SlotFullScreen()));
	connect(Quit,    SIGNAL(triggered()), qApp, SLOT(quit()));

	/* Buttons for the UI */
	m_pBtnPlay = new QPushButton("Play");
	QObject::connect(m_pBtnPlay, SIGNAL(clicked()), this, SLOT(SlotPlay()));

	QPushButton *stopBut = new QPushButton("Stop");
	QObject::connect(stopBut, SIGNAL(clicked()), this, SLOT(SlotStop()));

	QPushButton *muteBut = new QPushButton("Mute");
	QObject::connect(muteBut, SIGNAL(clicked()), this, SLOT(SlotMute()));

	QPushButton *fsBut = new QPushButton("Fullscreen");
	QObject::connect(fsBut, SIGNAL(clicked()), this, SLOT(SlotFullScreen()));

	volumeSlider = new QSlider(Qt::Horizontal);
	QObject::connect(volumeSlider, SIGNAL(sliderMoved(int)), this, SLOT(SlotChangeVolume(int)));
	volumeSlider->setValue(80);

	slider = new QSlider(Qt::Horizontal);
	slider->setMaximum(1000);
	QObject::connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(SlotChangePosition(int)));

	/* A timer to update the sliders */
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(SlotUpdateInterface()));
	timer->start(100);

	/* Central Widgets */
	QWidget* centralWidget = new QWidget;
	videoWidget = new QWidget;

	videoWidget->setAutoFillBackground( true );
	QPalette plt = palette();
	plt.setColor( QPalette::Window, Qt::black );
	videoWidget->setPalette( plt );

	/* Put all in layouts */
	QHBoxLayout *layout = new QHBoxLayout;
	layout->setMargin(0);
	layout->addWidget(m_pBtnPlay);
	layout->addWidget(stopBut);
	layout->addWidget(muteBut);
	layout->addWidget(fsBut);
	layout->addWidget(volumeSlider);

	QVBoxLayout *layout2 = new QVBoxLayout;
	layout2->setMargin(0);
	layout2->addWidget(videoWidget);
	layout2->addWidget(slider);
	layout2->addLayout(layout);

	centralWidget->setLayout(layout2);
	setCentralWidget(centralWidget);
	resize(600, 400);
}

void CMPlayer::SlotOpenFile() 
{
	if (m_pvlcInstance == NULL)
	{
		return;
	}

	/* The basic file-select box */
	QString fileOpen = QFileDialog::getOpenFileName(this, tr("Load a file"), "~");

	/* Stop if something is playing */
	if (m_vlcPlayer && libvlc_media_player_is_playing(m_vlcPlayer))
		SlotStop();


	//string strFileName(qtu(fileOpen));
	string strFileName((const char *)fileOpen.toLocal8Bit());
	fileOpen.replace("/", "\\");
	/* Create a new Media */
	libvlc_media_t *vlcMedia = libvlc_media_new_path(m_pvlcInstance, qtu(fileOpen));
	
	if (!vlcMedia)
		return;

	/* Create a new libvlc player */
	m_vlcPlayer = libvlc_media_player_new_from_media(vlcMedia);

	/* Release the media */
	libvlc_media_release(vlcMedia);

	/* Integrate the video in the interface */
#if defined(Q_OS_MAC)
	libvlc_media_player_set_nsobject(m_vlcPlayer, (void *)videoWidget->winId());
#elif defined(Q_OS_UNIX)
	libvlc_media_player_set_xwindow(m_vlcPlayer, videoWidget->winId());
#elif defined(Q_OS_WIN)
	libvlc_media_player_set_hwnd(m_vlcPlayer, videoWidget->winId());
#endif

	/* And start playback */
	libvlc_media_player_play (m_vlcPlayer);

	/* Update playback button */
	m_pBtnPlay->setText("Pause");
}

void CMPlayer::SlotPlay() {
	if (!m_vlcPlayer)
		return;

	if (libvlc_media_player_is_playing(m_vlcPlayer))
	{
		/* Pause */
		libvlc_media_player_pause(m_vlcPlayer);
		m_pBtnPlay->setText("Play");
	}
	else
	{
		/* Play again */
		libvlc_media_player_play(m_vlcPlayer);
		m_pBtnPlay->setText("Pause");
	}
}

int CMPlayer::SlotChangeVolume(int vol) { /* Called on volume slider change */

	if (m_vlcPlayer)
		return libvlc_audio_set_volume (m_vlcPlayer,vol);

	return 0;
}

void CMPlayer::SlotChangePosition(int pos) { /* Called on position slider change */

	if (m_vlcPlayer)
		libvlc_media_player_set_position(m_vlcPlayer, (float)pos/1000.0);
}

void CMPlayer::SlotUpdateInterface() { //Update interface and check if song is finished

	if (!m_vlcPlayer)
		return;

	/* update the timeline */
	float pos = libvlc_media_player_get_position(m_vlcPlayer);
	slider->setValue((int)(pos*1000.0));

	/* Stop the media */
	if (libvlc_media_player_get_state(m_vlcPlayer) == libvlc_Ended)
		this->SlotStop();
}

void CMPlayer::SlotStop() {
	if(m_vlcPlayer) {
		/* stop the media player */
		libvlc_media_player_stop(m_vlcPlayer);

		/* release the media player */
		libvlc_media_player_release(m_vlcPlayer);

		/* Reset application values */
		m_vlcPlayer = NULL;
		slider->setValue(0);
		m_pBtnPlay->setText("Play");
	}
}

void CMPlayer::SlotMute() {
	if(m_vlcPlayer) {
		if(volumeSlider->value() == 0) { //if already muted...

			this->SlotChangeVolume(80);
			volumeSlider->setValue(80);

		} else { //else mute volume

			this->SlotChangeVolume(0);
			volumeSlider->setValue(0);

		}
	}
}

void CMPlayer::SlotAbout()
{
	QMessageBox::about(this, "Qt libVLC player demo", QString::fromUtf8(libvlc_get_version()) );
}

void CMPlayer::SlotFullScreen()
{
	if (isFullScreen()) {
		showNormal();
		menuWidget()->show();
	}
	else {
		showFullScreen();
		menuWidget()->hide();
	}
}

void CMPlayer::closeEvent(QCloseEvent *event) {
	SlotStop();
	event->accept();
}




