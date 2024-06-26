﻿#include "dev.h"

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
	double offsetX = (centreX - (scaleX / 2.0)) * w;
	double offsetY = (centreY - (scaleY / 2.0)) * h;
	int pixelsX = std::max((int)intervalX,1);
	int pixelsY = std::max((int)intervalY,1);
	int stepX = (int)(divisorX && pixelsX >= divisorX ? pixelsX / divisorX : 1);
	int stepY = (int)(divisorY && pixelsY >= divisorY ? pixelsY / divisorY : 1);
	int rangeX = divisorX && stepX == 1 ? std::min(pixelsX,(int)divisorX) : pixelsX;
	int rangeY = divisorY && stepY == 1 ? std::min(pixelsY,(int)divisorY) : pixelsY;
	for (std::size_t i = 0; i < sizeX; i++)
		for (std::size_t j = 0; j < sizeY; j++)
		{
			auto x = (int)(intervalX * i + offsetX);
			auto y = (int)(intervalY * j + offsetY);
			if (x >= 0 && x < w && y >= 0 && y < h)
			{
				int count = 0;
				int total = 0;
				auto xp = x + rangeX;
				auto yp = y + rangeY;
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


WBOT02::Record::Record(
	const Record& record,
	std::size_t sizeX1, std::size_t sizeY1,
	std::size_t originX, std::size_t originY) :
	scaleX(record.scaleX), scaleY(record.scaleY), 
	centreX(record.centreX), centreY(record.centreY), 
	sizeX(sizeX1), sizeY(sizeY1) 
{
	sizeX = sizeX ? sizeX : 1;
	sizeY = sizeY ? sizeY : 1;
	arr = std::make_shared<std::vector<unsigned char>>(sizeX*sizeY,no_init_alloc<unsigned char>());
	auto arr1 = arr->data();	
	auto arr2 = record.arr->data();
	auto sizeX2 = record.sizeX;
	auto sizeY2 = record.sizeY;
	if (originX + sizeX <= sizeX2 && originY + sizeY <= sizeY2)
		for (std::size_t j = 0, k = 0; j < sizeY; j++)
		{
			auto jx1 = (j + originY) * sizeX2;
			for (std::size_t i = 0; i < sizeX; i++,k++)
				arr1[k] = arr2[jx1 + i + originX];				
		}
}

std::size_t WBOT02::Record::hash() const
{
	std::size_t hash = 0;
	if (arr)
	{
		hash++;
		auto size = arr->size();
		auto arr1 =  arr->data();
		for (std::size_t j = 0; j < size; j++)
			hash = hash * 13 + (hash >> 60) + arr1[j];
	}
	return hash;
}


double WBOT02::Record::entropy() const
{
	double e = 0.0;
	std::unordered_map<unsigned char,std::size_t> map(256);
	if (arr && arr->size())
	{
		auto size = arr->size();
		auto arr1 =  arr->data();
		for (std::size_t j = 0; j < size; j++)
			map[arr1[j]]++;
		auto z = (double)size;
		for(const auto& p : map) 
		{
			auto a = p.second / z;
			e -= a * log(a);
		}
	}
	return e;
}

std::unique_ptr<ValueList> WBOT02::Record::sorted() const
{
    auto result = std::make_unique<ValueList>();
	if (arr)
	{
		auto& arr1 = *arr;		
		auto& arr2 = *result;	
		arr2.reserve(sizeX*sizeY);
		for (std::size_t j = 0, k = 0; j < sizeY; j++)
			for (std::size_t i = 0; i < sizeX; i++, k++)
				arr2.push_back(std::make_pair(arr1[k],std::make_pair(i,j)));
		std::sort(arr2.begin(), arr2.end());
	}
	return result;
}

Record WBOT02::Record::valentFixed(std::size_t valency, bool balanced) const
{
	Record record(*this);
	if (valency && valency <= 128 && record.arr)
	{
		auto size = arr->size();
		auto arr1 = arr->data();
		record.arr = std::make_shared<std::vector<unsigned char>>(size,no_init_alloc<unsigned char>());
		auto arr2 = record.arr->data();
		if (balanced)
		{
			std::size_t av = 0;
			for (std::size_t j = 0; j < size; j++)
				av += arr1[j];
			av /= size;
			if (av < 127)
			{
				std::size_t d = 127-av;
				for (std::size_t j = 0; j < size; j++)
				{
					std::size_t x = arr1[j] + d;
					if (x<256)
						arr2[j] = x*valency/256;				
					else
						arr2[j] = valency - 1;
				}
			}
			else if (av > 127)
			{
				std::size_t d = av-127;
				for (std::size_t j = 0; j < size; j++)
				{
					std::size_t x = arr1[j];
					if (x>d)
						arr2[j] = (x-d)*valency/256;				
					else
						arr2[j] = 0;
				}
			}
			else
				for (std::size_t j = 0; j < size; j++)
					arr2[j] = arr1[j]*valency/256;
		}
		else
			for (std::size_t j = 0; j < size; j++)
				arr2[j] = arr1[j]*valency/256;
	}
	return record;
}

Record WBOT02::Record::valent(std::size_t valency, std::size_t factor) const
{
	Record record(*this);
	if (valency && record.arr && record.arr->size()/valency)
	{
		auto size = arr->size();
		auto arr1 =  arr->data();
		record.arr = std::make_shared<std::vector<unsigned char>>(size,no_init_alloc<unsigned char>());
		auto arr2 = record.arr->data();
		auto valencyminus = valency-1;
		unsigned char values[256];	
		{
			if (!factor) factor = 1;		
			std::size_t	size3 = size/factor;		
			std::vector<unsigned char> arr3(size3,no_init_alloc<unsigned char>());
			auto arr3p =  arr3.data();
			for (std::size_t j = 0, k = 0; j < size; j += factor, k++)
				arr3p[k] = arr1[j];
			std::sort(arr3.begin(), arr3.end());
			std::size_t zeros = 0;
			while (zeros < size3 && !arr3[zeros]) zeros++;
			std::size_t interval = size3/valency;
			if (zeros == size3)
			{
				for (std::size_t j = 0; j < size; j++)
					arr2[j] = 0;
				return record;
			}
			if (zeros > interval)
			{
				interval = (size3-zeros)/valency;
				interval = interval ? interval : 1;
				zeros--;
			}
			else 
				zeros = 0;
			std::size_t prev = 0;
			std::size_t next = 0;
			for (std::size_t i = 0; i < valencyminus; i++)
			{
				next = arr3[std::min((i+1)*interval+zeros, size3-1)];
				for (std::size_t k = prev; k <= next; k++)			
					values[k] = i;
				prev = next + 1;
			}
			for (std::size_t k = prev; k < 256; k++)			
				values[k] = valencyminus;
		}
		for (std::size_t j = 0; j < size; j++)
			arr2[j] = values[arr1[j]];
	}
	return record;
}

Record WBOT02::Record::valent(std::size_t valency,
	double scaleX1, double scaleY1, 
	double centreX1, double centreY1, 
	std::size_t sizeX1, std::size_t sizeY1, std::size_t originX, std::size_t originY,
	const ValueList& values) const
{
	Record record(scaleX1, scaleY1, centreX1, centreY1, sizeX1, sizeY1);
	if (valency && record.arr && record.arr->size()/valency
		&& originX + sizeX1 <= sizeX && originY + sizeY1 <= sizeY)
	{
		auto& arr1 = *arr;
		auto& arr2 = *record.arr;
		{
			std::size_t termX = originX + sizeX1 - 1;
			std::size_t termY = originY + sizeY1 - 1;
			std::size_t k = 0;
			for (auto pp : values)
				if (pp.second.first >= originX && pp.second.first <= termX
					&& pp.second.second >= originY && pp.second.second <= termY)
				{
					arr2[k] = pp.first;
					k++;
				}	
		}
		std::vector<std::size_t> values(valency-1);	
		{
			auto size = arr2.size();
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
				values[i] = arr2[std::min((i+1)*interval+zeros, size-1)];			
		}
		for (std::size_t j = 0, k = 0; j < sizeY1; j++)
		{
			auto jx1 = (j + originY) * sizeX;
			for (std::size_t i = 0; i < sizeX1; i++, k++)
			{
				auto v = arr1[jx1 + i + originX];
				bool found = false;
				for (std::size_t m = 0; m < valency-1; m++)	
					if (v <= values[m])
					{
						arr2[k] = m;
						found = true;
						break;
					}
				if (!found)
					arr2[k] = valency-1;				
			}
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


SystemSystemRepaTuple WBOT02::recordsSystemSystemRepaTuple(std::size_t scaleValency, std::size_t recordValency, std::size_t recordSize)
{
	auto lluu = listsSystem_u;

	ValSet recordBuckets;
	for (int i = 0; i < recordValency; i++)
		recordBuckets.insert(Value(i));
	ValSet scaleBuckets;
	for (int i = 0; i < scaleValency; i++)
		scaleBuckets.insert(Value(i));
	std::vector<VarValSetPair> ll;
	auto vrecord = std::make_shared<Variable>("record");
	for (std::size_t i = 0; i < recordSize; i++)
		ll.push_back(VarValSetPair(Variable(vrecord, std::make_shared<Variable>((int)i + 1)), recordBuckets));
	ll.push_back(VarValSetPair(Variable("scale"), scaleBuckets));
	auto uu = lluu(ll);
	auto ur = std::make_unique<SystemRepa>();
	auto& mm = ur->listVarSizePair;
	mm.reserve(ll.size());
	for (auto& vww : ll)
		mm.push_back(VarSizePair(std::make_shared<Variable>(vww.first), vww.second.size()));
	return SystemSystemRepaTuple(move(uu), move(ur));
}

std::unique_ptr<HistoryRepa> WBOT02::sizesHistoryRepa(std::size_t scaleValency, std::size_t recordValency, std::size_t recordSize, std::size_t size)
{
	auto hr = make_unique<HistoryRepa>();
	if (!scaleValency || !recordValency || !recordSize || !size)
		return hr;
	auto n = recordSize + 1;
	hr->dimension = n;
	hr->vectorVar = new size_t[n];
	auto vv = hr->vectorVar;
	hr->shape = new size_t[n];
	auto sh = hr->shape;
	size_t z = size;
	hr->size = z;
	hr->evient = true;
	hr->arr = new unsigned char[z*n];
	for (size_t i = 0; i < n; i++)
		vv[i] = i;
	for (size_t i = 0; i < n-1; i++)
		sh[i] = recordValency;
	sh[n-1] = scaleValency;
	return hr;
}

std::unique_ptr<HistoryRepa> WBOT02::recordsHistoryRepa(std::size_t scaleValency, std::size_t scale, std::size_t recordValency, const Record& record)
{
	if (!scaleValency || !recordValency || !record.arr || !record.arr->size())
		return std::unique_ptr<HistoryRepa>();
	auto& arr1 = *record.arr;
	auto hr = sizesHistoryRepa(scaleValency, recordValency, arr1.size());
	auto n = hr->dimension;
	auto rr = hr->arr;
	for (size_t i = 0; i < n-1; i++)
		rr[i] = arr1[i];
	rr[n-1] = (unsigned char)scale;
	return hr;
}

std::unique_ptr<HistoryRepa> WBOT02::recordSubsetsHistoryRepa(
	std::size_t scaleValency, std::size_t scale, std::size_t recordValency, 
	std::size_t sizeX, std::size_t sizeY, std::size_t originX, std::size_t originY,	
	const Record& record)
{
	if (!scaleValency || !recordValency || !record.arr || !record.arr->size()
		|| record.arr->size() != record.sizeX * record.sizeY
		|| originX + sizeX > record.sizeX || originY + sizeY > record.sizeY)
		return std::unique_ptr<HistoryRepa>();
	auto& arr1 = *record.arr;
	auto sizeX1 = record.sizeX;
	auto hr = sizesHistoryRepa(scaleValency, recordValency, sizeX*sizeY);
	auto n = hr->dimension;
	auto rr = hr->arr;
	for (std::size_t j = 0, k = 0; j < sizeY; j++)
	{
		auto jx1 = (j + originY) * sizeX1;
		for (std::size_t i = 0; i < sizeX; i++, k++)
			rr[k] = arr1[jx1 + i + originX];
	}
	rr[n-1] = (unsigned char)scale;
	return hr;
}

WBOT02::Representation::Representation(
	double scaleX1, double scaleY1, 
	std::size_t sizeX1, std::size_t sizeY1) :
	scaleX(scaleX1), scaleY(scaleY1), 
	sizeX(sizeX1), sizeY(sizeY1),
	count(0)
{
	sizeX = sizeX ? sizeX : 1;
	sizeY = sizeY ? sizeY : 1;
	arr = std::make_shared<std::vector<std::size_t>>(sizeX*sizeY);
}

double WBOT02::Representation::entropy() const
{
	double e = 0.0;
	std::unordered_map<std::size_t,std::size_t> map(256);
	if (arr && arr->size())
	{
		auto size = arr->size();
		auto arr1 =  arr->data();
		for (std::size_t j = 0; j < size; j++)
			map[arr1[j]]++;
		auto z = (double)size;
		for(const auto& p : map) 
		{
			auto a = p.second / z;
			e -= a * log(a);
		}
	}
	return e;
}

QImage WBOT02::Representation::image(std::size_t multiplier, std::size_t valency, std::size_t average) const
{
	return image(multiplier, valency, average, SizeSet());
}

QImage WBOT02::Representation::image(std::size_t multiplier, std::size_t valency, std::size_t average, SizeSet highlights) const
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
			if (average > 127 && average < 256)
			{
				std::size_t d = average-127;
				if (v+d<256)
					v += d;
				else
					v = 255;
			}
			else if (average < 127)
			{
				std::size_t d = 127-average;
				if (v>d)
					v -= d;
				else
					v = 0;				
			}
			auto i = (k % sizeX) * multiplier;
			auto j = (k / sizeX) * multiplier;
			auto rgb = highlights.count(k) ? qRgb(255,0,0) : qRgb(v,v,v);
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

void WBOT02::representationsPersistent(Representation& r, std::ostream& out)
{
	out.write(reinterpret_cast<char*>(&r.scaleX), sizeof(double));
	out.write(reinterpret_cast<char*>(&r.scaleY), sizeof(double));
	out.write(reinterpret_cast<char*>(&r.sizeX), sizeof(std::size_t));
	out.write(reinterpret_cast<char*>(&r.sizeY), sizeof(std::size_t));
	out.write(reinterpret_cast<char*>(&r.count), sizeof(std::size_t));
	out.write(reinterpret_cast<char*>((char*)r.arr->data()), r.sizeX*r.sizeY*sizeof(std::size_t));
}

Representation WBOT02::persistentsRepresentation(std::istream& in)
{
    Representation r;
    in.read(reinterpret_cast<char*>(&r.scaleX), sizeof(double));
    in.read(reinterpret_cast<char*>(&r.scaleY), sizeof(double));
    in.read(reinterpret_cast<char*>(&r.sizeX), sizeof(std::size_t));
    in.read(reinterpret_cast<char*>(&r.sizeY), sizeof(std::size_t));
    in.read(reinterpret_cast<char*>(&r.count), sizeof(std::size_t));
	r.arr = std::make_shared<std::vector<std::size_t>>(r.sizeX*r.sizeY);
	in.read(reinterpret_cast<char*>((char*)r.arr->data()), r.sizeX*r.sizeY*sizeof(std::size_t));
    return r;
}

void WBOT02::sliceRepresentationUMapsPersistent(SliceRepresentationUMap& rr, std::ostream& out)
{
	for (auto& r : rr)
	{
		out.write(reinterpret_cast<char*>((std::size_t*)&r.first), sizeof(std::size_t));
		representationsPersistent(r.second, out);
	}
}


std::unique_ptr<SliceRepresentationUMap> WBOT02::persistentsSliceRepresentationUMap(std::istream& in)
{
	auto rr = std::make_unique<SliceRepresentationUMap>();
	while (true)
	{
		std::size_t slice;
		in.read(reinterpret_cast<char*>(&slice), sizeof(std::size_t));
		if (in.eof())
			break;
		Representation rep = persistentsRepresentation(in);
		rr->insert_or_assign(slice,rep);
	}
	return rr;
}

std::ostream& operator<<(std::ostream& out, const Representation& r)
{
	out << "(" 
		<< r.scaleX << "," << r.scaleY << "," 
		<< r.sizeX << "," << r.sizeY  << ",";
	for (std::size_t h = 0; h < r.sizeX*r.sizeY; h++)	
		out << (h ? "," : "[") << (int)(*r.arr)[h];
	out << "])";
	return out;
}

std::ostream& operator<<(std::ostream& out, const SliceRepresentationUMap& rr)
{
	for (auto& r : rr)
		out << "(" << r.first  << ","  << r.second  << ")" << std::endl;
	return out;
}
