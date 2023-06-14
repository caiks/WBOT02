#include "dev.h"

#include "modeller001.h"
#include "win001.h"
#include "win002.h"
#include "win003.h"
#include "win004.h"
#include "win005.h"
#include "win006.h"
#include "win007.h"
#include "win008.h"

#include <QApplication>
#include <QLabel>
#include <QScreen>
#include <QPainter>
#include <QVBoxLayout>

#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

using namespace Alignment;
using namespace WBOT02;
using namespace std;
namespace js = rapidjson;

#define ARGS_STRING_DEF(x,y) args.HasMember(#x) && args[#x].IsString() ? args[#x].GetString() : y
#define ARGS_STRING(x) ARGS_STRING_DEF(x,"")
#define ARGS_INT_DEF(x,y) args.HasMember(#x) && args[#x].IsInt() ? args[#x].GetInt() : y
#define ARGS_INT(x) ARGS_INT_DEF(x,0)
#define ARGS_DOUBLE_DEF(x,y) args.HasMember(#x) && args[#x].IsDouble() ? args[#x].GetDouble() : y
#define ARGS_DOUBLE(x) ARGS_DOUBLE_DEF(x,0.0)
#define ARGS_BOOL_DEF(x,y) args.HasMember(#x) && args[#x].IsBool() ? args[#x].GetBool() : y
#define ARGS_BOOL(x) ARGS_BOOL_DEF(x,false)

#define UNLOG  << std::endl; }
#define LOG { std::cout <<

