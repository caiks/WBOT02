#ifndef WIN003_H
#define WIN003_H

#include "dev.h"
#include <QWidget>
#include <QTimer>
#include <QMediaPlayer>
#include <QVideoWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Win003; }
QT_END_NAMESPACE

class Win003 : public QWidget
{
    Q_OBJECT

public:
    Win003(QString file,
           int interval = 1000,
           QWidget *parent = nullptr);
    ~Win003();

private Q_SLOTS:
    void mediaStateChanged(QMediaPlayer::MediaStatus state);
    void captureInit();
    void capture();
    void handleError();

private:
    Ui::Win003 *ui;
    QMediaPlayer* _mediaPlayer;
    QVideoWidget* _videoWidget;
    std::chrono::time_point<std::chrono::high_resolution_clock> _mark;
    bool _first;
    int _interval;
    qint64 _position;
};
#endif // WIN003_H
