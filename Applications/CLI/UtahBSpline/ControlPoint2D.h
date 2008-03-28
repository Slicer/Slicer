#ifndef CONTROLPOINT2D_H
#define CONTROLPOINT2D_H

template <typename T>
class ControlPoint2D{
  
  private:

  public:
    typedef T TPrecision;
    T x, y, w;
    
    //Constructors
    ControlPoint2D(T xc, T yc):x(xc), y(yc), w(1) {};
    ControlPoint2D():x(0), y(0), w(0) {};
    ControlPoint2D(const ControlPoint2D<T> &copy){
      x = copy.x;
      y = copy.y;
      w = copy.w;  
    };


    void Set(T x, T y, T w){
      this->x = x;
      this->y = y;
      this->w = w;
    };

    void Zero(){
      x = 0;
      y = 0;
      w = 0;
    };


    void Weight(){
      x *= w;
      y *= w;
    };

    void Unweight(){
      x /= w;
      y /= w;
    };

    //Operators
    ControlPoint2D<T>& operator=(const ControlPoint2D<T>& rhs){
      x = rhs.x;
      y = rhs.y;
      w = rhs.w;
      return *this;
    };

    ControlPoint2D<T>& operator+=(const ControlPoint2D<T>& rhs){
      x = x+rhs.x;
      y = y+rhs.y;
      w = w+rhs.w;
      return *this;
    };
    
    ControlPoint2D<T> operator+(const ControlPoint2D<T>& rhs){
      ControlPoint2D<T> res = *this;
      res += rhs;
      return res;      
    };
    
    ControlPoint2D<T>& operator-=(const ControlPoint2D<T>& rhs){
      x = x-rhs.x;
      y = y-rhs.y;
      w = w-rhs.w;
      return *this;
    };
    
    ControlPoint2D<T> operator-(const ControlPoint2D<T>& rhs){
      ControlPoint2D<T> res = *this;
      res -= rhs;
      return res;      
    };

    ControlPoint2D<T>& operator*=(const double s){
      x = x*s;
      y = y*s;
      w = w*s;
      return *this;
    };
    
    ControlPoint2D<T> operator*(const double s){
      ControlPoint2D<T> res = *this;
      res *= s;
      return res;      
    };
    
    ControlPoint2D<T>& operator/=(const double s){
      x = x/s;
      y = y/s;
      w = w/s;
      return *this;
    };
    
    ControlPoint2D<T> operator/(const double s){
      ControlPoint2D<T> res = *this;
      res /= s;
      return res;      
    };

    friend std::ostream& operator << (std::ostream& os, ControlPoint2D<T>& cp){
       os << cp.x << std::endl;
       os << cp.y << std::endl;
       os << cp.w << std::endl;
    };

    friend std::istream& operator >> (std::istream& is, ControlPoint2D<T>& cp){
       is >> cp.x;
       is >> cp.y;
       is >> cp.w;

    };
};

#endif
