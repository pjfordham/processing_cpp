// Ariel and V3ga's arcball class with a couple tiny mods by Robert Hodgin


class Vec3 {
public:
  float x, y, z;

  Vec3(){
  }

  Vec3(float x, float y, float z){
    this->x = x;
    this->y = y;
    this->z = z;
  }

  void normalize(){
    float len = length();
    x /= len;
    y /= len;
    z /= len;
  }

  float length(){
    return sqrt(x * x + y * y + z * z);
  }

  static Vec3 cross(Vec3 v1, Vec3 v2){
    Vec3 res;
    res.x = v1.y * v2.z - v1.z * v2.y;
    res.y = v1.z * v2.x - v1.x * v2.z;
    res.z = v1.x * v2.y - v1.y * v2.x;
    return res;
  }

  static float dot(Vec3 v1, Vec3 v2){
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  static Vec3 mul(Vec3 v, float d){
    Vec3 res;
    res.x = v.x * d;
    res.y = v.y * d;
    res.z = v.z * d;
    return res;
  }

  void sub(Vec3 v1, Vec3 v2){
    x = v1.x - v2.x;
    y = v1.y - v2.y;
    z = v1.z - v2.z;
  }
};

class Quat{
public:
  float w, x, y, z;

  Quat(){
    reset();
  }

  Quat(float w, float x, float y, float z){
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;
  }

  void reset(){
    w = 1.0f;
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
  }

  void set(float w, Vec3 v){
    this->w = w;
    x = v.x;
    y = v.y;
    z = v.z;
  }

  void set(Quat q){
    w = q.w;
    x = q.x;
    y = q.y;
    z = q.z;
  }

  static Quat mul(Quat q1, Quat q2){
    Quat res;
    res.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    res.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    res.y = q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z;
    res.z = q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x;
    return res;
  }

#define EPSILON 0.0001

   std::vector<float> getValue(){
    // transforming this quat into an angle and an axis vector...

    std::vector<float> res(4);

    float sa = sqrt(1.0f - w * w);
    if (sa < EPSILON){
      sa = 1.0f;
    }

    res[0] = (float) acos(w) * 2.0f;
    res[1] = x / sa;
    res[2] = y / sa;
    res[3] = z / sa;

    return res;
  }
};

class Arcball {
public:
  PSurface *parent;
  float center_x, center_y, radius;
  Vec3 v_down, v_drag;
  Quat q_now, q_down, q_drag;
  std::vector<Vec3> axisSet;
  int axis;
  float mxv, myv;
  float x, y;

   Arcball() {}
   Arcball(PSurface *parent, float radius){
    this->parent = parent;
    this->radius = radius;

    axisSet = std::vector<Vec3>{
       Vec3(1.0f, 0.0f, 0.0f),
       Vec3(0.0f, 1.0f, 0.0f),
       Vec3(0.0f, 0.0f, 1.0f) };
    axis = -1;  // no constraints...
  }

  void mousePressed(){
    v_down = mouse_to_sphere(parent->mouseX, parent->mouseY);
    q_down.set(q_now);
    q_drag.reset();
  }

  void mouseDragged(){
    v_drag = mouse_to_sphere(parent->mouseX, parent->mouseY);
    q_drag.set(Vec3::dot(v_down, v_drag), Vec3::cross(v_down, v_drag));
  }

  void run(){
    center_x = parent->width/2.0;
    center_y = parent->height/2.0;

    q_now = Quat::mul(q_drag, q_down);
    parent->g.translate(center_x, center_y);
    applyQuat2Matrix(q_now);

    x += mxv;
    y += myv;
    mxv -= mxv * .01;
    myv -= myv * .01;
  }

  Vec3 mouse_to_sphere(float x, float y){
    Vec3 v;
    v.x = (x - center_x) / radius;
    v.y = (y - center_y) / radius;

    float mag = v.x * v.x + v.y * v.y;
    if (mag > 1.0f){
      v.normalize();
    } else {
      v.z = sqrt(1.0f - mag);
    }

    return (axis == -1) ? v : constrain_vector(v, axisSet[axis]);
  }

  Vec3 constrain_vector(Vec3 vector, Vec3 axis){
    Vec3 res;
    res.sub(vector, Vec3::mul(axis, Vec3::dot(axis, vector)));
    res.normalize();
    return res;
  }

  void applyQuat2Matrix(Quat q){
    // instead of transforming q into a matrix and applying it...

    std::vector<float> aa = q.getValue();
    parent->g.rotate(aa[0], aa[1], aa[2], aa[3]);
  }
};
