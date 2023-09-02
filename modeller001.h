#ifndef MODELLER001_H
#define MODELLER001_H

#include "dev.h"

namespace WBOT02
{
	class Modeller001
	{
	public:
		Modeller001(const std::string& config);
		~Modeller001();
			
		bool terminate;
		std::size_t eventId;
		bool gui;
		
	public:
		void model();
		void dump();

	public:
		std::chrono::time_point<std::chrono::high_resolution_clock> _mark;
		
		std::string _config;
		bool _eventLogging;	
		std::size_t _eventLoggingFactor;
		std::string _mode;
		bool _modeLogging;
		std::size_t _modeLoggingFactor;
		bool _modeTracing;
		bool _updateDisable;
		
		bool _checkpointing;	
		std::size_t _checkpointInterval;
		std::size_t _checkpointEvent;
		
		std::vector<std::thread> _threads;
		
		std::shared_ptr<Alignment::ActiveSystem> _system;
		std::shared_ptr<Alignment::System> _uu;
		std::shared_ptr<Alignment::SystemRepa> _ur;
		
		std::shared_ptr<Alignment::ActiveEventsRepa> _events;
		std::vector<std::shared_ptr<Alignment::ActiveEventsRepa>> _level1Events;
		
		std::size_t _eventIdMax;	
		
		std::string _struct;
		std::shared_ptr<Alignment::Active> _active;
		std::string _model;
		std::string _modelInitial;
		std::string _level1Model;
		std::shared_ptr<Alignment::DecompFudSlicedRepa> _level1Decomp;
		std::vector<std::shared_ptr<Alignment::Active>> _level1;

		bool _activeLogging;
		bool _activeSummary;
		bool _level1Logging;
		bool _level1Summary;
		bool _activeContinuous;
		bool _activeCumulative;
		std::size_t _activeSize;
		std::size_t _level1ActiveSize;
		Alignment::ActiveUpdateParameters _updateParameters;
		Alignment::ActiveInduceParameters _induceParameters;
		std::size_t _induceThreshold;
		std::size_t _induceThreadCount;
		std::size_t _fudsSize;	
		std::size_t _failCount;

		std::unique_ptr<WBOT02::SliceRepresentationUMap> _slicesRepresentation;
		
		int _eventSize;	
		int _scanSize;	
		double _scale;
		std::vector<double> _scales;
		std::size_t _scaleValency;
		std::size_t _valency;
		std::size_t _valencyBits;
		std::size_t _valencyFactor;
		bool _valencyFixed;
		bool _valencyBalanced;
		std::size_t _size;
		std::size_t _level1Size;
		std::size_t _level2Size;
		std::size_t _sizeRecords;
		std::size_t _sizeTile;
		std::size_t _threadCount;
		std::size_t _eventIdPrev;
		double _entropyMinimum;
		bool _substrateInclude;
		
		std::size_t _recordUniqueSize;
		std::unordered_set<std::size_t> _recordUniqueSet;
		std::queue<std::size_t> _recordUniqueQueue;
		
		std::vector<std::string> _recordsFileNames;
		std::size_t _recordsIndex;
		std::ifstream _recordsFile;
		std::string _recordsMode;
	};
}
#endif // MODELLER001_H
