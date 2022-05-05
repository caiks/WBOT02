#ifndef WIN004_H
#define WIN004_H

#include "dev.h"
#include <QWidget>
#include <QTimer>
#include <QMediaPlayer>
#include <QVideoWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Win004; }
QT_END_NAMESPACE

class Win004 : public QWidget
{
    Q_OBJECT

public:
    Win004(QString file,
           int interval = 0,
           int sleep = 0,
           double playbackRate = 1.0,
           int durationTrim = 50,
           QWidget *parent = nullptr);
    ~Win004();

private Q_SLOTS:
    void mediaStateChanged(QMediaPlayer::MediaStatus state);
    void capture();
    void next();
    void handleError();

private:
    Ui::Win004 *ui;
    QMediaPlayer* _mediaPlayer;
    QVideoWidget* _videoWidget;
    std::chrono::time_point<std::chrono::high_resolution_clock> _mark;
    std::chrono::time_point<std::chrono::high_resolution_clock> _markPrev;
    bool _first;
    int _interval;
    int _sleep;
    int _durationTrim;
    qint64 _position;
};
#endif // WIN004_H
