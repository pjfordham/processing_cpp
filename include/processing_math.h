#ifndef PROCESSING_MATH_H
#define PROCESSING_MATH_H

// Math functionality independent of any
// graphics, global state  or event loop
// functionality.

#include <cmath>
#include <cstdlib>
#include <random>

void noiseSeed(int seed);
void noiseDetail(int lod, float falloff = 0.5);
float noise(float x, float y = 0, float z = 0);

const float PI = M_PI;
const float TWO_PI = M_PI * 2.0;
const float HALF_PI = M_PI / 2.0;
const float QUARTER_PI = M_PI / 4.0;
const float DEG_TO_RAD = 0.01745329238474369f;

inline float random(float min, float max) {
   static std::mt19937 randomNumbers( 1 );
   std::uniform_real_distribution<float> randomLocationRange(min, max);
   return randomLocationRange( randomNumbers );
}

inline float random(float max) {
   return random(0,max);
}


inline float bezierPoint(float a, float b, float c, float d, float t) {
   float t_ = 1 - t;
   return
      1 * t_ * t_ * t_ * a +
      3 * t_ * t_ * t  * b +
      3 * t_ * t  * t  * c +
      1 * t  * t  * t  * d;
}

class PVector {
public:
   float x, y,z;
   PVector() : x(0), y(0),z(0) {}
   PVector(float _x, float _y, float _z=0.0) : x(_x), y(_y),z(_z) {}
   void print() const;
   void sub(PVector b) {
      x = x - b.x;
      y = y - b.y;
      z = z - b.z;
   }
   float heading() {
      return atan2(y, x);
   }
   void set( float _x, float _y, float _z ) {
      x = _x;
      y = _y;
      z = _z;
   }
   void add(PVector b) {
      x = x + b.x;
      y = y + b.y;
      z = z + b.z;
   }
   PVector &mult(float a) {
      x*=a;
      y*=a;
      z*=a;
      return *this;
   }
   void div(float a) {
      x/=a;
      y/=a;
      z/=a;
   }
   // Returns the magnitude (length) of the vector
   double mag() const {
      return std::sqrt(x * x + y * y + z * z);
   }

   void rotate(float angle) {
      float cosAngle = cos(angle);
      float sinAngle = sin(angle);
      float newX = x * cosAngle - y * sinAngle;
      float newY = x * sinAngle + y * cosAngle;
      x = newX;
      y = newY;
   }

   // Limits the magnitude of the vector to a specified value
   void limit(double maxMag) {
      double m = mag();
      if (m > maxMag) {
         x = x * maxMag / m;
         y = y * maxMag / m;
         z = z * maxMag / m;
      }
   }
   float norm() const {
      return sqrtf(x * x + y * y + z * z);
   }
   // Method to normalize the vector
   PVector &normalize() {
      float mag = sqrtf(x * x + y * y + z * z);
      if (mag != 0) {
         x /= mag;
         y /= mag;
         z /= mag;
      }
      return *this;
   }
   PVector cross(PVector v) {
      float crossX = y * v.z - z * v.y;
      float crossY = z * v.x - x * v.z;
      float crossZ = x * v.y - y * v.x;
      return PVector{crossX, crossY, crossZ};
   }

   PVector normal() {
      return PVector(-y, x);
   }

   void setMag(float mag) {
      normalize();
      x *= mag;
      y *= mag;
      z *= mag;
   }
   // Static method to create a PVector from an angle
   static PVector fromAngle(float a) {
      float x = cosf(a);
      float y = sinf(a);
      return {x, y};
   }

   // Static method to create a PVector from an angle
   static PVector random2D() {
      return PVector::fromAngle(random(TWO_PI));
   }

