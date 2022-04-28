#ifndef WIN003_H
#define WIN003_H

#include "dev.h"
#include <QWidget>
#include <QTimer>
#include <QScreen>

QT_BEGIN_NAMESPACE
namespace Ui { class Win003; }
QT_END_NAMESPACE

class Win003 : public QWidget
{
    Q_OBJECT

public:
    Win003(int interval = 1000,
           QWidget *parent = nullptr);
    ~Win003();

public Q_SLOTS:
	void start();
    void capture();

private:
    Ui::Win003 *ui;
	QScreen *screen;
	std::chrono::time_point<std::chrono::high_resolution_clock> mark;
	bool first; 
    int interval;

};
#endif // WIN003_H
