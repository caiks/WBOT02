#include "modeller001.h"
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

void run_induce(Active& active, ActiveInduceParameters& pp, ActiveUpdateParameters& ppu)
{
	active.induce(pp, ppu);
	return;
};

Modeller001::Modeller001(const std::string& configA)
{
	this->terminate = true;
	// parse config
	_config = configA;
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
				LOG "modeller\terror: failed to open arguments file " << _config UNLOG
				return;
			}	
			if (!args.IsObject())
			{
				LOG "modeller\terror: failed to read arguments file " << _config UNLOG
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
		_checkpointing = ARGS_BOOL(checkpointing);
		_checkpointInterval = ARGS_INT_DEF(checkpoint_interval,100000);
		_checkpointEvent = 0;
		_mode = ARGS_STRING(mode);
		_modeLogging = ARGS_BOOL(logging_mode);
		_modeLoggingFactor = ARGS_INT(logging_mode_factor); 
		_modeTracing = ARGS_BOOL(tracing_mode);
		_eventIdMax = ARGS_INT(event_maximum);
		_struct = ARGS_STRING(structure);
		_model = ARGS_STRING(model);
		_modelInitial = ARGS_STRING(model_initial);
		_level1Model = ARGS_STRING(level1_model);
		gui = ARGS_BOOL(gui);
		if (args.HasMember("records_sources") && args["records_sources"].IsArray())
		{
			auto& arr = args["records_sources"];
			for (int k = 0; k < arr.Size(); k++)
				_recordsFileNames.push_back(arr[k].GetString());	
		}
		_recordsIndex = ARGS_INT(records_index);
		_recordsStart = ARGS_INT(records_start);
		_recordsMode = ARGS_STRING_DEF(records_mode,_mode);
		_updateDisable = ARGS_BOOL(disable_update);
		_activeLogging = ARGS_BOOL(logging_active);
		_level1Logging = ARGS_BOOL(logging_level1);
		_activeContinuous = ARGS_BOOL(continuous_active);
		_activeCumulative = ARGS_BOOL_DEF(cumulative_active,true);
		_activeSummary = ARGS_BOOL(summary_active);
		_level1Summary = ARGS_BOOL(summary_level1);
		_activeSize = ARGS_INT_DEF(activeSize,1000000);
		_level1ActiveSize = ARGS_INT_DEF(level1_activeSize,10);
		_updateParameters.mapCapacity = ARGS_INT_DEF(updateParameters.mapCapacity,3); 
		_induceThreshold = ARGS_INT_DEF(induceThreshold,200);
		_induceThresholdOnLoad = ARGS_INT(on_load_induceThreshold);
		_induceThreadCount = ARGS_INT_DEF(induceThreadCount,4);
		_induceParameters.tint = _induceThreadCount;		
		_induceParameters.wmax = ARGS_INT_DEF(induceParameters.wmax,18);
		_induceParameters.lmax = ARGS_INT_DEF(induceParameters.lmax,8);
		_induceParameters.xmax = ARGS_INT_DEF(induceParameters.xmax,128);
		_induceParameters.znnmax = ARGS_DOUBLE_DEF(induceParameters.znnmax, 200000.0 * 2.0 * 300.0 * 300.0 * _induceThreadCount);
		_induceParameters.omax = ARGS_INT_DEF(induceParameters.omax,10);
		_induceParameters.bmax = ARGS_INT_DEF(induceParameters.bmax,10*3);
		_induceParameters.mmax = ARGS_INT_DEF(induceParameters.mmax,3);
		_induceParameters.umax = ARGS_INT_DEF(induceParameters.umax,128);
		_induceParameters.pmax = ARGS_INT_DEF(induceParameters.pmax,1);
		_induceParameters.mult = ARGS_INT_DEF(induceParameters.mult,1);
		_induceParameters.seed = ARGS_INT_DEF(induceParameters.seed,5);	
		_induceParameters.diagonalMin = ARGS_DOUBLE_DEF(induceParameters.diagonalMin,6.0);
		_induceParameters.asyncThreadMax = ARGS_INT(induceParameters.asyncThreadMax);	
		_induceParameters.asyncInterval = ARGS_INT_DEF(induceParameters.asyncInterval,10);	
		_induceParameters.asyncUpdateLimit = ARGS_INT(induceParameters.asyncUpdateLimit);	
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
		_scale = ARGS_DOUBLE(scale);
		_scaleValency = ARGS_INT_DEF(scale_valency,1);	
		if (args.HasMember("scales") && args["scales"].IsArray())
		{
			auto& arr = args["scales"];
			for (int k = 0; k < arr.Size(); k++)
				_scales.push_back(arr[k].GetDouble());	
			_scaleValency = _scales.size();
		}
		_valency = ARGS_INT_DEF(valency,10);	
		_valencyBits = ARGS_INT(valency_bits);	
		if (!_valencyBits)
		{
			while ((_valency - 1) >> _valencyBits)
				_valencyBits++;
		}
		_valencyFactor = ARGS_INT(valency_factor);	
		_valencyFixed = ARGS_BOOL(valency_fixed);	
		_valencyBalanced = ARGS_BOOL(valency_balanced);	
		_valencyFixed |= _valencyBalanced;
		_size = ARGS_INT_DEF(size,40);	
		_level1Size = ARGS_INT_DEF(level1_size,8);	
		_level2Size = ARGS_INT_DEF(level2_size,5);	
		_sizeRecords = ARGS_INT_DEF(records_size,40);	
		_sizeTile = ARGS_INT_DEF(tile_size,_size/2);	
		_sizeScanStep = ARGS_INT_DEF(scan_step,_level1Size);	
		_eventSize = ARGS_INT_DEF(event_size,1);	
		_scanSize = ARGS_INT_DEF(scan_size,1);	
		_threadCount = ARGS_INT_DEF(threads,1);	
		_recordUniqueSize = ARGS_INT(unique_records);	
		if (_recordUniqueSize)
			_recordUniqueSet.reserve(_recordUniqueSize);
		_entropyMinimum = ARGS_DOUBLE(entropy_minimum);
		_substrateInclude = ARGS_BOOL(include_substrate);
	}
	// open records file
	if (_recordsIndex < _recordsFileNames.size())
	{
		try
		{
			_recordsFile.open(_recordsFileNames[_recordsIndex] + ".rec", std::ios::binary);
			if (!_recordsFile.is_open())
			{
                LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
				return;
			}
			else
			{
                LOG "modeller\tinit: opened records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
			}
			if (_recordsStart)
			{
				_recordsFile.seekg((sizeof(double)*4 + sizeof(std::size_t)*2 + (_size+_sizeTile)*(_size+_sizeTile))*_recordsStart, std::ios_base::beg);
				if (_recordsFile.fail() || _recordsFile.eof())
				{
					LOG "modeller\terror: failed to seek in records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
					return;
				}
			}
		}
		catch (const std::exception&)
		{
            LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
			return;
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
                LOG "modeller\terror: failed to open slice-representations file" << _modelInitial + ".rep" UNLOG
				return;
			}
		}
		catch (const std::exception&)
		{
            LOG "modeller\terror: failed to read records file" << _modelInitial + ".rep" UNLOG
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
			if (_struct=="struct002" || _struct=="struct005" || _struct=="struct006")
			{
				Active activeB;
				{
					ActiveIOParameters ppio;
					ppio.filename = _level1Model + ".ac";
					activeB.logging = true;
					if (!activeB.load(ppio))
					{
						LOG "modeller\terror: failed to load level1 model" << ppio.filename UNLOG
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
			activeA.historySliceCachingIs = true;
			activeA.continousIs = _activeContinuous;
			activeA.historySliceCumulativeIs = _activeCumulative;
            if (_modelInitial.size())
			{
				ActiveIOParameters ppio;
                ppio.filename = _modelInitial + ".ac";
				activeA.logging = true;
				if (!activeA.load(ppio))
				{
					LOG "modeller\terror: failed to load model" << ppio.filename UNLOG
					_system.reset();
					return;
				}
				if (_induceThresholdOnLoad)
				{
					auto& slev = activeA.historySlicesSetEvent;		
					for (auto pp : slev)
						if (pp.second.size() >= _induceThresholdOnLoad)
						{
							activeA.induceSlices.erase(pp.first);
							activeA.induceSliceFailsSize[pp.first] = pp.second.size();
						}
				}
				activeA.historySliceCumulativeIs = _activeCumulative;
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
				if (_struct=="struct003" || _struct=="struct004")
				{
					_system->block = (_size * _size << 12 >> activeA.bits) + 1;	
				}
				activeA.var = activeA.system->next(activeA.bits);
				activeA.varSlice = activeA.system->next(activeA.bits);
				activeA.historySize = _activeSize;
				activeA.induceThreshold = _induceThreshold;
				activeA.decomp = std::make_unique<DecompFudSlicedRepa>();
				if (_struct!="struct006")
				{
                    auto hr = sizesHistoryRepa(_scaleValency, _valency, _size*_size, activeA.historySize);
					activeA.underlyingHistoryRepa.push_back(std::move(hr));
				}	
				if (_struct=="struct002" || _struct=="struct005" || _struct=="struct006")
				{
					for (std::size_t m = 0; m < _level1.size(); m++)
					{
						activeA.underlyingHistorySparse.push_back(std::make_shared<HistorySparseArray>(activeA.historySize,1));
					}
				}
				else if (_struct=="struct003")
				{
					for (std::size_t m = 0; m < _size*_size; m++)
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
			if (_struct!="struct006")
				activeA.underlyingEventsRepa.push_back(_events);
			if (_struct=="struct002" || _struct=="struct005" || _struct=="struct006")
			{
				if (!_substrateInclude && _struct!="struct006")
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
			else if (_struct=="struct003")
			{
				{
					std::shared_ptr<HistoryRepa> hr = activeA.underlyingHistoryRepa.front();
					auto n = hr->dimension - 1;
					auto vv = hr->vectorVar;
					for (std::size_t i = 0; i < n; i++)
						activeA.induceVarExclusions.insert(vv[i]);
				}
				for (std::size_t m = 0; m < _size*_size; m++)
				{
					activeA.underlyingEventsSparse.push_back(std::make_shared<ActiveEventsArray>(1));
				}
			}
			activeA.eventsSparse = std::make_shared<ActiveEventsArray>(0);
			if (_modelInitial.size() && !_induceParameters.asyncThreadMax)
			{
				if (!activeA.induce(_induceParameters))
				{
					LOG "modeller\tstatus: failed to run initial induce" UNLOG
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
	if (_system)
	{
		if (_induceParameters.asyncThreadMax)
			_threads.push_back(std::thread(run_induce, std::ref(*_active), std::ref(_induceParameters), std::ref(_updateParameters)));
		this->terminate = false;
	}
	else
	{
		LOG "modeller\terror: failed to initialise" UNLOG
	}
}

Modeller001::~Modeller001()
{
	terminate = true;
	_active->terminate = true;
	for (auto& t : _threads)
		t.join();	
	if (_recordsFile.is_open())
	{
		try
		{
			_recordsFile.close();
		}
		catch (const std::exception&)
		{
			LOG "modeller\terror: failed to close records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
		}				
	}
	LOG "modeller\tdumping" UNLOG
	dump();	
	LOG "modeller\tstatus: finished" UNLOG
}

void Modeller001::dump()
{
	if (_system && _model!="")
	{
		bool ok = true;
		if (ok && !_updateDisable)
		{
			auto& activeA = *_active;
			ActiveIOParameters ppio;
			ppio.filename = activeA.name+".ac";
			auto logging = activeA.logging;
			activeA.logging = true;
			ok = ok && activeA.dump(ppio);		
			activeA.logging = logging;
		}
		if (ok)
		{
			try
			{
				auto mark = Clock::now(); 
				std::ofstream out(_model + ".rep", std::ios::binary);
				sliceRepresentationUMapsPersistent(*_slicesRepresentation, out); 
				out.close();
				LOG "modeller\tdump\tfile name: " << _model + ".rep" << "\ttime " << ((Sec)(Clock::now() - mark)).count() << "s" UNLOG
				LOG "modeller\tevent id: " << this->eventId UNLOG
			}
			catch (const std::exception&)
			{
				LOG "modeller\terror: failed to write slice-representations file" <<_model + ".rep" UNLOG
				ok = false;
			}				
		}
		if (!ok)
		{
			terminate = true;
			_active->terminate = true;
			LOG "modeller\tstatus: quitting" UNLOG
		}
	}	
}

void Modeller001::model()
{
	while (_system && (!_eventIdMax || this->eventId < _eventIdMax)
		&& !this->terminate && !(_active && _active->terminate))
	{
		// sleep if async and updateProhibit
		if (_induceParameters.asyncThreadMax && _active->updateProhibit)
		{
			if (_induceParameters.asyncInterval)
				std::this_thread::sleep_for(std::chrono::milliseconds(_induceParameters.asyncInterval));
			else
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		_mark = Clock::now(); 	
		std::size_t eventCount = 0;
		if (_mode == "mode012" && _scales.size() && _recordsFile.is_open())
		{
			std::vector<Record> records;
			std::vector<std::tuple<std::size_t,std::size_t,std::size_t,double,double,std::size_t,std::size_t,std::size_t,std::size_t>> actsPotsCoordTop;
			for (std::size_t k = 0; records.size() < _scanSize && k < _scanSize * 10; k++)	
			{
                Record record;
				{
					try
					{
						_recordsFile.read(reinterpret_cast<char*>(&record.scaleX), sizeof(double));
						if (_recordsFile.eof())
						{
							_recordsFile.close();
							_recordsIndex++;
							if (_recordsIndex >= _recordsFileNames.size())
								_recordsIndex = 0;
							_recordsFile.open(_recordsFileNames[_recordsIndex] + ".rec", std::ios::binary);
							if (!_recordsFile.is_open() || _recordsFile.eof())
							{
								LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
								this->terminate = true;
								return;
							}	
							_recordsFile.read(reinterpret_cast<char*>(&record.scaleX), sizeof(double));
							if (_recordsFile.eof())
							{
								LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
								this->terminate = true;
								return;
							}
							else
							{
								LOG "modeller\tmodel: opened records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
							}								
						}
						_recordsFile.read(reinterpret_cast<char*>(&record.scaleY), sizeof(double));
						_recordsFile.read(reinterpret_cast<char*>(&record.centreX), sizeof(double));
						_recordsFile.read(reinterpret_cast<char*>(&record.centreY), sizeof(double));
						_recordsFile.read(reinterpret_cast<char*>(&record.sizeX), sizeof(std::size_t));
						_recordsFile.read(reinterpret_cast<char*>(&record.sizeY), sizeof(std::size_t));
						record.arr->resize(record.sizeX*record.sizeY);
						_recordsFile.read(reinterpret_cast<char*>((char*)record.arr->data()), record.sizeX*record.sizeY);
					}
					catch (const std::exception&)
					{
						LOG "modeller\terror: failed to read records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
						this->terminate = true;
						return;
					}
				}
				records.push_back(record);
				auto sizeD = record.sizeY - _size; // should be +1
				double interval = record.scaleY / record.sizeY;	
				auto scale =  interval * _size;	
				std::size_t scaleValue = 0;
				{
					for (auto scaleA : _scales)
					{
						if (scaleA - scale > -0.00001 && scaleA - scale < 0.00001)
							break;
						scaleValue++;
					}
					if (scaleValue >= _scales.size())
					{
						LOG "modeller\terror: failed to determine scale" UNLOG
						this->terminate = true;
						return;
					}					
				}
				std::vector<std::tuple<std::size_t,std::size_t,double,double,std::size_t,std::size_t>> actsPotsCoord(sizeD*sizeD);
				{
					auto& activeA = *_active;
					auto& actor = *this;
					std::lock_guard<std::mutex> guard(activeA.mutex);
					std::vector<std::thread> threads;
					threads.reserve(_threadCount);
					for (std::size_t t = 0; t < _threadCount; t++)
						threads.push_back(std::thread(
							[&actor, &activeA, sizeD, &record, &actsPotsCoord] (int t)
							{
								auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
								auto& sizes = activeA.historySlicesSize;
								auto& lengths = activeA.historySlicesLength;
								auto& fails = activeA.induceSliceFailsSize;
								auto& dr = *activeA.decomp;		
								auto cap = (unsigned char)(actor._updateParameters.mapCapacity);
								auto size = actor._size;
								auto valency = actor._valency;
								auto valencyBalanced = actor._valencyBalanced;
								auto hr = sizesHistoryRepa(actor._scaleValency, valency, size*size);
								auto n = hr->dimension;
								auto vv = hr->vectorVar;
								auto rr = hr->arr;
								rr[n-1] = 0;
								for (std::size_t y = 0, z = 0; y < sizeD; y++)	
									for (std::size_t x = 0; x < sizeD; x++, z++)	
										if (z % actor._threadCount == t)
										{
											Record recordSub(record,size,size,x,y);
											Record recordValent = recordSub.valentFixed(valency,valencyBalanced);
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
											if (ll && ll->size()) slice = ll->back();	
											if (slice && !fails.count(slice))
											{
												actsPotsCoord[z] = std::make_tuple(lengths[slice],sizes[slice],0.0,0.0,x,y);
											}
											else
												actsPotsCoord[z] = std::make_tuple(0,0,0.0,0.0,x,y);	
										}
							}, t));
					for (auto& t : threads)
						t.join();
				}
				if (actsPotsCoord.size())
				{
					std::sort(actsPotsCoord.begin(), actsPotsCoord.end());
					auto& pos = actsPotsCoord.back();
					auto length = std::get<0>(pos);
					auto likelihood = std::get<1>(pos);
					auto posX = std::get<2>(pos);
					auto posY = std::get<3>(pos);	
					auto x = std::get<4>(pos);
					auto y = std::get<5>(pos);
					actsPotsCoordTop.push_back(std::make_tuple(likelihood,length,k,posX,posY,x,y,records.size()-1,scaleValue));
				}		
			}
			std::sort(actsPotsCoordTop.rbegin(), actsPotsCoordTop.rend());
			for (std::size_t k = 0; eventCount < _eventSize && k < actsPotsCoordTop.size(); k++)	
			{
				auto pos = actsPotsCoordTop[k];
				auto likelihood = std::get<0>(pos);
				auto posX = std::get<3>(pos);
				auto posY = std::get<4>(pos);	
				auto x = std::get<5>(pos);
				auto y = std::get<6>(pos);
				auto& record = records[std::get<7>(pos)];
				auto scaleValue = std::get<8>(pos);
				auto scale =  _scales[scaleValue];
				Record recordSub(record,_size,_size,x,y);
				Record recordValent = recordSub.valentFixed(_valency,_valencyBalanced);
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
				auto hr = recordsHistoryRepa(_scaleValency, scaleValue, _valency, recordValent);
				if (!_updateDisable)
					_events->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr),_events->references);	
				this->eventId++;		
				eventCount++;	
			}	
		}
		else if (_mode == "mode013" && _scales.size() && _recordsFile.is_open())
		{
			Record record;
			{
				try
				{
					_recordsFile.read(reinterpret_cast<char*>(&record.scaleX), sizeof(double));
					if (_recordsFile.eof())
					{
						_recordsFile.close();
						_recordsIndex++;
						if (_recordsIndex >= _recordsFileNames.size())
							_recordsIndex = 0;
						_recordsFile.open(_recordsFileNames[_recordsIndex] + ".rec", std::ios::binary);
						if (!_recordsFile.is_open() || _recordsFile.eof())
						{
							LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
							this->terminate = true;
							return;
						}	
						_recordsFile.read(reinterpret_cast<char*>(&record.scaleX), sizeof(double));
						if (_recordsFile.eof())
						{
							LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
							this->terminate = true;
							return;
						}
						else
						{
							LOG "modeller\tmodel: opened records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
						}								
					}
					_recordsFile.read(reinterpret_cast<char*>(&record.scaleY), sizeof(double));
					_recordsFile.read(reinterpret_cast<char*>(&record.centreX), sizeof(double));
					_recordsFile.read(reinterpret_cast<char*>(&record.centreY), sizeof(double));
					_recordsFile.read(reinterpret_cast<char*>(&record.sizeX), sizeof(std::size_t));
					_recordsFile.read(reinterpret_cast<char*>(&record.sizeY), sizeof(std::size_t));
					record.arr->resize(record.sizeX*record.sizeY);
					_recordsFile.read(reinterpret_cast<char*>((char*)record.arr->data()), record.sizeX*record.sizeY);
				}
				catch (const std::exception&)
				{
					LOG "modeller\terror: failed to read records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
					this->terminate = true;
					return;
				}
			}
			double interval = record.scaleY / record.sizeY;	
			auto scale =  interval * _sizeRecords;	
			std::size_t scaleValue = 0;
			{
				for (auto scaleA : _scales)
				{
					if (scaleA - scale > -0.00001 && scaleA - scale < 0.00001)
						break;
					scaleValue++;
				}
				if (scaleValue >= _scales.size())
				{
					LOG "modeller\terror: failed to determine scale" UNLOG
					this->terminate = true;
					return;
				}					
			}			
			Record recordSub(record,_sizeRecords,_sizeRecords,_sizeTile/2,_sizeTile/2);
			Record recordValent = recordSub.valentFixed(_valency,_valencyBalanced);
			for (std::size_t x = 0; x < _sizeRecords/_size; x++)	
				for (std::size_t y = 0; y < _sizeRecords/_size; y++)	
				{
					Record recordTile(recordValent,_size,_size,x*_size,y*_size);
					auto hr = recordsHistoryRepa(_scaleValency, scaleValue, _valency, recordTile);
					if (!_updateDisable)
						_events->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr),_events->references);	
					this->eventId++;		
					eventCount++;	
				}	
		}
		else if (_mode == "mode014" && (_struct=="struct002"  || _struct=="struct005") && _scales.size() && _recordsFile.is_open())
		{
			Record record;
			{
				try
				{
					_recordsFile.read(reinterpret_cast<char*>(&record.scaleX), sizeof(double));
					if (_recordsFile.eof())
					{
						_recordsFile.close();
						_recordsIndex++;
						if (_recordsIndex >= _recordsFileNames.size())
							_recordsIndex = 0;
						_recordsFile.open(_recordsFileNames[_recordsIndex] + ".rec", std::ios::binary);
						if (!_recordsFile.is_open() || _recordsFile.eof())
						{
							LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
							this->terminate = true;
							return;
						}	
						_recordsFile.read(reinterpret_cast<char*>(&record.scaleX), sizeof(double));
						if (_recordsFile.eof())
						{
							LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
							this->terminate = true;
							return;
						}
						else
						{
							LOG "modeller\tmodel: opened records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
						}								
					}
					_recordsFile.read(reinterpret_cast<char*>(&record.scaleY), sizeof(double));
					_recordsFile.read(reinterpret_cast<char*>(&record.centreX), sizeof(double));
					_recordsFile.read(reinterpret_cast<char*>(&record.centreY), sizeof(double));
					_recordsFile.read(reinterpret_cast<char*>(&record.sizeX), sizeof(std::size_t));
					_recordsFile.read(reinterpret_cast<char*>(&record.sizeY), sizeof(std::size_t));
					record.arr->resize(record.sizeX*record.sizeY);
					_recordsFile.read(reinterpret_cast<char*>((char*)record.arr->data()), record.sizeX*record.sizeY);
				}
				catch (const std::exception&)
				{
					LOG "modeller\terror: failed to read records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
					this->terminate = true;
					return;
				}
			}
			double interval = record.scaleY / record.sizeY;	
			auto scale =  interval * _size;	
			std::size_t scaleValue = 0;
			{
				for (auto scaleA : _scales)
				{
					if (scaleA - scale > -0.00001 && scaleA - scale < 0.00001)
						break;
					scaleValue++;
				}
				if (scaleValue >= _scales.size())
				{
					LOG "modeller\terror: failed to determine scale" UNLOG
					this->terminate = true;
					return;
				}					
			}			
			Record recordSub(record,_size,_size,_size/4,_size/4);
			Record recordValent = recordSub.valentFixed(_valency,_valencyBalanced);
			if (!_updateDisable)
			{
				for (std::size_t y = 0, m = 0; y < _level2Size; y++)	
					for (std::size_t x = 0; x < _level2Size; x++, m++)	
					{
						Record recordTile(recordValent,_level1Size,_level1Size,x*_level1Size,y*_level1Size);
						auto hr = recordsHistoryRepa(_scaleValency,scaleValue,_valency,recordTile);
						_level1Events[m]->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr),_level1Events[m]->references);	
					}	
				auto hr = recordsHistoryRepa(_scaleValency, scaleValue, _valency, recordValent);
				_events->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr),_events->references);					
			}
			this->eventId++;		
			eventCount++;	
		}
		else if (_mode == "mode015" && _struct=="struct003" && _scales.size() && _recordsFile.is_open())
		{
			Record record;
			{
				try
				{
					_recordsFile.read(reinterpret_cast<char*>(&record.scaleX), sizeof(double));
					if (_recordsFile.eof())
					{
						_recordsFile.close();
						_recordsIndex++;
						if (_recordsIndex >= _recordsFileNames.size())
							_recordsIndex = 0;
						_recordsFile.open(_recordsFileNames[_recordsIndex] + ".rec", std::ios::binary);
						if (!_recordsFile.is_open() || _recordsFile.eof())
						{
							LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
							this->terminate = true;
							return;
						}	
						_recordsFile.read(reinterpret_cast<char*>(&record.scaleX), sizeof(double));
						if (_recordsFile.eof())
						{
							LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
							this->terminate = true;
							return;
						}
						else
						{
							LOG "modeller\tmodel: opened records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
						}								
					}
					_recordsFile.read(reinterpret_cast<char*>(&record.scaleY), sizeof(double));
					_recordsFile.read(reinterpret_cast<char*>(&record.centreX), sizeof(double));
					_recordsFile.read(reinterpret_cast<char*>(&record.centreY), sizeof(double));
					_recordsFile.read(reinterpret_cast<char*>(&record.sizeX), sizeof(std::size_t));
					_recordsFile.read(reinterpret_cast<char*>(&record.sizeY), sizeof(std::size_t));
					record.arr->resize(record.sizeX*record.sizeY);
					_recordsFile.read(reinterpret_cast<char*>((char*)record.arr->data()), record.sizeX*record.sizeY);
				}
				catch (const std::exception&)
				{
					LOG "modeller\terror: failed to read records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
					this->terminate = true;
					return;
				}
			}
			double interval = record.scaleY / record.sizeY;	
			auto scale =  interval * _sizeRecords;	
			std::size_t scaleValue = 0;
			{
				for (auto scaleA : _scales)
				{
					if (scaleA - scale > -0.00001 && scaleA - scale < 0.00001)
						break;
					scaleValue++;
				}
				if (scaleValue >= _scales.size())
				{
					LOG "modeller\terror: failed to determine scale" UNLOG
					this->terminate = true;
					return;
				}					
			}			
			Record recordSub(record,_sizeRecords,_sizeRecords,_sizeTile/2,_sizeTile/2);
			Record recordValent = recordSub.valentFixed(_valency,_valencyBalanced);
			for (std::size_t x = 0; x < _sizeRecords/_size; x++)	
				for (std::size_t y = 0; y < _sizeRecords/_size; y++)	
				{
					Record recordTile(recordValent,_size,_size,x*_size,y*_size);
					auto hr = recordsHistoryRepa(_scaleValency, scaleValue, _valency, recordTile);
					if (!_updateDisable)
					{
						auto size = recordTile.arr->size();
						auto arr1 =  recordTile.arr->data();
						for (std::size_t m = 0; m < size; m++)	
						{
							std::size_t n = _valencyBits;
							auto ha = std::make_shared<HistorySparseArray>(1,n);
							auto rr = ha->arr;	
							for (std::size_t i = 0; i < n; i++)				
							{
								rr[i] = 65536 + (m << 12) + (i+1 << 8) + (arr1[m] >> n-i-1);
							}
							auto& eventsA = *_active->underlyingEventsSparse[m];
							eventsA.mapIdEvent[this->eventId] = HistorySparseArrayPtrSizePair(ha, eventsA.references);
						}
						_events->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr), _events->references);	
					}
					this->eventId++;		
					eventCount++;	
				}	
		}
		else if (_mode == "mode016" && (_struct=="struct002" || _struct=="struct005" || _struct=="struct006") && _scales.size() && _recordsFile.is_open())
		{
			std::vector<Record> records;
			for (std::size_t k = 0; k < _scanSize; k++)	
			{
                Record record;
				{
					try
					{
						_recordsFile.read(reinterpret_cast<char*>(&record.scaleX), sizeof(double));
						if (_recordsFile.eof())
						{
							_recordsFile.close();
							_recordsIndex++;
							if (_recordsIndex >= _recordsFileNames.size())
								_recordsIndex = 0;
							_recordsFile.open(_recordsFileNames[_recordsIndex] + ".rec", std::ios::binary);
							if (!_recordsFile.is_open() || _recordsFile.eof())
							{
								LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
								this->terminate = true;
								return;
							}	
							_recordsFile.read(reinterpret_cast<char*>(&record.scaleX), sizeof(double));
							if (_recordsFile.eof())
							{
								LOG "modeller\terror: failed to open records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
								this->terminate = true;
								return;
							}
							else
							{
								LOG "modeller\tmodel: opened records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
							}								
						}
						_recordsFile.read(reinterpret_cast<char*>(&record.scaleY), sizeof(double));
						_recordsFile.read(reinterpret_cast<char*>(&record.centreX), sizeof(double));
						_recordsFile.read(reinterpret_cast<char*>(&record.centreY), sizeof(double));
						_recordsFile.read(reinterpret_cast<char*>(&record.sizeX), sizeof(std::size_t));
						_recordsFile.read(reinterpret_cast<char*>(&record.sizeY), sizeof(std::size_t));
						record.arr->resize(record.sizeX*record.sizeY);
						_recordsFile.read(reinterpret_cast<char*>((char*)record.arr->data()), record.sizeX*record.sizeY);
					}
					catch (const std::exception&)
					{
						LOG "modeller\terror: failed to read records file" << _recordsFileNames[_recordsIndex] + ".rec" UNLOG
						this->terminate = true;
						return;
					}
				}
				records.push_back(record.valentFixed(_valency,_valencyBalanced));
			}
			std::vector<std::tuple<std::size_t,std::size_t,std::size_t,std::size_t,std::size_t,std::size_t,std::size_t>> actsPotsCoordTop(records.size());
			{
				auto& activeA = *_active;
				auto& actor = *this;
				std::lock_guard<std::mutex> guard(activeA.mutex);
				std::vector<std::thread> threads;
				threads.reserve(_threadCount);
				for (std::size_t t = 0; t < records.size() && t < _threadCount; t++)
					threads.push_back(std::thread(
						[&actor, &activeA, &records,&actsPotsCoordTop] (int t)
						{
							auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
							auto cap = (unsigned char)(actor._updateParameters.mapCapacity);
							auto& dr = *activeA.decomp;	
							auto& dr1 = *actor._level1Decomp;	
							auto& cv = dr.mapVarParent();
							auto& vi = dr.mapVarInt();
							auto& cv1 = dr1.mapVarParent();
							auto& vi1 = dr1.mapVarInt();
							auto& sizes = activeA.historySlicesSize;
							auto& lengths = activeA.historySlicesLength;
							auto& fails = activeA.induceSliceFailsSize;
							auto size = actor._size;
							auto level1Size = actor._level1Size;
							auto level2Size = actor._level2Size;
							auto sizeScanStep = actor._sizeScanStep;
							auto level1Step = level1Size/sizeScanStep;
							auto valency = actor._valency;
							auto valencyBalanced = actor._valencyBalanced;
							auto scaleValency = actor._scaleValency;
							auto substrateInclude = actor._substrateInclude;
							auto hr = sizesHistoryRepa(scaleValency, valency, size*size);
							auto n = hr->dimension;
							auto vv = hr->vectorVar;
							auto hr1 = sizesHistoryRepa(scaleValency, valency, level1Size*level1Size);
							auto n1 = hr1->dimension;
							auto vv1 = hr1->vectorVar;
							auto& proms = activeA.underlyingsVarsOffset;
							bool isComputed = actor._struct == "struct005";
							std::size_t bits = 0; 
							if (isComputed)
							{
								std::size_t s = valency;
								if (s)
								{
									s--;
									while (s >> bits)
										bits++;
								}								
							}
							for (std::size_t r = 0; r < records.size(); r++)	
								if (r % actor._threadCount == t)
								{
									auto& record = records[r];
									auto sizeX = record.sizeX;
									auto sizeY = record.sizeY;
									double interval = record.scaleY / sizeY;	
									auto scale =  interval * size;	
									std::size_t scaleValue = 0;
									{
										for (auto scaleA : actor._scales)
										{
											if (scaleA - scale > -0.00001 && scaleA - scale < 0.00001)
												break;
											scaleValue++;
										}
										if (scaleValue >= actor._scales.size())
											scaleValue = 0;
									}
									auto& arr1 = *record.arr;	
									auto countX = (sizeX - level1Size)/sizeScanStep;
									auto countY = (sizeY - level1Size)/sizeScanStep;
									std::vector<SizeList> level1Slices(countX*countY);
									for (std::size_t y = 0, z = 0; y < countY; y++)	
									{
										auto y0 = y*sizeScanStep;
										for (std::size_t x = 0; x < countX; x++, z++)
										{
											auto x0 = x*sizeScanStep;
											SizeUCharStructList kk;
											kk.reserve(n1);
											for (std::size_t y1 = 0, i = 0; y1 < level1Size; y1++)
											{
												auto yx1 = (y1 + y0) * sizeX;
												for (std::size_t x1 = 0; x1 < level1Size; x1++, i++)
												{
													auto w = arr1[yx1 + x1 + x0];
													if (isComputed)
													{
														SizeUCharStruct qq;
														qq.uchar = 1;	
														for (int k = bits; k > 0; k--)
														{
															qq.size = 65536 + (vv1[i] << 12) + (k << 8) + (w >> bits-k);
															kk.push_back(qq);
														}
													}											
													else if (w)
													{
														SizeUCharStruct qq;
														qq.uchar = w;	
														qq.size = vv1[i];
														kk.push_back(qq);
													}
												}
											}	
											if (scaleValue)
											{
												SizeUCharStruct qq;
												qq.uchar = scaleValue;	
												qq.size = vv1[n1-1];
												kk.push_back(qq);
											}										
											auto ll = drmul(kk,dr1,cap);	
											if (ll && ll->size() && ll->back()) 
												level1Slices[z] = *ll;
										}
									}	
									auto countX2 = (sizeX - size)/sizeScanStep;
									auto countY2 = (sizeY - size)/sizeScanStep;
									std::vector<std::tuple<std::size_t,std::size_t,std::size_t,std::size_t,std::size_t,std::size_t>> actsPotsCoord(countX2*countY2);
									for (std::size_t y = 0, z = 0; y < countY2; y++)	
									{
										for (std::size_t x = 0; x < countX2; x++, z++)
										{
											SizeUCharStructList jj;
											jj.reserve(n + level2Size*level2Size*20);
											for (std::size_t y1 = 0, m = 0; y1 < level2Size; y1++)	
											{
												auto yx1 = (y1*level1Step + y) * countX;
												for (std::size_t x1 = 0; x1 < level2Size; x1++, m++)	
												{
													auto& path = level1Slices[yx1 + x1*level1Step + x];
													for (auto slice : path)
														if (slice)
														{
															SizeUCharStruct qq;
															qq.uchar = 1;			
															qq.size = slice;
															activeA.varPromote(proms[m], qq.size);
															jj.push_back(qq);
														}
												}
											}	
											{
												SizeUCharStruct qq;
												qq.uchar = scaleValue;	
												qq.size = vv[n-1];
												if (qq.uchar)
													jj.push_back(qq);
											}
											auto ll = drmul(jj,dr,cap);	
											std::size_t slice = 0;
											if (ll && ll->size()) slice = ll->back();	
											if (slice && !fails.count(slice))
											{
												actsPotsCoord[z] = std::make_tuple(lengths[slice],sizes[slice],x*sizeScanStep,y*sizeScanStep,r,slice);
											}
											else
												actsPotsCoord[z] = std::make_tuple(0,0,x*sizeScanStep,y*sizeScanStep,r,slice);
										}
									}
									{
										std::sort(actsPotsCoord.begin(), actsPotsCoord.end());
										auto& pos = actsPotsCoord.back();
										auto length = std::get<0>(pos);
										auto likelihood = std::get<1>(pos);
										auto x = std::get<2>(pos);
										auto y = std::get<3>(pos);
										auto r = std::get<4>(pos);
										auto slice = std::get<5>(pos);
										actsPotsCoordTop[r] = std::make_tuple(likelihood,length,x,y,r,scaleValue,slice);
									}
								}
						}, t));
				for (auto& t : threads)
					t.join();
			}
			std::sort(actsPotsCoordTop.rbegin(), actsPotsCoordTop.rend());
			for (std::size_t k = 0; eventCount < _eventSize && k < actsPotsCoordTop.size(); k++)	
			{
				auto pos = actsPotsCoordTop[k];
				auto x0 = std::get<2>(pos);
				auto y0 = std::get<3>(pos);
				auto& record = records[std::get<4>(pos)];
				auto scaleValue = std::get<5>(pos);
				auto scale = _scales[scaleValue];
				Record recordSub(record,_size,_size,x0,y0);
				if (_entropyMinimum > 0.0 && recordSub.entropy() < _entropyMinimum)
					continue;	
				if (!_updateDisable)
				{
					for (std::size_t y = 0, m = 0; y < _level2Size; y++)	
						for (std::size_t x = 0; x < _level2Size; x++, m++)	
						{
							Record recordTile(recordSub,_level1Size,_level1Size,x*_level1Size,y*_level1Size);
							auto hr = recordsHistoryRepa(_scaleValency,scaleValue,_valency,recordTile);
							_level1Events[m]->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr),_level1Events[m]->references);	
						}	
					if (_struct!="struct006")
					{
						auto hr = recordsHistoryRepa(_scaleValency, scaleValue, _valency, recordSub);
						_events->mapIdEvent[this->eventId] = HistoryRepaPtrSizePair(std::move(hr),_events->references);	
					}
					else
					{
						auto slice = std::get<6>(pos);
						auto& activeA = *_active;
						std::lock_guard<std::mutex> guard(activeA.mutex);
						auto& dr = *activeA.decomp;	
						auto& cv = dr.mapVarParent();
						auto& reps = *_slicesRepresentation;
						auto hr = recordsHistoryRepa(_scaleValency, scaleValue, _valency, recordSub);
						auto n = hr->dimension;
						auto rr = hr->arr;	
						while (true)
						{
							if (!reps.count(slice))
								reps.insert_or_assign(slice, Representation(1.0,1.0,_size,_size));
							auto& rep = reps[slice];
							auto& arr1 = *rep.arr;
							for (size_t i = 0; i < n-1; i++)
								arr1[i] += rr[i];
							rep.count++;
							if (!slice)
								break;
							slice = cv[slice];
						}
					}
				}
				this->eventId++;
				eventCount++;
			}
		}
		if (!_updateDisable)
		{
			if (_struct=="struct002" || _struct=="struct005")
			{
				for (auto& activeA : _level1)
				{
					if (!activeA->update(_updateParameters))
					{
						this->terminate = true;	
						LOG "modeller\tstatus: quitting" UNLOG
						return;
					}
				}
			}
			if (!_active->update(_updateParameters))
			{
				this->terminate = true;	
				LOG "modeller\tstatus: quitting" UNLOG
				return;
			}
			// induce if sync
			if (!_induceParameters.asyncThreadMax && !_active->induce(_induceParameters))
			{
				this->terminate = true;	
				LOG "modeller\tstatus: quitting" UNLOG
				return;
			}				
		}
		// representations
		if (_struct!="struct006")
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
            LOG "modeller\tcheckpointing" UNLOG
			dump();
			_checkpointEvent = this->eventId;
		}		
		if (_eventLogging && (_eventLoggingFactor <= 1 || this->eventId >= _eventIdPrev +  _eventLoggingFactor))
		{
            LOG "modeller\tevent id: " << this->eventId << "\ttime " << ((Sec)(Clock::now() - _mark)).count() << "s" UNLOG
			_eventIdPrev = this->eventId;
		}
	}
	if (_system && !this->terminate && _induceParameters.asyncThreadMax)
	{
		bool waiting = true;
		while (waiting || _active->updateProhibit)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));	
			{
				std::lock_guard<std::mutex> guard(_active->mutex);		
				std::size_t sliceA = 0;
				std::size_t sliceSizeA = 0;	
				for (auto sliceB : _active->induceSlices)
				{
					auto sliceSizeB = _active->historySlicesSetEvent[sliceB].size();
					if (sliceSizeB > sliceSizeA)
					{
						auto it = _active->induceSliceFailsSize.find(sliceB);
						if (it == _active->induceSliceFailsSize.end() 
							|| (it->second < sliceSizeB 
								&& _induceParameters.induceThresholdExceeded(it->second, sliceSizeB)))
						{
							sliceA = sliceB;
							sliceSizeA = sliceSizeB;							
						}
					}
				}
				waiting = sliceSizeA > 0;
			}			
		}
	}
}

