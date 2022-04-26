#ifndef WIN001_H
#define WIN001_H

#include "dev.h"
#include <QWidget>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QCamera>
#include <QImageCapture>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class Win001; }
QT_END_NAMESPACE

class Win001 : public QWidget
{
    Q_OBJECT

public:
    Win001(int interval = 1000, QWidget *parent = nullptr);
    ~Win001();

public Q_SLOTS:
	void start();
    void captureInit();
    void capture(int id, const QImage &preview);

public:
    QImage image;

private:
    Ui::Win001 *ui;
	QImageCapture* imageCapture;
	std::chrono::time_point<std::chrono::high_resolution_clock> mark;
	bool first; 
	int interval;
	
};
#endif // WIN001_H
