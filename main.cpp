#include "dev.h"
#include "win001.h"
#include <QApplication>
#include <cmath>
#include <cstring>

using namespace Alignment;
using namespace WBOT02;
using namespace std;

typedef std::chrono::duration<double> sec; 
typedef std::chrono::high_resolution_clock clk;

#define ECHO(x) cout << #x << endl; x
#define EVAL(x) cout << #x << ": " << (x) << endl
#define EVALL(x) cout << #x << ": " << endl << (x) << endl
#define TRUTH(x) cout << #x << ": " << ((x) ? "true" : "false") << endl

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Win001 w;
    w.show();
	
    return a.exec();
}
