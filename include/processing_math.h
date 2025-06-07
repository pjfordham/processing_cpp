#ifndef PROCESSING_MATH_H
#define PROCESSING_MATH_H

// Math functionality independent of any
// graphics, global state  or event loop
// functionality.

#include <numbers>
#include <cmath>
#include <cstdlib>
#include <random>
#include <array>
#include <fmt/core.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_perlin.h>

using std::lerp;

void noiseSeed(int seed);
void noiseDetail(int lod, float falloff = 0.5);
float noise(float x, float y = 0, float z = 0);

constexpr float PI = std::numbers::pi_v<float>;
constexpr float TWO_PI = PI * 2.0;
constexpr float HALF_PI = PI / 2.0;
constexpr float QUARTER_PI = PI / 4.0;
constexpr float DEG_TO_RAD = 0.01745329238474369f;

inline float random(float min, float max) {
   thread_local static std::mt19937 randomNumbers( 1 );
   std::uniform_real_distribution<float> randomLocationRange(min, max);
   return randomLocationRange( randomNumbers );
}

inline float random(float max) {
   return random(0,max);
}


inline float bezierPointCubic(float a, float b, float c, float d, float t) {
   float t_ = 1 - t;
   return
      1 * t_ * t_ * t_ * a +
      3 * t_ * t_ * t  * b +
      3 * t_ * t  * t  * c +
      1 * t  * t  * t  * d;
}

inline float bezierPoint(float a, float b, float c, float d, float t) {
   return bezierPointCubic(a, b, c, d, t);
}

inline float bezierPointQuadratic(float a, float b, float c, float t) {
   float t_ = 1 - t;
   return
      t_ * t_ * a +
      2 * t_ * t * b +
      t * t * c;
}

inline float angularDifference(float angle1, float angle2) {
   // Normalize angles to the range [0, 2π)
   angle1 = fmod(angle1, 2 * PI);
   angle2 = fmod(angle2, 2 * PI);

   // Calculate the absolute angular difference, taking wrapping into account
   float angularDifference = fabs(angle1 - angle2);

   // Ensure that the angular difference is within the specified tolerance
   if (angularDifference > PI) {
      // If the difference is greater than π, consider the smaller wrapped angle
      angularDifference = 2 * PI - angularDifference;
   }
   return angularDifference;
}

class PVector : public glm::vec3 {
public:
   PVector() : glm::vec3{0.0f,0.0f,0.0f} {}
   PVector(const glm::vec3 &o) : glm::vec3{o} {}
   PVector(float x, float y, float z = 0.0f) : glm::vec3{x, y, z} {}

   PVector &add(PVector b) {
      return *this = *this + b;
   }

   PVector &sub(PVector b) {
      return *this = *this - b;
   }

   PVector &mult(float a) {
      return *this = *this * a;
   }

   PVector &div(float a) {
      return *this = *this / a;
   }

   static float angleBetween(const PVector &a, const PVector &b) {
      return angularDifference(a.heading(), b.heading());
   }

   float heading() const {
      return atan2(y, x);
   }

   void set( std::array<float,3> a ) {
      x = a[0];
      y = a[1];
      z = a[2];
   }

   void set( float _x, float _y, float _z ) {
      x = _x;
      y = _y;
      z = _z;
   }

