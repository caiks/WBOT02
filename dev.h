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

}



#endif
