#include "dev.h"

#include <stdlib.h>
#include <algorithm>
#include <random>

using namespace Alignment;
using namespace WBOT02;
using namespace std;

WBOT02::Record::Record(
	double scaleX1, double scaleY1, 
	double centreX1, double centreY1, 
	std::size_t sizeX1, std::size_t sizeY1) :
	scaleX(scaleX1), scaleY(scaleY1), 
	centreX(centreX1), centreY(centreY1), 
	sizeX(sizeX1), sizeY(sizeY1) 
{
	sizeX = sizeX ? sizeX : 1;
	sizeY = sizeY ? sizeY : 1;
	arr = std::make_shared<std::vector<unsigned char>>(sizeX*sizeY);
}

WBOT02::Record::Record(QImage image, 
	double scaleX1, double scaleY1, 
	double centreX1, double centreY1, 
	std::size_t sizeX1, std::size_t sizeY1,
	std::size_t divisorX, std::size_t divisorY) :
	scaleX(scaleX1), scaleY(scaleY1), 
	centreX(centreX1), centreY(centreY1), 
	sizeX(sizeX1), sizeY(sizeY1) 
{
	sizeX = sizeX ? sizeX : 1;
	sizeY = sizeY ? sizeY : 1;
	arr = std::make_shared<std::vector<unsigned char>>(sizeX*sizeY);
	auto w = image.width();
	auto h = image.height();
	double intervalX = scaleX * w / sizeX;
	double intervalY = scaleY * h / sizeY;
	int pixelsX = std::max((int)intervalX,1);
	int pixelsY = std::max((int)intervalY,1);
	int stepX = (int)(divisorX && pixelsX >= divisorX ? pixelsX / divisorX : 1);
	int stepY = (int)(divisorY && pixelsY >= divisorY ? pixelsY / divisorY : 1);
	double offsetX = (centreX - (scaleX / 2.0)) * w;
	double offsetY = (centreY - (scaleY / 2.0)) * h;
	for (std::size_t i = 0; i < sizeX; i++)
		for (std::size_t j = 0; j < sizeY; j++)
		{
			auto x = (int)(intervalX * i + offsetX);
			auto y = (int)(intervalY * j + offsetY);
			if (x >= 0 && x < w && y >= 0 && y < h)
			{
				int count = 0;
				int total = 0;
				auto xp = x + (divisorX && stepX == 1 ? divisorX : pixelsX);
				auto yp = y + (divisorY && stepY == 1 ? divisorY : pixelsY);
				for (int x1 = x; x1 < xp && x1 < w; x1 += stepX)
					for (int y1 = y; y1 < yp && y1 < h; y1 += stepY)
					{
						auto rgb = image.pixel(x1,y1);
						total += std::max(std::max(qRed(rgb),qGreen(rgb)),qBlue(rgb));
						count++;
					}
				if (count)
					(*arr)[j*sizeX + i] = total/count;
			}
		}
}

Record WBOT02::Record::valent(std::size_t valency) const
{
	Record record(*this);
	if (valency && record.arr && record.arr->size()/valency)
	{
		auto& arr1 = *arr;
		record.arr = std::make_shared<std::vector<unsigned char>>(arr1);
		auto& arr2 = *record.arr;
		std::sort(arr2.begin(), arr2.end());
		std::vector<std::size_t> values(valency-1);	
		auto size = arr1.size();
		std::size_t zeros = 0;
		while (zeros < size && !arr2[zeros]) zeros++;
		std::size_t interval = size/valency;
		if (zeros == size)
			return record;
		if (zeros > interval)
		{
			interval = (size-zeros)/valency;
			interval = interval ? interval : 1;
			zeros--;
		}
		else 
			zeros = 0;
		for (std::size_t i = 0; i < valency-1; i++)
			values[i] = arr2[std::min(i*interval+zeros, size-1)];
		for (std::size_t j = 0; j < size; j++)
		{
			auto v = arr1[j];
			bool found = false;
			for (std::size_t i = 0; i < valency-1; i++)	
				if (v <= values[i])
				{
					arr2[j] = i;
					found = true;
					break;
				}
			if (!found)
				arr2[j] = valency-1;
		}
	}
	return record;
}


