#include "win008.h"
#include "./ui_win008.h"
#include <QUrl>
#include <QVideoSink>
#include <QVideoFrame>
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

Win008::Win008(const std::string& configA,
               QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::Win008),
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
		_eventLogging = ARGS_BOOL(logging_event);
		_eventLoggingFactor = ARGS_INT(logging_event_factor);
		_actLogging = ARGS_BOOL(logging_action);
		_actLoggingFactor = ARGS_INT(logging_action_factor);
		_actCount = 0;
		_interval = (std::chrono::milliseconds)(ARGS_INT_DEF(interval,1000));
		_actWarning = ARGS_BOOL(warning_action);
		_actLoggingSlice = ARGS_BOOL(logging_action_slice);
		_checkpointing = ARGS_BOOL(checkpointing);
		_checkpointInterval = ARGS_INT_DEF(checkpoint_interval,100000);
		_checkpointEvent = 0;
		_mode = ARGS_STRING(mode);
		_modeLogging = ARGS_BOOL(logging_mode);
		_modeLoggingFactor = ARGS_INT(logging_mode_factor); 
		_modeTracing = ARGS_BOOL(tracing_mode);
		_eventIdMax = ARGS_INT(event_maximum);
		_model = ARGS_STRING(model);
		_modelInitial = ARGS_STRING(model_initial);
		gui = ARGS_BOOL(gui);
		_videoSource = ARGS_STRING(video_source);
		if (args.HasMember("video_sources") && args["video_sources"].IsArray())
		{
			auto& arr = args["video_sources"];
			for (int k = 0; k < arr.Size(); k++)
				_videoSources.push_back(arr[k].GetString());	
		}
		_videoIndex = ARGS_INT(video_index);
		_videoStart = ARGS_INT_DEF(video_start,120);
		_videoEnd = ARGS_INT_DEF(video_end,30);
		_mediaStart = ARGS_INT_DEF(media_start,10000);
		_playbackRate = ARGS_DOUBLE(playback_rate);
		_mediaRetry = ARGS_BOOL(retry_media);
		_updateDisable = ARGS_BOOL(disable_update);
		_activeLogging = ARGS_BOOL(logging_active);
		_activeSummary = ARGS_BOOL(summary_active);
		_activeSize = ARGS_INT_DEF(activeSize,1000000);
		_updateParameters.mapCapacity = ARGS_INT_DEF(updateParameters.mapCapacity,3); 
		_induceThreshold = ARGS_INT_DEF(induceThreshold,200);
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
		_failCount = 0;
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
		_valency = ARGS_INT_DEF(valency,10);	
		_valencyFactor = ARGS_INT(valency_factor);	
		_valencyFixed = ARGS_BOOL(valency_fixed);	
		_size = ARGS_INT_DEF(size,40);	
		_sizeTile = ARGS_INT_DEF(tile_size,_size/2);	
		_divisor = ARGS_INT_DEF(divisor,4);	
		_multiplier = ARGS_INT_DEF(multiplier,2);	
		_eventSize = ARGS_INT_DEF(event_size,1);	
		_scanSize = ARGS_INT_DEF(scan_size,1);	
		_threadCount = ARGS_INT_DEF(threads,1);	
		_separation = ARGS_DOUBLE_DEF(separation,0.5);
		_recordUniqueSize = ARGS_INT(unique_records);	
		if (_recordUniqueSize)
			_recordUniqueSet.reserve(_recordUniqueSize);
		_entropyMinimum = ARGS_DOUBLE(entropy_minimum);
	}
	{
		_labelCentre = new QLabel(this); 
		_ui->layout04->addWidget(_labelCentre);
		_labelEvent = new QLabel(this); 
		_ui->layout04->addWidget(_labelEvent);
		_labelFuds = new QLabel(this); 
		_ui->layout04->addWidget(_labelFuds);
		_labelFails = new QLabel(this); 
		_ui->layout04->addWidget(_labelFails);
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
		_events = std::make_shared<ActiveEventsRepa>(1);
		_active = std::make_shared<Active>();
		{
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
				_checkpointEvent = this->eventId;
				this->eventId++;
				_fudsSize = activeA.decomp->fuds.size();
			}
			else
			{
				activeA.var = activeA.system->next(activeA.bits);
				activeA.varSlice = activeA.system->next(activeA.bits);
				activeA.historySize = _activeSize;
				activeA.induceThreshold = _induceThreshold;
				activeA.decomp = std::make_unique<DecompFudSlicedRepa>();	
				{
                    auto hr = sizesHistoryRepa(_scaleValency, _valency, _size*_size, activeA.historySize);
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
			activeA.historySliceCachingIs = true;
			activeA.name = (_model!="" ? _model : "model");			
			activeA.logging = _activeLogging;
			activeA.summary = _activeSummary;
			activeA.underlyingEventsRepa.push_back(_events);
			activeA.eventsSparse = std::make_shared<ActiveEventsArray>(1);
			if (_modelInitial.size())
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
		}
	}
	// start act timer
	if (_system)
	{
		this->terminate = false;	
		if (_videoSource.size() || _videoSources.size())
		{
			_isSeekable = false;
			_position = _videoStart*1000;
			_mediaPlayer = new QMediaPlayer(this);
			connect(_mediaPlayer, &QMediaPlayer::errorChanged,this, &Win008::handleError);
			_videoWidget = new QVideoWidget;
			_mediaPlayer->setVideoOutput(_videoWidget);
			QTimer::singleShot(_mediaStart, this, &Win008::mediaStart);
		}
		else
		{
			_screen = QGuiApplication::primaryScreen();
			QTimer::singleShot(_interval.count(), this, &Win008::capture);
		}
		LOG "actor\tstatus: initialised" UNLOG
	}
	else
	{
		LOG "actor\terror: failed to initialise" UNLOG
	}
}

