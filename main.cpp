#include "dev.h"

#include "win001.h"
#include "win002.h"
#include "win003.h"
#include "win004.h"

#include <QApplication>
#include <QLabel>
#include <QScreen>


using namespace Alignment;
using namespace WBOT02;
using namespace std;

int main(int argc, char *argv[])
{
	if (argc >= 2 && std::string(argv[1]) == "hello")
	{
		cout << "hello" << endl;
	}
	
	if (argc >= 3 && std::string(argv[1]) == "image001")
	{
		auto mark = Clock::now();
		QApplication a(argc, argv);
		QImage image;
		ECHOT(image.load(argv[2]));
		EVAL(image.format());
		EVAL(image.depth());
		EVAL(image.width());
		EVAL(image.height());
		EVAL(image.dotsPerMeterX());
		EVAL(image.dotsPerMeterY());
		ECHO(auto colour = image.pixel(QPoint(0,0)));
        EVALH(colour);
        EVALH(*(std::uint32_t*)image.constBits());
        EVALH(*(QRgb*)image.constBits());
		EVAL(qAlpha(colour));
		EVAL(qRed(colour));
		EVAL(qGreen(colour));
		EVAL(qBlue(colour));
        EVAL(qGray(colour));
        EVAL((qRed(colour)+qGreen(colour)+qBlue(colour))/3);
		// ECHO(image.fill(colour));
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
			EVAL(total);
			total /= 3;
			total /= size;
			cout << "average per pixel:" << total << "\t" << ((Sec)(Clock::now() - mark)).count() << "s" << endl;
		}
		{
			mark = Clock::now();
			std::size_t total = 0;
			for (int y = 0; y < image.height(); ++y) 
			{
				QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(y));
				for (int x = 0; x < image.width(); ++x) {
					QRgb &rgb = line[x];
					total += qRed(rgb) + qGreen(rgb) + qBlue(rgb);
				}
			}
			EVAL(total);
			total /= 3;
			total /= image.height();
			total /= image.height();
			cout << "average per pixel:" << total << "\t" << ((Sec)(Clock::now() - mark)).count() << "s" << endl;
		}
		QLabel myLabel;
		ECHOT(auto pixmap = QPixmap::fromImage(image));
		ECHOT(myLabel.setPixmap(pixmap));
		myLabel.show();
		EVAL(a.exec());
	}
	
	if (argc >= 2 && std::string(argv[1]) == "screen001")
	{
		auto mark = Clock::now();
        QGuiApplication application(argc, argv);
		QScreen *screen = QGuiApplication::primaryScreen();
		ECHOT(auto pixmap = screen->grabWindow(0));
		ECHOT(auto image = pixmap.toImage());
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
			EVAL(total);
			total /= 3;
			total /= size;
			cout << "average per pixel:" << total << "\t" << ((Sec)(Clock::now() - mark)).count() << "s" << endl;
		}
	}
		
    if (argc >= 2 && (std::string(argv[1]) == "image002" || std::string(argv[1]) == "win001"))
	{
        QApplication application(argc, argv);
        Win001 win001(argc >= 3 ? atoi(argv[2]) : 1000);
        win001.show();
        EVAL(application.exec());
	}
	
    if (argc >= 2 && (std::string(argv[1]) == "screen002" || std::string(argv[1]) == "win002"))
	{
        QApplication application(argc, argv);
        Win002 win002(argc >= 3 ? atoi(argv[2]) : 1000,
            argc >= 4 ? atoi(argv[3]) : 0,
            argc >= 5 ? atoi(argv[4]) : 0,
            argc >= 6 ? atoi(argv[5]) : -1,
            argc >= 7 ? atoi(argv[6]) : -1);
        win002.show();
        EVAL(application.exec());
	}
	
    if (argc >= 3 && (std::string(argv[1]) == "video001" || std::string(argv[1]) == "win003"))
	{
        QApplication application(argc, argv);

        Win003 win003(QString(argv[2]),
                      argc >= 4 ? atoi(argv[3]) : 1000);
        win003.show();
        EVAL(application.exec());
	}

    if (argc >= 3 && (std::string(argv[1]) == "video002" || std::string(argv[1]) == "win004"))
    {
        QApplication application(argc, argv);

        Win004 win004(QString(argv[2]),
            argc >= 4 ? atoi(argv[3]) : 0,
            argc >= 5 ? atoi(argv[4]) : 0);
        win004.show();
        EVAL(application.exec());
    }
	
	return 0;
}
