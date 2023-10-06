#include "win007.h"
#include "./ui_win007.h"
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

void run_induce(Win007& actor, Active& active, ActiveInduceParameters& param, std::chrono::milliseconds induceInterval)
{
	while (!actor.terminate && !active.terminate && active.induce(param))
		std::this_thread::sleep_for(induceInterval);
	return;
};

Win007::Win007(const std::string& configA,
               QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::Win007),
	  _config(configA)
{
	setCursor(Qt::CrossCursor);
    _ui->setupUi(this);
	this->terminate = true;
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
				LOG "actor\terror: failed to open arguments file " << _config UNLOG
				return;
			}	
			if (!args.IsObject())
			{
				LOG "actor\terror: failed to read arguments file " << _config UNLOG
				return;
			}
		}
		else
		{
			args.Parse("{}");
		}
		this->eventId = 0;
		_eventIdPrev = 0;
		_labelSize = ARGS_INT_DEF(label_size,8);
		_eventLogging = ARGS_BOOL(logging_event);
		_eventLoggingFactor = ARGS_INT(logging_event_factor);
		_actLogging = ARGS_BOOL(logging_action);
		_actLoggingFactor = ARGS_INT(logging_action_factor);
		_actCount = 0;
		_interval = (std::chrono::milliseconds)(ARGS_INT_DEF(interval,1000));
		_lagThreshold = ARGS_INT(lag_threshold);	
		_lagWaiting = false;
        _motionThreshold = ARGS_INT(motion_detection_threshold);
		_motionCount = 0;
        _motionHashStep = ARGS_INT_DEF(motion_detection_hash_step,17);
		_motionHash = 0; 
		_motionWaiting = false;
		_actWarning = ARGS_BOOL(warning_action);
		_actLoggingSlice = ARGS_BOOL(logging_action_slice);
		_mode = ARGS_STRING(mode);
		_modeLogging = ARGS_BOOL(logging_mode);
		_modeLoggingFactor = ARGS_INT(logging_mode_factor); 
		_modeTracing = ARGS_BOOL(tracing_mode);
		_eventIdMax = ARGS_INT(event_maximum);
		_struct = ARGS_STRING(structure);
		_model = ARGS_STRING(model);
		_modelInitial = ARGS_STRING(model_initial);
		_level1Model = ARGS_STRING(level1_model);
		_interactive = ARGS_BOOL(interactive);
		_interactiveExamples = ARGS_BOOL(interactive_examples);
		_interactiveEntropies = ARGS_BOOL(interactive_entropies);
		_guiLengthMaximum = ARGS_INT(length_maximum);
		_guiFrameRed = ARGS_BOOL(red_frame);
		_guiUnderlying = ARGS_BOOL(highlight_underlying);
		_updateDisable = ARGS_BOOL(disable_update);
		_activeLogging = ARGS_BOOL(logging_active);
		_level1Logging = ARGS_BOOL(logging_level1);
		_activeSummary = ARGS_BOOL(summary_active);
		_level1Summary = ARGS_BOOL(summary_level1);
		_activeSize = ARGS_INT_DEF(activeSize,1000000);
		_level1ActiveSize = ARGS_INT_DEF(level1_activeSize,10);
		_updateParameters.mapCapacity = ARGS_INT_DEF(updateParameters.mapCapacity,3); 
		_induceThreshold = ARGS_INT_DEF(induceThreshold,200);
		_induceInterval = (std::chrono::milliseconds)(ARGS_INT_DEF(induceInterval,_interval.count()));	
		_induceThreadCount = ARGS_INT_DEF(induceThreadCount,4);
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
			_induceParameters.induceThresholds = std::set<std::size_t>{200,225,250,300,400,500,800,1000,2000,3000};
		}	
		_fudsSize = 0;
		_inputFilename = ARGS_STRING(input_file);
		_captureX = ARGS_INT_DEF(x,791);	
		_captureY = ARGS_INT_DEF(y,244);	
		_captureWidth = ARGS_INT_DEF(width,728);	
		_captureHeight = ARGS_INT_DEF(height,410);	
		_centreX = ARGS_DOUBLE_DEF(centreX,0.5);
		_centreY = ARGS_DOUBLE_DEF(centreY,0.5);
        _centreRandomX = ARGS_DOUBLE(random_centreX);
		_centreRandomY = ARGS_DOUBLE(random_centreY);
		_centreRangeX = ARGS_DOUBLE(range_centreX);
		_centreRangeY = ARGS_DOUBLE(range_centreY);
		_scale = ARGS_DOUBLE_DEF(scale,0.5);
		_scaleValency = ARGS_INT_DEF(scale_valency,4);	
		_scaleValue = ARGS_INT_DEF(scale_value,0);	
		_valency = ARGS_INT_DEF(valency,10);	
		_valencyFactor = ARGS_INT(valency_factor);	
		_valencyFixed = ARGS_BOOL(valency_fixed);	
		_valencyBalanced = ARGS_BOOL(valency_balanced);	
		_valencyFixed |= _valencyBalanced;
		_size = ARGS_INT_DEF(size,40);	
		_level1Size = ARGS_INT_DEF(level1_size,8);	
		_level2Size = ARGS_INT_DEF(level2_size,5);	
		_sizeTile = ARGS_INT_DEF(tile_size,_size/2);	
		_divisor = ARGS_INT_DEF(divisor,4);	
		_multiplier = ARGS_INT_DEF(multiplier,2);	
		_eventSize = ARGS_INT_DEF(event_size,1);	
		_scanSize = ARGS_INT_DEF(scan_size,1);	
		_threadCount = ARGS_INT_DEF(threads,1);	
		_separation = ARGS_DOUBLE_DEF(separation,0.5);
		_entropyMinimum = ARGS_DOUBLE(entropy_minimum);
		_substrateInclude = ARGS_BOOL(include_substrate);
	}
	// input image
	if (_inputFilename.size())
	{
		QImage imageIn;		
		if (imageIn.load(QString(_inputFilename.c_str())))
		{
			_inputImage = imageIn.convertToFormat(QImage::Format_RGB32);
		}
		else
		{
			LOG "actor\terror: failed to load input image " << _inputFilename UNLOG
			return;	
		}
	}
	// add dynamic GUI
	if (_interactive)
	{
		_pixmapBlank = QPixmap(_size*_multiplier, _size*_multiplier);
		_pixmapBlank.fill(this->palette().color(QWidget::backgroundRole()));
		_ui->layout01->setAlignment(Qt::AlignLeft);
		_ui->layout02->setAlignment(Qt::AlignLeft);
		_ui->layout03->setAlignment(Qt::AlignLeft);
		_ui->layout05->setAlignment(Qt::AlignLeft);
		for (std::size_t k = 0; k < 3; k++)
		{
			QVBoxLayout* verticalLayout = new QVBoxLayout();
			_ui->layout01->addLayout(verticalLayout);
			QLabel* label1 = new QLabel(this);
			label1->setPixmap(_pixmapBlank);	
			label1->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			_labelRecords.push_back(label1);
			verticalLayout->addWidget(label1);
			QLabel* label2 = new QLabel(this);
			label2->setText("");	
			label2->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			verticalLayout->addWidget(label2);
			if (!k)
				_labelRecordLikelihood = label2;
			else if (k == 1)
				_labelRecordEntropy1 = label2;
			else if (k == 2)
				_labelRecordEntropy2 = label2;
		}
		for (std::size_t k = 0; k < _labelSize; k++)
		{
			QVBoxLayout* verticalLayout = new QVBoxLayout();
			_ui->layout02->addLayout(verticalLayout);
			QLabel* label1 = new QLabel(this);
			label1->setPixmap(_pixmapBlank);	
			label1->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			QFont font = label1->font();
			font.setPointSize(_multiplier == 1 ? 20 : 40);
			font.setBold(true);
			label1->setFont(font);
			_labelRecordSiblings.push_back(label1);
			verticalLayout->addWidget(label1);
			QLabel* label2 = new QLabel(this);
			label2->setText("");				
			label2->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			verticalLayout->addWidget(label2);
			_labelRecordSiblingLikelihoods.push_back(label2);
		}
		for (std::size_t k = 0; k < _labelSize; k++)
		{
			QVBoxLayout* verticalLayout = new QVBoxLayout();
			_ui->layout03->addLayout(verticalLayout);
			QLabel* label1 = new QLabel(this);
			label1->setPixmap(_pixmapBlank);	
			label1->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			QFont font = label1->font();
			font.setPointSize(_multiplier == 1 ? 20 : 40);
			font.setBold(true);
			label1->setFont(font);			
			_labelRecordAncestors.push_back(label1);
			verticalLayout->addWidget(label1);
			QLabel* label2 = new QLabel(this);
			label2->setText("");				
			label2->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			verticalLayout->addWidget(label2);
			_labelRecordAncestorLikelihoods.push_back(label2);
		}
		if (_interactiveExamples)		
			for (std::size_t k = 0; k < _labelSize; k++)
			{
				QVBoxLayout* verticalLayout = new QVBoxLayout();
				_ui->layout05->addLayout(verticalLayout);				
				QLabel* label1 = new QLabel(this);
				label1->setPixmap(_pixmapBlank);	
				label1->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);		
				_labelRecordExamples.push_back(label1);
				verticalLayout->addWidget(label1);
				QLabel* label2 = new QLabel(this);
				label2->setText("");				
				label2->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
				verticalLayout->addWidget(label2);				
				_labelRecordExampleEntropies.push_back(label2);
			}			
		{
			_labelCentre = new QLabel(this); 
			_ui->layout04->addWidget(_labelCentre);
		}		
	}
	{
		if (_mode.size())
		{
			_labelEvent = new QLabel(this); 
			_ui->layout04->addWidget(_labelEvent);
			_labelFuds = new QLabel(this); 
			_ui->layout04->addWidget(_labelFuds);
		}	
        if (_lagThreshold)
		{
			_labelLag = new QLabel(this); 
			_ui->layout04->addWidget(_labelLag);
		}			
		if (_lagThreshold)
		{
			_labelFails = new QLabel(this); 
			_ui->layout04->addWidget(_labelFails);
		}		
		if (_motionThreshold)
		{
			_labelMotion = new QLabel(this); 
			_ui->layout04->addWidget(_labelMotion);
		}
	}
	// load slice representations if modelInitial 
	if (_modelInitial.size())
	{
		try
		{
            std::ifstream in(_modelInitial + ".rep", std::ios::binary);
			if (in.is_open())
			{
                _slicesRepresentation = persistentsSliceRepresentationUMap(in);
				in.close();
			}
			else
			{
                LOG "actor\terror: failed to open slice-representations file" << _modelInitial + ".rep" UNLOG
				return;
			}
		}
		catch (const std::exception&)
		{
            LOG "actor\terror: failed to read records file" << _modelInitial + ".rep" UNLOG
         return;
		}		
	}
	else
	{
		_slicesRepresentation = std::make_unique<SliceRepresentationUMap>();
	}
	// create active
	{
		{
            SystemSystemRepaTuple xx(recordsSystemSystemRepaTuple(_scaleValency, _valency, _size*_size));
            _uu = std::move(std::get<0>(xx));
            _ur = std::move(std::get<1>(xx));
		}
		_system = std::make_shared<ActiveSystem>();
		_threads.reserve(1);
		_events = std::make_shared<ActiveEventsRepa>(1);
		_active = std::make_shared<Active>();
		{
			if (_struct=="struct002" || _struct=="struct005")
			{
				Active activeB;
				{
					ActiveIOParameters ppio;
					ppio.filename = _level1Model + ".ac";
					activeB.logging = true;
					if (!activeB.load(ppio))
					{
						LOG "actor\terror: failed to load level1 model" << ppio.filename UNLOG
						_system.reset();
						return;
					}
					_system->block = std::max(_system->block, activeB.varMax() >> activeB.bits);
					_level1Decomp = activeB.decomp;					
				}
				for (std::size_t m = 0; m < _level2Size*_level2Size; m++)
				{
					_level1Events.push_back(std::make_shared<ActiveEventsRepa>(1));
					_level1.push_back(std::make_shared<Active>());
					auto& activeA = *_level1.back();
					activeA.log = actor_log;
					activeA.layerer_log = layerer_actor_log;
					activeA.system = _system;
					activeA.decomp = _level1Decomp;
					activeA.induceVarComputeds = activeB.induceVarComputeds;
					activeA.underlyingSlicesParent = activeB.underlyingSlicesParent;
					activeA.historySize = _level1ActiveSize;
					{
						auto hr = sizesHistoryRepa(_scaleValency, _valency, _level1Size*_level1Size, activeA.historySize);
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
					activeA.name = (_model!="" ? _model : "model") + "_1_" + (m<10 ? "0" : "") + std::to_string(m);			
					activeA.logging = _level1Logging;
					activeA.summary = _level1Summary;
					activeA.underlyingEventsRepa.push_back(_level1Events.back());
					activeA.eventsSparse = std::make_shared<ActiveEventsArray>(1);
				}
				{
					LOG _level1Model << "\tactive cardinality: " << _level1.size() << "\tfuds cardinality: " << _level1Decomp->fuds.size() << "\tmodel cardinality: " << _level1Decomp->fudRepasSize UNLOG
				}
			}
			auto& activeA = *_active;
			activeA.log = actor_log;
			activeA.layerer_log = layerer_actor_log;
			activeA.system = _system;
			activeA.continousIs = true;
			activeA.historySliceCachingIs = true;
			activeA.historySliceCumulativeIs = true;
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
				this->eventId++;
				_fudsSize = activeA.decomp->fuds.size();
			}
			else
			{
				if (_struct=="struct004")
				{
					_system->block = (_size * _size << 12 >> activeA.bits) + 1;				
				}
				activeA.var = activeA.system->next(activeA.bits);
				activeA.varSlice = activeA.system->next(activeA.bits);
				activeA.historySize = _activeSize;
				activeA.induceThreshold = _induceThreshold;
				activeA.decomp = std::make_unique<DecompFudSlicedRepa>();	
				{
                    auto hr = sizesHistoryRepa(_scaleValency, _valency, _size*_size, activeA.historySize);
					activeA.underlyingHistoryRepa.push_back(std::move(hr));
				}	
				if (_struct=="struct002" || _struct=="struct005")
				{
					for (std::size_t m = 0; m < _level1.size(); m++)
					{
						activeA.underlyingHistorySparse.push_back(std::make_shared<HistorySparseArray>(activeA.historySize,1));
					}
				}
				else if (_struct=="struct004")
				{
					for (std::size_t m = 0; m < _size*_size; m++)
					{
						activeA.induceVarComputeds.insert(m);
					}
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
			activeA.historySliceCachingIs = true;
			activeA.name = (_model!="" ? _model : "model");			
			activeA.logging = _activeLogging;
			activeA.summary = _activeSummary;
			activeA.underlyingEventsRepa.push_back(_events);
			if (_struct=="struct002" || _struct=="struct005")
			{
				if (!_substrateInclude)
				{
					std::shared_ptr<HistoryRepa> hr = activeA.underlyingHistoryRepa.front();
					auto n = hr->dimension - 1;
					auto vv = hr->vectorVar;
					for (std::size_t i = 0; i < n; i++)
						activeA.induceVarExclusions.insert(vv[i]);
				}
				activeA.underlyingOffsetIs = true;
				for (std::size_t m = 0; m < _level1.size(); m++)
				{
					auto& activeB = *_level1[m];
					activeA.underlyingEventsSparse.push_back(activeB.eventsSparse);
				}
			}
			activeA.eventsSparse = std::make_shared<ActiveEventsArray>(0);
			if (_modelInitial.size() && _struct!="struct002" && _struct!="struct005"  && !_updateDisable)
			{
				if (!activeA.induce(_induceParameters))
				{
					LOG "actor\tstatus: failed to run initial induce" UNLOG
					_system.reset();
					return;
				}
			}
			std::size_t sizeA = activeA.historyOverflow ? activeA.historySize : activeA.historyEvent;
			if (sizeA)
			{
				LOG activeA.name << "\tfuds cardinality: " << activeA.decomp->fuds.size() << "\tmodel cardinality: " << activeA.decomp->fudRepasSize << "\tactive size: " << sizeA << "\tfuds per threshold: " << (double)activeA.decomp->fuds.size() * activeA.induceThreshold / sizeA UNLOG				
			}
			if (_mode.size() && _struct!="struct002" && _struct!="struct005" && !_updateDisable)
				_threads.push_back(std::thread(run_induce, std::ref(*this), std::ref(activeA), std::ref(_induceParameters), _induceInterval));
		}
	}
	// start act timer
	if (_system)
	{
		this->terminate = false;		
		_screen = QGuiApplication::primaryScreen();
		QTimer::singleShot(_interval.count(), this, &Win007::act);
		LOG "actor\tstatus: started" UNLOG
	}
	else
	{
		LOG "actor\terror: failed to initialise" UNLOG
	}
}

Win007::~Win007()
{
	terminate = true;
	if (_system)
	{
		_active->terminate = true;
		for (auto& t : _threads)
			t.join();
		if ( _model!="")
		{
			auto& activeA = *_active;
			std::shared_ptr<HistoryRepa> hr = activeA.underlyingHistoryRepa.front();
			auto& slev = activeA.historySlicesSetEvent;
			auto& dr = *activeA.decomp;		
			auto n = hr->dimension;
			auto rr = hr->arr;	
			auto& reps = *_slicesRepresentation;
			// check for new leaf slices and update representation map
            if (_mode.size() && _fudsSize < dr.fuds.size())
			{
				for (std::size_t i = _fudsSize; i < dr.fuds.size(); i++)
				{
					for (auto sliceB : dr.fuds[i].children)
					{
						Representation rep(1.0,1.0,_size,_size);
						auto& arr1 = *rep.arr;
						if (slev.count(sliceB))
						{
							for (auto j : slev[sliceB])
							{
								auto jn = j*n;
								for (size_t i = 0; i < n-1; i++)
									arr1[i] += rr[jn + i];
								rep.count++;
							}									
							reps.insert_or_assign(sliceB, rep);							
						}
					}
				}
				_fudsSize = dr.fuds.size();
			}
			ActiveIOParameters ppio;
			ppio.filename = activeA.name+".ac";
			activeA.logging = true;
			activeA.dump(ppio);		
			// dump slice representations
			try
			{
				std::ofstream out(_model + ".rep", std::ios::binary);
				sliceRepresentationUMapsPersistent(*_slicesRepresentation, out); 
				out.close();
				LOG "actor\tdump\tfile name:" << _model + ".rep" UNLOG
			}
			catch (const std::exception&)
			{
				LOG "actor\terror: failed to write slice-representations file" <<_model + ".rep" UNLOG
			}	
		}			
	}
    delete _ui;
	LOG "actor\tstatus: finished" UNLOG
}

void Win007::act()
{
	if (this->terminate || (_active && _active->terminate))
		return;
	auto actMark = Clock::now();	
	// capture
	_mark = Clock::now();
	QImage image;
	if (_inputFilename.size())
	{
		image = _inputImage;
		_captureWidth = image.width();
		_captureHeight = image.height();
		_ui->labelImage->setPixmap(QPixmap::fromImage(image));			
	}
	else
	{
		auto pixmap = _screen->grabWindow(0, _captureX, _captureY, _captureWidth, _captureHeight);
		image = pixmap.toImage();
		_ui->labelImage->setPixmap(QPixmap::fromImage(image));	
		if (_motionThreshold)
		{
			std::size_t hash = 0;
			for (int y = 0; y < _captureHeight; y += _motionHashStep) 
			{
				for (int x = 0; x < _captureWidth; x += _motionHashStep) 
				{
					auto rgb = image.pixel(x,y);
					hash = hash * 13 + (hash >> 60) + qRed(rgb);
					hash = hash * 13 + (hash >> 60) + qGreen(rgb);
					hash = hash * 13 + (hash >> 60) + qBlue(rgb);
				}
			}
			if (hash == _motionHash)
			{
				_motionCount++;
				if (!_motionWaiting)
				{
					if (_motionCount >= _motionThreshold)
					{
						this->eventId ++;	
						_motionWaiting = true;
					}					
				}
			}
			else
			{
				_motionWaiting = false;				
				_motionCount = 0;
			}
			_motionHash = hash;		
			{
				std::stringstream string;
				string << "still: " << std::fixed << _motionCount;
				_labelMotion->setText(string.str().data());
			}			
		}
	}
	if (_actLogging && (_actLoggingFactor <= 1 || _actCount % _actLoggingFactor == 0))	
	{
        std::stringstream string;
        string << "actor\tcaptured\t" << std::fixed << std::setprecision(6) << ((Sec)(Clock::now() - _mark)).count() << std::defaultfloat << "s";
		LOG string.str() UNLOG
	}
	// update
	_mark = Clock::now(); 
	if (_system)
	{
		// update events
		std::size_t eventCount = 0;
		std::size_t lag = 0;
		std::size_t failCount = 0;
		if (!_motionWaiting && !_lagWaiting && (!_eventIdMax || this->eventId < _eventIdMax))
		{
			if (_mode == "mode001")
			{
				for (std::size_t k = 0; k < _eventSize; k++)	
				{
					auto centreRandomX = _centreRandomX > 0.0 ? ((double) rand() / (RAND_MAX)) *_centreRandomX * 2.0 - _centreRandomX : 0.0;
					auto centreRandomY = _centreRandomY > 0.0 ? ((double) rand() / (RAND_MAX)) *_centreRandomY * 2.0 - _centreRandomY : 0.0;
					Record record(image, 
						_scale * _captureHeight / _captureWidth, _scale,
						_centreX + (centreRandomX * _captureHeight / _captureWidth), 
						_centreY + centreRandomY, 
						_size, _size, _divisor, _divisor);
					Record recordValent = _valencyFixed ? record.valentFixed(_valency,_valencyBalanced) : record.valent(_valency,_valencyFactor);
					if (_entropyMinimum > 0.0 && recordValent.entropy() < _entropyMinimum)
						continue;
					auto hr = recordsHistoryRepa(_scaleValency, 0, _valency, recordValent);
					if (!_updateDisable)
						_events->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr),_events->references);	
					this->eventId++;		
					eventCount++;		
				}
			}
			else if (_mode == "mode002")
			{
				std::vector<Record> records;
				for (std::size_t k = 0; k < _scanSize; k++)	
				{
					auto centreRandomX = _centreRandomX > 0.0 ? ((double) rand() / (RAND_MAX)) *_centreRandomX * 2.0 - _centreRandomX : 0.0;
					auto centreRandomY = _centreRandomY > 0.0 ? ((double) rand() / (RAND_MAX)) *_centreRandomY * 2.0 - _centreRandomY : 0.0;
					Record record(image, 
						_scale * _captureHeight / _captureWidth, _scale,
						_centreX + (centreRandomX * _captureHeight / _captureWidth), 
						_centreY + centreRandomY, 
						_size, _size, _divisor, _divisor);
					records.push_back(_valencyFixed ? record.valentFixed(_valency,_valencyBalanced) : record.valent(_valency,_valencyFactor));	
				}
				std::vector<std::pair<double,std::size_t>> likelihoodsRecord;		
				{		
					auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
					auto cap = (unsigned char)(_updateParameters.mapCapacity);
					double lnwmax = std::log(_induceParameters.wmax);
					auto& activeA = *_active;
					std::lock_guard<std::mutex> guard(activeA.mutex);
					auto& sizes = activeA.historySlicesSize;
					auto& dr = *activeA.decomp;		
					auto& cv = dr.mapVarParent();
					for (std::size_t k = 0; k < _scanSize; k++)	
					{
						auto hr = recordsHistoryRepa(_scaleValency, 0, _valency, records[k]);
						auto n = hr->dimension;
						auto vv = hr->vectorVar;
						auto rr = hr->arr;	
						SizeUCharStructList jj;
						jj.reserve(n);
						for (std::size_t i = 0; i < n; i++)
						{
							SizeUCharStruct qq;
							qq.uchar = rr[i];	
							qq.size = vv[i];
							jj.push_back(qq);
						}
						auto ll = drmul(jj,dr,cap);	
						if (ll && ll->size())
						{
							std::size_t slice = ll->back();	
							if (slice && cv.count(slice) && sizes.count(slice))
							{
								// EVAL(slice);	
								double likelihood = (std::log(sizes[slice]) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax;
								likelihoodsRecord.push_back(std::make_pair(likelihood, k));
							}
						}
					}
				}		
				if (likelihoodsRecord.size())
					std::sort(likelihoodsRecord.rbegin(), likelihoodsRecord.rend());	
				for (std::size_t k = 0; k < _eventSize && k < _scanSize; k++)	
				{
					std::size_t m =  likelihoodsRecord.size() > k ? likelihoodsRecord[k].second : k;
					auto& record = records[m];
					if (_entropyMinimum > 0.0 && record.entropy() < _entropyMinimum)
						continue;	
					auto hr = recordsHistoryRepa(_scaleValency, 0, _valency, record);
					if (!_updateDisable)
						_events->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr),_events->references);	
					this->eventId++;		
					eventCount++;		
				}
			}
			else if (_mode == "mode003")
			{
				std::vector<Record> records;
				for (std::size_t k = 0; k < _scanSize; k++)	
				{
					auto centreRandomX = _centreRandomX > 0.0 ? ((double) rand() / (RAND_MAX)) *_centreRandomX * 2.0 - _centreRandomX : 0.0;
					auto centreRandomY = _centreRandomY > 0.0 ? ((double) rand() / (RAND_MAX)) *_centreRandomY * 2.0 - _centreRandomY : 0.0;
					Record record(image, 
						_scale * _captureHeight / _captureWidth, _scale,
						_centreX + (centreRandomX * _captureHeight / _captureWidth), 
						_centreY + centreRandomY, 
						_size, _size, _divisor, _divisor);
					records.push_back(_valencyFixed ? record.valentFixed(_valency,_valencyBalanced) : record.valent(_valency,_valencyFactor));	
				}
				std::vector<std::pair<std::pair<std::size_t,double>,std::size_t>> actsPotsRecord;
				{		
					auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
					auto cap = (unsigned char)(_updateParameters.mapCapacity);
					double lnwmax = std::log(_induceParameters.wmax);
					auto& activeA = *_active;
					std::lock_guard<std::mutex> guard(activeA.mutex);
					auto& sizes = activeA.historySlicesSize;
					auto& lengths = activeA.historySlicesLength;
					auto& fails = activeA.induceSliceFailsSize;
					auto& dr = *activeA.decomp;		
					auto& cv = dr.mapVarParent();
					for (std::size_t k = 0; k < _scanSize; k++)	
					{
						auto hr = recordsHistoryRepa(_scaleValency, 0, _valency, records[k]);
						auto n = hr->dimension;
						auto vv = hr->vectorVar;
						auto rr = hr->arr;	
						SizeUCharStructList jj;
						jj.reserve(n);
						for (std::size_t i = 0; i < n; i++)
						{
							SizeUCharStruct qq;
							qq.uchar = rr[i];	
							qq.size = vv[i];
							jj.push_back(qq);
						}
						auto ll = drmul(jj,dr,cap);	
						if (ll && ll->size())
						{
							std::size_t slice = ll->back();	
							if (slice && cv.count(slice) && sizes.count(slice) 
								&& lengths.count(slice) && !fails.count(slice))
							{
								double likelihood = (std::log(sizes[slice]) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax;
								std::size_t length = lengths[slice];
								actsPotsRecord.push_back(std::make_pair(std::make_pair(length,likelihood), k));
							}
						}
					}
				}		
				if (actsPotsRecord.size())
					std::sort(actsPotsRecord.rbegin(), actsPotsRecord.rend());	
				for (std::size_t k = 0; k < _eventSize && k < _scanSize; k++)	
				{
					std::size_t m =  actsPotsRecord.size() > k ? actsPotsRecord[k].second : k;
					auto& record = records[m];
					if (_entropyMinimum > 0.0 && record.entropy() < _entropyMinimum)
						continue;	
					auto hr = recordsHistoryRepa(_scaleValency, 0, _valency, record);
					if (!_updateDisable)
						_events->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr),_events->references);	
					this->eventId++;		
					eventCount++;		
				}
			}
			else if (_mode == "mode004" || _mode == "mode007")
			{
				bool isSizePotential = _mode == "mode007";
                auto scaleX = _centreRangeX * 2.0 + _scale;
                auto scaleY = _centreRangeY * 2.0 + _scale;
				auto sizeX = (std::size_t)(scaleX * _size / _scale);
				if (sizeX % 2 != _size % 2) sizeX++;
				auto sizeY = (std::size_t)(scaleY * _size / _scale);	
				if (sizeY % 2 != _size % 2) sizeY++;
				double interval = _scale/_size;		
				scaleX = sizeX * interval;
				scaleY = sizeY * interval;
				auto centreX = _centreX;
				centreX = std::max(centreX, scaleX * _captureHeight / _captureWidth / 2.0);
				centreX = std::min(centreX, 1.0 - scaleX * _captureHeight / _captureWidth / 2.0);
				auto centreY = _centreY;
				centreY = std::max(centreY, scaleY / 2.0);
				centreY = std::min(centreY, 1.0 - scaleY / 2.0);
				Record record(image, 
					scaleX * _captureHeight / _captureWidth, scaleY,
					centreX, centreY, 
					sizeX, sizeY, 
					_divisor, _divisor);	
				std::vector<std::tuple<std::size_t,double,double,double,std::size_t,std::size_t>> actsPotsCoord(sizeY*sizeX);
				{
					auto& activeA = *_active;
					auto& actor = *this;
					std::lock_guard<std::mutex> guard(activeA.mutex);
					std::vector<std::thread> threads;
					threads.reserve(_threadCount);
					for (std::size_t t = 0; t < _threadCount; t++)
						threads.push_back(std::thread(
							[isSizePotential, &actor, &activeA,
							centreX, centreY, scaleX, scaleY, sizeX, sizeY, interval, &record,
							&actsPotsCoord] (int t)
							{
								auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
								auto& sizes = activeA.historySlicesSize;
								auto& lengths = activeA.historySlicesLength;
								auto& fails = activeA.induceSliceFailsSize;
								auto& dr = *activeA.decomp;		
								auto& cv = dr.mapVarParent();
								auto cap = (unsigned char)(actor._updateParameters.mapCapacity);
								double lnwmax = std::log(actor._induceParameters.wmax);
								auto heightWidth = (double)actor._captureHeight / (double)actor._captureWidth;
								auto offsetX = (scaleX - actor._scale) / 2.0;
								auto offsetY = (scaleY - actor._scale) / 2.0;
								auto size = actor._size;
								auto valency = actor._valency;
								auto valencyFactor = actor._valencyFactor;
								auto valencyFixed = actor._valencyFixed;
								auto valencyBalanced = actor._valencyBalanced;
								auto sizeX1 = sizeX - size;
								auto sizeY1 = sizeY - size;
								auto hr = sizesHistoryRepa(actor._scaleValency, valency, size*size);
								auto n = hr->dimension;
								auto vv = hr->vectorVar;
								auto rr = hr->arr;
								rr[n-1] = 0;
								for (std::size_t y = 0, z = 0; y < sizeY1; y++)	
									for (std::size_t x = 0; x < sizeX1; x++, z++)	
										if (z % actor._threadCount == t)
										{
											Record recordSub(record,size,size,x,y);
											Record recordValent = valencyFixed ? recordSub.valentFixed(valency,valencyBalanced) : recordSub.valent(valency,valencyFactor);
											auto& arr1 = *recordValent.arr;	
											SizeUCharStructList jj;
											jj.reserve(n);
											for (std::size_t i = 0; i < n-1; i++)
											{
												SizeUCharStruct qq;
												qq.uchar = arr1[i];	
												qq.size = vv[i];
												if (qq.uchar)
													jj.push_back(qq);
											}
											{
												SizeUCharStruct qq;
												qq.uchar = rr[n-1];	
												qq.size = vv[n-1];
												if (qq.uchar)
													jj.push_back(qq);
											}
											auto ll = drmul(jj,dr,cap);	
											std::size_t slice = 0;
											auto posX = centreX + (interval * x - offsetX) * heightWidth;
											auto posY = centreY + interval * y - offsetY;
											if (ll && ll->size()) slice = ll->back();	
											if (slice && cv.count(slice) && sizes.count(slice) 
												&& lengths.count(slice) && !fails.count(slice))
											{
												auto length = lengths[slice];
												auto sz = sizes[slice];
												auto likelihood = (std::log(sz) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax;
												if (isSizePotential) likelihood += sz;
												actsPotsCoord[z] = std::make_tuple(length,likelihood,posX,posY,x,y);
											}
											else
												actsPotsCoord[z] = std::make_tuple(0,-INFINITY,posX,posY,x,y);	
										}
							}, t));
					for (auto& t : threads)
						t.join();
				}
                std::sort(actsPotsCoord.rbegin(), actsPotsCoord.rend());
				std::vector<std::tuple<std::size_t,double,double,double,std::size_t,std::size_t>> actsPotsCoordTop;
				actsPotsCoordTop.reserve(_eventSize);
				for (std::size_t k = 0; k < actsPotsCoord.size() && actsPotsCoordTop.size() < _eventSize; k++)	
				{
					auto t = actsPotsCoord[k];
					auto posX = std::get<2>(t);
					auto posY = std::get<3>(t);	
					auto x = std::get<4>(t);
					auto y = std::get<5>(t);
					if (_entropyMinimum > 0.0)
					{
						Record recordSub(record,_size,_size,x,y);
						Record recordValent = _valencyFixed ? recordSub.valentFixed(_valency,_valencyBalanced) : recordSub.valent(_valency,_valencyFactor);
						if (_entropyMinimum > 0.0 && recordValent.entropy() < _entropyMinimum)
							continue;
					}					
					double d2 = _scale * _separation * _scale * _separation;
					bool separate = true;
					for (auto t1 : actsPotsCoordTop)
					{
						auto posX1 = std::get<2>(t1);
						auto posY1 = std::get<3>(t1);	
						auto d12 = (posX1 - posX) * (posX1 - posX) + (posY1 - posY) * (posY1 - posY);
						if (d12 < d2)
						{
							separate = false;
							break;
						}
					}
					if (separate)
						actsPotsCoordTop.push_back(t);
				}
				QImage image2 = image.copy();
				QPainter framePainter(&image2);
				framePainter.setPen(Qt::darkGray);
				framePainter.drawRect(
					centreX * _captureWidth - scaleX * _captureHeight / 2.0, 
					centreY * _captureHeight - scaleY * _captureHeight / 2.0, 
					scaleX * _captureHeight,
					scaleY * _captureHeight);
				bool centered = false;
				for (auto t : actsPotsCoordTop)
				{
					auto posX = std::get<2>(t);
					auto posY = std::get<3>(t);	
					auto x = std::get<4>(t);
					auto y = std::get<5>(t);
					Record recordSub(record,_size,_size,x,y);
					Record recordValent = _valencyFixed ? recordSub.valentFixed(_valency,_valencyBalanced) : recordSub.valent(_valency,_valencyFactor);
					if (!centered)
					{
						_centreX = posX;
						_centreY = posY;	
						centered = true;
						framePainter.setPen(_guiFrameRed ? Qt::red : Qt::white);		
					}
					else
						framePainter.setPen(_guiFrameRed ? Qt::magenta : Qt::gray);
					framePainter.drawRect(
						posX * _captureWidth - _scale * _captureHeight / 2.0, 
						posY * _captureHeight - _scale * _captureHeight / 2.0, 
						_scale * _captureHeight,
						_scale * _captureHeight);
					auto hr = recordsHistoryRepa(_scaleValency, 0, _valency, recordValent);
					if (!_updateDisable)
						_events->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr),_events->references);	
					this->eventId++;		
					eventCount++;		
				}
				_ui->labelImage->setPixmap(QPixmap::fromImage(image2));	
			}
			else if (_mode == "mode005" || _mode == "mode006")
			{
				bool isSizePotential = _mode == "mode006";
                auto scaleX = _centreRangeX * 2.0 + _scale;
                auto scaleY = _centreRangeY * 2.0 + _scale;
				auto sizeX = (std::size_t)(scaleX * _size / _scale);
				sizeX = (sizeX - _size + (_sizeTile/2)) / _sizeTile * _sizeTile + _size;
				auto sizeY = (std::size_t)(scaleY * _size / _scale);	
				sizeY = (sizeY - _size + (_sizeTile/2)) / _sizeTile * _sizeTile + _size;
				double interval = _scale/_size;		
				scaleX = sizeX * interval;
				scaleY = sizeY * interval;
				auto centreX = _centreX;
				centreX = std::max(centreX, scaleX * _captureHeight / _captureWidth / 2.0);
				centreX = std::min(centreX, 1.0 - scaleX * _captureHeight / _captureWidth / 2.0);
				auto centreY = _centreY;
				centreY = std::max(centreY, scaleY / 2.0);
				centreY = std::min(centreY, 1.0 - scaleY / 2.0);
				Record record(image, 
					scaleX * _captureHeight / _captureWidth, scaleY,
					centreX, centreY, 
					sizeX, sizeY, 
					_divisor, _divisor);
				auto hr = sizesHistoryRepa(_scaleValency, _valency, _size*_size);
				auto n = hr->dimension;
				auto vv = hr->vectorVar;
				std::vector<std::tuple<double,std::size_t,double,double,std::size_t,std::size_t>> actsPotsCoordTop;
				for (std::size_t ty = 0; ty < (sizeY-_size)/_sizeTile; ty++)	
					for (std::size_t tx = 0; tx < (sizeX-_size)/_sizeTile; tx++)
					{
						auto& activeA = *_active;
						auto& actor = *this;
						std::vector<std::tuple<std::size_t,double,double,double,std::size_t,std::size_t>> actsPotsCoord(_sizeTile*_sizeTile);
						std::lock_guard<std::mutex> guard(activeA.mutex);
						std::vector<std::thread> threads;
						threads.reserve(_threadCount);
						for (std::size_t t = 0; t < _threadCount; t++)
							threads.push_back(std::thread(
								[isSizePotential, &actor, &activeA, n, vv, 
								centreX, centreY, scaleX, scaleY, sizeX, sizeY, interval, &record, tx, ty,
								&actsPotsCoord] (int t)
								{
									auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
									auto& sizes = activeA.historySlicesSize;
									auto& lengths = activeA.historySlicesLength;
									auto& fails = activeA.induceSliceFailsSize;
									auto& dr = *activeA.decomp;		
									auto& cv = dr.mapVarParent();
									auto cap = (unsigned char)(actor._updateParameters.mapCapacity);
									double lnwmax = std::log(actor._induceParameters.wmax);
									auto heightWidth = (double)actor._captureHeight / (double)actor._captureWidth;
									auto offsetX = (scaleX - actor._scale) / 2.0;
									auto offsetY = (scaleY - actor._scale) / 2.0;
									auto size = actor._size;
									auto sizeTile = actor._sizeTile;
									auto valency = actor._valency;
									auto valencyFactor = actor._valencyFactor;
									auto valencyFixed = actor._valencyFixed;
									auto valencyBalanced = actor._valencyBalanced;
									for (std::size_t y = ty*sizeTile, z = 0; y < (ty+1)*sizeTile; y++)	
										for (std::size_t x = tx*sizeTile; x < (tx+1)*sizeTile; x++, z++)
											if (z % actor._threadCount == t)
											{
												Record recordSub(record,size,size,x,y);
												Record recordValent = valencyFixed ? recordSub.valentFixed(valency,valencyBalanced) : recordSub.valent(valency,valencyFactor);
												auto& arr1 = *recordValent.arr;	
												SizeUCharStructList jj;
												jj.reserve(n);
												for (std::size_t i = 0; i < n-1; i++)
												{
													SizeUCharStruct qq;
													qq.uchar = arr1[i];	
													qq.size = vv[i];
													if (qq.uchar)
														jj.push_back(qq);
												}
												auto ll = drmul(jj,dr,cap);	
												std::size_t slice = 0;
												auto posX = centreX + (interval * x - offsetX) * heightWidth;
												auto posY = centreY + interval * y - offsetY;
												if (ll && ll->size()) slice = ll->back();	
												if (slice && cv.count(slice) && sizes.count(slice) 
													&& lengths.count(slice) && !fails.count(slice))
												{
													auto length = lengths[slice];
													auto sz = sizes[slice];
													auto likelihood = (std::log(sz) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax;
													if (isSizePotential) likelihood += sz;
													actsPotsCoord[z] = std::make_tuple(length,likelihood,posX,posY,x,y);
												}
												else
													actsPotsCoord[z] = std::make_tuple(0,-INFINITY,posX,posY,x,y);	
											}
								}, t));
						for (auto& t : threads)
							t.join();
						if (actsPotsCoord.size())
						{
							std::sort(actsPotsCoord.begin(), actsPotsCoord.end());
							auto t = actsPotsCoord.back();
							auto length = std::get<0>(t);
							auto likelihood = std::get<1>(t);
							auto posX = std::get<2>(t);
							auto posY = std::get<3>(t);								
							auto x = std::get<4>(t);								
							auto y = std::get<5>(t);								
							actsPotsCoordTop.push_back(std::make_tuple(likelihood,length,posX,posY,x,y));
						}
					}
                std::sort(actsPotsCoordTop.rbegin(), actsPotsCoordTop.rend());
				QImage image2 = image.copy();
				QPainter framePainter(&image2);
				framePainter.setPen(Qt::darkGray);
				framePainter.drawRect(
					centreX * _captureWidth - scaleX * _captureHeight / 2.0, 
					centreY * _captureHeight - scaleY * _captureHeight / 2.0, 
					scaleX * _captureHeight,
					scaleY * _captureHeight);
				bool centered = false;
				for (std::size_t k = 0; k < actsPotsCoordTop.size() && eventCount < _eventSize; k++)	
				{
					// EVAL(k);
					auto t = actsPotsCoordTop[k];
					// EVAL(std::get<0>(t));
					// EVAL(std::get<1>(t));
					auto posX = std::get<2>(t);
					auto posY = std::get<3>(t);		
					auto x = std::get<4>(t);
					auto y = std::get<5>(t);
					Record recordSub(record,_size,_size,x,y);
					Record recordValent = _valencyFixed ? recordSub.valentFixed(_valency,_valencyBalanced) : recordSub.valent(_valency,_valencyFactor);
					if (_entropyMinimum > 0.0 && recordValent.entropy() < _entropyMinimum)
						continue;	
					if (!centered)
					{
						_centreX = posX;
						_centreY = posY;	
						centered = true;
						framePainter.setPen(_guiFrameRed ? Qt::red : Qt::white);		
					}
					else
						framePainter.setPen(_guiFrameRed ? Qt::magenta : Qt::gray);
					framePainter.drawRect(
						posX * _captureWidth - _scale * _captureHeight / 2.0, 
						posY * _captureHeight - _scale * _captureHeight / 2.0, 
						_scale * _captureHeight,
						_scale * _captureHeight);
					auto hr = recordsHistoryRepa(_scaleValency, 0, _valency, recordValent);
					if (!_updateDisable)
						_events->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr),_events->references);	
					this->eventId++;		
					eventCount++;		
				}
				_ui->labelImage->setPixmap(QPixmap::fromImage(image2));	
			}
			else
			{
				QImage image2 = image.copy();
				QPainter framePainter(&image2);
				framePainter.setPen(_guiFrameRed ? Qt::red : Qt::white);
				framePainter.drawRect(
					_centreX * _captureWidth - _scale * _captureHeight / 2.0, 
					_centreY * _captureHeight - _scale * _captureHeight / 2.0, 
					_scale * _captureHeight,
					_scale * _captureHeight);		
				_ui->labelImage->setPixmap(QPixmap::fromImage(image2));		
			}
			if (!_updateDisable)
			{
				if (_struct=="struct002")
				{
					for (auto& activeA : _level1)
					{
						if (!activeA->update(_updateParameters))
						{
							this->terminate = true;	
							return;
						}
					}
				}
				if (!_active->update(_updateParameters))
				{
					this->terminate = true;	
					return;
				}
			}		
		}
		// representations
		{		
			auto& activeA = *_active;
			std::lock_guard<std::mutex> guard(activeA.mutex);
			std::shared_ptr<HistoryRepa> hr = activeA.underlyingHistoryRepa.front();
			auto& hs = *activeA.historySparse;
			auto& slev = activeA.historySlicesSetEvent;
			auto n = hr->dimension;
			auto z = hr->size;
			auto y = activeA.historyEvent;
			auto rr = hr->arr;	
			auto rs = hs.arr;
			auto& dr = *activeA.decomp;		
			auto& cv = dr.mapVarParent();
			auto& reps = *_slicesRepresentation;
			for (std::size_t k = 0; k < eventCount; k++)	
			{
                auto j = (y + z - eventCount + k) % z;
				auto slice = rs[j];
				while (true)
				{
					if (!reps.count(slice))
						reps.insert_or_assign(slice, Representation(1.0,1.0,_size,_size));
					auto& rep = reps[slice];
					auto& arr1 = *rep.arr;
					auto jn = j*n;
					for (size_t i = 0; i < n-1; i++)
						arr1[i] += rr[jn + i];
					rep.count++;
					if (!slice)
						break;
					slice = cv[slice];
				}		
			}		
			// check for new leaf slices and update representation map
            if (_fudsSize < dr.fuds.size())
			{
				for (std::size_t i = _fudsSize; i < dr.fuds.size(); i++)
				{
					for (auto sliceB : dr.fuds[i].children)
					{
						Representation rep(1.0,1.0,_size,_size);
						auto& arr1 = *rep.arr;
						if (slev.count(sliceB))
						{
							for (auto j : slev[sliceB])
							{
								auto jn = j*n;
								for (size_t i = 0; i < n-1; i++)
									arr1[i] += rr[jn + i];
								rep.count++;
							}									
							reps.insert_or_assign(sliceB, rep);
						}
					}
				}
				_fudsSize = dr.fuds.size();
			}
			// determine if a lagging pause is needed
            if (_lagThreshold)
			{
				auto& thresholds = _induceParameters.induceThresholds;
				auto& fails = activeA.induceSliceFailsSize;
				failCount = fails.size();
				for (auto slice : activeA.induceSlices)
				{
					auto it = fails.find(slice);
					if (it != fails.end())
					{
						auto sliceSize = slev[slice].size();
						if (it->second < sliceSize 
							&& (!thresholds.size() || thresholds.count(sliceSize)))			
							lag++;							
					}
					else 
						lag++;
				}	
				if (lag >= _lagThreshold && !_lagWaiting)
				{
					this->eventId ++;	
					_lagWaiting = true;		
				}
				else if (!lag)
					_lagWaiting = false;	
			}
		}
        // event label
		if (_mode.size())
		{
			std::stringstream string;
			string << "event: " << std::fixed << this->eventId;
			_labelEvent->setText(string.str().data());
		}
		if (_mode.size())
		{
			std::stringstream string;
			string << "fuds: " << std::fixed << _fudsSize;
			_labelFuds->setText(string.str().data());
		}
		// lagging label
        if (_lagThreshold)
		{
			std::stringstream string;
			string << "lag: " << std::fixed << lag;
			_labelLag->setText(string.str().data());
		}
		// fails label
        if (_lagThreshold)
		{
			std::stringstream string;
			string << "fails: " << std::fixed << failCount;
			_labelFails->setText(string.str().data());
		}
		if (_eventLogging && (_eventLoggingFactor <= 1 || this->eventId >= _eventIdPrev +  _eventLoggingFactor))
		{
            LOG "actor\tevent id: " << this->eventId << "\ttime " << ((Sec)(Clock::now() - _mark)).count() << "s" UNLOG
			_eventIdPrev = this->eventId;
		}
	}
	if (_system && _actLogging && (_actLoggingFactor <= 1 || _actCount % _actLoggingFactor == 0))
	{
		std::stringstream string;
        string << "actor\tupdated\t" << std::fixed << std::setprecision(6) << ((Sec)(Clock::now() - _mark)).count() << std::defaultfloat << "s";
		LOG string.str() UNLOG
	}
	_mark = Clock::now(); 
	if (_system && _interactive)
	{
		// update events
		Record record(image, 
			_scale * _captureHeight / _captureWidth, _scale,
			_centreX, _centreY, _size, _size, _divisor, _divisor);
		Record recordValent = _valencyFixed ? record.valentFixed(_valency,_valencyBalanced) : record.valent(_valency,_valencyFactor);
		// representations
		std::size_t slice = 0;
		std::vector<Representation> examples;
		std::vector<std::pair<double,std::size_t>> ancestors;
		std::vector<SizeSet> ancestorHighlights;
		std::vector<std::pair<double,std::size_t>> siblings;
		{		
			auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
			auto cap = (unsigned char)(_updateParameters.mapCapacity);
			bool is2Level = _struct == "struct002" || _struct == "struct005";
			bool isComputed = _struct == "struct004" || _struct == "struct005";
			auto& activeA = *_active;
			std::lock_guard<std::mutex> guard(activeA.mutex);
			std::shared_ptr<HistoryRepa> hr1 = activeA.underlyingHistoryRepa.front();
			auto& slev = activeA.historySlicesSetEvent;
			auto hr = recordsHistoryRepa(_scaleValency, _scaleValue, _valency, recordValent);	
			auto n = hr->dimension;
			auto vv = hr->vectorVar;
			auto rr = hr->arr;	
			auto rr1 = hr1->arr;	
			auto& sizes = activeA.historySlicesSize;
			auto& lengths = activeA.historySlicesLength;
			auto& dr = *activeA.decomp;		
			auto& cv = dr.mapVarParent();
			auto& vi = dr.mapVarInt();
			SizeUCharStructList jj;
			jj.reserve(n + _level2Size*_level2Size*20);
			if (is2Level)
			{
				auto& proms = activeA.underlyingsVarsOffset;
				auto& dru = *_level1Decomp;	
				for (std::size_t y1 = 0, m = 0; y1 < _level2Size; y1++)	
					for (std::size_t x1 = 0; x1 < _level2Size; x1++, m++)	
					{
						Record recordTile(recordValent,_level1Size,_level1Size,x1*_level1Size,y1*_level1Size);
						auto hru = recordsHistoryRepa(_scaleValency, _scaleValue, _valency, recordTile);
						auto nu = hru->dimension;
						auto vvu = hru->vectorVar;
						auto rru = hru->arr;	
						SizeUCharStructList kk;
						kk.reserve(nu);
						if (isComputed)
						{
							std::size_t s = _valency;
							std::size_t b = 0; 
							if (s)
							{
								s--;
								while (s >> b)
									b++;
							}
							for (std::size_t i = 0; i < nu-1; i++)
							{
								SizeUCharStruct qq;
								qq.uchar = 1;	
								for (int k = b; k > 0; k--)
								{
									qq.size = 65536 + (vvu[i] << 12) + (k << 8) + (rru[i] >> b-k);
									jj.push_back(qq);
								}
							}											
						}
						else
							for (std::size_t i = 0; i < nu-1; i++)
							{
								SizeUCharStruct qq;
								qq.uchar = rru[i];	
								qq.size = vvu[i];
								if (qq.uchar)
									kk.push_back(qq);
							}										
						auto ll = drmul(kk,dru,cap);	
						if (ll && ll->size()) 
							for (auto sliceA : *ll)
								if (sliceA)
								{
									SizeUCharStruct qq;
									qq.uchar = 1;			
									qq.size = sliceA;
									activeA.varPromote(proms[m], qq.size);
									jj.push_back(qq);
								}
					}				
			}
			if (!is2Level || _substrateInclude)	
			{
				if (isComputed)
				{
					std::size_t s = _valency;
					std::size_t b = 0; 
					if (s)
					{
						s--;
						while (s >> b)
							b++;
					}
					for (std::size_t i = 0; i < n-1; i++)
					{
						SizeUCharStruct qq;
						qq.uchar = 1;	
						for (int k = b; k > 0; k--)
						{
							qq.size = 65536 + (vv[i] << 12) + (k << 8) + (rr[i] >> b-k);
							jj.push_back(qq);
						}
					}											
				}
				else			
					for (std::size_t i = 0; i < n - 1; i++)
					{
						SizeUCharStruct qq;
						qq.uchar = rr[i];	
						qq.size = vv[i];
						if (qq.uchar)
							jj.push_back(qq);
					}
			}											
			{
				SizeUCharStruct qq;
				qq.uchar = rr[n-1];	
				qq.size = vv[n-1];
				jj.push_back(qq);
			}				
			auto ll = drmul(jj,dr,cap);	
			if (ll && ll->size())
				slice = ll->back();
			if (_guiLengthMaximum)
			{
				while (lengths[slice] > _guiLengthMaximum)
					slice = cv[slice];					
			}
			// ancestors
			if (cv.count(slice) && sizes.count(slice))
			{
				double lnwmax = std::log(_induceParameters.wmax);
				std::size_t sliceA = slice;
				while (true)
				{
					auto sliceSize = sizes[sliceA];
					auto parent = cv[sliceA];
					std::size_t parentSize = sizes[parent];
					double likelihood = (std::log(sliceSize) - std::log(parentSize) + lnwmax)/lnwmax;
					ancestors.push_back(std::make_pair(likelihood, sliceA));
					ancestorHighlights.push_back(SizeSet());
					if (_guiUnderlying && sliceA)
					{
						auto& fud = dr.fuds[vi[parent]].fud;
						auto& highlights = ancestorHighlights.back();
						for (auto& tr : fud)
						{
							auto n = tr->dimension;
							auto vv = tr->vectorVar;
							for (std::size_t i = 0; i < n; i++)
								highlights.insert(vv[i]);
						}
					}
					if (!sliceA)
						break;
					sliceA = cv[sliceA];					
				}
				sliceA = cv[slice];	
				{
					std::size_t parentSize = sizes[sliceA];
					for (auto sliceB : dr.fuds[vi[sliceA]].children)
					{
						auto sliceSize = sizes[sliceB];
						double likelihood = (std::log(sliceSize) - std::log(parentSize) + lnwmax)/lnwmax;
						siblings.push_back(std::make_pair(likelihood, sliceB));
					}
				}
			}
			if (_interactiveExamples && slev.count(slice))
			{
				auto& events = slev[slice];
				std::size_t k = 0;
				std::size_t interval = events.size()/_labelSize;
				if (!interval) interval = 1;
				for (auto j : events)
				{
					if (k % interval == 0)
					{
						Representation rep(1.0,1.0,_size,_size);
						auto& arr1 = *rep.arr;
						auto jn = j*n;
						for (size_t i = 0; i < n-1; i++)
							arr1[i] += rr1[jn + i];
						rep.count++;
						examples.push_back(rep);
					}
					k++;
				}
			}
		}
		{
			std::sort(siblings.rbegin(), siblings.rend());
			auto& reps = *_slicesRepresentation;	
			_labelRecords[2]->setPixmap(QPixmap::fromImage(record.image(_multiplier,0)));
			_labelRecords[1]->setPixmap(QPixmap::fromImage(recordValent.image(_multiplier,_valency)));
			if (reps.count(slice))
				_labelRecords[0]->setPixmap(QPixmap::fromImage(reps[slice].image(_multiplier,_valency)));	
			else
				_labelRecords[0]->setPixmap(_pixmapBlank);	
			if (reps.count(slice) && ancestors.size())
			{
				std::stringstream string;
				string << std::fixed << std::setprecision(3) << ancestors[0].first << std::defaultfloat;
				_labelRecordLikelihood->setText(string.str().data());				
			}
			else
				_labelRecordLikelihood->setText("");	
			if (_interactiveEntropies)
			{
				std::stringstream string;
				string << std::fixed << std::setprecision(3) << recordValent.entropy() << std::defaultfloat;
				_labelRecordEntropy1->setText(string.str().data());				
			}
			if (_interactiveEntropies)
			{
				std::stringstream string;
				string << std::fixed << std::setprecision(3) << record.entropy() << std::defaultfloat;
				_labelRecordEntropy2->setText(string.str().data());				
			}				
			for (std::size_t k = 0; k < _labelSize; k++)	
			{
				if (k == _labelSize - 1 && siblings.size() && k < siblings.size() - 1)
				{
					std::stringstream string;
					string << std::fixed << std::setprecision(0) << siblings.size() - _labelSize + 1 << std::defaultfloat;
					_labelRecordSiblings[k]->setText(string.str().data());
					_labelRecordSiblingLikelihoods[k]->setText("");
				}
				else if (k < siblings.size() && reps.count(siblings[k].second))
				{
					_labelRecordSiblings[k]->setPixmap(QPixmap::fromImage(reps[siblings[k].second].image(_multiplier,_valency)));					
					std::stringstream string;
					string << std::fixed << std::setprecision(3) << siblings[k].first << std::defaultfloat;
					_labelRecordSiblingLikelihoods[k]->setText(string.str().data());
				}
				else
				{
					_labelRecordSiblings[k]->setPixmap(_pixmapBlank);
					_labelRecordSiblingLikelihoods[k]->setText("");
				}
				if (k == _labelSize - 1 && ancestors.size() && k < ancestors.size() - 1)
				{
					std::stringstream string;
					string << std::fixed << std::setprecision(0) << ancestors.size() - _labelSize + 1 << std::defaultfloat;
					_labelRecordAncestors[k]->setText(string.str().data());
					_labelRecordAncestorLikelihoods[k]->setText("");
				}
				else if (k < ancestors.size() && reps.count(ancestors[k].second))
				{
					_labelRecordAncestors[k]->setPixmap(QPixmap::fromImage(reps[ancestors[k].second].image(_multiplier,_valency, 256, ancestorHighlights[k])));
					std::stringstream string;
					string << std::fixed << std::setprecision(3) << ancestors[k].first << std::defaultfloat;
					_labelRecordAncestorLikelihoods[k]->setText(string.str().data());
				}
				else
				{
					_labelRecordAncestors[k]->setPixmap(_pixmapBlank);
					_labelRecordAncestorLikelihoods[k]->setText("");
				}
				if (_interactiveExamples && k < _labelRecordExamples.size() && k < examples.size())
				{
					auto rep = examples[k];
					auto image = rep.image(_multiplier,_valency);
					_labelRecordExamples[k]->setPixmap(QPixmap::fromImage(image));
					if (_interactiveEntropies)
					{
						std::stringstream string;
						string << std::fixed << std::setprecision(3) << rep.entropy() << std::defaultfloat;
						_labelRecordExampleEntropies[k]->setText(string.str().data());
					}
				}
				else if (_interactiveExamples && k < _labelRecordExamples.size())
				{
					_labelRecordExamples[k]->setPixmap(_pixmapBlank);
					if (_interactiveEntropies)
						_labelRecordExampleEntropies[k]->setText("");
				}
			}					
		}
		// centre label
		{
			std::stringstream string;
			string << "centre: (" << std::setprecision(3) << _centreX << "," << _centreY << ")";
			_labelCentre->setText(string.str().data());
		}
	}
	if (_system && _interactive && _actLogging && (_actLoggingFactor <= 1 || _actCount % _actLoggingFactor == 0))
	{
		std::stringstream string;
        string << "actor\tinteractive\t" << std::fixed << std::setprecision(6) << ((Sec)(Clock::now() - _mark)).count() << std::defaultfloat << "s";
		LOG string.str() UNLOG
	}
    auto t = (Sec)(Clock::now() - actMark);
	auto ti = (Sec)_interval;
    if (ti > t)
    {
		QTimer::singleShot((int)((ti - t).count()*1000.0), this, &Win007::act);
	}
	else		
	{
		QTimer::singleShot(0, this, &Win007::act);
		if (_actWarning)
		{
			LOG "actor\twarning: act time " << t.count() << "s" UNLOG
		}
	}	
	_actCount++;
}

