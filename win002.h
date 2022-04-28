#ifndef WIN002_H
#define WIN002_H

#include "dev.h"
#include <QWidget>
#include <QTimer>
#include <QScreen>

QT_BEGIN_NAMESPACE
namespace Ui { class Win002; }
QT_END_NAMESPACE

class Win002 : public QWidget
{
    Q_OBJECT

public:
    Win002(int interval = 1000,
           int x = 0,
           int y = 0,
           int width = -1,
           int height = -1,
           QWidget *parent = nullptr);
    ~Win002();

public Q_SLOTS:
	void start();
    void capture();

private:
    Ui::Win002 *ui;
	QScreen *screen;
	std::chrono::time_point<std::chrono::high_resolution_clock> mark;
	bool first; 
    int interval;
    int x;
    int y;
    int width;
    int height;

};
#endif // WIN002_H