Win008::~Win008()
{
	terminate = true;
	_active->terminate = true;
	LOG "actor\tdumping" UNLOG
	dump();	
    delete _ui;
	LOG "actor\tstatus: finished" UNLOG
}


void Win008::dump()
{
	if (_system && _model!="")
	{
		bool ok = true;
		auto& activeA = *_active;
		ActiveIOParameters ppio;
		ppio.filename = activeA.name+".ac";
		auto logging = activeA.logging;
		activeA.logging = true;
		ok = ok && activeA.dump(ppio);		
		activeA.logging = logging;
		if (ok)
		{
			// dump slice representations
			try
			{
				auto mark = Clock::now(); 
				std::ofstream out(_model + ".rep", std::ios::binary);
				sliceRepresentationUMapsPersistent(*_slicesRepresentation, out); 
				out.close();
				LOG "actor\tdump\tfile name: " << _model + ".rep" << "\ttime " << ((Sec)(Clock::now() - mark)).count() << "s" UNLOG
				LOG "actor\tevent id: " << this->eventId UNLOG
			}
			catch (const std::exception&)
			{
				LOG "actor\terror: failed to write slice-representations file" <<_model + ".rep" UNLOG
				ok = false;
			}				
		}
		if (!ok)
		{
			terminate = true;
			_active->terminate = true;
			LOG "actor\tstatus: quitting" UNLOG
		}
	}	
}

void Win008::handleError()
{
    if (_mediaPlayer->error() == QMediaPlayer::NoError)
        return;

    const QString errorString = _mediaPlayer->errorString();
    QString message;
    if (errorString.isEmpty())
        message += " #" + QString::number(int(_mediaPlayer->error()));
    else
        message += errorString;
	
	std::stringstream string;
	string << "actor\tmedia error\t" << message.toStdString()
		<< "\tposition\t" << std::fixed << _mediaPlayer->position()
		<< "\tduration\t" << std::fixed << _mediaPlayer->duration()
		<< std::defaultfloat;
	LOG string.str() UNLOG

	if (_mediaRetry)
	{
		_position = _videoStart*1000;
		disconnect(_mediaPlayer, &QMediaPlayer::positionChanged, 0, 0);
		disconnect(_mediaPlayer, &QMediaPlayer::errorChanged, 0, 0);
		_mediaPlayer->stop();	
		_mediaPlayer = new QMediaPlayer(this);
		connect(_mediaPlayer, &QMediaPlayer::errorChanged,this, &Win008::handleError);
		_videoWidget = new QVideoWidget;
		_mediaPlayer->setVideoOutput(_videoWidget);
		QTimer::singleShot(_mediaStart, this, &Win008::mediaStart);	
		LOG "actor\tretrying" UNLOG		
	}
	else
	{
		this->terminate = true;
		QCoreApplication::exit();
		LOG "actor\tstatus: quitting" UNLOG
	}
}