   // Returns the magnitude (length) of the vector
   float mag() const {
      const glm::vec3 &vec = *this;
      return glm::length( vec );
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
   void limit(float maxMag) {
      float m = mag();
      if (m > maxMag) {
         *this = *this * maxMag / m;
      }
   }

   float norm() const {
      return mag();
   }

   PVector &normalize() {
      float length = mag();
      if (length != 0) {
         return *this = *this / length;
      }
      return *this;
   }

   PVector cross(PVector v) {
      const glm::vec3 &veca = *this;
      const glm::vec3 &vecb = v;
      return glm::cross( veca, vecb );
   }

   PVector normalInPlane(const PVector& planeNormal) const {
      return glm::normalize(glm::cross(*this,planeNormal));
   }

   PVector normal() {
      if (x == 0 && y == 0) {
         return PVector{z > 0.0 ? 1.0f : -1.0f ,0,0};
      } else {
         return PVector(-y, x, z);
      }
   }

   void setMag(float mag) {
      normalize();
      *this = *this * mag;
   }

   // Static method to create a PVector from an angle
   static PVector fromAngle(float a) {
      float x = cosf(a);
      float y = sinf(a);
      return {x, y};
   }

   static PVector random2D() {
      return PVector::fromAngle(random(TWO_PI));
   }

   static PVector random3D() {
      // Ugly hack
      return {random(1), random(1), random(1)};
   }

   // Method to calculate the dot product of two vectors
   float dot(const PVector &v) const {
      const glm::vec3 &veca = *this;
      const glm::vec3 &vecb = v;
      return glm::dot( veca, vecb );
   }

   // Method to calculate the distance between two vectors
   static float dist(PVector a, PVector b) {
      const glm::vec3 &veca = a;
      const glm::vec3 &vecb = b;
      return glm::distance( veca, vecb );
   }

   static PVector sub(PVector a, PVector b) {
      float dx = a.x - b.x;
      float dy = a.y - b.y;
      float dz = a.z - b.z;
      return {dx,dy,dz};
   }

   static PVector mult(PVector a, float q) {
      return a * q;
   }

   static PVector div(PVector a, float q) {
      return a / q;
   }

   static PVector add(PVector a, PVector b) {
      return a + b;
   }

   void lerp(PVector v, float amt) {
      x = ::lerp(x, v.x, amt);
      y = ::lerp(y, v.y, amt);
      z = ::lerp(z, v.z, amt);
   }

   operator std::array<float,3>() {
      return { x, y, z };
   }
};

inline PVector &operator+=(PVector &a, const PVector &b) {
   glm::vec3 &veca = a;
   const glm::vec3 &vecb = b;
   veca = veca + vecb;
   return a;
}

inline PVector operator-(const PVector &a, const PVector &b) {
   const glm::vec3 &veca = a;
   const glm::vec3 &vecb = b;
   return veca - vecb;
}

inline PVector operator*(const PVector &a, const float &b) {
   const glm::vec3 &veca = a;
   return veca * b;
}

inline PVector operator/(const PVector &a, const float &b)  {
   const glm::vec3 &veca = a;
   return veca / b;
}

inline bool operator==(const PVector &a, const PVector &b)  {
   const glm::vec3 &veca = a;
   const glm::vec3 &vecb = b;
   return veca == vecb;
}

inline PVector operator+(const PVector &a, const PVector &b) {
   const glm::vec3 &veca = a;
   const glm::vec3 &vecb = b;
   return veca + vecb;
}

inline PVector operator*(float s, PVector a) {
   return a * s;
}

inline PVector lerp(PVector start, PVector end, float i) {
   return {
      lerp(start.x, end.x, i),
      lerp(start.y, end.y, i),
      lerp(start.z, end.z, i),
   };
}

class PVector2 : public glm::vec2 {
public:
   PVector2() : glm::vec2{0.0f,0.0f} {}
   PVector2(const glm::vec2 &o) : glm::vec2{o} {}
   PVector2(float x, float y) : glm::vec2{x,y} {}
   operator std::array<float,2>() {
      return { x , y };
   }
};

class PVector4 : public glm::vec4 {
public:
   PVector4() : glm::vec4{0.0f,0.0f,0.0f,0.0f} {}
   PVector4(const glm::vec4 &o) : glm::vec4{o} {}
   PVector4(float x, float y, float z, float w) : glm::vec4{x,y,z,w} {}
   operator PVector() {
      return { x, y, z };
   }
};

class PMatrix {
   glm::mat4 m_data;
   bool identity = true;
   constexpr static glm::mat4 i = {
      {1.0f, 0.0f, 0.0f, 0.0f},
      {0.0f, 1.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
   };

public:
   PMatrix() {}
   PMatrix( const glm::mat4& m) : m_data(m), identity{false} {}
   PMatrix( const PVector4 &a, const PVector4 &b, const PVector4 &c, const PVector4 &d) :
      m_data{
         {a.x,b.x,c.x,d.x},
         {a.y,b.y,c.y,d.y},
         {a.z,b.z,c.z,d.z},
         {a.w,b.w,c.w,d.w},
      }, identity{ false } {
   }

   void print() const;

   static PMatrix Identity() {
      return PMatrix();
   }

   static PMatrix FlipY() {
      return PMatrix(
         { 1.0f,  0.0f, 0.0f, 0.0f },
         { 0.0f, -1.0f, 0.0f, 0.0f },
         { 0.0f,  0.0f, 1.0f, 0.0f } ,
         { 0.0f,  0.0f, 0.0f, 1.0f } );
   }

   const float *data() const {
      if (identity) {
         return glm::value_ptr( i );
      } else {
         return glm::value_ptr( m_data );
      }
   }
   const glm::mat4 &glm_data() const {
      if (identity) {
         return i;
      } else {
         return m_data;
      }
   }

   friend bool operator==(const PMatrix &x, const PMatrix &y)  {
      if (x.identity && y.identity) {
         return true;
      } else if (y.identity) {
         return x.m_data == i;
      } else if (x.identity) {
         return i == y.m_data;
      } else {
         return x.m_data == y.m_data;
      }
   }

   friend PMatrix operator*(const PMatrix &x, const PMatrix &y)  {
      if (y.identity) {
         return x;
      } else if (x.identity) {
         return y;
      } else {
         return x.m_data * y.m_data;
      }
   }

   friend PVector4 operator*(const PMatrix &x, const PVector4 &y)  {
      if (x.identity) {
         return y;
      } else {
         const glm::vec4 &vec = y;
         return x.m_data * vec;
      }
   }

};

inline bool operator!=(const PMatrix &x, const PMatrix &y)  {
   return !(x == y);
}

inline PVector operator*(const PMatrix &x, const PVector &y) {
   return x * PVector4{y.x,y.y,y.z,1};
}


struct PLine {
   PVector start, end;

