#include "win001.h"
#include "./ui_win001.h"

Win001::Win001(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Win001), first(true)
{
    ui->setupUi(this);
	
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
    timer->start(1000);
}

Win001::~Win001()
{
    delete ui;
}

void Win001::captureInit()
{
    mark = Clock::now();
	std::cout << "capturing ..." << std::endl;
	imageCapture->capture();
}


void Win001::capture(int id, const QImage &image)
{
	std::cout << "captured\t" << ((Sec)(Clock::now() - mark)).count() << "s" << std::endl;
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
        std::cout << "average per pixel:" << total << "\t" << ((Sec)(Clock::now() - mark)).count() << "s" << std::endl;
    }
	mark = Clock::now(); 
	QImage scaledImage = image.scaled(ui->label->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->label->setPixmap(QPixmap::fromImage(scaledImage));
	std::cout << "label\t" << ((Sec)(Clock::now() - mark)).count() << "s" << std::endl;
}