void Win008::mediaStart()
{
	connect(_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &Win008::mediaStateChanged);
	if (_videoSources.size())
	{
		_mediaPlayer->setSource(QUrl::fromLocalFile(QString(_videoSources[_videoIndex].c_str())));		
		_videoIndex++;
		if (_videoIndex >= _videoSources.size())
			_videoIndex = 0;
	}
	else
		_mediaPlayer->setSource(QUrl::fromLocalFile(QString(_videoSource.c_str())));
	if (_actLogging)	
	{
		std::string string = "actor\tsource\t";
		string += _mediaPlayer->source().toString().toStdString();
		LOG string UNLOG
	}
	if (_actLogging && _videoSources.size())	
	{
		LOG "actor\tnext video index: " << _videoIndex UNLOG
	}
}

void Win008::mediaStateChanged(QMediaPlayer::MediaStatus state)
{
    if (state == QMediaPlayer::LoadedMedia)
    {
		_isSeekable = _mediaPlayer->isSeekable();
		if (_isSeekable)
			_mediaPlayer->setPosition(_position);
        disconnect(_mediaPlayer, &QMediaPlayer::mediaStatusChanged, 0, 0);
        connect(_mediaPlayer, &QMediaPlayer::positionChanged, this, &Win008::capture);
		if (_playbackRate != 1.0 && _playbackRate > 0.0)
			_mediaPlayer->setPlaybackRate(_playbackRate);
		_mediaPlayer->play();
		if (_actLogging)	
		{
			LOG "actor\tseekable: " << (_isSeekable ? " true" : "false") << "\tduration: " << _mediaPlayer->duration() UNLOG
		}
    }
}

void Win008::capture()
{
	if (this->terminate || (_active && _active->terminate))
		return;
	auto actMark = Clock::now();	
	// capture
	_mark = Clock::now();
	if (_videoSource.size() || _videoSources.size())
	{
		if (_mediaPlayer->playbackState() != QMediaPlayer::PlayingState)
			return;
		// EVAL(_mediaPlayer->position());
		if (!_isSeekable && _mediaPlayer->position() < _position)
			return;
		auto videoframe = _mediaPlayer->videoSink()->videoFrame();
		_image = videoframe.toImage();
		_captureWidth = _image.width();
		_captureHeight = _image.height();
		if (_actLogging && (_actLoggingFactor <= 1 || _actCount % _actLoggingFactor == 0))	
		{
			std::stringstream string;
			string << "actor\tposition\t" << std::fixed << _mediaPlayer->position() << std::defaultfloat;
			LOG string.str() UNLOG
		}
		_position = _mediaPlayer->position() + _interval.count();
		if (_position >= _mediaPlayer->duration() - _videoEnd*1000)
		{
			_position = _videoStart*1000;
			disconnect(_mediaPlayer, &QMediaPlayer::positionChanged, 0, 0);
			disconnect(_mediaPlayer, &QMediaPlayer::errorChanged, 0, 0);
			_mediaPlayer->stop();	
			_mediaPlayer = new QMediaPlayer(this);
			connect(_mediaPlayer, &QMediaPlayer::errorChanged,this, &Win008::handleError);
			_videoWidget = new QVideoWidget;
			_mediaPlayer->setVideoOutput(_videoWidget);
			QTimer::singleShot(_mediaStart, this, &Win008::mediaStart);	
			return;			
		}
		else if (_isSeekable)
			_mediaPlayer->setPosition(_position);								
	}
	else
	{
		auto pixmap = _screen->grabWindow(0, _captureX, _captureY, _captureWidth, _captureHeight);
		_image = pixmap.toImage();
	}
	if (_actLogging && (_actLoggingFactor <= 1 || _actCount % _actLoggingFactor == 0))	
	{
        std::stringstream string;
        string << "actor\tcaptured\t" << std::fixed << std::setprecision(6) << ((Sec)(Clock::now() - _mark)).count() << std::defaultfloat << "s";
		LOG string.str() UNLOG
	}
	this->act();
	_ui->labelImage->setPixmap(QPixmap::fromImage(_image));	
	{
		std::stringstream string;
		string << "centre: (" << std::setprecision(3) << _centreX << "," << _centreY << ")";
		_labelCentre->setText(string.str().data());
	}
	{
		std::stringstream string;
		string << "event: " << std::fixed << this->eventId;
		_labelEvent->setText(string.str().data());
	}
	{
		std::stringstream string;
		string << "fuds: " << std::fixed << _fudsSize;
		_labelFuds->setText(string.str().data());
	}
	{
		std::stringstream string;
		string << "fails: " << std::fixed << _failCount;
		_labelFails->setText(string.str().data());
	}
	if (!(_videoSource.size() || _videoSources.size()))
	{
		auto t = (Sec)(Clock::now() - actMark);
		auto ti = (Sec)_interval;
		if (ti > t)
		{
			QTimer::singleShot((int)((ti - t).count()*1000.0), this, &Win008::capture);
		}
		else		
		{
			QTimer::singleShot(0, this, &Win008::capture);
			if (_actWarning)
			{
				LOG "actor\twarning: act time " << t.count() << "s" UNLOG
			}
		}	
	}
}

