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
           int interval = 1000,
           QWidget *parent = nullptr);
    ~Win004();

private Q_SLOTS:
    void mediaStateChanged(QMediaPlayer::MediaStatus state);
    void capture();
    void handleError();

private:
    Ui::Win004 *ui;
    QMediaPlayer* _mediaPlayer;
    QVideoWidget* _videoWidget;
    std::chrono::time_point<std::chrono::high_resolution_clock> _mark;
    bool _first;
    int _interval;
    qint64 _position;
};
#endif // WIN004_H
