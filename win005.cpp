#include "win005.h"
#include "./ui_win005.h"
#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

using namespace WBOT02;
namespace js = rapidjson;

#define ARGS_STRING_DEF(x,y) args.HasMember(#x) && args[#x].IsString() ? args[#x].GetString() : y
#define ARGS_STRING(x) ARGS_STRING_DEF(x,"")
#define ARGS_INT_DEF(x,y) args.HasMember(#x) && args[#x].IsInt() ? args[#x].GetInt() : y
#define ARGS_INT(x) ARGS_INT_DEF(x,0)
#define ARGS_DOUBLE_DEF(x,y) args.HasMember(#x) && args[#x].IsDouble() ? args[#x].GetDouble() : y
#define ARGS_DOUBLE(x) ARGS_DOUBLE_DEF(x,0.0)
#define ARGS_BOOL_DEF(x,y) args.HasMember(#x) && args[#x].IsBool() ? args[#x].GetBool() : y
#define ARGS_BOOL(x) ARGS_BOOL_DEF(x,false)


Win005::Win005(int intervalA,
               int xA,
               int yA,
               int widthA,
               int heightA,
               QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Win005), 
      interval(intervalA),
      x(xA),
      y(yA),
      width(widthA),
      height(heightA),
	  centreX(0.5),
	  centreY(0.5)
{
	setCursor(Qt::CrossCursor);
    ui->setupUi(this);
	
	start();
}

Win005::Win005(std::string config,
               QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Win005)
{
	setCursor(Qt::CrossCursor);
    ui->setupUi(this);
	configParse();
	start();
}

Win005::~Win005()
{
    delete ui;
}

void Win005::configParse()
{
	js::Document args;
	if (!config.empty())
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
			std::cout << "Win005\terror: failed to open arguments file " << config << std::endl;
			return;
		}	
		if (!args.IsObject())
		{
			std::cout << "Win005\terror: failed to read arguments file " << config << std::endl;
			return;
		}
	}
	else
	{
		args.Parse("{}");
	}
	interval = ARGS_INT_DEF(interval,1000);	
	
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
	
	std::size_t scaleSize = 5;
	std::size_t valency = 10;
	std::vector<Record> records;
	std::vector<Record> recordValents;
	{
		mark = Clock::now(); 
		std::size_t size = 40;
		std::size_t divisor = 4;
		for (std::size_t k = 0; k < scaleSize; k++)	
		{
			double scale = std::pow(0.5, k);
			Record recordA(image, 
				scale * image.height() / image.width(), scale, 
				centreX, centreY, size, size, divisor, divisor);
			records.push_back(recordA);
            Record recordB = recordA.valent(valency);
			recordValents.push_back(recordB);
		}	
		std::stringstream string;
        string << "recorded\t" << ((Sec)(Clock::now() - mark)).count() << "s";
		// std::cout << string.str() << std::endl;
        ui->labelRecordedTime->setText(string.str().data());
	}
	
	{
		mark = Clock::now(); 
		std::size_t multiplier = 3;
		std::vector<QLabel*> labelRecords{ 
			ui->labelRecord0, ui->labelRecord1, ui->labelRecord2, 
			ui->labelRecord3, ui->labelRecord4};
		std::vector<QLabel*> labelRecordValents{ 
			ui->labelEvent0, ui->labelEvent1, ui->labelEvent2, 
			ui->labelEvent3, ui->labelEvent4};
		for (std::size_t k = 0; k < scaleSize; k++)	
		{			
			labelRecords[k]->setPixmap(QPixmap::fromImage(records[k].image(multiplier,0)));
			labelRecordValents[k]->setPixmap(QPixmap::fromImage(recordValents[k].image(multiplier,valency)));
		}			
	    std::stringstream string;
		ui->labelImage->setPixmap(QPixmap::fromImage(image));
        string << "imaged\t" << ((Sec)(Clock::now() - mark)).count() << "s";
		// std::cout << string.str() << std::endl;
        ui->labelImagedTime->setText(string.str().data());	
	}
	
	{
        std::stringstream string;
        string << "centre\t(" << std::setprecision(3) << centreX << "," << centreY << ")";
		// std::cout << string.str() << std::endl;
        ui->labelCentre->setText(string.str().data());
	}
}

void Win005::mousePressEvent(QMouseEvent *event)
{
    auto geo = ui->labelImage->geometry();
    auto point = event->position().toPoint() - geo.topLeft();
    centreX = (double)point.x()/geo.size().width();
    centreY = (double)point.y()/geo.size().height();
	
	{
        std::stringstream string;
        string << "centre\t(" << std::setprecision(3) << centreX << "," << centreY << ")";
        // std::cout << string.str() << std::endl;
        ui->labelCentre->setText(string.str().data());
	}
}
