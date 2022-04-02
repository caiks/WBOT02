#ifndef ACTOR_H
#define ACTOR_H

#include "dev.h"

#include <QWidget>

class Actor : public QWidget
{
	Q_OBJECT

public:
	explicit Actor(const std::string&, QWidget *parent = 0);
	~Actor();
	
	bool _terminate;
	
	enum Status {START, WAIT_ODOM, WAIT_SCAN, AHEAD, LEFT, RIGHT, STOP, CRASH};
	Status _status;
	TimePoint _startTimestamp;
	TimePoint _statusTimestamp;

	std::array<double,7> _posePrevious;
	std::array<double,7> _pose;
	TimePoint _poseTimestampPrevious;
	TimePoint _poseTimestamp;
	std::array<double,7> _poseStop;
	TimePoint _poseStopTimestamp;
	double _linearStopMaximum;
	double _linearMaximum;
	double _linearVelocity;
	double _angularStopMaximum;
	double _angularMaximum;
	double _angularMaximumLag;
	double _angularVelocity;
	std::array<double,360> _scan;
	TimePoint _scanTimestampPrevious;
	TimePoint _scanTimestamp;
	Status _actionPrevious;
	std::string _actionManual;
	
	bool _updateLogging;
	
	bool _actLogging;
	std::size_t _actLoggingFactor;
	bool _actWarning;
	std::chrono::milliseconds _actInterval;
	std::string _struct;
	std::string _model;
	std::size_t _induceThreadCount;
	std::chrono::milliseconds _induceInterval;
	std::string _mode;
	
	std::string _goal;
	
	std::shared_ptr<Alignment::ActiveSystem> _system;
	std::shared_ptr<Alignment::ActiveEventsRepa> _events;
	std::vector<std::thread> _threads;
	std::vector<std::shared_ptr<Alignment::Active>> _level1;
	std::size_t _valencyScan;
	std::size_t _valencyDirection;
	std::size_t _level1Count;
	std::vector<std::shared_ptr<Alignment::Active>> _level2;
	std::vector<std::shared_ptr<Alignment::Active>> _level3;
	Alignment::ActiveUpdateParameters _updateParameters;
	Alignment::ActiveInduceParameters _induceParametersLevel1;
	Alignment::ActiveInduceParameters _induceParameters;
	std::size_t _eventId;
	std::size_t _eventIdMax;
	
	std::shared_ptr<Alignment::System> _uu;
	std::shared_ptr<Alignment::SystemRepa> _ur;
	
	std::map<Status, double> _distribution;
	std::map<Status, double> _distributionTurn;
	double _collisionRange;
	std::size_t _collisionFOV;
	bool _collisionRectangular;	
	bool _turnBiasRight;	
	std::size_t _turnBiasFactor;
	double _collisionRangeAngle;
	std::size_t _fudsSize;
	std::unordered_map<std::size_t, Alignment::SizeSet> _slicesSliceSetNext;
	std::unordered_map<std::size_t, Alignment::SizeSet> _slicesSliceSetPrev;
	std::unordered_map<std::size_t, std::size_t> _slicesSize;
	double _configDeviationMax;
	std::map<std::size_t, std::unordered_map<std::size_t, std::size_t>> _locationsSlicesStepCount;
	std::set<std::size_t> _neighbours;
	std::unordered_map<std::size_t, std::map<std::size_t, std::size_t>> _neighboursActionsCount;
	std::set<std::size_t> _neighbourLeasts;
	std::size_t _slicePrevious;
	std::size_t _transistionSuccessCount;
	std::size_t _transistionNullCount;
	double _transistionExpectedSuccessCount;
	std::size_t _transistionCount;
	std::size_t _actCount;
	std::size_t _effectiveCount;
	std::size_t _decidableCount;
	bool _modeLogging;
	std::size_t _modeLoggingFactor;
	bool _modeTracing;
	bool _hitLogging;
	std::map<std::size_t,std::size_t> _goalsAct;
	std::size_t _goalCount;
	std::size_t _hitCount;
	std::size_t _hitLength;
	std::size_t _transitionMax;
	std::size_t _openSlicesMax;
	std::size_t _goalSizeMax;
	bool _setSliceSizeMaxRandom;
	bool _biasIfBlocked;
	bool _randomOverride;
	bool _unusual;
	std::size_t _sliceCount;
	std::size_t _sliceSizeTotal;
	std::size_t _parentSizeTotal;
	std::size_t _positiveCount;
	double _likelihoodTotal;
	double _likelihoodHitTotal;
	double _likelihoodPositiveTotal;
	std::size_t _negativeCount;
	double _likelihoodNegativeTotal;
	std::size_t _level3Model;
	bool _sizeOverride;
	std::size_t _scoresTop;
	bool _sliceCumulative;

private:
	// rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr _publisherCmdVel;

	// rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr _subscriptionScan;
	// rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr _subscriptionOdom;
	// rclcpp::Subscription<std_msgs::msg::String>::SharedPtr _subscriptionGoal;

	// rclcpp::TimerBase::SharedPtr _timerUpdate;
	
	void callbackUpdate();

};

#endif // ACTOR_H
