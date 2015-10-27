#include "cmplayer.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QTextCodec *codec = QTextCodec::codecForName("System"); //获取系统编码
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec);

// 	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
// 	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
// 	QTextCodec::setCodecForTr(QTextCodec::codecForName("GBK"));

	CMPlayer w;
	w.show();
	return a.exec();
}
