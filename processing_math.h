#ifndef PROCESSING_MATH_H
#define PROCESSING_MATH_H

// Math functionality independent of any
// graphics, global state  or event loop
// functionality.

#include <cmath>
#include <cstdlib>
#include <random>

const float PI = M_PI;
const float TWO_PI = M_PI * 2.0;
const float HALF_PI = M_PI / 2.0;

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

class Matrix3D {
private:
    float m_data[16];
public:
   void print() {
      fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n\n",
                 m_data[0], m_data[1], m_data[2], m_data[3],
                 m_data[4], m_data[5], m_data[6], m_data[7],
                 m_data[8], m_data[9], m_data[10], m_data[11],
                 m_data[12], m_data[13], m_data[14], m_data[15] );
   }
              
   const float *data() const {
      return m_data;
   }

   Matrix3D() :
        m_data{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1} {}

    Matrix3D(float m00, float m01, float m02, float m03,
             float m10, float m11, float m12, float m13,
             float m20, float m21, float m22, float m23,
             float m30, float m31, float m32, float m33) :
        m_data{m00, m10, m20, m30, m01, m11, m21, m31,
               m02, m12, m22, m32, m03, m13, m23, m33} {}

    float operator() (int row, int col) {
        return m_data[col * 4 + row];
    }

    float operator() (int row, int col) const {
        return m_data[col * 4 + row];
    }

    Matrix3D operator* (const Matrix3D& other) const {
        Matrix3D result;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0;
                for (int i = 0; i < 4; ++i) {
                    sum += m_data[i * 4 + row] * other.m_data[col * 4 + i];
                }
                result.m_data[col * 4 + row] = sum;
            }
        }
        return result;
    }

    static Matrix3D identity() {
        return Matrix3D();
    }

    static Matrix3D translate(const PVector& v) {
        return Matrix3D(1, 0, 0, v.x,
                        0, 1, 0, v.y,
                        0, 0, 1, v.z,
                        0, 0, 0, 1);
    }

    static Matrix3D scale(const PVector& v) {
        return Matrix3D(v.x, 0, 0, 0,
                        0, v.y, 0, 0,
                        0, 0, v.z, 0,
                        0, 0, 0, 1);
    }

    static Matrix3D rotate(float angle, const PVector& axis) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        float x = axis.x;
        float y = axis.y;
        float z = axis.z;
        float omc = 1 - c;

        return Matrix3D(x * x * omc + c, x * y * omc - z * s, x * z * omc + y * s, 0,
                        x * y * omc + z * s, y * y * omc + c, y * z * omc - x * s, 0,
                        x * z * omc - y * s, y * z * omc + x * s, z * z * omc + c, 0,
                        0, 0, 0, 1);
    }
};

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
