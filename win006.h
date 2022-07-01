#ifndef WIN006_H
#define WIN006_H

#include "dev.h"
#include <QWidget>
#include <QTimer>
#include <QScreen>
#include <QMouseEvent>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class Win006; }
QT_END_NAMESPACE

class Win006 : public QWidget
{
    Q_OBJECT

public:
	Win006(const std::string& config,
           QWidget *parent = nullptr);
    ~Win006();
		
	bool terminate;
	std::size_t eventId;
	
public Q_SLOTS:
    void act();
	
protected:
	void mousePressEvent(QMouseEvent *event) override;
	void keyPressEvent(QKeyEvent *event);

private:
    Ui::Win006 *_ui;
	QScreen *_screen;
	std::vector<QLabel*> _labelRecords;
	std::vector<QLabel*> _labelRecordValents;
	std::vector<QLabel*> _labelRecordSlices;
	std::vector<QLabel*> _labelRecordLikelihoods;
	std::chrono::time_point<std::chrono::high_resolution_clock> _mark;

	std::string _config;
	bool _actLogging;	
	std::size_t _actLoggingFactor;
	bool _actWarning;
    std::chrono::milliseconds _interval;
	std::string _mode;
	bool _modeLogging;
	std::size_t _modeLoggingFactor;
	bool _modeTracing;

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
	std::size_t _induceThresholdInitial;
    std::chrono::milliseconds _induceInterval;
	std::size_t _induceThreadCount;
	bool _induceNot;	
	std::size_t _fudsSize;

    std::unique_ptr<WBOT02::SliceRepresentationUMap> _slicesRepresentation;
	
    int _x;
    int _y;
    int _width;
    int _height;	
	double _centreX;
	double _centreY;
	double _centreRandomX;
	double _centreRandomY;
    std::vector<double> _scales;
    std::vector<std::pair<double,double>> _offsets;
	std::size_t _valency;
	std::size_t _size;
	std::size_t _divisor;
	std::size_t _multiplier;
};
#endif // WIN006_H
