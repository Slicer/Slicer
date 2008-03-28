#ifndef POINT2D_H
#define POINT2D_H

template <typename T>
class Point2D{
  
  private:

  public:
    typedef T TPrecision;
    T x, y;
    
    //Constructors
    Point2D(T xc, T yc):x(xc), y(yc) {};
    Point2D():x(0), y(0){};
    Point2D(const Point2D<T> &copy){
      x = copy.x;
      y = copy.y;
    };


    void Set(T x, T y){
      this->x = x;
      this->y = y;
    }


    void Zero(){
      x = 0;
      y = 0;
    };

    //Operators
    Point2D<T>& operator=(const Point2D<T>& rhs){
      x = rhs.x;
      y = rhs.y;
      return *this;
    };

    Point2D<T>& operator+=(const Point2D<T>& rhs){
      x = x+rhs.x;
      y = y+rhs.y;
      return *this;
    };
    
    Point2D<T> operator+(const Point2D<T>& rhs){
      Point2D<T> res = *this;
      res += rhs;
      return res;      
    };
    
    Point2D<T>& operator-=(const Point2D<T>& rhs){
      x = x-rhs.x;
      y = y-rhs.y;
      return *this;
    };
    
    Point2D<T> operator-(const Point2D<T>& rhs){
      Point2D<T> res = *this;
      res -= rhs;
      return res;      
    };

    Point2D<T>& operator*=(const double s){
      x = x*s;
      y = y*s;
      return *this;
    };
    
    Point2D<T> operator*(const double s){
      Point2D<T> res = *this;
      res *= s;
      return res;      
    };
    
    Point2D<T>& operator/=(const double s){
      x = x/s;
      y = y/s;
      return *this;
    };
    
    Point2D<T> operator/(const double s){
      Point2D<T> res = *this;
      res /= s;
      return res;      
    };
    
    friend std::ostream& operator << (std::ostream& os, Point2D<T>& cp){
       os << cp.x << std::endl;
       os << cp.y << std::endl;
       return os;
    };

    friend std::istream& operator >> (std::istream& is, Point2D<T>& cp){
       is >> cp.x; 
       is >> cp.y;
       return is;
    };;

};

#endif
