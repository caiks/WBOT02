#include "dev.h"

#include "win001.h"

#include <QApplication>
#include <QLabel>

using namespace Alignment;
using namespace WBOT02;
using namespace std;

#define ECHO(x) cout << #x << endl; x
#define ECHOT(x) cout << #x ; mark = Clock::now(); x ; cout << "\t" << ((Sec)(Clock::now() - mark)).count() << "s" << endl
#define EVAL(x) cout << #x << ": " << (x) << endl
#define EVALL(x) cout << #x << ": " << endl << (x) << endl
#define TRUTH(x) cout << #x << ": " << ((x) ? "true" : "false") << endl

int main(int argc, char *argv[])
{
	if (argc >= 2 && std::string(argv[1]) == "hello")
	{
		cout << "hello" << endl;
	}
	
	if (argc >= 3 && std::string(argv[1]) == "image001")
	{
		QApplication a(argc, argv);
		QImage image;
		auto mark = Clock::now();
		ECHOT(image.load(argv[2]));
		EVAL(image.format());
		EVAL(image.depth());
		EVAL(image.width());
		EVAL(image.height());
		EVAL(image.dotsPerMeterX());
		EVAL(image.dotsPerMeterY());
		ECHO(auto colour = image.pixel(QPoint(0,0)));
		EVAL(colour);
		EVAL(*(std::uint32_t*)image.constBits());
		EVAL(qAlpha(colour));
		EVAL(qRed(colour));
		EVAL(qGreen(colour));
		EVAL(qBlue(colour));
		// ECHO(image.fill(colour));
		QLabel myLabel;
		ECHOT(auto pixmap = QPixmap::fromImage(image));
		ECHOT(myLabel.setPixmap(pixmap));
		myLabel.show();
		EVAL(a.exec());
	}
	
	if (argc >= 2 && std::string(argv[1]) == "win001")
	{
		QApplication a(argc, argv);
		Win001 w;
		w.show();
		EVAL(a.exec());
	}
	return 0;
}