void Win008::act()
{
	if (this->terminate || (_active && _active->terminate))
		return;
	// update
	_mark = Clock::now(); 
	if (_system && (!_eventIdMax || this->eventId < _eventIdMax))
	{
		// update events
		std::size_t eventCount = 0;
		if (_mode == "mode001")
		{
			for (std::size_t k = 0; k < _eventSize; k++)	
			{
				auto centreRandomX = _centreRandomX > 0.0 ? ((double) rand() / (RAND_MAX)) *_centreRandomX * 2.0 - _centreRandomX : 0.0;
				auto centreRandomY = _centreRandomY > 0.0 ? ((double) rand() / (RAND_MAX)) *_centreRandomY * 2.0 - _centreRandomY : 0.0;
                Record record(_image,
					_scale * _captureHeight / _captureWidth, _scale,
					_centreX + (centreRandomX * _captureHeight / _captureWidth), 
					_centreY + centreRandomY, 
					_size, _size, _divisor, _divisor);
				Record recordValent = _valencyFixed ? record.valentFixed(_valency) : record.valent(_valency,_valencyFactor);
				if (_recordUniqueSize)
				{
					auto recordHash = recordValent.hash();
					if (_recordUniqueSet.count(recordHash))
						continue;		
					while (_recordUniqueQueue.size() >= _recordUniqueSize)
					{
						_recordUniqueSet.erase(_recordUniqueQueue.front());
						_recordUniqueQueue.pop();
					}
					_recordUniqueSet.insert(recordHash);
					_recordUniqueQueue.push(recordHash);
				}
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
                Record record(_image,
					_scale * _captureHeight / _captureWidth, _scale,
					_centreX + (centreRandomX * _captureHeight / _captureWidth), 
					_centreY + centreRandomY, 
					_size, _size, _divisor, _divisor);
				records.push_back(_valencyFixed ? record.valentFixed(_valency) : record.valent(_valency,_valencyFactor));	
			}
			std::vector<std::pair<double,std::size_t>> likelihoodsRecord;		
			{		
				auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
				auto cap = (unsigned char)(_updateParameters.mapCapacity);
				double lnwmax = std::log(_induceParameters.wmax);
				auto& activeA = *_active;
				// std::lock_guard<std::mutex> guard(activeA.mutex);
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
			for (std::size_t k = 0; eventCount < _eventSize && k < _scanSize; k++)	
			{
				std::size_t m =  likelihoodsRecord.size() > k ? likelihoodsRecord[k].second : k;
				auto& record = records[m];
				if (_recordUniqueSize)
				{
					auto recordHash = record.hash();
					if (_recordUniqueSet.count(recordHash))
						continue;		
					while (_recordUniqueQueue.size() >= _recordUniqueSize)
					{
						_recordUniqueSet.erase(_recordUniqueQueue.front());
						_recordUniqueQueue.pop();
					}
					_recordUniqueSet.insert(recordHash);
					_recordUniqueQueue.push(recordHash);
				}
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
                Record record(_image,
					_scale * _captureHeight / _captureWidth, _scale,
					_centreX + (centreRandomX * _captureHeight / _captureWidth), 
					_centreY + centreRandomY, 
					_size, _size, _divisor, _divisor);
				records.push_back(_valencyFixed ? record.valentFixed(_valency) : record.valent(_valency,_valencyFactor));	
			}
			std::vector<std::pair<std::pair<std::size_t,double>,std::size_t>> actsPotsRecord;
			{		
				auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
				auto cap = (unsigned char)(_updateParameters.mapCapacity);
				double lnwmax = std::log(_induceParameters.wmax);
				auto& activeA = *_active;
				// std::lock_guard<std::mutex> guard(activeA.mutex);
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
			for (std::size_t k = 0; eventCount < _eventSize && k < _scanSize; k++)	
			{
				std::size_t m =  actsPotsRecord.size() > k ? actsPotsRecord[k].second : k;
				auto& record = records[m];
				if (_recordUniqueSize)
				{
					auto recordHash = record.hash();
					if (_recordUniqueSet.count(recordHash))
						continue;		
					while (_recordUniqueQueue.size() >= _recordUniqueSize)
					{
						_recordUniqueSet.erase(_recordUniqueQueue.front());
						_recordUniqueQueue.pop();
					}
					_recordUniqueSet.insert(recordHash);
					_recordUniqueQueue.push(recordHash);
				}
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
            Record record(_image,
				scaleX * _captureHeight / _captureWidth, scaleY,
				centreX, centreY, 
				sizeX, sizeY, 
				_divisor, _divisor);	
			std::vector<std::tuple<std::size_t,double,double,double,std::size_t,std::size_t>> actsPotsCoord(sizeY*sizeX);
			{
				auto& activeA = *_active;
				auto& actor = *this;
				// std::lock_guard<std::mutex> guard(activeA.mutex);
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
										Record recordValent = valencyFixed ? recordSub.valentFixed(valency) : recordSub.valent(valency,valencyFactor);
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
				if (_recordUniqueSize || _entropyMinimum > 0.0)
				{
					Record recordSub(record,_size,_size,x,y);
					Record recordValent = _valencyFixed ? recordSub.valentFixed(_valency) : recordSub.valent(_valency,_valencyFactor);
					if (_recordUniqueSize)
					{
						auto recordHash = recordValent.hash();
						if (_recordUniqueSet.count(recordHash))
							continue;		
						while (_recordUniqueQueue.size() >= _recordUniqueSize)
						{
							_recordUniqueSet.erase(_recordUniqueQueue.front());
							_recordUniqueQueue.pop();
						}
						_recordUniqueSet.insert(recordHash);
						_recordUniqueQueue.push(recordHash);
					}
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
			QPainter framePainter(&_image);
			if (gui)
			{
				framePainter.setPen(Qt::darkGray);
				framePainter.drawRect(
					centreX * _captureWidth - scaleX * _captureHeight / 2.0, 
					centreY * _captureHeight - scaleY * _captureHeight / 2.0, 
					scaleX * _captureHeight,
					scaleY * _captureHeight);
			}
			bool centered = false;
			for (auto t : actsPotsCoordTop)
			{
				auto posX = std::get<2>(t);
				auto posY = std::get<3>(t);	
				auto x = std::get<4>(t);
				auto y = std::get<5>(t);
				Record recordSub(record,_size,_size,x,y);
				Record recordValent = _valencyFixed ? recordSub.valentFixed(_valency) : recordSub.valent(_valency,_valencyFactor);
				if (!centered)
				{
					_centreX = posX;
					_centreY = posY;	
					centered = true;
					if (gui)
						framePainter.setPen(Qt::white);		
				}
				else if (gui)
					framePainter.setPen(Qt::gray);
				if (gui)
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
            Record record(_image,
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
					// std::lock_guard<std::mutex> guard(activeA.mutex);
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
								for (std::size_t y = ty*sizeTile, z = 0; y < (ty+1)*sizeTile; y++)	
									for (std::size_t x = tx*sizeTile; x < (tx+1)*sizeTile; x++, z++)
										if (z % actor._threadCount == t)
										{
											Record recordSub(record,size,size,x,y);
											Record recordValent = valencyFixed ? recordSub.valentFixed(valency) : recordSub.valent(valency,valencyFactor);
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
			QPainter framePainter(&_image);
			if (gui)
			{
				framePainter.setPen(Qt::darkGray);
				framePainter.drawRect(
					centreX * _captureWidth - scaleX * _captureHeight / 2.0, 
					centreY * _captureHeight - scaleY * _captureHeight / 2.0, 
					scaleX * _captureHeight,
					scaleY * _captureHeight);
			}
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
				Record recordValent = _valencyFixed ? recordSub.valentFixed(_valency) : recordSub.valent(_valency,_valencyFactor);
				if (_recordUniqueSize)
				{
					auto recordHash = recordValent.hash();
					if (_recordUniqueSet.count(recordHash))
						continue;		
					while (_recordUniqueQueue.size() >= _recordUniqueSize)
					{
						_recordUniqueSet.erase(_recordUniqueQueue.front());
						_recordUniqueQueue.pop();
					}
					_recordUniqueSet.insert(recordHash);
					_recordUniqueQueue.push(recordHash);
				}
				if (_entropyMinimum > 0.0 && recordValent.entropy() < _entropyMinimum)
					continue;	
				if (!centered)
				{
					_centreX = posX;
					_centreY = posY;	
					centered = true;
					if (gui)
						framePainter.setPen(Qt::white);		
				}
				else if (gui)
					framePainter.setPen(Qt::gray);
				if (gui)
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
		}
		if (!_updateDisable)
		{
			if (!_active->update(_updateParameters))
			{
				this->terminate = true;	
				LOG "actor\tstatus: quitting" UNLOG
				return;
			}
			if (!_active->induce(_induceParameters))
			{
				this->terminate = true;	
				LOG "actor\tstatus: quitting" UNLOG
				return;
			}
		}
		// representations
		{		
			auto& activeA = *_active;
			// std::lock_guard<std::mutex> guard(activeA.mutex);
			std::shared_ptr<HistoryRepa> hr = activeA.underlyingHistoryRepa.front();
			auto& hs = *activeA.historySparse;
			auto& slev = activeA.historySlicesSetEvent;
			auto n = hr->dimension;
			auto z = hr->size;
			auto y = activeA.historyEvent;
			auto rr = hr->arr;	
			auto rs = hs.arr;
			auto& fails = activeA.induceSliceFailsSize;
			_failCount = fails.size();
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
		}
		if (_checkpointing && _system && _model!="" && this->eventId >= _checkpointEvent + _checkpointInterval)
		{
            LOG "actor\tcheckpointing" UNLOG
			dump();
			_checkpointEvent = this->eventId;
		}		
		if (_eventLogging && (_eventLoggingFactor <= 1 || this->eventId >= _eventIdPrev +  _eventLoggingFactor))
		{
            LOG "actor\tevent id: " << this->eventId << "\ttime " << ((Sec)(Clock::now() - _mark)).count() << "s" UNLOG
			_eventIdPrev = this->eventId;
		}
	}
	else 
	{
		this->terminate = true;
		QCoreApplication::exit();
		LOG "actor\tstatus: quitting" UNLOG
	}
	if (_system && _actLogging && (_actLoggingFactor <= 1 || _actCount % _actLoggingFactor == 0))
	{
		std::stringstream string;
        string << "actor\tupdated\t" << std::fixed << std::setprecision(6) << ((Sec)(Clock::now() - _mark)).count() << std::defaultfloat << "s";
		LOG string.str() UNLOG
	}
	_actCount++;
}