   static PVector random3D() {
      // Ugly hack
      return {random(1), random(1), random(1)};
   }
   // Method to calculate the dot product of two vectors
   float dot(PVector v) {
      return x * v.x + y * v.y + z * v.z;
   }

// Method to calculate the distance between two vectors
   static float dist(PVector a, PVector b) {
      float dx = a.x - b.x;
      float dy = a.y - b.y;
      float dz = a.z - b.z;
      return sqrtf(dx*dx + dy*dy + dz*dz);
   }
   // Method to calculate the distance between two vectors
   static PVector sub(PVector a, PVector b) {
      float dx = a.x - b.x;
      float dy = a.y - b.y;
      float dz = a.z - b.z;
      return {dx,dy,dz};
   }
   static PVector mult(PVector a, float q) {
      float dx = a.x * q;
      float dy = a.y * q;
      float dz = a.z * q;
      return {dx,dy,dz};
   }
   static PVector div(PVector a, float q) {
      float dx = a.x / q;
      float dy = a.y / q;
      float dz = a.z / q;
      return {dx,dy,dz};
   }
   static PVector add(PVector a, PVector b) {
      float dx = a.x + b.x;
      float dy = a.y + b.y;
      float dz = a.z + b.z;
      return {dx,dy,dz};
   }
   // Method to calculate a point on a straight line between two vectors
   void lerp(PVector v, float amt) {
      x = x + (v.x - x) * amt;
      y = y + (v.y - y) * amt;
      z = z + (v.z - z) * amt;
   }

   PVector operator+(const PVector &other) const {
      return PVector{
         x + other.x,
         y + other.y,
         z + other.z};
   }

   PVector operator-(const PVector &other) const {
      return PVector{
         x - other.x,
         y - other.y,
         z - other.z};
   }

   PVector operator*(const float &other) const {
      return PVector{
         x * other,
         y * other,
         z * other};
   }
   bool operator==(const PVector &other) const {
      return x == other.x && y == other.y && z == other.z;
   }
};

struct PVector4 {
   float data[4];
};

struct PMatrix {
private:
   float m_data[16];
   bool identity = true;
   constexpr static float i[] = {
      1.0 ,0 ,0 ,0,
      0 ,1.0 ,0 ,0,
      0 ,0 ,1.0 ,0,
      0 ,0 ,0 ,1.0,
   };
public:
   PMatrix() {}
   PMatrix( const PVector4 &a, const PVector4 &b, const PVector4 &c, const PVector4 &d) {
      identity = false;
      for(int i = 0; i< 4; i++ ){
         m_data[i*4] = a.data[i];
         m_data[(i*4)+1] = b.data[i];
         m_data[(i*4)+2] = c.data[i];
         m_data[(i*4)+3] = d.data[i];
      }
   }

   void print() const;

   static PMatrix Identity() {
      return PMatrix();
   }

   static PMatrix FlipY() {
      return PMatrix(
         PVector4{ 1.0,  0.0, 0.0, 0.0 },
         PVector4{ 0.0, -1.0, 0.0, 0.0 },
         PVector4{ 0.0,  0.0, 1.0, 0.0 } ,
         PVector4{ 0.0,  0.0, 0.0, 1.0 } );
   }

   const float *data() const {
      if (identity)
         return i;
      else
         return m_data;
   }

   bool operator==(const PMatrix &x) const {
      if (identity && x.identity) return true;
      for (int i = 0 ; i< 16 ; i++ ) {
         if (data()[i] != x.data()[i]) {
            return false;
         }
      }
      return true;
   }

   bool operator!=(const PMatrix &x) const {
      return !(*this == x);
   }

private:
   const float &mat(int row, int col) const {
      if (identity)
         abort();
      return m_data[row * 4 + col];
   }
   float &mat(int row, int col) {
      if (identity)
         abort();
      return m_data[row * 4 + col];
   }
public:

   PMatrix operator*(const PMatrix &x) const {
      PMatrix result;
      if (identity) return x;
      if (x.identity) return *this;
      result.identity = false;
      for (int col = 0; col < 4; col++) {
         for (int row = 0; row < 4; row++) {
            result.mat(col,row) = 0.0;
            for (int k = 0; k < 4; k++) {
               result.mat(col,row) += mat(k, row) * x.mat(col, k);
            }
         }
      }
      return result;
   }

