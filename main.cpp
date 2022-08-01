#include "dev.h"

#include "win001.h"
#include "win002.h"
#include "win003.h"
#include "win004.h"
#include "win005.h"
#include "win006.h"
#include "win007.h"

#include <QApplication>
#include <QLabel>
#include <QScreen>

#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

using namespace Alignment;
using namespace WBOT02;
using namespace std;
namespace js = rapidjson;

#define ARGS_STRING_DEF(x,y) args.HasMember(#x) && args[#x].IsString() ? args[#x].GetString() : y
#define ARGS_STRING(x) ARGS_STRING_DEF(x,"")
#define ARGS_INT_DEF(x,y) args.HasMember(#x) && args[#x].IsInt() ? args[#x].GetInt() : y
#define ARGS_INT(x) ARGS_INT_DEF(x,0)
#define ARGS_DOUBLE_DEF(x,y) args.HasMember(#x) && args[#x].IsDouble() ? args[#x].GetDouble() : y
#define ARGS_DOUBLE(x) ARGS_DOUBLE_DEF(x,0.0)
#define ARGS_BOOL_DEF(x,y) args.HasMember(#x) && args[#x].IsBool() ? args[#x].GetBool() : y
#define ARGS_BOOL(x) ARGS_BOOL_DEF(x,false)

#define UNLOG  << std::endl; }
#define LOG { std::cout <<

