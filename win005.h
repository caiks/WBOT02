#ifndef WIN005_H
#define WIN005_H

#include "dev.h"
#include <QWidget>
#include <QTimer>
#include <QScreen>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class Win005; }
QT_END_NAMESPACE

class Win005 : public QWidget
{
    Q_OBJECT

public:
    Win005(int interval = 1000,
           int x = 0,
           int y = 0,
           int width = -1,
           int height = -1,
           QWidget *parent = nullptr);
	Win005(const std::string& config,
           QWidget *parent = nullptr);
    ~Win005();

public Q_SLOTS:
	void start();
    void capture();
	void configParse();
	
protected:
	void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::Win005 *ui;
	QScreen *screen;
	std::chrono::time_point<std::chrono::high_resolution_clock> mark;
	std::string config;
    int interval;
    int x;
    int y;
    int width;
    int height;	
	double centreX;
	double centreY;
    std::vector<double> scales;
	std::size_t valency;
	std::size_t size;
	std::size_t divisor;
	std::size_t multiplier;
};
#endif // WIN005_H