   PVector4 operator*(const PVector4 &x) const {
      PVector4 result;
      if (identity) return x;
      for (int col = 0; col < 4; col++) {
         result.data[col] = 0.0;
         for (int row = 0; row < 4; row++) {
            result.data[col] += mat(row,col) * x.data[row];
         }
      }
      return result;
   }
};

struct PLine {
   PVector start, end;

   PVector normal() {
      PVector l1_norm = (end - start).normal();
      l1_norm.normalize();
      return l1_norm;
   }

   PLine offset(float value) {
      PVector l1_norm = normal();
      return { start + l1_norm * value , end + l1_norm * value };
   }

   PVector intersect(PLine other) {
      PVector d1 = end - start;
      float a1 = d1.x;
      float b1 = -d1.y;
      float c1 = d1.x * start.y - d1.y * start.x;

      PVector d2 = other.end - other.start;
      float a2 = d2.x;
      float b2 = -d2.y;
      float c2 = d2.x * other.start.y - d2.y * other.start.x;

      // Z Calculation is totally wrong here
      return {  (a1*c2 - a2*c1) / (a1*b2 - a2*b1), (b2*c1 - b1*c2) / (a1*b2 - a2*b1), other.start.z };

   }

};

inline PMatrix TranslateMatrix(const PVector &in) {
   PMatrix ret {
      {1.0,    0,     0,    in.x},
      {0,    1.0,     0,    in.y},
      {0,      0,   1.0,    in.z},
      {0.0f, 0.0f,  0.0f,    1.0f} };
   return ret;
}

inline PMatrix ScaleMatrix(const PVector &in) {
   PMatrix ret {
      {in.x,    0,     0,    0},
      {0,    in.y,     0,    0},
      {0,      0,   in.z,    0},
      {0.0f, 0.0f,  0.0f, 1.0f} };
   return ret;
}

inline PMatrix RotateMatrix(const float angle, const PVector& in) {
   float c = cos(angle);
   float s = sin(angle);
   float omc = 1.0f - c;
   float x = in.x;
   float y = in.y;
   float z = in.z;

   PMatrix ret {
      {x * x * omc + c, x * y * omc - z * s, x * z * omc + y * s, 0},
      {x * y * omc + z * s, y * y * omc + c, y * z * omc - x * s, 0},
      {x * z * omc - y * s, y * z * omc + x * s, z * z * omc + c, 0},
      {0, 0, 0, 1}};
   return ret;
}

inline float map(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
   float result = (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
   return result;
}

inline float lerp(float start, float stop, float amt) {
   return start + (stop - start) * amt;
}

inline float dist(float x1, float y1, float x2, float y2) {
   float dx = x2 - x1;
   float dy = y2 - y1;
   return std::sqrt(dx * dx + dy * dy);
}

inline float randomGaussian()
{
   static std::mt19937 randomNumbers( 1 );
   static std::normal_distribution<float> dist(0.0f, 1.0f);
   return dist(randomNumbers);
}

inline float radians(float degrees) {
   return degrees * M_PI / 180.0;
}

inline float norm(float value, float start, float stop) {
   return (value - start) / (stop - start);
}

inline float norm(float value, float start1, float stop1, float start2, float stop2) {
   float adjustedValue = (value - start1) / (stop1 - start1);
   return start2 + (stop2 - start2) * adjustedValue;
}

inline float constrain(float value, float lower, float upper) {
   if (value < lower) {
      return lower;
   } else if (value > upper) {
      return upper;
   } else {
      return value;
   }
}

inline float sq(float a) {
   return a * a;
}

template <typename Container>
typename Container::value_type random(const Container& c) {
   auto random_it = std::next(std::begin(c), random(std::size(c)));
   return *random_it;
}

template <typename T>
T random(const std::initializer_list<T>& c) {
   auto random_it = std::next(std::begin(c), random(std::size(c)));
   return *random_it;
}

#endif
