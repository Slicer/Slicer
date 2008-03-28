#ifndef POINT3D_H
#define POINT3D_H

template <typename T>
class Point3D{
  
  private:

  public:
    typedef T TPrecision;
    T x, y, z;
    
    //Constructors
    Point3D(T xc, T yc, T zc):x(xc), y(yc), z(zc) {};
    Point3D():x(0), y(0), z(0){};
    Point3D(const Point3D<T> &copy){
      x = copy.x;
      y = copy.y;
      z = copy.z;
    };


    void Set(T x, T y, T z){
      this->x = x;
      this->y = y;
      this->z = z;
    };


    void Zero(){
      x = 0;
      y = 0;
      z = 0;
    };

    //Operators
    Point3D<T>& operator=(const Point3D<T>& rhs){
      x = rhs.x;
      y = rhs.y;
      z = rhs.z;
      return *this;
    };

    Point3D<T>& operator+=(const Point3D<T>& rhs){
      x = x+rhs.x;
      y = y+rhs.y;
      z = z+rhs.z;
      return *this;
    };
    
    Point3D<T> operator+(const Point3D<T>& rhs){
      Point3D<T> res = *this;
      res += rhs;
      return res;      
    };
    
    Point3D<T>& operator-=(const Point3D<T>& rhs){
      x = x-rhs.x;
      y = y-rhs.y;
      z = z-rhs.z;
      return *this;
    };
    
    Point3D<T> operator-(const Point3D<T>& rhs){
      Point3D<T> res = *this;
      res -= rhs;
      return res;      
    };

    Point3D<T>& operator*=(const double s){
      x = x*s;
      y = y*s;
      z = z*s;
      return *this;
    };
    
    Point3D<T> operator*(const double s){
      Point3D<T> res = *this;
      res *= s;
      return res;      
    };
    
    Point3D<T>& operator/=(const double s){
      x = x/s;
      y = y/s;
      z = z/s;
      return *this;
    };
    
    Point3D<T> operator/(const double s){
      Point3D<T> res = *this;
      res /= s;
      return res;      
    };
    
    friend std::ostream& operator << (std::ostream& os, Point3D<T>& cp){
       os << cp.x << std::endl;
       os << cp.y << std::endl;
       os << cp.z << std::endl;
       return os;
    };

    friend std::istream& operator >> (std::istream& is, Point3D<T>& cp){
       is >> cp.x; 
       is >> cp.y;
       is >> cp.z;
       return is;
    };

};

#endif
