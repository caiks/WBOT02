#ifndef WIN008_H
#define WIN008_H

#include "dev.h"
#include <QWidget>
#include <QTimer>
#include <QScreen>
#include <QMediaPlayer>
#include <QVideoWidget>
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
	std::size_t eventId;
	bool gui;
	
public Q_SLOTS:
    void mediaStart();
    void mediaStateChanged(QMediaPlayer::MediaStatus state);
    void capture();
    void act();
	void handleError();

private:
    Ui::Win008 *_ui;
	QScreen *_screen;
	QMediaPlayer* _mediaPlayer;
    QVideoWidget* _videoWidget;
	QLabel* _labelCentre;
	QLabel* _labelEvent;
	QLabel* _labelFuds;
	QLabel* _labelFails;
	std::chrono::time_point<std::chrono::high_resolution_clock> _mark;
	
	QImage _image;

	std::string _config;
	bool _eventLogging;	
	std::size_t _eventLoggingFactor;
	bool _actLogging;	
	std::size_t _actLoggingFactor;
	std::size_t _actCount;
	bool _actWarning;
	bool _actLoggingSlice;	
    std::chrono::milliseconds _interval;
	std::string _mode;
	bool _modeLogging;
	std::size_t _modeLoggingFactor;
	bool _modeTracing;
	bool _updateDisable;
	
	std::string _videoSource;
	std::vector<std::string> _videoSources;
	std::size_t _videoIndex;
	std::size_t _videoStart;	
	std::size_t _videoEnd;	
	bool _isSeekable;
	std::size_t _position;	
	double _playbackRate;

	std::shared_ptr<Alignment::ActiveSystem> _system;
	std::shared_ptr<Alignment::System> _uu;
	std::shared_ptr<Alignment::SystemRepa> _ur;
	
	std::shared_ptr<Alignment::ActiveEventsRepa> _events;
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
	
	std::size_t _recordUniqueSize;
	std::unordered_set<std::size_t> _recordUniqueSet;
	std::queue<std::size_t> _recordUniqueQueue;

};
#endif // WIN008_H