int main(int argc, char *argv[])
{
	if (argc >= 2 && std::string(argv[1]) == "hello")
	{
		cout << "hello" << endl;
	}
	
	if (argc >= 2 && std::string(argv[1]) == "records")
	{
        RecordList rr {Record(0.9,0.8,0.7,0.6,3,2),Record(0.1,0.2,0.3,0.4,2,1)};
        *(rr[0].arr) = std::vector<unsigned char> {0,1,2,3,4,5};
        *(rr[1].arr) = std::vector<unsigned char> {6,7};

		cout << "rr" << endl
			<< rr << endl;

		try 
		{
			std::ofstream out("test.bin", std::ios::binary);
			ECHO(recordListsPersistent(rr, out));
			out.close();
		}
        catch (const std::exception&)
		{
			cout << "recordListsPersistent dump failed" << endl;
		}
		std::unique_ptr<RecordList> rr2;
		try 
		{
			std::ifstream in("test.bin", std::ios::binary);
			ECHO(rr2 = persistentsRecordList(in));
			in.close();
		}
        catch (const std::exception&)
		{
			cout << "recordListsPersistent load failed" << endl;
		}
		cout << "rr2" << endl
			<< *rr2 << endl;

		std::ifstream in2("test.bin", std::ios::binary);
		cout << in2 << endl;
		in2.close();
	}
	
	if (argc >= 2 && std::string(argv[1]) == "representations")
	{
        Representation r1(0.9,0.8,3,2), r2(0.1,0.2,2,1);
        *(r1.arr) = std::vector<std::size_t> {0,1,2,3,4,5};
        *(r2.arr) = std::vector<std::size_t> {6,7};

		EVAL(r1);
		EVAL(r2);
		
		SliceRepresentationUMap rr;
		rr[1] = r1;
		rr[2] = r2;
		
		cout << "rr" << endl
			<< rr << endl;
		
		try 
		{
			std::ofstream out("test.bin", std::ios::binary);
			ECHO(sliceRepresentationUMapsPersistent(rr, out));
			out.close();
		}
        catch (const std::exception&)
		{
			cout << "sliceRepresentationUMapsPersistent dump failed" << endl;
		}
		std::unique_ptr<SliceRepresentationUMap> rr2;
		try 
		{
			std::ifstream in("test.bin", std::ios::binary);
			ECHO(rr2 = persistentsSliceRepresentationUMap(in));
			in.close();
		}
        catch (const std::exception&)
		{
			cout << "persistentsSliceRepresentationUMap load failed" << endl;
		}
		cout << "rr2" << endl
			<< *rr2 << endl;
	}
	
	if (argc >= 3 && (string(argv[1]) == "view_active" || string(argv[1]) == "view_active_concise"))
	{
		bool ok = true;
		string model = string(argv[2]);
	
		EVAL(model);
		
		bool concise = string(argv[1]) == "view_active_concise";
		TRUTH(concise);

		Active activeA;
		activeA.logging = true;		
		if (ok) 
		{
			activeA.historySliceCachingIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			TRUTH(ok);				
		}		
		if (ok)
		{
			EVAL(activeA.name);				
			EVAL(activeA.underlyingEventUpdateds);		
			std::size_t sizeA = activeA.historyOverflow ? activeA.historySize : activeA.historyEvent;				
			EVAL(activeA.historySize);				
			TRUTH(activeA.historyOverflow);				
			EVAL(activeA.historyEvent);				
			EVAL(sizeA);
			TRUTH(activeA.continousIs);				
			EVAL(activeA.continousHistoryEventsEvent);	
			TRUTH(activeA.historySliceCachingIs);				
			TRUTH(activeA.historySliceCumulativeIs);				
			EVAL(activeA.historySlicesSize.size());		
			EVAL(activeA.historySlicesLength.size());		
			EVAL(activeA.historySlicesSlicesSizeNext.size());		
			EVAL(activeA.historySlicesSliceSetPrev.size());				
			{
				std::map<std::size_t, std::size_t> lengthsDist;
				std::vector<std::size_t> lengths;
				double lengthsTotal = 0;
				auto& vi = activeA.decomp->mapVarInt();
				for (auto& pp : activeA.historySlicesLength)
					if (!vi.count(pp.first))
					{
						lengths.push_back(pp.second);
						lengthsTotal += pp.second;
						lengthsDist[pp.second] += 1;
					}
				EVAL(lengthsDist);
				std::size_t lengthsCount = lengths.size();
				EVAL(lengthsCount);
				double lengthsMean = lengthsTotal / lengthsCount;
				EVAL(lengthsMean);
				double lengthsSquare = 0;
				double lengthsCube = 0;
				double lengthsQuad = 0;
				double lengthsQuin = 0;
				for (auto length : lengths)
				{
					lengthsSquare += std::pow((double)length - lengthsMean, 2.0);
					lengthsCube += std::pow((double)length - lengthsMean, 3.0);
					lengthsQuad += std::pow((double)length - lengthsMean, 4.0);
					lengthsQuin += std::pow((double)length - lengthsMean, 5.0);
				}
				double lengthsDeviation =  std::sqrt(lengthsSquare/(lengthsCount-1));
				EVAL(lengthsDeviation);
				double lengthsSkewness =  lengthsCube/lengthsCount/std::pow(lengthsSquare/lengthsCount,1.5);
				EVAL(lengthsSkewness);
				double lengthsKurtosisExcess =  lengthsQuad/lengthsCount/std::pow(lengthsSquare/lengthsCount,2.0) - 3.0;
				EVAL(lengthsKurtosisExcess);
				double lengthsHyperSkewness =  lengthsQuin/lengthsCount/std::pow(lengthsSquare/lengthsCount,2.5);
				EVAL(lengthsHyperSkewness);
			}
			for (auto& hr : activeA.underlyingHistoryRepa)
			{
				EVAL(hr->dimension);				
				EVAL(hr->size);				
				// EVAL(*hr);				
			}
			EVAL(activeA.underlyingHistorySparse.size());
			if (activeA.underlyingHistorySparse.size())
			{
				EVAL(activeA.underlyingHistorySparse.front()->size);
			}
			if (!concise)
			{
				EVAL(sorted(activeA.underlyingSlicesParent));				
			}
			else 
			{
				EVAL(activeA.underlyingSlicesParent.size());				
			}			
			EVAL(activeA.bits);				
			EVAL(activeA.var);				
			EVAL(activeA.varSlice);				
			EVAL(activeA.induceThreshold);				
			EVAL(activeA.induceVarExclusions.size());				
			if (activeA.historySparse) {EVAL(activeA.historySparse->size);}
			if (!concise)
			{
				if (activeA.historySparse) {EVAL(*activeA.historySparse);}				
				EVAL(activeA.historySlicesSetEvent);			
			}	
			else 
			{
				EVAL(activeA.underlyingSlicesParent.size());				
			}			
			EVAL(activeA.historySlicesSetEvent.size());				
			EVAL(activeA.induceSlices);				
			EVAL(activeA.induceSliceFailsSize);				
			EVAL(activeA.frameUnderlyings);				
			EVAL(activeA.frameHistorys);				
			// EVAL(activeA.framesVarsOffset);	
			TRUTH(activeA.underlyingOffsetIs);				
			EVAL(activeA.underlyingsVarsOffset.size());				
			if (activeA.decomp) {EVAL(activeA.decomp->fuds.size());}
			if (activeA.decomp) {EVAL(activeA.decomp->fudRepasSize);}
			if (activeA.decomp) {EVAL((double)activeA.decomp->fuds.size() * activeA.induceThreshold / sizeA);}
			// if (activeA.decomp) 
			// {
				// auto er = dfrer(*activeA.decomp);
				// EVAL(sorted(er->substrate));
			// }
			if (!concise)
			{
				if (activeA.decomp) {EVAL(*activeA.decomp);}
			}	
			else 
			{
				TRUTH(activeA.decomp);	
				{
					auto& dr = *activeA.decomp;		
					std::map<std::size_t, std::size_t> undsDist;
					std::vector<std::size_t> unds;
					double undsTotal = 0;
					for (auto& fr : dr.fuds)
					{
						SizeSet und;
						for (auto& tr : fr.fud)
						{
							auto n = tr->dimension;
							auto vv = tr->vectorVar;
							for (std::size_t i = 0; i < n; i++)
								und.insert(vv[i]);
						}		
						for (auto& tr : fr.fud)
							und.erase(tr->derived);
						unds.push_back(und.size());
						undsTotal += und.size();
						undsDist[und.size()] += 1;
					}
					EVAL(undsDist);
					std::size_t undsCount = unds.size();
					EVAL(undsCount);
					double undsMean = undsTotal / undsCount;
					EVAL(undsMean);
					double undsSquare = 0;
					double undsCube = 0;
					double undsQuad = 0;
					double undsQuin = 0;
					for (auto length : unds)
					{
						undsSquare += std::pow((double)length - undsMean, 2.0);
						undsCube += std::pow((double)length - undsMean, 3.0);
						undsQuad += std::pow((double)length - undsMean, 4.0);
						undsQuin += std::pow((double)length - undsMean, 5.0);
					}
					double undsDeviation =  std::sqrt(undsSquare/(undsCount-1));
					EVAL(undsDeviation);
					double undsSkewness =  undsCube/undsCount/std::pow(undsSquare/undsCount,1.5);
					EVAL(undsSkewness);
					double undsKurtosisExcess =  undsQuad/undsCount/std::pow(undsSquare/undsCount,2.0) - 3.0;
					EVAL(undsKurtosisExcess);
					double undsHyperSkewness =  undsQuin/undsCount/std::pow(undsSquare/undsCount,2.5);
					EVAL(undsHyperSkewness);
				}
			}
		}
	}
		
	if (argc >= 3 && string(argv[1]) == "view_event_lengths_by_scale")
	{
		bool ok = true;
		string model = string(argv[2]);
	
		EVAL(model);
		
		Active activeA;
		activeA.logging = true;		
		if (ok) 
		{
			activeA.historySliceCachingIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			TRUTH(ok);				
		}		
		if (ok)
		{
			EVAL(activeA.name);		
			std::map<std::size_t, std::size_t> scalesTotal;
			std::map<std::size_t, std::map<std::size_t, std::size_t>> scalesLengthsDist;
			auto& hr = *activeA.underlyingHistoryRepa.back();
			auto z = hr.size;
			auto n = hr.dimension;
			auto rr = hr.arr;
			auto& slices = activeA.historySlicesSetEvent;			
			auto& lengths = activeA.historySlicesLength;
			for (auto pp : slices)
			{
				auto slice = pp.first;
				auto length = lengths[slice];
				for (auto j : pp.second)
				{
					if (hr.evient)
					{
						auto scale = rr[j*n + n-1];
						scalesTotal[scale]++;
						scalesLengthsDist[scale][length]++;
					}
					else
					{
						auto scale = rr[(n-1)*z + j];
						scalesTotal[scale]++;
						scalesLengthsDist[scale][length]++;
					}
				}
			}
			for (auto pp : scalesLengthsDist)			
			{
				auto scale = pp.first;
				auto total = scalesTotal[scale];
				auto& lengthsDist = pp.second;
				EVAL(scale);
				EVAL(total);
				EVAL(lengthsDist);
				double mean = 0.0;
				for (auto& qq : lengthsDist)
					mean += qq.first * qq.second;
				mean /= total;
				EVAL(mean);
			}
		}
	}
	
	if (argc >= 3 && string(argv[1]) == "view_decomp")
	{
		bool ok = true;
		int stage = 0;
		string model = string(argv[2]);
		std::size_t depth = argc >= 4 ? atoi(argv[3]) : 5;
		std::size_t wmax = argc >= 5 ? atoi(argv[4]) : 0;
		bool flip = argc >= 6;
		
		Active activeA;
		activeA.logging = true;		
		if (ok) 
		{
			activeA.historySliceCachingIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		std::set<std::size_t> fuds;
		if (ok)
		{
			auto& dr = *activeA.decomp;		
			auto& vi = dr.mapVarInt();			
			auto& cv = dr.mapVarParent();	
			for (auto& pp : activeA.historySlicesLength)
				if (pp.second <= depth)
					fuds.insert(vi[cv[pp.first]]);
			ok = ok && fuds.size() > 0 && *fuds.rbegin() < dr.fuds.size();
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}		
		if (ok)
		{
			auto& dr = *activeA.decomp;		
			auto& sizes = activeA.historySlicesSize;
			for (auto fud : fuds)		
			{
				auto& fs = dr.fuds[fud];
				auto parent = fs.parent;
				double lnwmax = std::log(wmax ? wmax : fs.children.size());
				std::vector<std::pair<double,std::size_t>> slices;
				for (auto slice : fs.children)		
				{
					double likelihood = (std::log(sizes[slice]) - std::log(sizes[parent]) + lnwmax)/lnwmax;	
					slices.push_back(std::make_pair(likelihood,slice));
				}
				std::sort(slices.rbegin(), slices.rend());
				if (flip)
					std::cout << std::setprecision(3) << slices.front().first << "; ";
				std::cout << fud << ", " << activeA.historySlicesLength[parent] << ", " << parent << ", " << fs.children.size();
				for (auto& pp : slices)
				{
					std::cout << "\t(" << std::setprecision(3) << pp.first << ", " << pp.second << ")\t";
				}
				std::cout << std::endl;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}		
	}
		
	if (argc >= 3 && string(argv[1]) == "view_underlying")
	{
		bool ok = true;
		int stage = 0;
		string model = string(argv[2]);
		
		Active activeA;
		activeA.logging = true;		
		if (ok) 
		{
			activeA.historySliceCachingIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		std::set<std::size_t> fuds;
		if (ok)
		{
			auto& dr = *activeA.decomp;	
			std::size_t fud = 1;			
			for (auto& fr : dr.fuds)
			{
				auto parent = fr.parent;
				SizeSet und;
				for (auto& tr : fr.fud)
				{
					auto n = tr->dimension;
					auto vv = tr->vectorVar;
					for (std::size_t i = 0; i < n; i++)
						und.insert(vv[i]);
				}		
				for (auto& tr : fr.fud)
					und.erase(tr->derived);
				std::cout << fud << ", " << activeA.historySlicesLength[parent] << ", ";
				bool first = true;
				for (auto v : und)
				{
					std::cout << (first ? "(" : ", ") << std::hex << v;
					first = false;
				}
				std::cout << std::dec << ")" << endl;
				fud++;
			}
		
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}		
	}
	
	if (argc >= 3 && (string(argv[1]) == "view_sizes" || string(argv[1]) == "view_fractions"))
	{
		bool ok = true;
		int stage = 0;
		string model = string(argv[2]);
		std::size_t depth = argc >= 4 ? atoi(argv[3]) : 5;
		bool isFractions = string(argv[1]) == "view_fractions";
		
		Active activeA;
		activeA.logging = true;		
		if (ok) 
		{
			activeA.historySliceCachingIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		std::set<std::size_t> fuds;
		if (ok)
		{
			auto& dr = *activeA.decomp;		
			auto& vi = dr.mapVarInt();			
			auto& cv = dr.mapVarParent();	
			for (auto& pp : activeA.historySlicesLength)
				if (pp.second <= depth)
					fuds.insert(vi[cv[pp.first]]);
			ok = ok && fuds.size() > 0 && *fuds.rbegin() < dr.fuds.size();
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}		
		if (ok)
		{
			auto& dr = *activeA.decomp;		
			auto& sizes = activeA.historySlicesSize;
			for (auto fud : fuds)		
			{
				auto& fs = dr.fuds[fud];
				auto parent = fs.parent;
				std::vector<std::size_t> sliceSizes;
				std::size_t total = 0;
				for (auto slice : fs.children)		
				{
					auto sizeA = sizes[slice];
					total += sizeA;
					sliceSizes.push_back(sizeA);
				}
				std::sort(sliceSizes.rbegin(), sliceSizes.rend());
				std::cout << fud << ", " << activeA.historySlicesLength[parent] << ", " << sizes[parent]<< ", " << total << ", " << (sizes[parent]==total ? "ok" : "fail");
				double accum = 0.0;
				for (auto sizeA : sliceSizes)
				{
					if (isFractions)
					{
						accum += (double)sizeA / (double)total;
						std::cout << "\t" << std::setprecision(3) << accum * 100.0 << "\t";
					}
					else
						std::cout << "\t" << sizeA << "\t";
				}
				std::cout << std::endl;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}		
	}
	
	if (argc >= 4 && (string(argv[1]) == "resize" || string(argv[1]) == "resize_tidy"))
	{
		bool ok = true;
		int stage = 0;
		bool tidy = string(argv[1]) == "resize_tidy";
		string model = string(argv[2]);
		string model_new = string(argv[3]);
		std::size_t size = argc >= 5 ? atoi(argv[4]) : 1000000;
		
		EVAL(model);
		EVAL(model_new);
		TRUTH(tidy);
		EVAL(size);
		
		Active activeA;
		activeA.logging = true;		
		if (ok) 
		{
			activeA.historySliceCachingIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}
		if (ok)
		{
			ok = ok && size < activeA.historySize;
			ok = ok && activeA.historySparse;
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}	
		if (ok)
		{
			activeA.name = model_new;	
			if (tidy) activeA.continousHistoryEventsEvent.clear();
			if (tidy) activeA.historySlicesSlicesSizeNext.clear();
			if (tidy) activeA.historySlicesSliceSetPrev.clear();
			auto over = activeA.historyOverflow || size < activeA.historyEvent;
			for (auto& hr : activeA.underlyingHistoryRepa)
			{
				auto n = hr->dimension;
				auto arr = new unsigned char[size*n];
				std::memcpy(arr,hr->arr,size*n);
				delete[] hr->arr;
				hr->arr = arr;
				hr->size = size;
			}
			for (auto& hr : activeA.underlyingHistorySparse)
			{
				auto n = hr->capacity;
				auto arr = new std::size_t[size*n];
				std::memcpy(arr,hr->arr,size*n*sizeof(std::size_t));
				delete[] hr->arr;
				hr->arr = arr;
				hr->size = size;
			}
			{
				auto& hr = activeA.historySparse;
				auto n = hr->capacity;
				auto arr = new std::size_t[size*n];
				std::memcpy(arr,hr->arr,size*n*sizeof(std::size_t));
				delete[] hr->arr;
				hr->arr = arr;
				hr->size = size;
			}
			activeA.historySize = size;				
			activeA.historyOverflow = over;		
			if (over)
				activeA.historyEvent = 0;				
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok) 
		{
			ActiveIOParameters ppio;
			ppio.filename = activeA.name+".ac";
			activeA.logging = true;
			ok = ok && activeA.dump(ppio);		
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}
	}
	
	if (argc >= 2 && string(argv[1]) == "rethreshold")
	{
		bool ok = true;
		int stage = 0;

		js::Document args;
		if (ok)
		{
			string config = "rethreshold.json";
			if (argc >= 3) config = string(argv[2]);
			if (ok && !config.empty())
			{
				std::ifstream in;
				try 
				{
					in.open(config);
					js::IStreamWrapper isw(in);
					args.ParseStream(isw);
				}
				catch (const std::exception&) 
				{
					ok = false;
				}	
				if (!args.IsObject())
				{
					ok = false;
				}
			}
			else
			{
				args.Parse("{}");
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}
		
		string model = ARGS_STRING(model);
		string model_initial = ARGS_STRING(model_initial);
		int size = ARGS_INT_DEF(size,40);	
		Alignment::ActiveInduceParameters induceParameters;
		int induceThreadCount = ARGS_INT_DEF(induceThreadCount,4);
		int induceThreshold = ARGS_INT_DEF(induceThreshold,200);
		induceParameters.tint = induceThreadCount;		
		induceParameters.wmax = ARGS_INT_DEF(induceParameters.wmax,18);
		induceParameters.lmax = ARGS_INT_DEF(induceParameters.lmax,8);
		induceParameters.xmax = ARGS_INT_DEF(induceParameters.xmax,128);
		induceParameters.znnmax = 200000.0 * 2.0 * 300.0 * 300.0 * induceThreadCount;
		induceParameters.omax = ARGS_INT_DEF(induceParameters.omax,10);
		induceParameters.bmax = ARGS_INT_DEF(induceParameters.bmax,10*3);
		induceParameters.mmax = ARGS_INT_DEF(induceParameters.mmax,3);
		induceParameters.umax = ARGS_INT_DEF(induceParameters.umax,128);
		induceParameters.pmax = ARGS_INT_DEF(induceParameters.pmax,1);
		induceParameters.mult = ARGS_INT_DEF(induceParameters.mult,1);
		induceParameters.seed = ARGS_INT_DEF(induceParameters.seed,5);	
		induceParameters.diagonalMin = ARGS_DOUBLE_DEF(induceParameters.diagonalMin,6.0);
		if (args.HasMember("induceParameters.induceThresholds"))
		{
			auto& a = args["induceParameters.induceThresholds"];
			if (a.IsArray())
				for (auto& v : a.GetArray())
					if (v.IsInt())
						induceParameters.induceThresholds.insert(v.GetInt());
		}
		else
		{
			induceParameters.induceThresholds = std::set<std::size_t>{200,225,250,300,400,500,800,1000,2000,3000};
		}	

		std::shared_ptr<Alignment::ActiveSystem> systemA = std::make_shared<ActiveSystem>();
		Active activeA;
		std::size_t fudsSize;	
		std::unique_ptr<WBOT02::SliceRepresentationUMap> slicesRepresentation;
		if (ok) 
		{
			try
			{
				std::ifstream in(model_initial + ".rep", std::ios::binary);
				if (in.is_open())
				{
					slicesRepresentation = persistentsSliceRepresentationUMap(in);
					in.close();
				}
				else
				{
					LOG "rethreshold\terror: failed to open slice-representations file" << model_initial + ".rep" UNLOG
					ok = false;
				}
			}
			catch (const std::exception&)
			{
				LOG "rethreshold\terror: failed to read records file" << model_initial + ".rep" UNLOG
				ok = false;
			}	
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}
		if (ok) 
		{
			activeA.system = systemA;
			activeA.logging = true;		
			ActiveIOParameters ppio;
			ppio.filename = model_initial +".ac";
			ok = ok && activeA.load(ppio);
			systemA->block = std::max(systemA->block, activeA.varMax() >> activeA.bits);
			fudsSize = activeA.decomp->fuds.size();
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}
		if (ok)
		{
			ok = ok && induceThreshold > 0 && induceThreshold < activeA.induceThreshold;
			ok = ok && activeA.historySparse;
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}	
		if (ok)
		{
			activeA.name = model;	
			activeA.induceSlices.clear();
			activeA.induceSliceFailsSize.clear();
			activeA.induceThreshold = induceThreshold;				
			{
				auto& hr = activeA.historySparse;
				auto& slev = activeA.historySlicesSetEvent;	
				auto& induces = activeA.induceSlices;		
				for (auto pp : slev)
					if (pp.second.size() >= induceThreshold)
						induces.insert(pp.first);
			}					
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok)
		{
			activeA.logging = false;		
			activeA.summary = true;		
			ok = ok && activeA.induce(induceParameters);					
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok)
		{		
			std::shared_ptr<HistoryRepa> hr = activeA.underlyingHistoryRepa.front();
			auto& slev = activeA.historySlicesSetEvent;
			auto n = hr->dimension;
			auto rr = hr->arr;	
			auto& dr = *activeA.decomp;	
			auto& reps = *slicesRepresentation;
			for (std::size_t i = fudsSize; i < dr.fuds.size(); i++)
			{
				for (auto sliceB : dr.fuds[i].children)
				{
					Representation rep(1.0,1.0,size,size);
					auto& arr1 = *rep.arr;
					if (slev.count(sliceB))
					{
						for (auto j : slev[sliceB])
						{
							auto jn = j*n;
							for (size_t i = 0; i < n-1; i++)
								arr1[i] += rr[jn + i];
							rep.count++;
						}									
						reps.insert_or_assign(sliceB, rep);
					}
				}
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok) 
		{
			ActiveIOParameters ppio;
			ppio.filename = activeA.name+".ac";
			activeA.logging = true;
			activeA.summary = false;		
			ok = ok && activeA.dump(ppio);		
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}
		if (ok) 
		{
			try
			{
				auto mark = Clock::now(); 
				std::ofstream out(activeA.name + ".rep", std::ios::binary);
				sliceRepresentationUMapsPersistent(*slicesRepresentation, out); 
				out.close();
				LOG "rethreshold\tdump\tfile name: " << activeA.name + ".rep" << "\ttime " << ((Sec)(Clock::now() - mark)).count() << "s" UNLOG
			}
			catch (const std::exception&)
			{
				LOG "rethreshold\terror: failed to write slice-representations file" << activeA.name + ".rep" UNLOG
				ok = false;
			}				
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}
	}
	
	if (argc >= 4 && string(argv[1]) == "randomise_records")
	{
		bool ok = true;
		int stage = 0;
		string records = string(argv[2]);
		string records_new = string(argv[3]);
		
		EVAL(records);
		EVAL(records_new);

		std::ifstream recordsFile;
		if (ok) 
		{
			try
			{
				recordsFile.open(records + ".rec", std::ios::binary);
				if (!recordsFile.is_open())
				{
					ok = false;
				}			
			}
			catch (const std::exception&)
			{
				ok = false;
			}	
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}
		std::unique_ptr<RecordList> recordsList;
		if (ok) 
		{
			try
			{
				recordsList = persistentsRecordList(recordsFile);
				recordsFile.close();
			}
			catch (const std::exception&)
			{
				ok = false;
			}	
			ok = ok && recordsList && recordsList->size() > 0;
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}
		if (ok) 
		{
			try
			{
				EVAL(recordsList->size());
				std::mt19937_64 gen64;		
				std::shuffle(recordsList->begin(), recordsList->end(), gen64);
			}
			catch (const std::exception&)
			{
				ok = false;
			}	
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}
		std::ofstream recordsFileNew;
		if (ok) 
		{
			try
			{
				recordsFileNew.open(records_new + ".rec", std::ios::binary);
				if (!recordsFileNew.is_open())
				{
					ok = false;
				}			
			}
			catch (const std::exception&)
			{
				ok = false;
			}	
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}
		if (ok) 
		{
			try
			{
				recordListsPersistent(*recordsList,recordsFileNew);
				recordsFileNew.close();
			}
			catch (const std::exception&)
			{
				ok = false;
			}	
			stage++;
			EVAL(stage);
			TRUTH(ok);		
		}
	}

	if (argc >= 3 && std::string(argv[1]) == "image001")
	{
		auto mark = Clock::now();
		QApplication a(argc, argv);
		QImage image;
		ECHOT(image.load(argv[2]));
		EVAL(image.format());
		EVAL(image.depth());
		EVAL(image.width());
		EVAL(image.height());
		EVAL(image.dotsPerMeterX());
		EVAL(image.dotsPerMeterY());
		ECHO(auto colour = image.pixel(QPoint(0,0)));
        EVALH(colour);
        EVALH(*(std::uint32_t*)image.constBits());
        EVALH(*(QRgb*)image.constBits());
		EVAL(qAlpha(colour));
		EVAL(qRed(colour));
		EVAL(qGreen(colour));
		EVAL(qBlue(colour));
        EVAL(qGray(colour));
        EVAL((qRed(colour)+qGreen(colour)+qBlue(colour))/3);
		// ECHO(image.fill(colour));
		{
			mark = Clock::now();
			std::size_t total = 0;
			std::size_t size = image.sizeInBytes()/4;
			auto rgb = (QRgb*)image.constBits();
			for (std::size_t x = 0; x < size; x++) 
			{
				total += qRed(*rgb) + qGreen(*rgb) + qBlue(*rgb);
				rgb++;
			}
			EVAL(total);
			total /= 3;
			total /= size;
			cout << "average per pixel:" << total << "\t" << ((Sec)(Clock::now() - mark)).count() << "s" << endl;
		}
		{
			mark = Clock::now();
			std::size_t total = 0;
			for (int y = 0; y < image.height(); ++y) 
			{
				QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(y));
				for (int x = 0; x < image.width(); ++x) {
					QRgb &rgb = line[x];
					total += qRed(rgb) + qGreen(rgb) + qBlue(rgb);
				}
			}
			EVAL(total);
			total /= 3;
			total /= image.height();
			total /= image.height();
			cout << "average per pixel:" << total << "\t" << ((Sec)(Clock::now() - mark)).count() << "s" << endl;
		}
		QLabel myLabel;
		ECHOT(auto pixmap = QPixmap::fromImage(image));
		ECHOT(myLabel.setPixmap(pixmap));
		myLabel.show();
		EVAL(a.exec());
	}
	
	if (argc >= 3 && std::string(argv[1]) == "image003")
	{
		auto mark = Clock::now();
		QApplication a(argc, argv);
		std::size_t valency = argc >= 4 ? atoi(argv[3]) : 0;
		double scale = argc >= 5 ? 1.0 / atof(argv[4]) : 1.0;
		double centreX = argc >= 6 ? atof(argv[5]) : 0.5;
		double centreY = argc >= 7 ? atof(argv[6]) : 0.5;
		std::size_t size = argc >= 8 ? atoi(argv[7]) : 40;
		std::size_t divisor = argc >= 9 ? atoi(argv[8]) : 0;
		std::size_t multiplier = argc >= 10 ? atoi(argv[9]) : 120/size;
		QImage imageA;
		ECHOT(imageA.load(argv[2]));
        ECHOT(Record recordA(imageA, scale * imageA.height() / imageA.width(), scale, centreX, centreY, size, size, divisor, divisor));
		// EVAL(recordA);
        ECHOT(Record recordB = recordA.valent(valency));
		// EVAL(recordB);
        ECHOT(QImage imageB = recordB.image(multiplier,valency));
		QLabel myLabel;
		ECHOT(auto pixmap = QPixmap::fromImage(imageB));
		ECHOT(myLabel.setPixmap(pixmap));
		myLabel.show();
		EVAL(a.exec());
	}
	
	if (argc >= 2 && std::string(argv[1]) == "screen001")
	{
		auto mark = Clock::now();
        QGuiApplication application(argc, argv);
		QScreen *screen = QGuiApplication::primaryScreen();
		ECHOT(auto pixmap = screen->grabWindow(0));
		ECHOT(auto image = pixmap.toImage());
		EVAL(image.format());
		EVAL(image.depth());
		EVAL(image.width());
		EVAL(image.height());
		EVAL(image.dotsPerMeterX());
		EVAL(image.dotsPerMeterY());
		ECHO(auto colour = image.pixel(QPoint(0,0)));
        EVALH(colour);
		EVAL(qAlpha(colour));
		EVAL(qRed(colour));
		EVAL(qGreen(colour));
		EVAL(qBlue(colour));
        EVAL(qGray(colour));
        EVAL((qRed(colour)+qGreen(colour)+qBlue(colour))/3);
		{
			mark = Clock::now();
			std::size_t total = 0;
			std::size_t size = image.sizeInBytes()/4;
			auto rgb = (QRgb*)image.constBits();
			for (std::size_t x = 0; x < size; x++) 
			{
				total += qRed(*rgb) + qGreen(*rgb) + qBlue(*rgb);
				rgb++;
			}
			EVAL(total);
			total /= 3;
			total /= size;
			cout << "average per pixel:" << total << "\t" << ((Sec)(Clock::now() - mark)).count() << "s" << endl;
		}
	}
		
    if (argc >= 2 && (std::string(argv[1]) == "camera001" || std::string(argv[1]) == "image002" || std::string(argv[1]) == "win001"))
    {
        QApplication application(argc, argv);
        Win001 win001(argc >= 3 ? atoi(argv[2]) : 1000);
        win001.show();
        EVAL(application.exec());
	}
	
    if (argc >= 2 && (std::string(argv[1]) == "screen002" || std::string(argv[1]) == "win002"))
	{
        QApplication application(argc, argv);
        Win002 win002(argc >= 3 ? atoi(argv[2]) : 1000,
            argc >= 4 ? atoi(argv[3]) : 0,
            argc >= 5 ? atoi(argv[4]) : 0,
            argc >= 6 ? atoi(argv[5]) : -1,
            argc >= 7 ? atoi(argv[6]) : -1);
        win002.show();
        EVAL(application.exec());
	}
	
    if (argc >= 3 && (std::string(argv[1]) == "video001" || std::string(argv[1]) == "win003"))
	{
        QApplication application(argc, argv);

        Win003 win003(QString(argv[2]),
                      argc >= 4 ? atoi(argv[3]) : 1000);
        win003.show();
        EVAL(application.exec());
	}

    if (argc >= 3 && (std::string(argv[1]) == "video002" || std::string(argv[1]) == "win004"))
    {
        QApplication application(argc, argv);

        Win004 win004(QString(argv[2]),
            argc >= 4 ? atoi(argv[3]) : 0,
            argc >= 5 ? atoi(argv[4]) : 0,
            argc >= 6 ? atof(argv[5]) : 0.0);
        win004.show();
        EVAL(application.exec());
    }
	
	if (argc >= 2 && (std::string(argv[1]) == "screen003" || std::string(argv[1]) == "win005"))
	{
        QApplication application(argc, argv);
        Win005 win005(argc >= 3 ? atoi(argv[2]) : 1000,
            argc >= 4 ? atoi(argv[3]) : 0,
            argc >= 5 ? atoi(argv[4]) : 0,
            argc >= 6 ? atoi(argv[5]) : -1,
            argc >= 7 ? atoi(argv[6]) : -1);

        const auto screenSize = QGuiApplication::primaryScreen()->availableGeometry();
        win005.resize(screenSize.width()/2, screenSize.height()*0.95);
        win005.move(screenSize.topLeft());

        win005.show();
        EVAL(application.exec());
	}
	
	if (argc >= 3 && std::string(argv[1]) == "screen004")
	{
        QApplication application(argc, argv);
        Win005 win005(std::string(argv[2]),nullptr);

        const auto screenSize = QGuiApplication::primaryScreen()->availableGeometry();
        win005.resize(screenSize.width()/2, screenSize.height()*0.95);
        win005.move(screenSize.topLeft());

        win005.show();
        EVAL(application.exec());
	}
	
	if (argc >= 2 && (std::string(argv[1]) == "actor001" || std::string(argv[1]) == "win006"))
	{
        QApplication application(argc, argv);
        Win006 win006(std::string(argc >= 3 ? argv[2] : ""),nullptr);

        const auto screenSize = QGuiApplication::primaryScreen()->availableGeometry();
        win006.resize(screenSize.width()/2, screenSize.height()*0.95);
        win006.move(screenSize.topLeft());

        win006.show();
        EVAL(application.exec());
	}
	
	if (argc >= 2 && (std::string(argv[1]) == "actor002" || std::string(argv[1]) == "win007"))
	{
        QApplication application(argc, argv);
        Win007 win007(std::string(argc >= 3 ? argv[2] : ""),nullptr);

        const auto screenSize = QGuiApplication::primaryScreen()->availableGeometry();
        win007.resize(screenSize.width()/2, screenSize.height()*0.95);
        win007.move(screenSize.topLeft());

        win007.show();
        EVAL(application.exec());
	}
	
	if (argc >= 2 && (std::string(argv[1]) == "actor003" || std::string(argv[1]) == "win008"))
	{
        QApplication application(argc, argv);
        Win008 win008(std::string(argc >= 3 ? argv[2] : ""),nullptr);
		if (win008.gui)
			win008.show();
		application.exec();			
	}

	if (argc >= 2 && std::string(argv[1]) == "modeller001")
	{
        Modeller001 modeller001(std::string(argc >= 3 ? argv[2] : ""));
		modeller001.model();			
	}

	if (argc >= 2 
		&& (string(argv[1]) == "generate_contour" 
			|| string(argv[1]) == "generate_contour001"))
	{
		bool ok = true;
		int stage = 0;
		
		js::Document args;
		if (ok)
		{
			string config = "contour.json";
			if (argc >= 3) config = string(argv[2]);
			if (ok && !config.empty())
			{
				std::ifstream in;
				try 
				{
					in.open(config);
					js::IStreamWrapper isw(in);
					args.ParseStream(isw);
				}
				catch (const std::exception&) 
				{
					ok = false;
				}	
				if (!args.IsObject())
				{
					ok = false;
				}
			}
			else
			{
				args.Parse("{}");
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}
		string model = ARGS_STRING(model);
		string inputFilename = ARGS_STRING(input_file);
		string likelihoodFilename = ARGS_STRING(likelihood_file);
		string lengthFilename = ARGS_STRING(length_file);
		double centreX = ARGS_DOUBLE_DEF(centreX,0.5);
		double centreY = ARGS_DOUBLE_DEF(centreY,0.5);
		double centreRangeX = ARGS_DOUBLE_DEF(range_centreX,0.41);
		double centreRangeY = ARGS_DOUBLE_DEF(range_centreY,0.25);
		double scale = ARGS_DOUBLE_DEF(scale,0.5);
		int scaleValency = ARGS_INT_DEF(scale_valency,4);	
		int valency = ARGS_INT_DEF(valency,10);	
		int size = ARGS_INT_DEF(size,40);	
		int divisor = ARGS_INT_DEF(divisor,4);	
		int induceParameters_wmax = ARGS_INT_DEF(induceParameters.wmax,18);
		if (ok)
		{
			ok = ok && model.size();
			ok = ok && inputFilename.size();
			ok = ok && likelihoodFilename.size();
			ok = ok && lengthFilename.size();
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		Active activeA;
		if (ok) 
		{
			activeA.continousIs = true;
			activeA.historySliceCachingIs = true;
			activeA.historySliceCumulativeIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			activeA.historySliceCachingIs = true;
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		QImage image;
		QImage likelihoodImage;
		QImage lengthImage;
		int captureWidth = 0;
		int captureHeight = 0;	
		if (ok)
		{
			QImage imageIn;
			ok = ok && imageIn.load(QString(inputFilename.c_str()));
			EVAL(imageIn.format());
			image = imageIn.convertToFormat(QImage::Format_RGB32);
			EVAL(image.format());
			likelihoodImage = image.copy();
			lengthImage = image.copy();
			captureWidth = image.width();
			EVAL(captureWidth);
			captureHeight = image.height();
			EVAL(captureHeight);
			ok = ok && captureWidth > 0 && captureHeight > 0;
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok)
		{
			auto mark = Clock::now();
			double recordTime = 0.0;
			double recordValentTime = 0.0;
			double repaTime = 0.0;
			double applyTime = 0.0;
			std::size_t applyCount = 0;
			QPainter likelihoodPainter(&likelihoodImage);
			QPainter lengthPainter(&lengthImage);
			QBrush brush;
            brush.setStyle(Qt::SolidPattern);
			// brush.setStyle(Qt::Dense3Pattern);
			auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
			auto cap = (unsigned char)(ActiveUpdateParameters().mapCapacity);
			auto& dr = *activeA.decomp;	
			auto& cv = dr.mapVarParent();
			auto& sizes = activeA.historySlicesSize;
            auto& lengths = activeA.historySlicesLength;
			double lnwmax = std::log(induceParameters_wmax);
			double interval = scale/size;
			std::size_t lengthMax = 0;
			{
				for (auto& pp : lengths)
					lengthMax = std::max(lengthMax,pp.second);
			}	
			for (double y = -centreRangeY; y < centreRangeY; y += interval)	
				for (double x = -centreRangeX; x <centreRangeX; x += interval)	
				{
					auto posX = centreX + (x * captureHeight / captureWidth);
					auto posY = centreY + y;
					mark = Clock::now();
					Record record(image, 
						scale * captureHeight / captureWidth, scale,
						posX, posY, 
						size, size, divisor, divisor);
					recordTime += ((Sec)(Clock::now() - mark)).count();
					mark = Clock::now();
					Record recordValent = record.valent(valency);
					recordValentTime += ((Sec)(Clock::now() - mark)).count();
					mark = Clock::now();
					auto hr = recordsHistoryRepa(scaleValency, 0, valency, recordValent);
					repaTime += ((Sec)(Clock::now() - mark)).count();
					mark = Clock::now();
					auto n = hr->dimension;
					auto vv = hr->vectorVar;
					auto rr = hr->arr;	
					SizeUCharStructList jj;
					jj.reserve(n);
					for (std::size_t i = 0; i < n; i++)
					{
						SizeUCharStruct qq;
						qq.uchar = rr[i];	
						qq.size = vv[i];
						jj.push_back(qq);
					}
					auto ll = drmul(jj,dr,cap);	
					applyTime += ((Sec)(Clock::now() - mark)).count();
					applyCount++;
					std::size_t slice = 0;
					ok = ok && ll && ll->size();
					if (ok) slice = ll->back();		
					// EVAL(x);					
					// EVAL(y);					
					// EVAL(slice);	
					double likelihood = (std::log(sizes[slice]) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax;		
					// EVAL(likelihood);		
					auto length = lengths[slice];
					// EVAL(length);
					QRectF rectangle(posX*captureWidth, posY*captureHeight, 
						interval*captureHeight,interval*captureHeight);
					{
						int brightness = likelihood > 0.0 ? likelihood * 255 : 0;
						brush.setColor(QColor(brightness,brightness,brightness));
						likelihoodPainter.fillRect(rectangle,brush);					
					}
					{
						int brightness = length * 255 / lengthMax;
						brush.setColor(QColor(brightness,brightness,brightness));
						lengthPainter.fillRect(rectangle,brush);					
					}
				}
			EVAL(recordTime);
			EVAL(recordValentTime);
			EVAL(repaTime);
			EVAL(applyTime);
			EVAL(applyCount);
			ok = ok && likelihoodImage.save(QString(likelihoodFilename.c_str()));
			ok = ok && lengthImage.save(QString(lengthFilename.c_str()));
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
	}
	
	if (argc >= 2 && string(argv[1]) == "generate_contour002")
	{
		bool ok = true;
		int stage = 0;
		
		js::Document args;
		if (ok)
		{
			string config = "contour.json";
			if (argc >= 3) config = string(argv[2]);
			if (ok && !config.empty())
			{
				std::ifstream in;
				try 
				{
					in.open(config);
					js::IStreamWrapper isw(in);
					args.ParseStream(isw);
				}
				catch (const std::exception&) 
				{
					ok = false;
				}	
				if (!args.IsObject())
				{
					ok = false;
				}
			}
			else
			{
				args.Parse("{}");
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}
		string model = ARGS_STRING(model);
		string inputFilename = ARGS_STRING(input_file);
		string likelihoodFilename = ARGS_STRING(likelihood_file);
		string lengthFilename = ARGS_STRING(length_file);
		double centreX = ARGS_DOUBLE_DEF(centreX,0.5);
		double centreY = ARGS_DOUBLE_DEF(centreY,0.5);
		double centreRangeX = ARGS_DOUBLE_DEF(range_centreX,0.41);
		double centreRangeY = ARGS_DOUBLE_DEF(range_centreY,0.25);
		double scale = ARGS_DOUBLE_DEF(scale,0.5);
		int scaleValency = ARGS_INT_DEF(scale_valency,4);	
		int valency = ARGS_INT_DEF(valency,10);	
		int size = ARGS_INT_DEF(size,40);	
		int divisor = ARGS_INT_DEF(divisor,4);	
		int induceParameters_wmax = ARGS_INT_DEF(induceParameters.wmax,18);
		if (ok)
		{
			ok = ok && model.size();
			ok = ok && inputFilename.size();
			ok = ok && likelihoodFilename.size();
			ok = ok && lengthFilename.size();
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		Active activeA;
		if (ok) 
		{
			activeA.continousIs = true;
			activeA.historySliceCachingIs = true;
			activeA.historySliceCumulativeIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			activeA.historySliceCachingIs = true;
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		QImage image;
		QImage likelihoodImage;
		QImage lengthImage;
		int captureWidth = 0;
		int captureHeight = 0;	
		if (ok)
		{
			QImage imageIn;
			ok = ok && imageIn.load(QString(inputFilename.c_str()));
			EVAL(imageIn.format());
			image = imageIn.convertToFormat(QImage::Format_RGB32);
			EVAL(image.format());
			likelihoodImage = image.copy();
			lengthImage = image.copy();
			captureWidth = image.width();
			EVAL(captureWidth);
			captureHeight = image.height();
			EVAL(captureHeight);
			ok = ok && captureWidth > 0 && captureHeight > 0;
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok)
		{
			auto mark = Clock::now();
			double recordTime = 0.0;
			double recordValentTime = 0.0;
			double repaTime = 0.0;
			double applyTime = 0.0;
			std::size_t applyCount = 0;
			QPainter likelihoodPainter(&likelihoodImage);
			QPainter lengthPainter(&lengthImage);
			QBrush brush;
            brush.setStyle(Qt::SolidPattern);
			// brush.setStyle(Qt::Dense3Pattern);
			auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
			auto cap = (unsigned char)(ActiveUpdateParameters().mapCapacity);
			auto& dr = *activeA.decomp;	
			auto& cv = dr.mapVarParent();
			auto& sizes = activeA.historySlicesSize;
            auto& lengths = activeA.historySlicesLength;
			double lnwmax = std::log(induceParameters_wmax);
			double interval = scale/size;
			std::size_t lengthMax = 0;
			{
				for (auto& pp : lengths)
					lengthMax = std::max(lengthMax,pp.second);
			}	
			auto scaleX = centreRangeX * 2.0 + scale;
			auto scaleY = centreRangeY * 2.0 + scale;
			auto sizeX = (std::size_t)(scaleX * size / scale);
			auto sizeY = (std::size_t)(scaleY * size / scale);
			mark = Clock::now();
			Record record(image, 
				scaleX * captureHeight / captureWidth, scaleY,
				centreX, centreY, 
				sizeX, sizeY, 
				divisor, divisor);
			recordTime += ((Sec)(Clock::now() - mark)).count();
			mark = Clock::now();
			Record recordValent = record.valent(valency);
			recordValentTime += ((Sec)(Clock::now() - mark)).count();
			for (std::size_t y = 0; y < sizeY - size; y++)	
				for (std::size_t x = 0; x < sizeX - size; x++)	
				{
					auto posX = centreX + (interval * x - (scaleX - scale) / 2.0) * captureHeight / captureWidth;
					auto posY = centreY + interval * y - (scaleY - scale) / 2.0;
					mark = Clock::now();
					auto hr = recordSubsetsHistoryRepa(
						scaleValency, 0, valency, 
						size, size, x, y,
						recordValent);
					repaTime += ((Sec)(Clock::now() - mark)).count();
					mark = Clock::now();
					auto n = hr->dimension;
					auto vv = hr->vectorVar;
					auto rr = hr->arr;	
					SizeUCharStructList jj;
					jj.reserve(n);
					for (std::size_t i = 0; i < n; i++)
					{
						SizeUCharStruct qq;
						qq.uchar = rr[i];	
						qq.size = vv[i];
						jj.push_back(qq);
					}
					auto ll = drmul(jj,dr,cap);	
					applyTime += ((Sec)(Clock::now() - mark)).count();
					applyCount++;
					std::size_t slice = 0;
					ok = ok && ll && ll->size();
					if (ok) slice = ll->back();		
					// EVAL(x);					
					// EVAL(y);					
					// EVAL(slice);	
					double likelihood = (std::log(sizes[slice]) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax;		
					// EVAL(likelihood);		
					auto length = lengths[slice];
					// EVAL(length);
					// EVAL(posX);
					// EVAL(posY);
					QRectF rectangle(posX*captureWidth, posY*captureHeight, 
						interval*captureHeight,interval*captureHeight);
					{
						int brightness = likelihood > 0.0 ? likelihood * 255 : 0;
						brush.setColor(QColor(brightness,brightness,brightness));
						likelihoodPainter.fillRect(rectangle,brush);					
					}
					{
						int brightness = length * 255 / lengthMax;
						brush.setColor(QColor(brightness,brightness,brightness));
						lengthPainter.fillRect(rectangle,brush);					
					}
				}
			EVAL(recordTime);
			EVAL(recordValentTime);
			EVAL(repaTime);
			EVAL(applyTime);
			EVAL(applyCount);
			ok = ok && likelihoodImage.save(QString(likelihoodFilename.c_str()));
			ok = ok && lengthImage.save(QString(lengthFilename.c_str()));
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
	}

	if (argc >= 2 && string(argv[1]) == "generate_contour003")
	{
		bool ok = true;
		int stage = 0;
		
		js::Document args;
		if (ok)
		{
			string config = "contour.json";
			if (argc >= 3) config = string(argv[2]);
			if (ok && !config.empty())
			{
				std::ifstream in;
				try 
				{
					in.open(config);
					js::IStreamWrapper isw(in);
					args.ParseStream(isw);
				}
				catch (const std::exception&) 
				{
					ok = false;
				}	
				if (!args.IsObject())
				{
					ok = false;
				}
			}
			else
			{
				args.Parse("{}");
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}
		string model = ARGS_STRING(model);
		string inputFilename = ARGS_STRING(input_file);
		string likelihoodFilename = ARGS_STRING(likelihood_file);
		string lengthFilename = ARGS_STRING(length_file);
		double centreX = ARGS_DOUBLE_DEF(centreX,0.5);
		double centreY = ARGS_DOUBLE_DEF(centreY,0.5);
		double centreRangeX = ARGS_DOUBLE_DEF(range_centreX,0.41);
		double centreRangeY = ARGS_DOUBLE_DEF(range_centreY,0.25);
		double scale = ARGS_DOUBLE_DEF(scale,0.5);
		int scaleValency = ARGS_INT_DEF(scale_valency,4);	
		int valency = ARGS_INT_DEF(valency,10);	
		int size = ARGS_INT_DEF(size,40);	
		int divisor = ARGS_INT_DEF(divisor,4);	
		int induceParameters_wmax = ARGS_INT_DEF(induceParameters.wmax,18);
		if (ok)
		{
			ok = ok && model.size();
			ok = ok && inputFilename.size();
			ok = ok && likelihoodFilename.size();
			ok = ok && lengthFilename.size();
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		Active activeA;
		if (ok) 
		{
			activeA.continousIs = true;
			activeA.historySliceCachingIs = true;
			activeA.historySliceCumulativeIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			activeA.historySliceCachingIs = true;
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		QImage image;
		QImage likelihoodImage;
		QImage lengthImage;
		int captureWidth = 0;
		int captureHeight = 0;	
		if (ok)
		{
			QImage imageIn;
			ok = ok && imageIn.load(QString(inputFilename.c_str()));
			EVAL(imageIn.format());
			image = imageIn.convertToFormat(QImage::Format_RGB32);
			EVAL(image.format());
			likelihoodImage = image.copy();
			lengthImage = image.copy();
			captureWidth = image.width();
			EVAL(captureWidth);
			captureHeight = image.height();
			EVAL(captureHeight);
			ok = ok && captureWidth > 0 && captureHeight > 0;
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok)
		{
			auto mark = Clock::now();
			double recordTime = 0.0;
			double recordValentTime = 0.0;
			double repaTime = 0.0;
			double applyTime = 0.0;
			std::size_t applyCount = 0;
			QPainter likelihoodPainter(&likelihoodImage);
			QPainter lengthPainter(&lengthImage);
			QBrush brush;
            brush.setStyle(Qt::SolidPattern);
			// brush.setStyle(Qt::Dense3Pattern);
			auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
			auto cap = (unsigned char)(ActiveUpdateParameters().mapCapacity);
			auto& dr = *activeA.decomp;	
			auto& cv = dr.mapVarParent();
			auto& sizes = activeA.historySlicesSize;
            auto& lengths = activeA.historySlicesLength;
			double lnwmax = std::log(induceParameters_wmax);
			double interval = scale/size;
			std::size_t lengthMax = 0;
			{
				for (auto& pp : lengths)
					lengthMax = std::max(lengthMax,pp.second);
			}	
			auto scaleX = centreRangeX * 2.0 + scale;
			auto scaleY = centreRangeY * 2.0 + scale;
			auto sizeX = (std::size_t)(scaleX * size / scale);
			auto sizeY = (std::size_t)(scaleY * size / scale);
			mark = Clock::now();
			Record record(image, 
				scaleX * captureHeight / captureWidth, scaleY,
				centreX, centreY, 
				sizeX, sizeY, 
				divisor, divisor);
			recordTime += ((Sec)(Clock::now() - mark)).count();
			mark = Clock::now();
			Record recordValent = record.valent(valency);
			recordValentTime += ((Sec)(Clock::now() - mark)).count();
			mark = Clock::now();
			auto& arr1 = *recordValent.arr;
			auto hr = sizesHistoryRepa(scaleValency, valency, size*size);
			auto n = hr->dimension;
			auto vv = hr->vectorVar;
			auto rr = hr->arr;
			rr[n-1] = 0;
			std::vector<std::size_t> lengthResults;
			lengthResults.reserve(sizeY*sizeX);
			std::vector<double> likelihoodResults;
			likelihoodResults.reserve(sizeY*sizeX);
			for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
				for (std::size_t x = 0; x < sizeX - size; x++, z++)	
				{
					for (std::size_t j = 0, k = 0; j < size; j++)
					{
						auto jx1 = (j + y) * sizeX;
						for (std::size_t i = 0; i < size; i++, k++)
							rr[k] = arr1[jx1 + i + x];
					}
					SizeUCharStructList jj;
					jj.reserve(n);
					for (std::size_t i = 0; i < n; i++)
					{
						SizeUCharStruct qq;
						qq.uchar = rr[i];	
						qq.size = vv[i];
						jj.push_back(qq);
					}
					auto ll = drmul(jj,dr,cap);	
					applyCount++;
					std::size_t slice = 0;
					ok = ok && ll && ll->size();
					if (ok) slice = ll->back();		
					likelihoodResults.push_back((std::log(sizes[slice]) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax);				
					lengthResults.push_back(lengths[slice]);						
				}
			applyTime += ((Sec)(Clock::now() - mark)).count();
			for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
				for (std::size_t x = 0; x < sizeX - size; x++,z++)	
				{
					auto posX = centreX + (interval * x - (scaleX - scale) / 2.0) * captureHeight / captureWidth;
					auto posY = centreY + interval * y - (scaleY - scale) / 2.0;
					double likelihood = likelihoodResults[z];				
					auto length = lengthResults[z];
					QRectF rectangle(posX*captureWidth, posY*captureHeight, 
						interval*captureHeight,interval*captureHeight);
					{
						int brightness = likelihood > 0.0 ? likelihood * 255 : 0;
						brush.setColor(QColor(brightness,brightness,brightness));
						likelihoodPainter.fillRect(rectangle,brush);					
					}
					{
						int brightness = length * 255 / lengthMax;
						brush.setColor(QColor(brightness,brightness,brightness));
						lengthPainter.fillRect(rectangle,brush);					
					}
				}	
			EVAL(recordTime);
			EVAL(recordValentTime);
			EVAL(applyTime);
			EVAL(applyCount);
			ok = ok && likelihoodImage.save(QString(likelihoodFilename.c_str()));
			ok = ok && lengthImage.save(QString(lengthFilename.c_str()));
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
	}
	
	if (argc >= 2 && string(argv[1]) == "generate_contour004")
	{
		bool ok = true;
		int stage = 0;
		
		js::Document args;
		if (ok)
		{
			string config = "contour.json";
			if (argc >= 3) config = string(argv[2]);
			if (ok && !config.empty())
			{
				std::ifstream in;
				try 
				{
					in.open(config);
					js::IStreamWrapper isw(in);
					args.ParseStream(isw);
				}
				catch (const std::exception&) 
				{
					ok = false;
				}	
				if (!args.IsObject())
				{
					ok = false;
				}
			}
			else
			{
				args.Parse("{}");
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}
		string model = ARGS_STRING(model);
		string inputFilename = ARGS_STRING(input_file);
		string likelihoodFilename = ARGS_STRING(likelihood_file);
		string lengthFilename = ARGS_STRING(length_file);
		double centreX = ARGS_DOUBLE_DEF(centreX,0.5);
		double centreY = ARGS_DOUBLE_DEF(centreY,0.5);
		double centreRangeX = ARGS_DOUBLE_DEF(range_centreX,0.41);
		double centreRangeY = ARGS_DOUBLE_DEF(range_centreY,0.25);
		double scale = ARGS_DOUBLE_DEF(scale,0.5);
		int scaleValency = ARGS_INT_DEF(scale_valency,4);	
		int valency = ARGS_INT_DEF(valency,10);	
		int size = ARGS_INT_DEF(size,40);	
		int divisor = ARGS_INT_DEF(divisor,4);	
		int induceParameters_wmax = ARGS_INT_DEF(induceParameters.wmax,18);
		if (ok)
		{
			ok = ok && model.size();
			ok = ok && inputFilename.size();
			ok = ok && likelihoodFilename.size();
			ok = ok && lengthFilename.size();
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		Active activeA;
		if (ok) 
		{
			activeA.continousIs = true;
			activeA.historySliceCachingIs = true;
			activeA.historySliceCumulativeIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			activeA.historySliceCachingIs = true;
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		QImage image;
		QImage likelihoodImage;
		QImage lengthImage;
		int captureWidth = 0;
		int captureHeight = 0;	
		if (ok)
		{
			QImage imageIn;
			ok = ok && imageIn.load(QString(inputFilename.c_str()));
			EVAL(imageIn.format());
			image = imageIn.convertToFormat(QImage::Format_RGB32);
			EVAL(image.format());
			likelihoodImage = image.copy();
			lengthImage = image.copy();
			captureWidth = image.width();
			EVAL(captureWidth);
			captureHeight = image.height();
			EVAL(captureHeight);
			ok = ok && captureWidth > 0 && captureHeight > 0;
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok)
		{
			auto mark = Clock::now();
			double recordTime = 0.0;
			double recordValentTime = 0.0;
			double repaTime = 0.0;
			double applyTime = 0.0;
			std::size_t applyCount = 0;
			QPainter likelihoodPainter(&likelihoodImage);
			QPainter lengthPainter(&lengthImage);
			QBrush brush;
            brush.setStyle(Qt::SolidPattern);
			// brush.setStyle(Qt::Dense3Pattern);
			auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
			auto cap = (unsigned char)(ActiveUpdateParameters().mapCapacity);
			auto& dr = *activeA.decomp;	
			auto& cv = dr.mapVarParent();
			auto& sizes = activeA.historySlicesSize;
            auto& lengths = activeA.historySlicesLength;
			double lnwmax = std::log(induceParameters_wmax);
			double interval = scale/size;
			std::size_t lengthMax = 0;
			{
				for (auto& pp : lengths)
					lengthMax = std::max(lengthMax,pp.second);
			}	
			auto scaleX = centreRangeX * 2.0 + scale;
			auto scaleY = centreRangeY * 2.0 + scale;
			auto sizeX = (std::size_t)(scaleX * size / scale);
			auto sizeY = (std::size_t)(scaleY * size / scale);
			mark = Clock::now();
			Record record(image, 
				scaleX * captureHeight / captureWidth, scaleY,
				centreX, centreY, 
				sizeX, sizeY, 
				divisor, divisor);
			auto valuesSorted = record.sorted();
			recordTime += ((Sec)(Clock::now() - mark)).count();
			auto hr = sizesHistoryRepa(scaleValency, valency, size*size);
			auto n = hr->dimension;
			auto vv = hr->vectorVar;
			auto rr = hr->arr;
			rr[n-1] = 0;
			std::vector<std::size_t> lengthResults;
			lengthResults.reserve(sizeY*sizeX);
			std::vector<double> likelihoodResults;
			likelihoodResults.reserve(sizeY*sizeX);
			for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
				for (std::size_t x = 0; x < sizeX - size; x++, z++)	
				{
					auto posX = centreX + (interval * x - (scaleX - scale) / 2.0) * captureHeight / captureWidth;
					auto posY = centreY + interval * y - (scaleY - scale) / 2.0;
					mark = Clock::now();
					Record recordValent = record.valent(valency,
						scale,scale,posX,posY,size,size,x,y,*valuesSorted);
					recordValentTime += ((Sec)(Clock::now() - mark)).count();
					mark = Clock::now();
					auto& arr1 = *recordValent.arr;	
					SizeUCharStructList jj;
					jj.reserve(n);
					for (std::size_t i = 0; i < n-1; i++)
					{
						SizeUCharStruct qq;
						qq.uchar = arr1[i];	
						qq.size = vv[i];
						jj.push_back(qq);
					}
					{
						SizeUCharStruct qq;
						qq.uchar = rr[n-1];	
						qq.size = vv[n-1];
						jj.push_back(qq);
					}
					auto ll = drmul(jj,dr,cap);	
					applyTime += ((Sec)(Clock::now() - mark)).count();
					applyCount++;
					std::size_t slice = 0;
					ok = ok && ll && ll->size();
					if (ok) slice = ll->back();		
					likelihoodResults.push_back((std::log(sizes[slice]) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax);				
					lengthResults.push_back(lengths[slice]);						
				}
			for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
				for (std::size_t x = 0; x < sizeX - size; x++,z++)	
				{
					auto posX = centreX + (interval * x - (scaleX - scale) / 2.0) * captureHeight / captureWidth;
					auto posY = centreY + interval * y - (scaleY - scale) / 2.0;
					double likelihood = likelihoodResults[z];				
					auto length = lengthResults[z];
					QRectF rectangle(posX*captureWidth, posY*captureHeight, 
						interval*captureHeight,interval*captureHeight);
					{
						int brightness = likelihood > 0.0 ? likelihood * 255 : 0;
						brush.setColor(QColor(brightness,brightness,brightness));
						likelihoodPainter.fillRect(rectangle,brush);					
					}
					{
						int brightness = length * 255 / lengthMax;
						brush.setColor(QColor(brightness,brightness,brightness));
						lengthPainter.fillRect(rectangle,brush);					
					}
				}	
			EVAL(recordTime);
			EVAL(recordValentTime);
			EVAL(applyTime);
			EVAL(applyCount);			
			EVAL(sizeY*sizeX);
			ok = ok && likelihoodImage.save(QString(likelihoodFilename.c_str()));
			ok = ok && lengthImage.save(QString(lengthFilename.c_str()));
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
	}
		
	if (argc >= 2 && string(argv[1]) == "generate_contour005")
	{
		bool ok = true;
		int stage = 0;
		
		js::Document args;
		if (ok)
		{
			string config = "contour.json";
			if (argc >= 3) config = string(argv[2]);
			if (ok && !config.empty())
			{
				std::ifstream in;
				try 
				{
					in.open(config);
					js::IStreamWrapper isw(in);
					args.ParseStream(isw);
				}
				catch (const std::exception&) 
				{
					ok = false;
				}	
				if (!args.IsObject())
				{
					ok = false;
				}
			}
			else
			{
				args.Parse("{}");
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}
		string model = ARGS_STRING(model);
		string inputFilename = ARGS_STRING(input_file);
		string likelihoodFilename = ARGS_STRING(likelihood_file);
		string lengthFilename = ARGS_STRING(length_file);
		double centreX = ARGS_DOUBLE_DEF(centreX,0.5);
		double centreY = ARGS_DOUBLE_DEF(centreY,0.5);
		double centreRangeX = ARGS_DOUBLE_DEF(range_centreX,0.41);
		double centreRangeY = ARGS_DOUBLE_DEF(range_centreY,0.25);
		double scale = ARGS_DOUBLE_DEF(scale,0.5);
		int scaleValency = ARGS_INT_DEF(scale_valency,4);	
		int valency = ARGS_INT_DEF(valency,10);	
		int size = ARGS_INT_DEF(size,40);	
		int divisor = ARGS_INT_DEF(divisor,4);	
		int induceParameters_wmax = ARGS_INT_DEF(induceParameters.wmax,18);
		if (ok)
		{
			ok = ok && model.size();
			ok = ok && inputFilename.size();
			ok = ok && likelihoodFilename.size();
			ok = ok && lengthFilename.size();
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		Active activeA;
		if (ok) 
		{
			activeA.continousIs = true;
			activeA.historySliceCachingIs = true;
			activeA.historySliceCumulativeIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			activeA.historySliceCachingIs = true;
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		QImage image;
		QImage likelihoodImage;
		QImage lengthImage;
		int captureWidth = 0;
		int captureHeight = 0;	
		if (ok)
		{
			QImage imageIn;
			ok = ok && imageIn.load(QString(inputFilename.c_str()));
			EVAL(imageIn.format());
			image = imageIn.convertToFormat(QImage::Format_RGB32);
			EVAL(image.format());
			likelihoodImage = image.copy();
			lengthImage = image.copy();
			captureWidth = image.width();
			EVAL(captureWidth);
			captureHeight = image.height();
			EVAL(captureHeight);
			ok = ok && captureWidth > 0 && captureHeight > 0;
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok)
		{
			auto mark = Clock::now();
			double recordTime = 0.0;
			double recordValentTime = 0.0;
			double repaTime = 0.0;
			double applyTime = 0.0;
			std::size_t applyCount = 0;
			QPainter likelihoodPainter(&likelihoodImage);
			QPainter lengthPainter(&lengthImage);
			QBrush brush;
            brush.setStyle(Qt::SolidPattern);
			// brush.setStyle(Qt::Dense3Pattern);
			auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
			auto cap = (unsigned char)(ActiveUpdateParameters().mapCapacity);
			auto& dr = *activeA.decomp;	
			auto& cv = dr.mapVarParent();
			auto& sizes = activeA.historySlicesSize;
            auto& lengths = activeA.historySlicesLength;
			double lnwmax = std::log(induceParameters_wmax);
			double interval = scale/size;
			std::size_t lengthMax = 0;
			{
				for (auto& pp : lengths)
					lengthMax = std::max(lengthMax,pp.second);
			}	
			auto scaleX = centreRangeX * 2.0 + scale;
			auto scaleY = centreRangeY * 2.0 + scale;
			auto sizeX = (std::size_t)(scaleX * size / scale);
			auto sizeY = (std::size_t)(scaleY * size / scale);
			mark = Clock::now();
			Record record(image, 
				scaleX * captureHeight / captureWidth, scaleY,
				centreX, centreY, 
				sizeX, sizeY, 
				divisor, divisor);
			recordTime += ((Sec)(Clock::now() - mark)).count();
			auto hr = sizesHistoryRepa(scaleValency, valency, size*size);
			auto n = hr->dimension;
			auto vv = hr->vectorVar;
			auto rr = hr->arr;
			rr[n-1] = 0;
			std::vector<std::size_t> lengthResults;
			lengthResults.reserve(sizeY*sizeX);
			std::vector<double> likelihoodResults;
			likelihoodResults.reserve(sizeY*sizeX);
			for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
				for (std::size_t x = 0; x < sizeX - size; x++, z++)	
				{
					auto posX = centreX + (interval * x - (scaleX - scale) / 2.0) * captureHeight / captureWidth;
					auto posY = centreY + interval * y - (scaleY - scale) / 2.0;
					mark = Clock::now();
					Record recordSub(record,size,size,x,y);
					Record recordValent = recordSub.valent(valency);
					recordValentTime += ((Sec)(Clock::now() - mark)).count();
					mark = Clock::now();
					auto& arr1 = *recordValent.arr;	
					SizeUCharStructList jj;
					jj.reserve(n);
					for (std::size_t i = 0; i < n-1; i++)
					{
						SizeUCharStruct qq;
						qq.uchar = arr1[i];	
						qq.size = vv[i];
						jj.push_back(qq);
					}
					{
						SizeUCharStruct qq;
						qq.uchar = rr[n-1];	
						qq.size = vv[n-1];
						jj.push_back(qq);
					}
					auto ll = drmul(jj,dr,cap);	
					applyTime += ((Sec)(Clock::now() - mark)).count();
					applyCount++;
					std::size_t slice = 0;
					ok = ok && ll && ll->size();
					if (ok) slice = ll->back();		
					likelihoodResults.push_back((std::log(sizes[slice]) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax);				
					lengthResults.push_back(lengths[slice]);						
				}
			for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
				for (std::size_t x = 0; x < sizeX - size; x++,z++)	
				{
					auto posX = centreX + (interval * x - (scaleX - scale) / 2.0) * captureHeight / captureWidth;
					auto posY = centreY + interval * y - (scaleY - scale) / 2.0;
					double likelihood = likelihoodResults[z];				
					auto length = lengthResults[z];
					QRectF rectangle(posX*captureWidth, posY*captureHeight, 
						interval*captureHeight,interval*captureHeight);
					{
						int brightness = likelihood > 0.0 ? likelihood * 255 : 0;
						brush.setColor(QColor(brightness,brightness,brightness));
						likelihoodPainter.fillRect(rectangle,brush);					
					}
					{
						int brightness = length * 255 / lengthMax;
						brush.setColor(QColor(brightness,brightness,brightness));
						lengthPainter.fillRect(rectangle,brush);					
					}
				}	
			EVAL(recordTime);
			EVAL(recordValentTime);
			EVAL(applyTime);
			EVAL(applyCount);			
			EVAL(sizeY*sizeX);
			ok = ok && likelihoodImage.save(QString(likelihoodFilename.c_str()));
			ok = ok && lengthImage.save(QString(lengthFilename.c_str()));
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
	}
	
	if (argc >= 2 && string(argv[1]) == "generate_contour006")
	{
		bool ok = true;
		int stage = 0;
		
		js::Document args;
		if (ok)
		{
			string config = "contour.json";
			if (argc >= 3) config = string(argv[2]);
			if (ok && !config.empty())
			{
				std::ifstream in;
				try 
				{
					in.open(config);
					js::IStreamWrapper isw(in);
					args.ParseStream(isw);
				}
				catch (const std::exception&) 
				{
					ok = false;
				}	
				if (!args.IsObject())
				{
					ok = false;
				}
			}
			else
			{
				args.Parse("{}");
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}
		string model = ARGS_STRING(model);
		string inputFilename = ARGS_STRING(input_file);
		string likelihoodFilename = ARGS_STRING(likelihood_file);
		string lengthFilename = ARGS_STRING(length_file);
		double centreX = ARGS_DOUBLE_DEF(centreX,0.5);
		double centreY = ARGS_DOUBLE_DEF(centreY,0.5);
		double centreRangeX = ARGS_DOUBLE_DEF(range_centreX,0.41);
		double centreRangeY = ARGS_DOUBLE_DEF(range_centreY,0.25);
		double scale = ARGS_DOUBLE_DEF(scale,0.5);
		int scaleValency = ARGS_INT_DEF(scale_valency,4);	
		int valency = ARGS_INT_DEF(valency,10);	
		int size = ARGS_INT_DEF(size,40);	
		int divisor = ARGS_INT_DEF(divisor,4);	
		int induceParameters_wmax = ARGS_INT_DEF(induceParameters.wmax,18);
		int threadCount = ARGS_INT_DEF(threads,1);
		if (ok)
		{
			ok = ok && model.size();
			ok = ok && inputFilename.size();
			ok = ok && likelihoodFilename.size();
			ok = ok && lengthFilename.size();
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		Active activeA;
		if (ok) 
		{
			activeA.continousIs = true;
			activeA.historySliceCachingIs = true;
			activeA.historySliceCumulativeIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			activeA.historySliceCachingIs = true;
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		QImage image;
		QImage likelihoodImage;
		QImage lengthImage;
		int captureWidth = 0;
		int captureHeight = 0;	
		if (ok)
		{
			QImage imageIn;
			ok = ok && imageIn.load(QString(inputFilename.c_str()));
			EVAL(imageIn.format());
			image = imageIn.convertToFormat(QImage::Format_RGB32);
			EVAL(image.format());
			likelihoodImage = image.copy();
			lengthImage = image.copy();
			captureWidth = image.width();
			EVAL(captureWidth);
			captureHeight = image.height();
			EVAL(captureHeight);
			ok = ok && captureWidth > 0 && captureHeight > 0;
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok)
		{
			auto mark = Clock::now();
			double recordTime = 0.0;
			double recordValentTime = 0.0;
			double repaTime = 0.0;
			double applyTime = 0.0;
			std::size_t applyCount = 0;
			QPainter likelihoodPainter(&likelihoodImage);
			QPainter lengthPainter(&lengthImage);
			QBrush brush;
            brush.setStyle(Qt::SolidPattern);
			// brush.setStyle(Qt::Dense3Pattern);
			auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
			auto cap = (unsigned char)(ActiveUpdateParameters().mapCapacity);
			auto& dr = *activeA.decomp;	
			auto& cv = dr.mapVarParent();
			auto& sizes = activeA.historySlicesSize;
            auto& lengths = activeA.historySlicesLength;
			double lnwmax = std::log(induceParameters_wmax);
			double interval = scale/size;
			std::size_t lengthMax = 0;
			{
				for (auto& pp : lengths)
					lengthMax = std::max(lengthMax,pp.second);
			}	
			auto scaleX = centreRangeX * 2.0 + scale;
			auto scaleY = centreRangeY * 2.0 + scale;
			auto sizeX = (std::size_t)(scaleX * size / scale);
			auto sizeY = (std::size_t)(scaleY * size / scale);
			mark = Clock::now();
			Record record(image, 
				scaleX * captureHeight / captureWidth, scaleY,
				centreX, centreY, 
				sizeX, sizeY, 
				divisor, divisor);
			recordTime += ((Sec)(Clock::now() - mark)).count();
			auto hr = sizesHistoryRepa(scaleValency, valency, size*size);
			auto n = hr->dimension;
			auto vv = hr->vectorVar;
			auto rr = hr->arr;
			rr[n-1] = 0;
			std::vector<std::size_t> lengthResults(sizeY*sizeX);
			std::vector<double> likelihoodResults(sizeY*sizeX);
			std::vector<std::thread> threads;
			threads.reserve(threadCount);
			mark = Clock::now();
			for (std::size_t t = 0; t < threadCount; t++)
				threads.push_back(std::thread(
                    [threadCount,
                    sizeX,sizeY,size,&record,valency,n,vv,rr,
					drmul,&dr,&cv,cap,&sizes,&lengths,lnwmax,
					&likelihoodResults,&lengthResults] (int t)
					{
						for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
							for (std::size_t x = 0; x < sizeX - size; x++, z++)	
								if (z % threadCount == t)
								{
									Record recordSub(record,size,size,x,y);
									Record recordValent = recordSub.valent(valency);
									auto& arr1 = *recordValent.arr;	
									SizeUCharStructList jj;
									jj.reserve(n);
									for (std::size_t i = 0; i < n-1; i++)
									{
										SizeUCharStruct qq;
										qq.uchar = arr1[i];	
										qq.size = vv[i];
										jj.push_back(qq);
									}
									{
										SizeUCharStruct qq;
										qq.uchar = rr[n-1];	
										qq.size = vv[n-1];
										jj.push_back(qq);
									}
									auto ll = drmul(jj,dr,cap);	
									std::size_t slice = 0;
									if (ll && ll->size()) slice = ll->back();		
									likelihoodResults[z] = (std::log(sizes[slice]) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax;				
									lengthResults[z] = lengths[slice];						
								}
					}, t));
			for (auto& t : threads)
				t.join();
			applyTime += ((Sec)(Clock::now() - mark)).count();			
			for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
				for (std::size_t x = 0; x < sizeX - size; x++,z++)	
				{
					auto posX = centreX + (interval * x - (scaleX - scale) / 2.0) * captureHeight / captureWidth;
					auto posY = centreY + interval * y - (scaleY - scale) / 2.0;
					double likelihood = likelihoodResults[z];				
					auto length = lengthResults[z];
					QRectF rectangle(posX*captureWidth, posY*captureHeight, 
						interval*captureHeight,interval*captureHeight);
					{
						int brightness = likelihood > 0.0 ? likelihood * 255 : 0;
						brush.setColor(QColor(brightness,brightness,brightness));
						likelihoodPainter.fillRect(rectangle,brush);					
					}
					{
						int brightness = length * 255 / lengthMax;
						brush.setColor(QColor(brightness,brightness,brightness));
						lengthPainter.fillRect(rectangle,brush);					
					}
				}	
			EVAL(recordTime);
			EVAL(applyTime);		
			EVAL(sizeY*sizeX);
			ok = ok && likelihoodImage.save(QString(likelihoodFilename.c_str()));
			ok = ok && lengthImage.save(QString(lengthFilename.c_str()));
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
	}
	
	if (argc >= 2 && string(argv[1]) == "generate_contour007")
	{
		bool ok = true;
		int stage = 0;
		
		js::Document args;
		if (ok)
		{
			string config = "contour.json";
			if (argc >= 3) config = string(argv[2]);
			if (ok && !config.empty())
			{
				std::ifstream in;
				try 
				{
					in.open(config);
					js::IStreamWrapper isw(in);
					args.ParseStream(isw);
				}
				catch (const std::exception&) 
				{
					ok = false;
				}	
				if (!args.IsObject())
				{
					ok = false;
				}
			}
			else
			{
				args.Parse("{}");
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}
		string model = ARGS_STRING(model);
		string inputFilename = ARGS_STRING(input_file);
		string likelihoodFilename = ARGS_STRING(likelihood_file);
		string lengthFilename = ARGS_STRING(length_file);
		string representationFilename = ARGS_STRING(representation_file);
		double centreX = ARGS_DOUBLE_DEF(centreX,0.5);
		double centreY = ARGS_DOUBLE_DEF(centreY,0.5);
		double centreRangeX = ARGS_DOUBLE_DEF(range_centreX,0.41);
		double centreRangeY = ARGS_DOUBLE_DEF(range_centreY,0.25);
		double scale = ARGS_DOUBLE_DEF(scale,0.5);
		int scaleValency = ARGS_INT_DEF(scale_valency,4);	
		int valency = ARGS_INT_DEF(valency,10);	
		int valencyFactor = ARGS_INT(valency_factor);	
		bool valencyFixed = ARGS_BOOL(valency_fixed);	
		int size = ARGS_INT_DEF(size,40);	
		int divisor = ARGS_INT_DEF(divisor,4);	
		int induceParameters_wmax = ARGS_INT_DEF(induceParameters.wmax,18);
		int threadCount = ARGS_INT_DEF(threads,1);
		if (ok)
		{
			ok = ok && model.size();
			ok = ok && inputFilename.size();
			ok = ok && (likelihoodFilename.size() || lengthFilename.size() || representationFilename.size());
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		Active activeA;
		if (ok) 
		{
			activeA.continousIs = true;
			activeA.historySliceCachingIs = true;
			activeA.historySliceCumulativeIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			activeA.historySliceCachingIs = true;
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		std::unique_ptr<WBOT02::SliceRepresentationUMap> slicesRepresentation;
		if (ok) 
		{
			try
			{
				std::ifstream in(model + ".rep", std::ios::binary);
				if (in.is_open())
				{
					slicesRepresentation = persistentsSliceRepresentationUMap(in);
					in.close();
				}
				else
				{
					ok = false;
				}
				ok = ok && slicesRepresentation;
			}
			catch (const std::exception&)
			{
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		QImage image;
		QImage likelihoodImage;
		QImage lengthImage;
		QImage representationImage;
		int captureWidth = 0;
		int captureHeight = 0;	
		if (ok)
		{
			QImage imageIn;
			ok = ok && imageIn.load(QString(inputFilename.c_str()));
			EVAL(imageIn.format());
			image = imageIn.convertToFormat(QImage::Format_RGB32);
			EVAL(image.format());
			likelihoodImage = image.copy();
			lengthImage = image.copy();
			representationImage = image.copy();
			captureWidth = image.width();
			EVAL(captureWidth);
			captureHeight = image.height();
			EVAL(captureHeight);
			ok = ok && captureWidth > 0 && captureHeight > 0;
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok)
		{
			auto mark = Clock::now();
			double recordTime = 0.0;
			double recordValentTime = 0.0;
			double repaTime = 0.0;
			double applyTime = 0.0;
			std::size_t applyCount = 0;
			QPainter likelihoodPainter(&likelihoodImage);
			QPainter lengthPainter(&lengthImage);
			QPainter representationPainter(&representationImage);
			QBrush brush;
            brush.setStyle(Qt::SolidPattern);
			// brush.setStyle(Qt::Dense3Pattern);
			auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
			auto cap = (unsigned char)(ActiveUpdateParameters().mapCapacity);
			auto& dr = *activeA.decomp;	
			auto& cv = dr.mapVarParent();
			auto& sizes = activeA.historySlicesSize;
            auto& lengths = activeA.historySlicesLength;
			double lnwmax = std::log(induceParameters_wmax);
			double interval = scale/size;
			std::size_t lengthMax = 0;
			{
				for (auto& pp : lengths)
					lengthMax = std::max(lengthMax,pp.second);
			}	
			auto scaleX = centreRangeX * 2.0 + scale;
			auto scaleY = centreRangeY * 2.0 + scale;
			auto sizeX = (std::size_t)(scaleX * size / scale);
			if (sizeX % 2 != size % 2) sizeX++;
			auto sizeY = (std::size_t)(scaleY * size / scale);
			if (sizeY % 2 != size % 2) sizeY++;
			scaleX = sizeX * interval;
			scaleY = sizeY * interval;
			auto offsetX = (scaleX - scale) / 2.0;
			auto offsetY = (scaleY - scale) / 2.0;
			auto heightWidth = (double)captureHeight / (double)captureWidth;
			mark = Clock::now();
			Record record(image, 
				scaleX * captureHeight / captureWidth, scaleY,
				centreX, centreY, 
				sizeX, sizeY, 
				divisor, divisor);
			recordTime += ((Sec)(Clock::now() - mark)).count();
			auto hr = sizesHistoryRepa(scaleValency, valency, size*size);
			auto n = hr->dimension;
			auto vv = hr->vectorVar;
			auto rr = hr->arr;
			rr[n-1] = 0;
			std::vector<std::size_t> lengthResults(sizeY*sizeX);
			std::vector<double> likelihoodResults(sizeY*sizeX);
			std::vector<std::tuple<std::size_t,double,std::size_t,std::size_t,std::size_t>> actsPotsCoord(sizeY*sizeX);
			std::vector<std::thread> threads;
			threads.reserve(threadCount);
			mark = Clock::now();
			for (std::size_t t = 0; t < threadCount; t++)
				threads.push_back(std::thread(
                    [threadCount,valencyFixed,
					sizeX,sizeY,size,&record,valency,valencyFactor,n,vv,rr,
					drmul,&dr,&cv,cap,&sizes,&lengths,lnwmax,&actsPotsCoord,
					&likelihoodResults,&lengthResults] (int t)
					{
						for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
							for (std::size_t x = 0; x < sizeX - size; x++, z++)	
								if (z % threadCount == t)
								{
									Record recordSub(record,size,size,x,y);
									Record recordValent = valencyFixed ? recordSub.valentFixed(valency) : recordSub.valent(valency,valencyFactor);
									auto& arr1 = *recordValent.arr;	
									SizeUCharStructList jj;
									jj.reserve(n);
									for (std::size_t i = 0; i < n-1; i++)
									{
										SizeUCharStruct qq;
										qq.uchar = arr1[i];	
										qq.size = vv[i];
										if (qq.uchar)
											jj.push_back(qq);
									}
									{
										SizeUCharStruct qq;
										qq.uchar = rr[n-1];	
										qq.size = vv[n-1];
										if (qq.uchar)
											jj.push_back(qq);
									}
									auto ll = drmul(jj,dr,cap);	
									std::size_t slice = 0;
									if (ll && ll->size()) slice = ll->back();				
									auto length = lengths[slice];
									auto likelihood = (std::log(sizes[slice]) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax;
									likelihoodResults[z] = likelihood;
									lengthResults[z] = length;
									if (slice)
										actsPotsCoord[z] = std::make_tuple(length,likelihood,x,y,slice);
									else
										actsPotsCoord[z] = std::make_tuple(0,-INFINITY,x,y,0);
								}
					}, t));
			for (auto& t : threads)
				t.join();
			applyTime += ((Sec)(Clock::now() - mark)).count();	
			for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
				for (std::size_t x = 0; x < sizeX - size; x++,z++)	
				{
					auto posX = centreX + (interval * x - (scaleX - scale) / 2.0) * captureHeight / captureWidth;
					auto posY = centreY + interval * y - (scaleY - scale) / 2.0;
					double likelihood = likelihoodResults[z];				
					auto length = lengthResults[z];
					QRectF rectangle(posX*captureWidth, posY*captureHeight, 
						interval*captureHeight,interval*captureHeight);
					{
						int brightness = likelihood > 0.0 ? likelihood * 255 : 0;
						brush.setColor(QColor(brightness,brightness,brightness));
						likelihoodPainter.fillRect(rectangle,brush);					
					}
					{
						int brightness = length * 255 / lengthMax;
						brush.setColor(QColor(brightness,brightness,brightness));
						lengthPainter.fillRect(rectangle,brush);					
					}
				}	
            std::sort(actsPotsCoord.begin(), actsPotsCoord.end());			
            auto& reps = *slicesRepresentation;
			for (auto t : actsPotsCoord)	
			{
				auto slice = std::get<4>(t);
				if (slice && reps.count(slice))
				{
					auto x = std::get<2>(t);
					auto y = std::get<3>(t);
					auto posX = centreX + (interval * x - scaleX / 2.0) * captureHeight / captureWidth;
					auto posY = centreY + interval * y - scaleY / 2.0;
					QPointF point(posX*captureWidth,posY*captureHeight);
					auto rep = reps[slice].image(1,valency).scaledToHeight(scale*captureHeight);
					representationPainter.drawImage(point,rep);
				}
			}
			EVAL(recordTime);
			EVAL(applyTime);		
			EVAL(sizeY*sizeX);
			if (likelihoodFilename.size())
				ok = ok && likelihoodImage.save(QString(likelihoodFilename.c_str()));
			if (lengthFilename.size())
				ok = ok && lengthImage.save(QString(lengthFilename.c_str()));			
			if (representationFilename.size())
				ok = ok && representationImage.save(QString(representationFilename.c_str()));
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
	}
		
	if (argc >= 2 && (string(argv[1]) == "generate_contour008"
		|| string(argv[1]) == "generate_contour009"))
	{
		bool ok = true;
		int stage = 0;
		
		js::Document args;
		if (ok)
		{
			string config = "contour.json";
			if (argc >= 3) config = string(argv[2]);
			if (ok && !config.empty())
			{
				std::ifstream in;
				try 
				{
					in.open(config);
					js::IStreamWrapper isw(in);
					args.ParseStream(isw);
				}
				catch (const std::exception&) 
				{
					ok = false;
				}	
				if (!args.IsObject())
				{
					ok = false;
				}
			}
			else
			{
				args.Parse("{}");
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}
		bool isLengthNormalise = string(argv[1]) == "generate_contour009";
		string model = ARGS_STRING(model);
		string inputFilename = ARGS_STRING(input_file);
		string likelihoodFilename = ARGS_STRING(likelihood_file);
		string lengthFilename = ARGS_STRING(length_file);
		bool lengthByHue = ARGS_BOOL(length_by_hue);	
		string positionFilename = ARGS_STRING(position_file);
		string lengthPositionFilename = ARGS_STRING(length_position_file);
		string representationFilename = ARGS_STRING(representation_file);
		double centreX = ARGS_DOUBLE_DEF(centreX,0.5);
		double centreY = ARGS_DOUBLE_DEF(centreY,0.5);
		double centreRangeX = ARGS_DOUBLE_DEF(range_centreX,0.41);
		double centreRangeY = ARGS_DOUBLE_DEF(range_centreY,0.25);
		double scale = ARGS_DOUBLE_DEF(scale,0.5);
		int scaleValency = ARGS_INT_DEF(scale_valency,4);	
		int valency = ARGS_INT_DEF(valency,10);	
		int valencyFactor = ARGS_INT(valency_factor);	
		bool valencyFixed = ARGS_BOOL(valency_fixed);	
		bool valencyBalanced = ARGS_BOOL(valency_balanced);	
		valencyFixed |= valencyBalanced;
		int size = ARGS_INT_DEF(size,40);	
		int divisor = ARGS_INT_DEF(divisor,4);	
		int induceParameters_wmax = ARGS_INT_DEF(induceParameters.wmax,18);
		int threadCount = ARGS_INT_DEF(threads,1);
		double entropyMinimum = ARGS_DOUBLE(entropy_minimum);
		if (ok)
		{
			ok = ok && model.size();
			ok = ok && inputFilename.size();
			ok = ok && (likelihoodFilename.size() || lengthFilename.size() || positionFilename.size() || lengthPositionFilename.size() || representationFilename.size());
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		Active activeA;
		if (ok) 
		{
			activeA.continousIs = true;
			activeA.historySliceCachingIs = true;
			activeA.historySliceCumulativeIs = true;
			ActiveIOParameters ppio;
			ppio.filename = model +".ac";
			ok = ok && activeA.load(ppio);
			activeA.historySliceCachingIs = true;
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		std::unique_ptr<WBOT02::SliceRepresentationUMap> slicesRepresentation;
		if (ok) 
		{
			try
			{
				std::ifstream in(model + ".rep", std::ios::binary);
				if (in.is_open())
				{
					slicesRepresentation = persistentsSliceRepresentationUMap(in);
					in.close();
				}
				else
				{
					ok = false;
				}
				ok = ok && slicesRepresentation;
			}
			catch (const std::exception&)
			{
				ok = false;
			}
			stage++;
			EVAL(stage);
			TRUTH(ok);				
		}		
		QImage image;
		QImage likelihoodImage;
		QImage lengthImage;
		QImage positionImage;
		QImage lengthPositionImage;
		QImage representationImage;
		int captureWidth = 0;
		int captureHeight = 0;	
		if (ok)
		{
			QImage imageIn;
			ok = ok && imageIn.load(QString(inputFilename.c_str()));
			EVAL(imageIn.format());
			image = imageIn.convertToFormat(QImage::Format_RGB32);
			EVAL(image.format());
			likelihoodImage = image.copy();
			lengthImage = image.copy();
			positionImage = image.copy();
			lengthPositionImage = image.copy();
			representationImage = image.copy();
			captureWidth = image.width();
			EVAL(captureWidth);
			captureHeight = image.height();
			EVAL(captureHeight);
			ok = ok && captureWidth > 0 && captureHeight > 0;
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
		if (ok)
		{
			auto mark = Clock::now();
			double recordTime = 0.0;
			double recordValentTime = 0.0;
			double repaTime = 0.0;
			double applyTime = 0.0;
			std::size_t applyCount = 0;
			QPainter likelihoodPainter(&likelihoodImage);
			QPainter lengthPainter(&lengthImage);
			QPainter positionPainter(&positionImage);
			QPainter lengthPositionPainter(&lengthPositionImage);
			QPainter representationPainter(&representationImage);
			QBrush brush;
            brush.setStyle(Qt::SolidPattern);
			// brush.setStyle(Qt::Dense3Pattern);
			auto drmul = listVarValuesDecompFudSlicedRepasPathSlice_u;
			auto cap = (unsigned char)(ActiveUpdateParameters().mapCapacity);
			auto& dr = *activeA.decomp;	
			auto& cv = dr.mapVarParent();
			auto& vi = dr.mapVarInt();
			auto& sizes = activeA.historySlicesSize;
            auto& lengths = activeA.historySlicesLength;
			double lnwmax = std::log(induceParameters_wmax);
			double interval = scale/size;
			std::size_t lengthMax = 1;
			if (!isLengthNormalise)
			{
				for (auto& pp : lengths)
					lengthMax = std::max(lengthMax,pp.second);
			}	
			auto scaleX = centreRangeX * 2.0 + scale;
			auto scaleY = centreRangeY * 2.0 + scale;
			auto sizeX = (std::size_t)(scaleX * size / scale);
			if (sizeX % 2 != size % 2) sizeX++;
			auto sizeY = (std::size_t)(scaleY * size / scale);
			if (sizeY % 2 != size % 2) sizeY++;
			scaleX = sizeX * interval;
			scaleY = sizeY * interval;
			auto offsetX = (scaleX - scale) / 2.0;
			auto offsetY = (scaleY - scale) / 2.0;
			auto heightWidth = (double)captureHeight / (double)captureWidth;
			mark = Clock::now();
			Record record(image, 
				scaleX * captureHeight / captureWidth, scaleY,
				centreX, centreY, 
				sizeX, sizeY, 
				divisor, divisor);
			recordTime += ((Sec)(Clock::now() - mark)).count();
			auto hr = sizesHistoryRepa(scaleValency, valency, size*size);
			auto n = hr->dimension;
			auto vv = hr->vectorVar;
			auto rr = hr->arr;
			rr[n-1] = 0;
			std::vector<std::size_t> lengthResults(sizeY*sizeX);
			std::vector<double> likelihoodResults(sizeY*sizeX);
			std::vector<std::tuple<std::size_t,double,std::size_t,std::size_t,std::size_t,std::size_t>> actsPotsCoord(sizeY*sizeX);
			std::vector<std::thread> threads;
			threads.reserve(threadCount);
			mark = Clock::now();
			for (std::size_t t = 0; t < threadCount; t++)
				threads.push_back(std::thread(
                    [threadCount,valencyFixed,valencyBalanced,entropyMinimum,
					sizeX,sizeY,size,&record,valency,valencyFactor,n,vv,rr,
					drmul,&dr,&cv,cap,&sizes,&lengths,lnwmax,&actsPotsCoord,
					&likelihoodResults,&lengthResults] (int t)
					{
						for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
							for (std::size_t x = 0; x < sizeX - size; x++, z++)	
								if (z % threadCount == t)
								{
									Record recordSub(record,size,size,x,y);
									std::size_t average = 256;
									if (valencyBalanced)
									{
										auto sizeSub = recordSub.arr->size();
										auto arrSub = recordSub.arr->data();
										average = 0;
										for (std::size_t j = 0; j < sizeSub; j++)
											average += arrSub[j];	
										average /= sizeSub;			
									}
									Record recordValent = valencyFixed ? recordSub.valentFixed(valency,valencyBalanced) : recordSub.valent(valency,valencyFactor);
									std::size_t slice = 0;
									if (entropyMinimum <= 0.0 || recordValent.entropy() >=entropyMinimum)
									{
										auto& arr1 = *recordValent.arr;	
										SizeUCharStructList jj;
										jj.reserve(n);
										for (std::size_t i = 0; i < n-1; i++)
										{
											SizeUCharStruct qq;
											qq.uchar = arr1[i];	
											qq.size = vv[i];
											if (qq.uchar)
												jj.push_back(qq);
										}
										{
											SizeUCharStruct qq;
											qq.uchar = rr[n-1];	
											qq.size = vv[n-1];
											if (qq.uchar)
												jj.push_back(qq);
										}
										auto ll = drmul(jj,dr,cap);	
										if (ll && ll->size()) slice = ll->back();
									}
									auto length = lengths[slice];
									auto likelihood = (std::log(sizes[slice]) - std::log(sizes[cv[slice]]) + lnwmax)/lnwmax;
									likelihoodResults[z] = likelihood;
									lengthResults[z] = length;
									if (slice)
										actsPotsCoord[z] = std::make_tuple(length,likelihood,x,y,slice,average);
									else
										actsPotsCoord[z] = std::make_tuple(0,-INFINITY,x,y,0,average);
								}
					}, t));
			for (auto& t : threads)
				t.join();
			applyTime += ((Sec)(Clock::now() - mark)).count();	
			if (isLengthNormalise)
			{
				lengthMax = 1;
				for (auto length : lengthResults)
					lengthMax = std::max(lengthMax,length);
			}	
			std::map<std::size_t,double> positions;
			{
				std::vector<std::size_t> slices;
				for (auto& pp : lengths)
					if (!vi.count(pp.first))
						slices.push_back(pp.first);		
				typedef std::pair<std::size_t,std::size_t> SizePair;
				std::vector<std::vector<SizePair>> paths;
				for (auto slice : slices)
				{
					std::vector<SizePair> path;
					while (slice)
					{
						path.push_back(SizePair(sizes[slice],slice));
						slice = cv[slice];
					}
					std::reverse(path.begin(), path.end());
					paths.push_back(path);
				}
				std::sort(paths.begin(), paths.end());	
				std::reverse(paths.begin(), paths.end());
				double total = 0;
				for (auto& path : paths)
				{
					auto& pp = path.back();
					positions[pp.second] = total;
					total += pp.first;
				}
				for (auto& pp : positions)
					pp.second /= total;
			}
			for (std::size_t y = 0, z = 0; y < sizeY - size; y++)	
				for (std::size_t x = 0; x < sizeX - size; x++,z++)	
				{
					auto posX = centreX + (interval * x - (scaleX - scale) / 2.0) * captureHeight / captureWidth;
					auto posY = centreY + interval * y - (scaleY - scale) / 2.0;
					double likelihood = likelihoodResults[z];				
					auto length = lengthResults[z];
					auto slice = std::get<4>(actsPotsCoord[z]);
					QRectF rectangle(posX*captureWidth, posY*captureHeight, 
						interval*captureHeight,interval*captureHeight);
					{
						int brightness = likelihood > 0.0 ? likelihood * 255 : 0;
						brush.setColor(QColor(brightness,brightness,brightness));
						likelihoodPainter.fillRect(rectangle,brush);					
					}
					if (lengthByHue)
					{
						QColor colour;
						int hue = (lengthMax - length) * 300 / lengthMax;
						int saturation = (likelihood > 0.0 ? likelihood * 127 : 0) + 128;
						int brightness = 255;
						colour.setHsv(hue, saturation, brightness);
						if (length)
							brush.setColor(colour);
						else
							brush.setColor(Qt::black);
						lengthPainter.fillRect(rectangle,brush);					
					}
					else
					{
						int brightness = length * 255 / lengthMax;
						brush.setColor(QColor(brightness,brightness,brightness));
						lengthPainter.fillRect(rectangle,brush);					
					}						
					{
						QColor colour;
						int position = (int)(positions[slice] * 46080);
						int hue = position/128;
						int saturation = 128 + position%128;
						int brightness = 255;
						colour.setHsv(hue, saturation, brightness);
						if (length)
							brush.setColor(colour);
						else
							brush.setColor(Qt::black);
						positionPainter.fillRect(rectangle,brush);					
					}
					{
						QColor colour;
						int position = (int)(positions[slice] * 46080);
						int hue = position/128;
						int saturation = 128 + position%128;
						int brightness = length * 255 / lengthMax;
						colour.setHsv(hue, saturation, brightness);
						brush.setColor(colour);
						lengthPositionPainter.fillRect(rectangle,brush);
					}
				}	
            std::sort(actsPotsCoord.begin(), actsPotsCoord.end());			
            auto& reps = *slicesRepresentation;
			for (auto t : actsPotsCoord)	
			{
				auto slice = std::get<4>(t);
				if (slice && reps.count(slice))
				{
					auto x = std::get<2>(t);
					auto y = std::get<3>(t);
					auto average = std::get<5>(t);
					auto posX = centreX + (interval * x - scaleX / 2.0) * captureHeight / captureWidth;
					auto posY = centreY + interval * y - scaleY / 2.0;
					QPointF point(posX*captureWidth,posY*captureHeight);
					auto rep = reps[slice].image(1,valency,average).scaledToHeight(scale*captureHeight);
					representationPainter.drawImage(point,rep);
				}
			}
			EVAL(recordTime);
			EVAL(applyTime);		
			EVAL(sizeY*sizeX);
			if (likelihoodFilename.size())
				ok = ok && likelihoodImage.save(QString(likelihoodFilename.c_str()));
			if (lengthFilename.size())
				ok = ok && lengthImage.save(QString(lengthFilename.c_str()));			
			if (positionFilename.size())
				ok = ok && positionImage.save(QString(positionFilename.c_str()));
			if (lengthPositionFilename.size())
				ok = ok && lengthPositionImage.save(QString(lengthPositionFilename.c_str()));
			if (representationFilename.size())
				ok = ok && representationImage.save(QString(representationFilename.c_str()));
			stage++;
			EVAL(stage);
			TRUTH(ok);	
		}
	}
	
	return 0;
}
