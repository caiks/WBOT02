#ifndef DEV_H
#define DEV_H

#include "AlignmentUtil.h"
#include "Alignment.h"
#include "AlignmentApprox.h"
#include "AlignmentAeson.h"
#include "AlignmentRepa.h"
#include "AlignmentAesonRepa.h"
#include "AlignmentRandomRepa.h"
#include "AlignmentPracticableRepa.h"
#include "AlignmentPracticableIORepa.h"
#include "AlignmentActive.h"

#include <QImage>

#include <iomanip>
#include <set>
#include <unordered_set>
#include <vector>
#include <array>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <thread>
#include <chrono>
#include <ctime>
#include <string>
#include <cmath>

using Sec = std::chrono::duration<double>;
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

#define ECHO(x) std::cout << #x << std::endl; x
#define STARTT auto mark = Clock::now()
#define ECHOT(x) std::cout << #x ; mark = Clock::now(); x ; std::cout << "\t" << ((Sec)(Clock::now() - mark)).count() << "s" << std::endl
#define EVAL(x) std::cout << #x << ": " << (x) << std::endl
#define EVALH(x) std::cout << #x << ": " << std::hex << (x) << std::dec << std::endl
#define EVALL(x) std::cout << #x << ": " << std::endl << (x) << std::endl
#define TRUTH(x) std::cout << #x << ": " << ((x) ? "true" : "false") << std::endl


namespace WBOT02
{
	struct Record
	{
		Record(
			double scaleX1 = 1.0, double scaleY1 = 1.0, 
			double centreX1 = 0.5, double centreY1 = 0.5, 
			std::size_t sizeX1 = 40, std::size_t sizeY1 = 30);
			
		Record(QImage image, 
			double scaleX1 = 1.0, double scaleY1 = 1.0, 
			double centreX1 = 0.5, double centreY1 = 0.5, 
			std::size_t sizeX1 = 40, std::size_t sizeY1 = 30,
			std::size_t divisorX = 0, std::size_t divisorY = 0);
			
		Record valent(std::size_t valency) const;
		QImage image(std::size_t multiplier = 1, std::size_t valency = 0) const;
		
		double scaleX;
		double scaleY;
		double centreX;
		double centreY;
		std::size_t sizeX;
		std::size_t sizeY;
		std::shared_ptr<std::vector<unsigned char>> arr;
	};
	typedef std::vector<Record> RecordList;
	
	void recordsPersistent(Record&, std::ostream&);

	void recordListsPersistent(RecordList&, std::ostream&);

	std::unique_ptr<RecordList> persistentsRecordList(std::istream&);

	// typedef std::tuple<std::unique_ptr<Alignment::System>, std::unique_ptr<Alignment::SystemRepa>, std::unique_ptr<Alignment::HistoryRepa>> SystemHistoryRepaTuple;

	// SystemHistoryRepaTuple posesScansHistoryRepa(int, const std::array<double,7>&, const std::array<double,360>&);
	
	struct Representation
	{
		Representation(
			double scaleX1 = 1.0, double scaleY1 = 1.0, 
			std::size_t sizeX1 = 40, std::size_t sizeY1 = 30);
			
		QImage image(std::size_t multiplier = 1, std::size_t valency = 0) const;
		void add(const Record& record);
		
		double scaleX;
		double scaleY;
		std::size_t sizeX;
		std::size_t sizeY;
		std::size_t count;
		std::shared_ptr<std::vector<std::size_t>> arr;
	};
}

std::ostream& operator<<(std::ostream& out, const WBOT02::Record&);
std::ostream& operator<<(std::ostream& out, const WBOT02::RecordList&);
std::ostream& operator<<(std::ostream& out, std::istream&);



#endif
