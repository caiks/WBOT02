#ifndef WIN008_H
#define WIN008_H

#include "dev.h"
#include <QWidget>
#include <QTimer>
#include <QScreen>
#include <QMouseEvent>
#include <QLabel>
#include <QPainter>

QT_BEGIN_NAMESPACE
namespace Ui { class Win008; }
QT_END_NAMESPACE

class Win008 : public QWidget
{
    Q_OBJECT

public:
	Win008(const std::string& config,
           QWidget *parent = nullptr);
    ~Win008();
		
	bool terminate;
	bool captured;
	std::size_t eventId;
	
public Q_SLOTS:
    void capture();
    void act();
	
protected:
	void mousePressEvent(QMouseEvent *event) override;
	void keyPressEvent(QKeyEvent *event);

private:
    Ui::Win008 *_ui;
	QScreen *_screen;
	std::vector<QLabel*> _labelRecords;
	QLabel* _labelRecordLikelihood;
	std::size_t _labelSize;
	std::vector<QLabel*> _labelRecordSiblings;
	std::vector<QLabel*> _labelRecordSiblingLikelihoods;
	std::vector<QLabel*> _labelRecordAncestors;
	std::vector<QLabel*> _labelRecordAncestorLikelihoods;
	QLabel* _labelCentre;
	QLabel* _labelEvent;
	QLabel* _labelFuds;
	QLabel* _labelActs;
	QLabel* _labelFails;
	QLabel* _labelMotion;
	QPixmap _pixmapBlank;
	std::chrono::time_point<std::chrono::high_resolution_clock> _mark;

	std::string _config;
	bool _eventLogging;	
	std::size_t _eventLoggingFactor;
	bool _actLogging;	
	std::size_t _actLoggingFactor;
	std::size_t _actCount;
	bool _actWarning;
	bool _actLoggingSlice;	
    std::chrono::milliseconds _interval;
	std::size_t _motionThreshold;
	std::size_t _motionCount;
	std::size_t _motionHashStep;
	std::size_t _motionHash;	
	bool _motionWaiting;
	std::string _mode;
	bool _modeLogging;
	std::size_t _modeLoggingFactor;
	bool _modeTracing;
	bool _interactive;
	bool _updateDisable;
	
	QImage _image;

	std::shared_ptr<Alignment::ActiveSystem> _system;
	std::shared_ptr<Alignment::System> _uu;
	std::shared_ptr<Alignment::SystemRepa> _ur;
	
	std::shared_ptr<Alignment::ActiveEventsRepa> _events;
	std::vector<std::thread> _threads;
	std::shared_ptr<Alignment::Active> _active;
	
	std::size_t _eventIdMax;	
	
	std::string _model;
	std::string _modelInitial;

	bool _activeLogging;
	bool _activeSummary;
	std::size_t _activeSize;
	Alignment::ActiveUpdateParameters _updateParameters;
	Alignment::ActiveInduceParameters _induceParameters;
	std::size_t _induceThreshold;
    std::chrono::milliseconds _induceInterval;
	std::size_t _induceThreadCount;
	std::size_t _fudsSize;	
	std::size_t _failCount;

    std::unique_ptr<WBOT02::SliceRepresentationUMap> _slicesRepresentation;
	
    int _captureX;
    int _captureY;
    int _captureWidth;
    int _captureHeight;	
	double _centreX;
	double _centreY;
	int _eventSize;	
	int _scanSize;	
	double _centreRandomX;
	double _centreRandomY;
	double _centreRangeX;
	double _centreRangeY;
    double _scale;
	std::size_t _scaleValency;
	std::size_t _valency;
	std::size_t _valencyFactor;
	bool _valencyFixed;
	std::size_t _size;
	std::size_t _sizeTile;
	std::size_t _divisor;
	std::size_t _multiplier;
	std::size_t _threadCount;
	double _separation;
	std::size_t _eventIdPrev;

};
#endif // WIN008_H