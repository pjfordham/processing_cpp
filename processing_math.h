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
   float x, y;
   PVector() : x(0), y(0) {}
   PVector(float _x, float _y) : x(_x), y(_y) {}
   void sub(PVector b) {
      x = x - b.x;
      y = y - b.y;
   }
   void add(PVector b) {
      x = x + b.x;
      y = y + b.y;
   }
   void mult(float a) {
      x*=a;
      y*=a;
   }
     // Returns the magnitude (length) of the vector
    double mag() const {
        return std::sqrt(x * x + y * y);
    }

    // Limits the magnitude of the vector to a specified value
    void limit(double maxMag) {
        double m = mag();
        if (m > maxMag) {
            x = x * maxMag / m;
            y = y * maxMag / m;
        }
    }
   // Method to normalize the vector
   void normalize() {
      float mag = sqrtf(x * x + y * y);
      if (mag != 0) {
         x /= mag;
         y /= mag;
      }
   }
   void setMag(float mag) {
      normalize();
      x *= mag;
      y *= mag;
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
      return {random(1), random(1)};
   }
   // Method to calculate the dot product of two vectors
   float dot(PVector v) {
      return x * v.x + y * v.y;
   }

// Method to calculate the distance between two vectors
   static float dist(PVector a, PVector b) {
      float dx = a.x - b.x;
      float dy = a.y - b.y;
      return sqrtf(dx*dx + dy*dy);
   }
   // Method to calculate the distance between two vectors
   static PVector sub(PVector a, PVector b) {
      float dx = a.x - b.x;
      float dy = a.y - b.y;
      return {dx,dy};
   }
   static PVector mult(PVector a, float q) {
      float dx = a.x * q;
      float dy = a.y * q;
      return {dx,dy};
   }
   static PVector add(PVector a, PVector b) {
      float dx = a.x + b.x;
      float dy = a.y + b.y;
      return {dx,dy};
   }
   // Method to calculate a point on a straight line between two vectors
   void lerp(PVector v, float amt) {
      x = x + (v.x - x) * amt;
      y = y + (v.y - y) * amt;
   }
};

class Matrix2D {
private:
   float m_matrix[3][3];
public:
   Matrix2D(float a, float b, float c, float d, float e, float f):
      m_matrix{
         {a,b,c},
         {d,e,f},
         {0,0,1} } {
   }
   Matrix2D(float a, float b, float c, float d, float e, float f, float g, float h, float i):
      m_matrix{
         {a,b,c},
         {d,e,f},
         {g,h,i} } {
   }

   static Matrix2D Identity() {
      return {
         1,0,0,
         0,1,0};
   }

   static Matrix2D translate(float x, float y) {
      return {
         1,0,x,
         0,1,y};
   }

   static Matrix2D rotate(float angle) {
      return {
         cosf(angle), -sinf(angle), 0,
         sinf(angle), cosf(angle), 0 };
   }

   static Matrix2D scale(float x, float y) {
      return {
         x, 0, 0,
         0, y, 0 };
   }

   Matrix2D multiply(const Matrix2D& other) const {
      return {
         m_matrix[0][0] * other.m_matrix[0][0] + m_matrix[0][1] * other.m_matrix[1][0] + m_matrix[0][2] * other.m_matrix[2][0],
         m_matrix[0][0] * other.m_matrix[0][1] + m_matrix[0][1] * other.m_matrix[1][1] + m_matrix[0][2] * other.m_matrix[2][1],
         m_matrix[0][0] * other.m_matrix[0][2] + m_matrix[0][1] * other.m_matrix[1][2] + m_matrix[0][2] * other.m_matrix[2][2],

         m_matrix[1][0] * other.m_matrix[0][0] + m_matrix[1][1] * other.m_matrix[1][0] + m_matrix[1][2] * other.m_matrix[2][0],
         m_matrix[1][0] * other.m_matrix[0][1] + m_matrix[1][1] * other.m_matrix[1][1] + m_matrix[1][2] * other.m_matrix[2][1],
         m_matrix[1][0] * other.m_matrix[0][2] + m_matrix[1][1] * other.m_matrix[1][2] + m_matrix[1][2] * other.m_matrix[2][2],

         m_matrix[2][0] * other.m_matrix[0][0] + m_matrix[2][1] * other.m_matrix[1][0] + m_matrix[2][2] * other.m_matrix[2][0],
         m_matrix[2][0] * other.m_matrix[0][1] + m_matrix[2][1] * other.m_matrix[1][1] + m_matrix[2][2] * other.m_matrix[2][1],
         m_matrix[2][0] * other.m_matrix[0][2] + m_matrix[2][1] * other.m_matrix[1][2] + m_matrix[2][2] * other.m_matrix[2][2] };
   }

   PVector multiply(const PVector& v) const {
      float x = m_matrix[0][0] * v.x + m_matrix[0][1] * v.y + m_matrix[0][2];
      float y = m_matrix[1][0] * v.x + m_matrix[1][1] * v.y + m_matrix[1][2];
      return {x, y};
   }

   PVector get_translation() const {
      return {m_matrix[0][2], m_matrix[1][2]};
   }

   PVector get_scale() const {
      // Extract scaling and rotation
      float a = m_matrix[0][0];
      float b = m_matrix[0][1];
      float c = m_matrix[1][0];
      float d = m_matrix[1][1];

      float sx = sqrt(a * a + b * b);
      float sy = sqrt(c * c + d * d);
      return {sx, sy};
   }

   float get_angle() const {
      // Extract scaling and rotation
      float a = m_matrix[0][0];
      float b = m_matrix[0][1];
      float c = m_matrix[1][0];
      float d = m_matrix[1][1];

      float sx = sqrt(a * a + b * b);
      float sy = sqrt(c * c + d * d);
      if (sx != 0) {
         a /= sx;
         b /= sx;
      }

      if (sy != 0) {
         c /= sy;
         d /= sy;
      }
      // Extract scaling and rotation
      return atan2(m_matrix[0][1], m_matrix[0][0]);
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
