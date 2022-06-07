#ifndef WIN006_H
#define WIN006_H

#include "dev.h"
#include <QWidget>
#include <QTimer>
#include <QScreen>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class Win006; }
QT_END_NAMESPACE

class Win006 : public QWidget
{
    Q_OBJECT

public:
    Win006(int interval = 1000,
           int x = 0,
           int y = 0,
           int width = -1,
           int height = -1,
           QWidget *parent = nullptr);
	Win006(const std::string& config,
           QWidget *parent = nullptr);
    ~Win006();

public Q_SLOTS:
	void start();
    void capture();
	void configParse();
	
protected:
	void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::Win006 *_ui;
	QScreen *_screen;
	std::chrono::time_point<std::chrono::high_resolution_clock> _mark;
	std::string _config;
    int _interval;
    int _x;
    int _y;
    int _width;
    int _height;	
	double _centreX;
	double _centreY;
    std::vector<double> _scales;
	std::size_t _valency;
	std::size_t _size;
	std::size_t _divisor;
	std::size_t _multiplier;
};
#endif // WIN006_H