void Win007::mousePressEvent(QMouseEvent *event)
{
	if (_interactive)	
	{
		auto geo = _ui->labelImage->geometry();
		auto point = event->position().toPoint() - geo.topLeft();
		_centreX = (double)point.x()/geo.size().width();
		_centreY = (double)point.y()/geo.size().height();
        std::stringstream string;
        string << "centre: (" << std::setprecision(3) << _centreX << "," << _centreY << ")";
        // LOG string.str() UNLOG
        _labelCentre->setText(string.str().data());
	}
}

void Win007::keyPressEvent(QKeyEvent *event)
{
	if (_interactive)	
	{
		if(event->key() == Qt::Key_Left)
		{
			_centreX -= 0.25/40.0;
		}
		else if(event->key() == Qt::Key_Up)
		{
			_centreY -= 0.25/40.0;
		}
		else if(event->key() == Qt::Key_Down)
		{
			_centreY += 0.25/40.0;
		}
		else if(event->key() == Qt::Key_Right)
		{
			_centreX += 0.25/40.0;
		}
		else if(event->key() == Qt::Key_Space)
		{
			_centreX = 0.5;
			_centreY = 0.5;
		}
        std::stringstream string;
        string << "centre: (" << std::setprecision(3) << _centreX << "," << _centreY << ")";
        // LOG string.str() UNLOG
        _labelCentre->setText(string.str().data());
	}
}
