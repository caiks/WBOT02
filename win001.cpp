#include "win001.h"
#include "./ui_win001.h"
#include <strstream>

Win001::Win001(int intervalA, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Win001), first(true), interval(intervalA)
{
    ui->setupUi(this);
	
	start();
}

Win001::~Win001()
{
    delete ui;
}

void Win001::start()
{
	EVAL(QMediaDevices::videoInputs().count());

	QMediaCaptureSession* captureSession = new QMediaCaptureSession(this);
	QCamera* camera = new QCamera(this);
    captureSession->setCamera(camera);
	imageCapture = new QImageCapture(this);
    captureSession->setImageCapture(imageCapture);
	connect(imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(capture(int,QImage)));	
    camera->start();
	
	QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Win001::captureInit);	
    timer->start(interval);
}


void Win001::captureInit()
{
    mark = Clock::now();
	std::cout << "capturing ..." << std::endl;
	imageCapture->capture();
}
void Win001::capture(int id, const QImage &image)
{
	{
		std::strstream string;
		string << "captured\t" << ((Sec)(Clock::now() - mark)).count() << "s" << std::ends;
		std::cout << string.str() << std::endl;
		ui->labelCapturedTime->setText(string.str());		
	}

	if (first)
	{
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
		first = false;
	}

    {
        mark = Clock::now();
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
			std::strstream string;
			string << "average:" << total << "\t" << ((Sec)(Clock::now() - mark)).count() << "s" << std::ends;
			std::cout << string.str() << std::endl;
			ui->labelAverage->setText(string.str());		
		}
    }
	
	{
		mark = Clock::now(); 
		std::strstream string;
		QImage scaledImage = image.scaled(ui->labelImage->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
		ui->labelImage->setPixmap(QPixmap::fromImage(scaledImage));
		string << "imaged\t" << ((Sec)(Clock::now() - mark)).count() << "s" << std::ends;
		std::cout << string.str() << std::endl;
		ui->labelImagedTime->setText(string.str());
	}
}

