#include "win006.h"
#include "./ui_win006.h"
#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

using namespace Alignment;
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

#define UNLOG  << std::endl; }
#define LOG { std::cout <<

static void actor_log(Active& active, const std::string& str)
{
	LOG str UNLOG
	return;
};

static void layerer_actor_log(const std::string& str)
{
	LOG str UNLOG
	return;
};

void run_induce(Win006& actor, Active& active, ActiveInduceParameters& param, std::size_t induceThresholdInitial, std::size_t induceInterval)
{
	while (!actor.terminate && !active.terminate)
	{
		if (actor.eventId >= induceThresholdInitial)
			active.induce(param);
		std::this_thread::sleep_for(std::chrono::milliseconds(induceInterval));
	}	
	return;
};

Win006::Win006(const std::string& configA,
               QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::Win006),
	  _config(configA)
{
	setCursor(Qt::CrossCursor);
    _ui->setupUi(this);
	// parse config
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
				LOG "Win006\terror: failed to open arguments file " << _config UNLOG
				return;
			}	
			if (!args.IsObject())
			{
				LOG "Win006\terror: failed to read arguments file " << _config UNLOG
				return;
			}
		}
		else
		{
			args.Parse("{}");
		}
		terminate = false;
		eventId = 0;
		_actLogging = ARGS_BOOL(logging_action);
		_actLoggingFactor = ARGS_INT(logging_action_factor);
		_interval = ARGS_INT_DEF(interval,1000);	
		_mode = ARGS_STRING_DEF(mode, "mode001");
		_modeLogging = ARGS_BOOL(logging_mode);
		_modeLoggingFactor = ARGS_INT(logging_mode_factor); 
		_modeTracing = ARGS_BOOL(tracing_mode);
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
		
		// EVAL(_model);
		// EVAL(_mode);
	}
	// load slice representations if modelInitial TODO
	/*
	{
		try
		{
			std::ifstream in(modelInitial + ".rec", std::ios::binary);
			if (in.is_open())
			{
				_records = std::move(persistentsRecordList(in));
				_records->push_back(Record());
				in.close();
			}
			else
			{
				LOG "actor\terror: failed to open records file" << modelInitial + ".rec" UNLOG
				return;
			}
		}
		catch (const exception&)
		{
			LOG "actor\terror: failed to read records file" << modelInitial + ".rec" UNLOG
         return;
		}		
	}
	*/
	// create active
	{
		{
            SystemSystemRepaTuple xx(recordsSystemSystemRepaTuple(_scales.size(), _valency, _size*_size));
            _uu = std::move(std::get<0>(xx));
            _ur = std::move(std::get<1>(xx));
		}
		_system = std::make_shared<ActiveSystem>();
		_threads.reserve(1);
		_events = std::make_shared<ActiveEventsRepa>(1);
		_active = std::make_shared<Active>();
		{
			auto& activeA = *_active;
			activeA.log = actor_log;
			activeA.layerer_log = layerer_actor_log;
			activeA.system = _system;
			activeA.continousIs = true;
            if (_modelInitial.size())
			{
				ActiveIOParameters ppio;
                ppio.filename = _modelInitial + ".ac";
				activeA.logging = true;
				if (!activeA.load(ppio))
				{
					LOG "actor\terror: failed to load model" << ppio.filename UNLOG				
					_system.reset();
					return;
				}								
				_system->block = std::max(_system->block, activeA.varMax() >> activeA.bits);
				if (activeA.underlyingEventUpdateds.size())
					this->eventId = std::max(this->eventId,*(activeA.underlyingEventUpdateds.rbegin()));					
				else if (activeA.historyOverflow)
					this->eventId = std::max(this->eventId,activeA.historySize);	
				else					
					this->eventId = std::max(this->eventId,activeA.historyEvent);	
			}
			else
			{
				activeA.var = activeA.system->next(activeA.bits);
				activeA.varSlice = activeA.system->next(activeA.bits);
				activeA.historySize = _activeSize;
				activeA.induceThreshold = _induceThreshold;
				activeA.decomp = std::make_unique<DecompFudSlicedRepa>();	
				{
					auto hr = sizesHistoryRepa(_scales.size(), _valency, _size*_size, activeA.historySize);
					activeA.underlyingHistoryRepa.push_back(std::move(hr));
				}				
				{
					auto hr = std::make_unique<HistorySparseArray>();
					{
						auto z = activeA.historySize;
						hr->size = z;
						hr->capacity = 1;
						hr->arr = new std::size_t[z];		
					}		
					activeA.historySparse = std::move(hr);			
				}
			}
			activeA.name = (_model!="" ? _model : "model");			
			activeA.logging = _activeLogging;
			activeA.summary = _activeSummary;
			activeA.underlyingEventsRepa.push_back(_events);
			activeA.eventsSparse = std::make_shared<ActiveEventsArray>(1);
			std::size_t sizeA = activeA.historyOverflow ? activeA.historySize : activeA.historyEvent;
			if (sizeA)
			{
				LOG activeA.name << "\tfuds cardinality: " << activeA.decomp->fuds.size() << "\tmodel cardinality: " << activeA.decomp->fudRepasSize << "\tactive size: " << sizeA << "\tfuds per threshold: " << (double)activeA.decomp->fuds.size() * activeA.induceThreshold / sizeA UNLOG				
			}
			if (!_induceNot)
				_threads.push_back(std::thread(run_induce, std::ref(*this), std::ref(activeA), std::ref(_induceParameters), _induceThresholdInitial, _induceInterval));
		}
	}
	// start act timer
	{
		_screen = QGuiApplication::primaryScreen();
		
		QTimer *timer = new QTimer(this);
		connect(timer, &QTimer::timeout, this, &Win006::act);
		timer->start(_interval);
	}
	LOG "actor\tSTART" UNLOG
}

Win006::~Win006()
{
	terminate = true;
	// dump slice representations TODO
/* 	if (_model.size())
	{
		try
		{
			std::ofstream out(_model + ".rec", std::ios::binary);
			recordListsPersistent(*_records, out); 
			out.close();
			LOG "actor\tdump\tfile name:" << _model + ".rec" UNLOG
		}
		catch (const exception&)
		{
			LOG "actor\terror: failed to write records file" <<_model + ".rec" UNLOG
		}			
	} */
	if (_system)
	{
		_active->terminate = true;
		for (auto& t : _threads)
			t.join();
		if ( _model!="")
		{
			auto& activeA = *_active;
			ActiveIOParameters ppio;
			ppio.filename = activeA.name+".ac";
			activeA.logging = true;
			activeA.dump(ppio);		
		}			

	}
    delete _ui;
	LOG "actor\tTERMINATE" UNLOG
}

void Win006::act()
{
	_mark = Clock::now();
    auto pixmap = _screen->grabWindow(0, _x, _y, _width, _height);
	auto image = pixmap.toImage();
	{
        std::stringstream string;
        string << "actd\t" << ((Sec)(Clock::now() - _mark)).count() << "s";
		// LOG string.str() UNLOG
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
		// LOG string.str() UNLOG
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
		// LOG string.str() UNLOG
        _ui->labelImagedTime->setText(string.str().data());	
	}
	
	{
        std::stringstream string;
        string << "centre\t(" << std::setprecision(3) << _centreX << "," << _centreY << ")";
		// LOG string.str() UNLOG
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
        // LOG string.str() UNLOG
        _ui->labelCentre->setText(string.str().data());
	}
}
