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
	{
		
	}
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
	_actLogging = ARGS_BOOL(logging_action);
	_actLoggingFactor = ARGS_INT(logging_action_factor);
 	_interval = ARGS_INT_DEF(interval,1000);	
	_mode = ARGS_STRING_DEF(mode, "mode001");
	_modeLogging = ARGS_BOOL(logging_mode);
	_modeLoggingFactor = ARGS_INT(logging_mode_factor); 
	_modeTracing = ARGS_BOOL(tracing_mode);
	_eventId = 0;
	_eventIdMax = ARGS_INT(event_maximum);
	_model = ARGS_STRING(model);
	_modelInitial = ARGS_STRING(model_initial);
	_activeLogging = ARGS_BOOL(logging_active);
	_activeSummary = ARGS_BOOL(summary_active);
	_activeSize = ARGS_INT_DEF(activeSize,1000000);
	_induceThreshold = ARGS_INT_DEF(induceThreshold,200);
	_induceThresholdInitial = ARGS_INT_DEF(induceThresholdInitial,1000);
	_induceInterval = ARGS_INT_DEF(induceInterval,10);	
	_induceThreadCount = ARGS_INT_DEF(induceThreadCount,4);
	_induceNot = ARGS_BOOL(no_induce);
	_induceParameters.tint = _induceThreadCount;		
	_induceParameters.wmax = ARGS_INT_DEF(induceParameters.wmax,18);
	_induceParameters.lmax = ARGS_INT_DEF(induceParameters.lmax,8);
	_induceParameters.xmax = ARGS_INT_DEF(induceParameters.xmax,128);
	_induceParameters.znnmax = 200000.0 * 2.0 * 300.0 * 300.0 * _induceThreadCount;
	_induceParameters.omax = ARGS_INT_DEF(induceParameters.omax,10);
	_induceParameters.bmax = ARGS_INT_DEF(induceParameters.bmax,10*3);
	_induceParameters.mmax = ARGS_INT_DEF(induceParameters.mmax,3);
	_induceParameters.umax = ARGS_INT_DEF(induceParameters.umax,128);
	_induceParameters.pmax = ARGS_INT_DEF(induceParameters.pmax,1);
	_induceParameters.mult = ARGS_INT_DEF(induceParameters.mult,1);
	_induceParameters.seed = ARGS_INT_DEF(induceParameters.seed,5);	
	_induceParameters.diagonalMin = ARGS_DOUBLE_DEF(induceParameters.diagonalMin,6.0);
	if (args.HasMember("induceParameters.induceThresholds"))
	{
		auto& a = args["induceParameters.induceThresholds"];
		if (a.IsArray())
			for (auto& v : a.GetArray())
				if (v.IsInt())
					_induceParameters.induceThresholds.insert(v.GetInt());
	}
	else
	{
		_induceParameters.induceThresholds = std::set<std::size_t>{110,120,150,180,200,300,400,500,800,1000};
	}	
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
	_multiplier = ARGS_INT_DEF(multiplier,2);	
}

void Win006::start()
{
	_screen = QGuiApplication::primaryScreen();
	
	QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Win006::act);
    timer->start(_interval);
}

void Win006::act()
{
	_mark = Clock::now();
    auto pixmap = _screen->grabWindow(0, _x, _y, _width, _height);
	auto image = pixmap.toImage();
	{
        std::stringstream string;
        string << "actd\t" << ((Sec)(Clock::now() - _mark)).count() << "s";
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
		std::vector<QLabel*> labelRecordSlices{ 
			_ui->labelSlice0, _ui->labelSlice1, _ui->labelSlice2, 
			_ui->labelSlice3, _ui->labelSlice4};
        for (std::size_t k = 0; k < _scales.size() && k < 5; k++)
		{			
			labelRecords[k]->setPixmap(QPixmap::fromImage(records[k].image(_multiplier,0)));
			labelRecordValents[k]->setPixmap(QPixmap::fromImage(recordValents[k].image(_multiplier,_valency)));
			labelRecordSlices[k]->setPixmap(QPixmap::fromImage(recordValents[k].image(_multiplier,_valency)));	// TODO point to slice rep
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
