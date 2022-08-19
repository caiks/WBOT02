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
#include <algorithm>

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
    typedef std::vector<std::pair<unsigned char,std::pair<std::size_t,std::size_t>>> ValueList;
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
			
		Record(const Record& record,
			std::size_t sizeX1, std::size_t sizeY1,
			std::size_t originX, std::size_t originY);
			
		Record valent(std::size_t valency) const;
		
		std::unique_ptr<ValueList> sorted() const;
		
		Record valent(std::size_t valency,
			double scaleX1, double scaleY1, 
			double centreX1, double centreY1, 
			std::size_t sizeX1, std::size_t sizeY1, std::size_t originX, std::size_t originY,
			const ValueList& values) const;
			
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

	typedef std::tuple<std::unique_ptr<Alignment::System>, std::unique_ptr<Alignment::SystemRepa>> SystemSystemRepaTuple;

	SystemSystemRepaTuple recordsSystemSystemRepaTuple(std::size_t scaleValency, std::size_t recordValency, std::size_t recordSize);
	
	std::unique_ptr<Alignment::HistoryRepa> sizesHistoryRepa(std::size_t scaleValency, std::size_t recordValency, std::size_t recordSize, std::size_t size = 1);
	
	std::unique_ptr<Alignment::HistoryRepa> recordsHistoryRepa(std::size_t scaleValency, std::size_t scale, std::size_t recordValency, const Record& record);
	
	std::unique_ptr<Alignment::HistoryRepa> recordSubsetsHistoryRepa(
		std::size_t scaleValency, std::size_t scale, std::size_t recordValency, 
		std::size_t sizeX, std::size_t sizeY, std::size_t originX, std::size_t originY,
		const Record& record);

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
	
	void representationsPersistent(Representation&, std::ostream&);
	Representation persistentsRepresentation(std::istream&);
	
	typedef std::unordered_map<std::size_t, WBOT02::Representation> SliceRepresentationUMap;
	
	void sliceRepresentationUMapsPersistent(SliceRepresentationUMap&, std::ostream&);
	std::unique_ptr<SliceRepresentationUMap> persistentsSliceRepresentationUMap(std::istream&);

}

std::ostream& operator<<(std::ostream& out, const WBOT02::Record&);
std::ostream& operator<<(std::ostream& out, const WBOT02::RecordList&);
std::ostream& operator<<(std::ostream& out, std::istream&);

std::ostream& operator<<(std::ostream& out, const WBOT02::Representation&);
std::ostream& operator<<(std::ostream& out, const WBOT02::SliceRepresentationUMap&);


#endif