QImage WBOT02::Record::image(std::size_t multiplier, std::size_t valency) const
{
	QImage image(sizeX*multiplier, sizeY*multiplier, QImage::Format_RGB32);
	if (arr && multiplier)
	{
		auto& arr1 = *arr;
		auto size = arr1.size();
		std::size_t factor = valency ? 256/valency : 1;
		std::size_t offset = factor/2 ;
		for (std::size_t k = 0; k < size; k++)
		{
			auto v = arr1[k] * factor + offset;
			auto i = (k % sizeX) * multiplier;
			auto j = (k / sizeX) * multiplier;
			auto rgb = qRgb(v,v,v);
			for (std::size_t di = 0; di < multiplier; di++)
				for (std::size_t dj = 0; dj < multiplier; dj++)
					image.setPixel(i+di, j+dj, rgb);
		}		
	}
	return image;
}


void WBOT02::recordsPersistent(Record& r, std::ostream& out)
{
	out.write(reinterpret_cast<char*>(&r.scaleX), sizeof(double));
	out.write(reinterpret_cast<char*>(&r.scaleY), sizeof(double));
	out.write(reinterpret_cast<char*>(&r.centreX), sizeof(double));
	out.write(reinterpret_cast<char*>(&r.centreY), sizeof(double));
	out.write(reinterpret_cast<char*>(&r.sizeX), sizeof(std::size_t));
	out.write(reinterpret_cast<char*>(&r.sizeY), sizeof(std::size_t));
	out.write(reinterpret_cast<char*>((char*)r.arr->data()), r.sizeX*r.sizeY);
}

void WBOT02::recordListsPersistent(RecordList& rr, std::ostream& out)
{
	for (auto& r : rr)
		recordsPersistent(r, out);
}

std::unique_ptr<RecordList> WBOT02::persistentsRecordList(std::istream& in)
{
	auto rr = std::make_unique<RecordList>();
	while (true)
	{
		Record r;
		in.read(reinterpret_cast<char*>(&r.scaleX), sizeof(double));
		if (in.eof())
			break;
		in.read(reinterpret_cast<char*>(&r.scaleY), sizeof(double));
		in.read(reinterpret_cast<char*>(&r.centreX), sizeof(double));
		in.read(reinterpret_cast<char*>(&r.centreY), sizeof(double));
		in.read(reinterpret_cast<char*>(&r.sizeX), sizeof(std::size_t));
		in.read(reinterpret_cast<char*>(&r.sizeY), sizeof(std::size_t));
		r.arr->resize(r.sizeX*r.sizeY);
		in.read(reinterpret_cast<char*>((char*)r.arr->data()), r.sizeX*r.sizeY);
		rr->push_back(r);
	}
	return rr;
}

std::ostream& operator<<(std::ostream& out, const Record& r)
{
	out << "(" 
		<< r.scaleX << "," << r.scaleY << "," 
		<< r.centreX << "," << r.centreX  << "," 
		<< r.sizeX << "," << r.sizeY  << ",";
	for (std::size_t h = 0; h < r.sizeX*r.sizeY; h++)	
		out << (h ? "," : "[") << (int)(*r.arr)[h];
	out << "])";
	return out;
}

std::ostream& operator<<(std::ostream& out, const RecordList& rr)
{
	for (auto& r : rr)
		out << r << std::endl;
	return out;
}


std::ostream& operator<<(std::ostream& out, std::istream& in)
{
	auto rr  = persistentsRecordList(in);
	out << *rr;
	return out;
}

// typedef std::pair<double, double> Coord;
// typedef std::pair<Coord, Coord> CoordP;

