#ifndef PROCESSING_JAVA_COMPATABILITY_H
#define PROCESSING_JAVA_COMPATABILITY_H

#include <vector>
#include <algorithm>
#include <cmath>

// Various hacks to make C++ slightly
// more like Java to get sketches to
// compile unmodified.

#define y1 processing_y1
#define index processing_index

using FloatArrayList = std::vector<float>;

using std::max;
using std::min;
using std::abs;
using std::sqrt;

typedef bool boolean;

typedef unsigned char byte;

#endif


