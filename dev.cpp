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