int main(int argc, char *argv[])
{
	if (argc >= 2 && std::string(argv[1]) == "hello")
	{
		cout << "hello" << endl;
	}
	
	if (argc >= 2 && std::string(argv[1]) == "records")
	{
        RecordList rr {Record(0.9,0.8,0.7,0.6,3,2),Record(0.1,0.2,0.3,0.4,2,1)};
        *(rr[0].arr) = std::vector<unsigned char> {0,1,2,3,4,5};
        *(rr[1].arr) = std::vector<unsigned char> {6,7};

		cout << "rr" << endl
			<< rr << endl;

		try 
		{
			std::ofstream out("test.bin", std::ios::binary);
			ECHO(recordListsPersistent(rr, out));
			out.close();
		}
        catch (const std::exception&)
		{
			cout << "recordListsPersistent dump failed" << endl;
		}
		std::unique_ptr<RecordList> rr2;
		try 
		{
			std::ifstream in("test.bin", std::ios::binary);
			ECHO(rr2 = persistentsRecordList(in));
			in.close();
		}
        catch (const std::exception&)
		{
			cout << "recordListsPersistent load failed" << endl;
		}
		cout << "rr2" << endl
			<< *rr2 << endl;

		std::ifstream in2("test.bin", std::ios::binary);
		cout << in2 << endl;
		in2.close();
	}
	
	if (argc >= 2 && std::string(argv[1]) == "representations")
	{
        Representation r1(0.9,0.8,3,2), r2(0.1,0.2,2,1);
        *(r1.arr) = std::vector<std::size_t> {0,1,2,3,4,5};
        *(r2.arr) = std::vector<std::size_t> {6,7};

		EVAL(r1);
		EVAL(r2);
		
		SliceRepresentationUMap rr;
		rr[1] = r1;
		rr[2] = r2;
		
		cout << "rr" << endl
			<< rr << endl;
		
		try 
		{
			std::ofstream out("test.bin", std::ios::binary);
			ECHO(sliceRepresentationUMapsPersistent(rr, out));
			out.close();
		}
        catch (const std::exception&)
		{
			cout << "sliceRepresentationUMapsPersistent dump failed" << endl;
		}
		std::unique_ptr<SliceRepresentationUMap> rr2;
		try 
		{
			std::ifstream in("test.bin", std::ios::binary);
			ECHO(rr2 = persistentsSliceRepresentationUMap(in));
			in.close();
		}
        catch (const std::exception&)
		{
			cout << "persistentsSliceRepresentationUMap load failed" << endl;
		}
		cout << "rr2" << endl
			<< *rr2 << endl;
	}
	
	if (argc >= 3 && (string(argv[1]) == "view_active" || string(argv[1]) == "view_active_concise"))
	{
		bool ok = true;
		string model = string(argv[2]);
	
		EVAL(model);
		
		bool concise = string(argv[1]) == "view_active_concise";
		TRUTH(concise);

		Active activeA;
		activeA.logging = true;		
		if (ok) 
		{
			activeA.historySliceCachingIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			TRUTH(ok);				
		}		
		if (ok)
		{
			EVAL(activeA.name);				
			EVAL(activeA.underlyingEventUpdateds);		
			std::size_t sizeA = activeA.historyOverflow ? activeA.historySize : activeA.historyEvent;				
			EVAL(activeA.historySize);				
			TRUTH(activeA.historyOverflow);				
			EVAL(activeA.historyEvent);				
			EVAL(sizeA);
			EVAL(activeA.historyEvent);		
			TRUTH(activeA.continousIs);				
			EVAL(activeA.continousHistoryEventsEvent);	
			TRUTH(activeA.historySliceCachingIs);				
			TRUTH(activeA.historySliceCumulativeIs);				
			EVAL(activeA.historySlicesSize.size());		
			EVAL(activeA.historySlicesSlicesSizeNext.size());		
			EVAL(activeA.historySlicesSliceSetPrev.size());				
			for (auto& hr : activeA.underlyingHistoryRepa)
			{
				EVAL(hr->dimension);				
				EVAL(hr->size);				
				// EVAL(*hr);				
			}
			for (auto& hr : activeA.underlyingHistorySparse)
			{
				EVAL(hr->size);				
				// EVAL(*hr);				
			}			
			if (!concise)
			{
				EVAL(sorted(activeA.underlyingSlicesParent));				
			}
			else 
			{
				EVAL(activeA.underlyingSlicesParent.size());				
			}			
			EVAL(activeA.bits);				
			EVAL(activeA.var);				
			EVAL(activeA.varSlice);				
			EVAL(activeA.induceThreshold);				
			EVAL(activeA.induceVarExclusions);				
			if (activeA.historySparse) {EVAL(activeA.historySparse->size);}
			if (!concise)
			{
				if (activeA.historySparse) {EVAL(*activeA.historySparse);}				
				EVAL(activeA.historySlicesSetEvent);			
			}	
			else 
			{
				EVAL(activeA.underlyingSlicesParent.size());				
			}			
			EVAL(activeA.historySlicesSetEvent.size());				
			EVAL(activeA.induceSlices);				
			EVAL(activeA.induceSliceFailsSize);				
			EVAL(activeA.frameUnderlyings);				
			EVAL(activeA.frameHistorys);				
			// EVAL(activeA.framesVarsOffset);				
			if (activeA.decomp) {EVAL(activeA.decomp->fuds.size());}
			if (activeA.decomp) {EVAL(activeA.decomp->fudRepasSize);}
			if (activeA.decomp) {EVAL((double)activeA.decomp->fuds.size() * activeA.induceThreshold / sizeA);}
			// if (activeA.decomp) 
			// {
				// auto er = dfrer(*activeA.decomp);
				// EVAL(sorted(er->substrate));
			// }
			if (!concise)
			{
				if (activeA.decomp) {EVAL(*activeA.decomp);}
			}	
			else 
			{
				TRUTH(activeA.decomp);				
			}
		}
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
	
	if (argc >= 3 && std::string(argv[1]) == "image003")
	{
		auto mark = Clock::now();
		QApplication a(argc, argv);
		std::size_t valency = argc >= 4 ? atoi(argv[3]) : 0;
		double scale = argc >= 5 ? 1.0 / atof(argv[4]) : 1.0;
		double centreX = argc >= 6 ? atof(argv[5]) : 0.5;
		double centreY = argc >= 7 ? atof(argv[6]) : 0.5;
		std::size_t size = argc >= 8 ? atoi(argv[7]) : 40;
		std::size_t divisor = argc >= 9 ? atoi(argv[8]) : 0;
		std::size_t multiplier = argc >= 10 ? atoi(argv[9]) : 120/size;
		QImage imageA;
		ECHOT(imageA.load(argv[2]));
        ECHOT(Record recordA(imageA, scale * imageA.height() / imageA.width(), scale, centreX, centreY, size, size, divisor, divisor));
		// EVAL(recordA);
        ECHOT(Record recordB = recordA.valent(valency));
		// EVAL(recordB);
        ECHOT(QImage imageB = recordB.image(multiplier,valency));
		QLabel myLabel;
		ECHOT(auto pixmap = QPixmap::fromImage(imageB));
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
		
    if (argc >= 2 && (std::string(argv[1]) == "camera001" || std::string(argv[1]) == "image002" || std::string(argv[1]) == "win001"))
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
            argc >= 5 ? atoi(argv[4]) : 0,
            argc >= 6 ? atof(argv[5]) : 0.0);
        win004.show();
        EVAL(application.exec());
    }
	
	if (argc >= 2 && (std::string(argv[1]) == "screen003" || std::string(argv[1]) == "win005"))
	{
        QApplication application(argc, argv);
        Win005 win005(argc >= 3 ? atoi(argv[2]) : 1000,
            argc >= 4 ? atoi(argv[3]) : 0,
            argc >= 5 ? atoi(argv[4]) : 0,
            argc >= 6 ? atoi(argv[5]) : -1,
            argc >= 7 ? atoi(argv[6]) : -1);

        const auto screenSize = QGuiApplication::primaryScreen()->availableGeometry();
        win005.resize(screenSize.width()/2, screenSize.height()*0.95);
        win005.move(screenSize.topLeft());

        win005.show();
        EVAL(application.exec());
	}
	
	if (argc >= 3 && std::string(argv[1]) == "screen004")
	{
        QApplication application(argc, argv);
        Win005 win005(std::string(argv[2]),nullptr);

        const auto screenSize = QGuiApplication::primaryScreen()->availableGeometry();
        win005.resize(screenSize.width()/2, screenSize.height()*0.95);
        win005.move(screenSize.topLeft());

        win005.show();
        EVAL(application.exec());
	}
	
	if (argc >= 2 && (std::string(argv[1]) == "actor001" || std::string(argv[1]) == "win006"))
	{
        QApplication application(argc, argv);
        Win006 win006(std::string(argc >= 3 ? argv[2] : ""),nullptr);

        const auto screenSize = QGuiApplication::primaryScreen()->availableGeometry();
        win006.resize(screenSize.width()/2, screenSize.height()*0.95);
        win006.move(screenSize.topLeft());

        win006.show();
        EVAL(application.exec());
	}
	
	if (argc >= 2 && (std::string(argv[1]) == "actor002" || std::string(argv[1]) == "win007"))
	{
        QApplication application(argc, argv);
        Win007 win007(std::string(argc >= 3 ? argv[2] : ""),nullptr);

        const auto screenSize = QGuiApplication::primaryScreen()->availableGeometry();
        win007.resize(screenSize.width()/2, screenSize.height()*0.95);
        win007.move(screenSize.topLeft());

        win007.show();
        EVAL(application.exec());
	}
		
	if (argc >= 2 && string(argv[1]) == "generate_contour")
	{
		bool ok = true;
		int stage = 0;
		
		js::Document args;
		if (ok)
		{
			string config = "contour.json";
			if (argc >= 3) config = string(argv[2]);
			if (ok && !config.empty())
			{
				std::ifstream in;
				try 
				{
					in.open(config);
					js::IStreamWrapper isw(in);
					args.ParseStream(isw);
				}
				catch (const std::exception&) 
				{
					ok = false;
				}	
				if (!args.IsObject())
				{
					ok = false;
				}
			}
			else
			{
				args.Parse("{}");
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}
		string model = ARGS_STRING(model);
		string inputFilename = ARGS_STRING(input_file);
		string outputFilename = ARGS_STRING(output_file);
		auto centreX = ARGS_DOUBLE_DEF(centreX,0.5);
		auto centreY = ARGS_DOUBLE_DEF(centreY,0.5);
		auto centreRangeX = ARGS_DOUBLE_DEF(range_centreX,0.41);
		auto centreRangeY = ARGS_DOUBLE_DEF(range_centreY,0.25);
		auto scale = ARGS_DOUBLE_DEF(scale,0.5);
		auto valency = ARGS_INT_DEF(valency,10);	
		auto size = ARGS_INT_DEF(size,40);	
		auto divisor = ARGS_INT_DEF(divisor,4);	
		if (ok)
		{
			ok = ok && model.size();
			ok = ok && inputFilename.size();
			ok = ok && outputFilename.size();
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		Active activeA;
		if (ok) 
		{
			activeA.continousIs = true;
			activeA.historySliceCachingIs = true;
			activeA.historySliceCumulativeIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			activeA.historySliceCachingIs = true;
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		QImage image;
		int captureWidth = 0;
		int captureHeight = 0;	
		if (ok)
		{
			ok = ok && image.load(QString(inputFilename.c_str()));
			captureWidth = image.width();
			EVAL(captureWidth);
			captureHeight = image.height();
			EVAL(captureHeight);
			ok = ok && captureWidth > 0 && captureHeight > 0;
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		
		if (ok)
		{

			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		
		
	}
	return 0;
}
