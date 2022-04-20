#include "dev.h"

#include "win001.h"

#include <QApplication>
#include <QLabel>

using namespace Alignment;
using namespace WBOT02;
using namespace std;

#define ECHO(x) cout << #x << endl; x
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
		QImage myImage;
		myImage.load(argv[2]);
		QLabel myLabel;
		myLabel.setPixmap(QPixmap::fromImage(myImage));
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
