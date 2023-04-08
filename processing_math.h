#ifndef PROCESSING_MATH_H
#define PROCESSING_MATH_H

// Math functionality independent of any
// graphics, global state  or event loop
// functionality.

#include <cmath>
#include <cstdlib>
#include <random>
#include <Eigen/Dense>

const float PI = M_PI;
const float TWO_PI = M_PI * 2.0;
const float HALF_PI = M_PI / 2.0;
const float QUARTER_PI = M_PI / 4.0;

class PVector {
public:
   float x, y,z;
   PVector() : x(0), y(0),z(0) {}
   PVector(float _x, float _y, float _z=0.0) : x(_x), y(_y),z(_z) {}
   void print() {
      fmt::print(" {:>8.2} {:>8.2} {:>8.2}\n",x,y,z);
   }
          void sub(PVector b) {
      x = x - b.x;
      y = y - b.y;
      z = z - b.z;
   }
   float get_angle() {
      return atan2(y, x);
   }
   void add(PVector b) {
      x = x + b.x;
      y = y + b.y;
      z = z + b.z;
   }
   void mult(float a) {
      x*=a;
      y*=a;
      z*=a;
   }
   // Returns the magnitude (length) of the vector
   double mag() const {
      return std::sqrt(x * x + y * y + z * z);
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
   void normalize() {
      float mag = sqrtf(x * x + y * y + z * z);
      if (mag != 0) {
         x /= mag;
         y /= mag;
         z /= mag;
      }
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
      // Ugly hack
      float random(float max);
      return {random(1), random(1), 0};
   }
   static PVector random3D() {
      // Ugly hack
      float random(float max);
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

   PVector operator+(const PVector &other) {
      return PVector{
         x + other.x,
         y + other.y,
         z + other.z};
   }

   PVector operator-(const PVector &other) {
      return PVector{
         x - other.x,
         y - other.y,
         z - other.z};
   }
   PVector operator*(const float &other) {
      return PVector{
         x * other,
         y * other,
         z * other};
   }
   bool operator==(const PVector &other) {
      return x == other.x && y == other.y && z == other.z;
   }
};

struct PLine {
   PVector start, end;

   PLine offset(float value) {
      PVector l1_norm = (end - start).normal();
      l1_norm.normalize();
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

   return {  (a1*c2 - a2*c1) / (a1*b2 - a2*b1), (b2*c1 - b1*c2) / (a1*b2 - a2*b1) };

}

};

Eigen::Matrix4f TranslateMatrix(const PVector &in) {
  Eigen::Matrix4f ret {
     {1.0,    0,     0,    in.x} ,
     {0,    1.0,     0,    in.y},
     {0,      0,   1.0,    in.z},
     {0.0f, 0.0f,  0.0f,    1.0f} };
  return ret;
}

Eigen::Matrix4f ScaleMatrix(const PVector &in) {
   Eigen::Matrix4f ret {
      {in.x,    0,     0,    0} ,
      {0,    in.y,     0,    0},
      {0,      0,   in.z,   0},
      {0.0f, 0.0f,  0.0f,    1.0f} };
   return ret;
}

Eigen::Matrix4f RotateMatrix(const float angle, const PVector& in) {
   float c = cos(angle);
   float s = sin(angle);
   float omc = 1.0f - c;
   float x = in.x;
   float y = in.y;
   float z = in.z;

   Eigen::Matrix4f ret;
   ret << x * x * omc + c, x * y * omc - z * s, x * z * omc + y * s, 0,
      x * y * omc + z * s, y * y * omc + c, y * z * omc - x * s, 0,
      x * z * omc - y * s, y * z * omc + x * s, z * z * omc + c, 0,
      0, 0, 0, 1;
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

std::mt19937 randomNumbers( 1 );
inline float random(float min, float max) {
   std::uniform_real_distribution<float> randomLocationRange(min, max);
   return randomLocationRange( randomNumbers );
}

inline float random(float max) {
   return random(0,max);
}

inline float randomGaussian()
{
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

#endif