   float heading() const {
      return (end - start).heading();
   }

   PVector normal() {
      PVector l1_norm = (end - start).normal();
      l1_norm.normalize();
      return l1_norm;
   }

   PVector normalInPlane(const PVector& planeNormal) const {
      PVector l1_norm = (end - start).normalInPlane(planeNormal);
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

   PVector intersectInPlane(PLine other, PVector planeNormal) {
    // Direction vectors of the two lines
    PVector d1 = end - start;
    PVector d2 = other.end - other.start;

    // Normal to the plane (we assume both lines lie in the same plane)
    PVector planeNormalNormalized = planeNormal.normalize();

    // Project the direction vectors onto the plane
    PVector proj_d1 = d1 - planeNormalNormalized * d1.dot(planeNormalNormalized);
    PVector proj_d2 = d2 - planeNormalNormalized * d2.dot(planeNormalNormalized);

    // Create a vector between the starting points of the two lines
    PVector r = other.start - start;

    // Project r onto the plane
    PVector proj_r = r - planeNormalNormalized * r.dot(planeNormalNormalized);

    // Denominator for the 2D intersection calculation
    float denom = proj_d1.x * proj_d2.y - proj_d1.y * proj_d2.x;

    if (denom == 0) {
        // If denom is zero, lines are parallel or coincident, no unique intersection
        return PVector(); // No intersection, return a default vector
    }

    // Solve for t1 (parameter along the first line)
    float t1 = (proj_r.x * proj_d2.y - proj_r.y * proj_d2.x) / denom;

    // Calculate the intersection point in the plane
    PVector intersectionPlane = start + proj_d1 * t1;

    // Return the intersection point in 3D (still lying on the original plane)
    return intersectionPlane;
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

inline float dist(float x1, float y1, float x2, float y2) {
   float dx = x2 - x1;
   float dy = y2 - y1;
   return std::sqrt(dx * dx + dy * dy);
}

inline float randomGaussian()
{
   thread_local static std::mt19937 randomNumbers( 1 );
   thread_local static std::normal_distribution<float> dist(0.0f, 1.0f);
   return dist(randomNumbers);
}

inline float radians(float degrees) {
   return degrees * PI / 180.0;
}

inline float degrees(float radians) {
   return (radians * 180) / PI;
}

inline float norm(float value, float start, float stop) {
   return (value - start) / (stop - start);
}

inline float norm(float value, float start1, float stop1, float start2, float stop2) {
   float adjustedValue = (value - start1) / (stop1 - start1);
   return start2 + (stop2 - start2) * adjustedValue;
}

template <typename T>
inline T constrain(T value, T lower, T upper) {
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

template <>
struct fmt::formatter<glm::vec2> {
    // Format the MyClass object
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const glm::vec2& v, FormatContext& ctx) {
       return fmt::format_to(ctx.out(), "{:8.2f},{:8.2f}",v.x,v.y);
    }
};

template <>
struct fmt::formatter<glm::vec3> {
    // Format the MyClass object
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const glm::vec3& v, FormatContext& ctx) {
       return fmt::format_to(ctx.out(), "{:8.2f},{:8.2f},{:8.2f}",v.x,v.y,v.z);
    }
};

template <>
struct fmt::formatter<glm::vec4> {
    // Format the MyClass object
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const glm::vec4& v, FormatContext& ctx) {
       return fmt::format_to(ctx.out(), "{:8.2f},{:8.2f},{:8.2f},{:8.2f}",v.x,v.y,v.z,v.w);
    }
};

template <>
struct fmt::formatter<glm::mat4> {
    // Format the glm::mat4 object
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const glm::mat4& m, FormatContext& ctx) {
        return fmt::format_to(ctx.out(),
            "[\n"
            "  {:8.2f}, {:8.2f}, {:8.2f}, {:8.2f}\n"
            "  {:8.2f}, {:8.2f}, {:8.2f}, {:8.2f}\n"
            "  {:8.2f}, {:8.2f}, {:8.2f}, {:8.2f}\n"
            "  {:8.2f}, {:8.2f}, {:8.2f}, {:8.2f}\n"
            "]",
            m[0][0], m[0][1], m[0][2], m[0][3],
            m[1][0], m[1][1], m[1][2], m[1][3],
            m[2][0], m[2][1], m[2][2], m[2][3],
            m[3][0], m[3][1], m[3][2], m[3][3]);
    }
};
#endif
