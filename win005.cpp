#include "win005.h"
#include "./ui_win005.h"

Win005::Win005(int intervalA,
               int xA,
               int yA,
               int widthA,
               int heightA,
               QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Win005), first(true),
      interval(intervalA),
      x(xA),
      y(yA),
      width(widthA),
      height(heightA)
{
    ui->setupUi(this);
	
	start();
}

Win005::~Win005()
{
    delete ui;
}

void Win005::start()
{
	screen = QGuiApplication::primaryScreen();
	
	QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Win005::capture);
    timer->start(interval);
}

void Win005::capture()
{
	mark = Clock::now();
    auto pixmap = screen->grabWindow(0, x, y, width, height);
	auto image = pixmap.toImage();
	{
        std::stringstream string;
        string << "captured\t" << ((Sec)(Clock::now() - mark)).count() << "s";
		// std::cout << string.str() << std::endl;
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
		// std::cout << string.str() << std::endl;
        ui->labelImagedTime->setText(string.str().data());
	}
	
	{
		std::size_t valency = 10;
		double centreX = 0.5;
		double centreY = 0.5;
		std::size_t size = 40;
		std::size_t divisor = 4;
		std::size_t multiplier = 2;
		std::vector<QLabel*> records{ 
			ui->labelRecord0, ui->labelRecord1, ui->labelRecord2, 
			ui->labelRecord3, ui->labelRecord4};
		std::vector<QLabel*> events{ 
			ui->labelEvent0, ui->labelEvent1, ui->labelEvent2, 
			ui->labelEvent3, ui->labelEvent4};
		for (std::size_t k = 0; k < records.size(); k++)	
		{
			double scale = std::pow(0.5, k);
			Record record(image, 
				scale * image.height() / image.width(), scale, 
				centreX, centreY, size, size, divisor, divisor);
				
			ui->labelImage->setPixmap(QPixmap::fromImage(scaledImage));

		}			
		
	}
}

