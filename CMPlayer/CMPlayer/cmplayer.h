#ifndef CMPLAYER_H
#define CMPLAYER_H

#include <QtGui/QMainWindow>
#include "vlc/vlc.h"
#include <QtGui>

class CMPlayer : public QMainWindow
{
	Q_OBJECT

public:
	CMPlayer(QWidget *parent = 0, Qt::WFlags flags = 0);
	~CMPlayer();

private slots:
	void SlotOpenFile();
	void SlotPlay();
	void SlotStop();
	void SlotMute();
	void SlotAbout();
	void SlotFullScreen();

	int  SlotChangeVolume(int);
	void SlotChangePosition(int);
	void SlotUpdateInterface();

protected:
	virtual void closeEvent(QCloseEvent*);

private:
	void initUI();

private:
	QPushButton *m_pBtnPlay;
	QSlider *volumeSlider;
	QSlider *slider;
	QWidget *videoWidget;

	libvlc_instance_t *m_pvlcInstance;
	libvlc_media_player_t *m_vlcPlayer;
};

#endif // CMPLAYER_H
