#include "win006.h"
#include "./ui_win006.h"
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


Win006::Win006(int intervalA,
               int xA,
               int yA,
               int widthA,
               int heightA,
               QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Win006), 
      interval(intervalA),
      x(xA),
      y(yA),
      width(widthA),
      height(heightA),
	  centreX(0.5),
	  centreY(0.5),
	  valency(10),
	  size(40),
	  divisor(4),
	  multiplier(3)
{
	setCursor(Qt::CrossCursor);
    ui->setupUi(this);
	
	for (std::size_t k = 0; k < 5; k++)	
		scales.push_back(std::pow(0.5, k));
	
	start();
}

Win006::Win006(const std::string& configA,
               QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Win006),
	  config(configA)
{
	setCursor(Qt::CrossCursor);
    ui->setupUi(this);
	configParse();
	start();
}

Win006::~Win006()
{
    delete ui;
}

void Win006::configParse()
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
			std::cout << "Win006\terror: failed to open arguments file " << config << std::endl;
			return;
		}	
		if (!args.IsObject())
		{
			std::cout << "Win006\terror: failed to read arguments file " << config << std::endl;
			return;
		}
	}
	else
	{
		args.Parse("{}");
	}
	interval = ARGS_INT_DEF(interval,1000);	
	x = ARGS_INT_DEF(x,791);	
	y = ARGS_INT_DEF(y,244);	
	width = ARGS_INT_DEF(width,728);	
	height = ARGS_INT_DEF(height,410);	
	centreX = ARGS_DOUBLE_DEF(centreX,0.5);
	centreY = ARGS_DOUBLE_DEF(centreY,0.5);
	if (args.HasMember("scales") && args["scales"].IsArray())
	{
		auto& arr = args["scales"];
		for (int k = 0; k < arr.Size(); k++)
			scales.push_back(arr[k].GetDouble());	
	}
	if (!scales.size())
		for (std::size_t k = 0; k < 5; k++)	
			scales.push_back(std::pow(0.5, k));
	valency = ARGS_INT_DEF(valency,10);	
	size = ARGS_INT_DEF(size,40);	
	divisor = ARGS_INT_DEF(divisor,4);	
	multiplier = ARGS_INT_DEF(multiplier,3);	
}

void Win006::start()
{
	screen = QGuiApplication::primaryScreen();
	
	QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Win006::capture);
    timer->start(interval);
}

void Win006::capture()
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
	
	std::vector<Record> records;
	std::vector<Record> recordValents;
	{
		mark = Clock::now(); 
		for (std::size_t k = 0; k < scales.size(); k++)	
		{
			Record recordA(image, 
                scales[k] * image.height() / image.width(), scales[k],
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
		std::vector<QLabel*> labelRecords{ 
			ui->labelRecord0, ui->labelRecord1, ui->labelRecord2, 
			ui->labelRecord3, ui->labelRecord4};
		std::vector<QLabel*> labelRecordValents{ 
			ui->labelEvent0, ui->labelEvent1, ui->labelEvent2, 
			ui->labelEvent3, ui->labelEvent4};
        for (std::size_t k = 0; k < scales.size() && k < 5; k++)
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

void Win006::mousePressEvent(QMouseEvent *event)
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
