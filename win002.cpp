#include "win002.h"
#include "./ui_win002.h"

Win002::Win002(int intervalA,
               int xA,
               int yA,
               int widthA,
               int heightA,
               QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Win002), first(true),
      interval(intervalA),
      x(xA),
      y(yA),
      width(widthA),
      height(heightA)
{
    ui->setupUi(this);
	
	start();
}

Win002::~Win002()
{
    delete ui;
}

void Win002::start()
{
	screen = QGuiApplication::primaryScreen();
	
	QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Win002::capture);
    timer->start(interval);
}

void Win002::capture()
{
	mark = Clock::now();
    auto pixmap = screen->grabWindow(0, x, y, width, height);
	if (first)
	{
		EVAL(pixmap.devicePixelRatio());
	}
	auto image = pixmap.toImage();
	{
        std::stringstream string;
        string << "captured\t" << ((Sec)(Clock::now() - mark)).count() << "s";
		std::cout << string.str() << std::endl;
        ui->labelCapturedTime->setText(string.str().data());
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
            std::stringstream string;
            string << "average:" << total << "\t" << ((Sec)(Clock::now() - mark)).count() << "s";
			std::cout << string.str() << std::endl;
            ui->labelAverage->setText(string.str().data());
		}
    }
	
	{
		mark = Clock::now(); 
        std::stringstream string;
		QImage scaledImage = image.scaled(ui->labelImage->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
		ui->labelImage->setPixmap(QPixmap::fromImage(scaledImage));
        string << "imaged\t" << ((Sec)(Clock::now() - mark)).count() << "s";
		std::cout << string.str() << std::endl;
        ui->labelImagedTime->setText(string.str().data());
	}
}

