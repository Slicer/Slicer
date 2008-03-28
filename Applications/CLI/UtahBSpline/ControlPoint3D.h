#ifndef CONTROLPOINT3D_H
#define CONTROLPOINT3D_H

template <typename T>
class ControlPoint3D{
  
  private:

  public:
    typedef T TPrecision;
    T x, y, z, w;
    
    //Constructors
    ControlPoint3D(T xc, T yc, T zc):x(xc), y(yc), z(zc), w(1) {};
    ControlPoint3D():x(0), y(0), z(0), w(0) {};
    ControlPoint3D(const ControlPoint3D<T> &copy){
      x = copy.x;
      y = copy.y;
      z = copy.z;
      w = copy.w;  
    };


    void Set(T x, T y, T z, T w){
      this->x = x;
      this->y = y;
      this->z = z;
      this->w = w;
    };

    void Zero(){
      x = 0;
      y = 0;
      z = 0;
      w = 0;
    };


    void Weight(){
      x *= w;
      y *= w;
      z *= w;
    };

    void Unweight(){
      x /= w;
      y /= w;
      z /= w;
    };

    //Operators
    ControlPoint3D<T>& operator=(const ControlPoint3D<T>& rhs){
      x = rhs.x;
      y = rhs.y;
      z = rhs.z;
      w = rhs.w;
      return *this;
    };

    ControlPoint3D<T>& operator+=(const ControlPoint3D<T>& rhs){
      x = x+rhs.x;
      y = y+rhs.y;
      z = z+rhs.z;
      w = w+rhs.w;
      return *this;
    };
    
    ControlPoint3D<T> operator+(const ControlPoint3D<T>& rhs){
      ControlPoint3D<T> res = *this;
      res += rhs;
      return res;      
    };
    
    ControlPoint3D<T>& operator-=(const ControlPoint3D<T>& rhs){
      x = x-rhs.x;
      y = y-rhs.y;
      z = z-rhs.z;
      w = w-rhs.w;
      return *this;
    };
    
    ControlPoint3D<T> operator-(const ControlPoint3D<T>& rhs){
      ControlPoint3D<T> res = *this;
      res -= rhs;
      return res;      
    };

    ControlPoint3D<T>& operator*=(const double s){
      x = x*s;
      y = y*s;
      z = z*s;
      w = w*s;
      return *this;
    };
    
    ControlPoint3D<T> operator*(const double s){
      ControlPoint3D<T> res = *this;
      res *= s;
      return res;      
    };
    
    ControlPoint3D<T>& operator/=(const double s){
      x = x/s;
      y = y/s;
      z = z/s;
      w = w/s;
      return *this;
    };
    
    ControlPoint3D<T> operator/(const double s){
      ControlPoint3D<T> res = *this;
      res /= s;
      return res;      
    };

    friend std::ostream& operator << (std::ostream& os, ControlPoint3D<T>& cp){
       os << cp.x << std::endl;
       os << cp.y << std::endl;
       os << cp.z << std::endl;
       os << cp.w << std::endl;
    };

    friend std::istream& operator >> (std::istream& is, ControlPoint3D<T>& cp){
       is >> cp.x;
       is >> cp.y;
       is >> cp.z;
       is >> cp.w;

    };
};

#endif
