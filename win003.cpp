#include "win003.h"
#include "./ui_win003.h"
#include <QUrl>
#include <QVideoSink>
#include <QVideoFrame>

Win003::Win003(QString file,
               int interval,
               QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Win003), _first(true), _interval(interval), _position(0)
{
    ui->setupUi(this);
	
    _mediaPlayer = new QMediaPlayer(this);
    connect(_mediaPlayer, &QMediaPlayer::errorChanged,this, &Win003::handleError);
    _videoWidget = new QVideoWidget;
    _mediaPlayer->setVideoOutput(_videoWidget);
    _mediaPlayer->setSource(QUrl::fromLocalFile(file));
    connect(_mediaPlayer, &::QMediaPlayer::mediaStatusChanged, this, &Win003::mediaStateChanged);
}

Win003::~Win003()
{
    delete ui;
}

void Win003::mediaStateChanged(QMediaPlayer::MediaStatus state)
{
    if (state == QMediaPlayer::LoadedMedia)
    {
        TRUTH(_mediaPlayer->isSeekable());
        EVAL(_mediaPlayer->duration());
        connect(_mediaPlayer, &QMediaPlayer::positionChanged, this, &Win003::capture);
        captureInit();
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Win003::captureInit);
        timer->start(_interval);
    }
}


void Win003::captureInit()
{
    _mark = Clock::now();
    if (!_first)
        _position += _interval;
    if (_position >= _mediaPlayer->duration())
        _position = 0;
    std::cout << "capturing " << _position << " ... " << std::endl;
    _mediaPlayer->setPosition(_position);
    _mediaPlayer->play();
}

void Win003::capture()
{
    _mediaPlayer->pause();
    auto videoframe = _mediaPlayer->videoSink()->videoFrame();
    auto image = videoframe.toImage();
    {
        std::stringstream string;
        string << "captured\t" << ((Sec)(Clock::now() - _mark)).count() << "s";
        std::cout << string.str() << std::endl;
        ui->labelCapturedTime->setText(string.str().data());
    }
    if (_first)
	{
        EVAL(videoframe.planeCount());
        EVAL(videoframe.mappedBytes(0));
        EVAL(videoframe.bytesPerLine(0));
        EVAL(videoframe.handleType());
        TRUTH(videoframe.mirrored());
        EVAL(videoframe.rotationAngle());
        EVAL(videoframe.width());
        EVAL(videoframe.height());
        EVAL(videoframe.pixelFormat());
        EVAL(videoframe.surfaceFormat().frameWidth());
        EVAL(videoframe.surfaceFormat().frameHeight());
        EVAL(videoframe.surfaceFormat().frameRate());
        TRUTH(videoframe.surfaceFormat().isMirrored());

        EVAL(image.format());
        EVAL(image.depth());
		EVAL(image.width());
		EVAL(image.height());
		EVAL(image.dotsPerMeterX());
		EVAL(image.dotsPerMeterY());
		ECHO(auto colour = image.pixel(QPoint(0,0)));
		EVALH(colour);
		EVAL(qAlpha(colour));
		EVAL(qRed(colour));
		EVAL(qGreen(colour));
		EVAL(qBlue(colour));
		EVAL(qGray(colour));
		EVAL((qRed(colour)+qGreen(colour)+qBlue(colour))/3);	
        _first = false;
	}
    EVAL(videoframe.startTime());
    EVAL(videoframe.endTime());

    {
        _mark = Clock::now();
        std::size_t total = 0;
        std::size_t size = image.sizeInBytes()/4;
        auto rgb = (QRgb*)image.constBits();
        for (std::size_t x = 0; x < size; x++)
        {
            total += qRed(*rgb) + qGreen(*rgb) + qBlue(*rgb);
            rgb++;
        }
        total /= 3;
        total /= size;
		{
            std::stringstream string;
            string << "average:" << total << "\t" << ((Sec)(Clock::now() - _mark)).count() << "s";
			std::cout << string.str() << std::endl;
            ui->labelAverage->setText(string.str().data());
		}
    }

	{
        _mark = Clock::now();
        std::stringstream string;
		QImage scaledImage = image.scaled(ui->labelImage->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
		ui->labelImage->setPixmap(QPixmap::fromImage(scaledImage));
        string << "imaged\t" << ((Sec)(Clock::now() - _mark)).count() << "s";
		std::cout << string.str() << std::endl;
        ui->labelImagedTime->setText(string.str().data());
	}

}

void Win003::handleError()
{
    if (_mediaPlayer->error() == QMediaPlayer::NoError)
        return;

    const QString errorString = _mediaPlayer->errorString();
    QString message = "Error: ";
    if (errorString.isEmpty())
        message += " #" + QString::number(int(_mediaPlayer->error()));
    else
        message += errorString;
    std::cout << message.toStdString() << std::endl;
}