// SystemHistoryRepaTuple WBOT02::posesScansHistoryRepa(int d, const std::array<double,7>& pose, const std::array<double,360>& scan)
// {
	// auto lluu = listsSystem_u;

	// std::size_t n = 360 + 3;
	// std::size_t z = 1;
	// ValSet buckets;
	// for (int i = 0; i < d; i++)
		// buckets.insert(Value(i));
	// ValSet actions;
	// for (int i = 0; i < 3; i++)
		// actions.insert(Value(i));
	// ValSet locations{ Value("door12"), Value("door13"), Value("door14"), Value("door45"), Value("door56"),
		// Value("room1"), Value("room2"), Value("room3"), Value("room4"), Value("room5"), Value("room6") };
	// ValSet states{ Value("fail"), Value("success") };
	// vector<Coord> doors{ Coord(6.2,-0.175), Coord(2.3,4.5), Coord(2.3,0.375), Coord(-5.15,3.1), Coord(-6.325,0.925) };
	// vector<CoordP> rooms{
		// CoordP(Coord(2.3,-0.175),Coord(7.5,5.27)),
		// CoordP(Coord(4.9,-5.275),Coord(7.5,-0.175)),
		// CoordP(Coord(-0.05,0.925),Coord(2.3,5.27)),
		// CoordP(Coord(-5.15,-0.175),Coord(2.3,5.27)),
		// CoordP(Coord(-7.5,0.925),Coord(-5.15,5.27)),
		// CoordP(Coord(-7.5,-3.925),Coord(-5.15,0.925)) };
	// vector<VarValSetPair> ll;
	// auto vscan = std::make_shared<Variable>("scan");
	// for (std::size_t i = 0; i < n - 3; i++)
		// ll.push_back(VarValSetPair(Variable(vscan, std::make_shared<Variable>((int)i + 1)), buckets));
	// ll.push_back(VarValSetPair(Variable("motor"), actions));
	// ll.push_back(VarValSetPair(Variable("status"), states));
	// ll.push_back(VarValSetPair(Variable("location"), locations));
	// auto uu = lluu(ll);
	// auto ur = std::make_unique<SystemRepa>();
	// auto& mm = ur->listVarSizePair;
	// mm.reserve(ll.size());
	// for (auto& vww : ll)
		// mm.push_back(VarSizePair(std::make_shared<Variable>(vww.first), vww.second.size()));
	// auto hr = make_unique<HistoryRepa>();
	// hr->dimension = n;
	// hr->vectorVar = new size_t[n];
	// auto vv = hr->vectorVar;
	// hr->shape = new size_t[n];
	// auto sh = hr->shape;
	// hr->size = z;
	// hr->evient = true;
	// hr->arr = new unsigned char[z*n];
	// auto rr = hr->arr;
	// for (size_t i = 0; i < n; i++)
		// vv[i] = i;
	// for (size_t i = 0; i < n - 3; i++)
		// sh[i] = d;
	// sh[n - 3] = actions.size();
	// sh[n - 2] = states.size();
	// sh[n - 1] = locations.size();
	// double f = (double)d / 4.0;
	// for (size_t i = 0; i < n - 3; i++)
		// rr[i] = (unsigned char)(scan[i] * f);
	// rr[n - 3] = 0;
	// rr[n - 2] = 0;
	// double x = pose[0];
	// double y = pose[1];
	// size_t k = 0;
	// while (k < doors.size())
	// {
		// if ((doors[k].first - x)*(doors[k].first - x) + (doors[k].second - y)*(doors[k].second - y) <= 0.25)
			// break;
		// k++;
	// }
	// while (k >= doors.size() && k < doors.size() + rooms.size())
	// {
		// auto room = rooms[k - doors.size()];
		// if (room.first.first <= x && x <= room.second.first && room.first.second <= y && y <= room.second.second)
			// break;
		// k++;
	// }
	// if (k >= doors.size() + rooms.size())
		// k = 9;
	// rr[n - 1] = (unsigned char)k;
	// hr->transpose();
	// return SystemHistoryRepaTuple(move(uu), move(ur), move(hr));
// }


WBOT02::Representation::Representation(
	double scaleX1, double scaleY1, 
	std::size_t sizeX1, std::size_t sizeY1) :
	scaleX(scaleX1), scaleY(scaleY1), 
	sizeX(sizeX1), sizeY(sizeY1) 
{
	sizeX = sizeX ? sizeX : 1;
	sizeY = sizeY ? sizeY : 1;
	arr = std::make_shared<std::vector<std::size_t>>(sizeX*sizeY);
}

QImage WBOT02::Representation::image(std::size_t multiplier, std::size_t valency) const
{
	QImage image(sizeX*multiplier, sizeY*multiplier, QImage::Format_RGB32);
	if (arr && count && multiplier)
	{
		auto& arr1 = *arr;
		auto size = arr1.size();
		std::size_t factor = valency ? 256/valency : 1;
		std::size_t offset = factor/2 ;
		for (std::size_t k = 0; k < size; k++)
		{
			auto v = arr1[k] * factor / count + offset;
			auto i = (k % sizeX) * multiplier;
			auto j = (k / sizeX) * multiplier;
			auto rgb = qRgb(v,v,v);
			for (std::size_t di = 0; di < multiplier; di++)
				for (std::size_t dj = 0; dj < multiplier; dj++)
					image.setPixel(i+di, j+dj, rgb);
		}		
	}
	return image;
}


void WBOT02::Representation::add(const Record& record)
{
	if (arr && record.arr && arr->size() == record.arr->size())
	{
		auto& arr1 = *arr;
		auto& arr2 = *record.arr;	
		auto size = arr1.size();
		for (std::size_t j = 0; j < size; j++)
			arr1[j] += arr2[j];
		count++;
	}
}
