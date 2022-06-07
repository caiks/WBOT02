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
      _ui(new Ui::Win006), 
      _interval(intervalA),
      _x(xA),
      _y(yA),
      _width(widthA),
      _height(heightA),
	  _centreX(0.5),
	  _centreY(0.5),
	  _valency(10),
	  _size(40),
	  _divisor(4),
	  _multiplier(3)
{
	setCursor(Qt::CrossCursor);
    _ui->setupUi(this);
	
	for (std::size_t k = 0; k < 5; k++)	
		_scales.push_back(std::pow(0.5, k));
	
	start();
}

Win006::Win006(const std::string& configA,
               QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::Win006),
	  _config(configA)
{
	setCursor(Qt::CrossCursor);
    _ui->setupUi(this);
	configParse();
	start();
}

Win006::~Win006()
{
    delete _ui;
}

void Win006::configParse()
{
	js::Document args;
	if (!_config.empty())
	{
		std::ifstream in;
		try 
		{
			in.open(_config);
			js::IStreamWrapper isw(in);
			args.ParseStream(isw);
		}
		catch (const std::exception&) 
		{
            std::cout << "Win006\terror: failed to open arguments file " << _config << std::endl;
			return;
		}	
		if (!args.IsObject())
		{
            std::cout << "Win006\terror: failed to read arguments file " << _config << std::endl;
			return;
		}
	}
	else
	{
		args.Parse("{}");
	}
	_interval = ARGS_INT_DEF(interval,1000);	
	_x = ARGS_INT_DEF(x,791);	
	_y = ARGS_INT_DEF(y,244);	
	_width = ARGS_INT_DEF(width,728);	
	_height = ARGS_INT_DEF(height,410);	
	_centreX = ARGS_DOUBLE_DEF(centreX,0.5);
	_centreY = ARGS_DOUBLE_DEF(centreY,0.5);
	if (args.HasMember("scales") && args["scales"].IsArray())
	{
		auto& arr = args["scales"];
		for (int k = 0; k < arr.Size(); k++)
			_scales.push_back(arr[k].GetDouble());	
	}
	if (!_scales.size())
		for (std::size_t k = 0; k < 5; k++)	
			_scales.push_back(std::pow(0.5, k));
	_valency = ARGS_INT_DEF(valency,10);	
	_size = ARGS_INT_DEF(size,40);	
	_divisor = ARGS_INT_DEF(divisor,4);	
	_multiplier = ARGS_INT_DEF(multiplier,3);	
}

void Win006::start()
{
	_screen = QGuiApplication::primaryScreen();
	
	QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Win006::capture);
    timer->start(_interval);
}

void Win006::capture()
{
	_mark = Clock::now();
    auto pixmap = _screen->grabWindow(0, _x, _y, _width, _height);
	auto image = pixmap.toImage();
	{
        std::stringstream string;
        string << "captured\t" << ((Sec)(Clock::now() - _mark)).count() << "s";
		// std::cout << string.str() << std::endl;
        _ui->labelCapturedTime->setText(string.str().data());
	}
	
	std::vector<Record> records;
	std::vector<Record> recordValents;
	{
		_mark = Clock::now(); 
		for (std::size_t k = 0; k < _scales.size(); k++)	
		{
			Record recordA(image, 
                _scales[k] * image.height() / image.width(), _scales[k],
				_centreX, _centreY, _size, _size, _divisor, _divisor);
			records.push_back(recordA);
            Record recordB = recordA.valent(_valency);
			recordValents.push_back(recordB);
		}	
		std::stringstream string;
        string << "recorded\t" << ((Sec)(Clock::now() - _mark)).count() << "s";
		// std::cout << string.str() << std::endl;
        _ui->labelRecordedTime->setText(string.str().data());
	}
	
	{
		_mark = Clock::now(); 
		std::vector<QLabel*> labelRecords{ 
			_ui->labelRecord0, _ui->labelRecord1, _ui->labelRecord2, 
			_ui->labelRecord3, _ui->labelRecord4};
		std::vector<QLabel*> labelRecordValents{ 
			_ui->labelEvent0, _ui->labelEvent1, _ui->labelEvent2, 
			_ui->labelEvent3, _ui->labelEvent4};
        for (std::size_t k = 0; k < _scales.size() && k < 5; k++)
		{			
			labelRecords[k]->setPixmap(QPixmap::fromImage(records[k].image(_multiplier,0)));
			labelRecordValents[k]->setPixmap(QPixmap::fromImage(recordValents[k].image(_multiplier,_valency)));
		}			
	    std::stringstream string;
		_ui->labelImage->setPixmap(QPixmap::fromImage(image));
        string << "imaged\t" << ((Sec)(Clock::now() - _mark)).count() << "s";
		// std::cout << string.str() << std::endl;
        _ui->labelImagedTime->setText(string.str().data());	
	}
	
	{
        std::stringstream string;
        string << "centre\t(" << std::setprecision(3) << _centreX << "," << _centreY << ")";
		// std::cout << string.str() << std::endl;
        _ui->labelCentre->setText(string.str().data());
	}
}

void Win006::mousePressEvent(QMouseEvent *event)
{
    auto geo = _ui->labelImage->geometry();
    auto point = event->position().toPoint() - geo.topLeft();
    _centreX = (double)point.x()/geo.size().width();
    _centreY = (double)point.y()/geo.size().height();
	
	{
        std::stringstream string;
        string << "centre\t(" << std::setprecision(3) << _centreX << "," << _centreY << ")";
        // std::cout << string.str() << std::endl;
        _ui->labelCentre->setText(string.str().data());
	}
}
