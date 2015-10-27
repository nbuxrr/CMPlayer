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
	void openFile();
	void play();
	void stop();
	void mute();
	void about();
	void fullscreen();

	int changeVolume(int);
	void changePosition(int);
	void updateInterface();

protected:
	virtual void closeEvent(QCloseEvent*);

private:
	void initUI();

private:
	QPushButton *playBut;
	QSlider *volumeSlider;
	QSlider *slider;
	QWidget *videoWidget;

	libvlc_instance_t *pvlcInstance;
	libvlc_media_player_t *vlcPlayer;
};

#endif // CMPLAYER_H
