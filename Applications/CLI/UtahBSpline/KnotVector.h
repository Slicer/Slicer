#ifndef KNOTVECTOR_H
#define KNOTVECTOR_H

template <typename T>
class KnotVector{
  private:
    T *knots;
    int degree;
    int length;
    int n;
    int lower, upper;
    T *left;
    T *right;

    void deleteArrays(){
      if(knots != NULL) delete[] knots;
      if(left != NULL) delete[] left;
      if(right != NULL) delete[] right;
    };

    void instanciateArrays(){
       left = new T[degree + 1];
       right = new T[degree + 1];
       knots = new T[length];
    }

  public:
    KnotVector(){
      degree = 0;
      length = 0;
      n = 0;
      lower = 0;
      upper = 0;
      left = NULL;
      right = NULL;
      knots = NULL;
    };

    

    KnotVector(T *knots, int length, int degree);
   
    ~KnotVector(){
      deleteArrays(); 
    };

    int FindSpan(T u);
    T *BasisFunctions(T u);
    void BasisFunctions(T u, T* res);
    T *BasisFunctions(int span, T u);
    void BasisFunctions(int span, T u, T* res);

    void findRange(int i, T &start, T &end){
      int index = i + degree - lower;
      if(index < degree){
        index = degree;
      }
      start = knots[index];
      index = i+degree+upper;
      if(index > n+1){
        index = n+1;
      }
      end = knots[index];
    };

    inline int Length();
    inline T *GetKnots();
    inline T GetKnot(int i);
    inline void SetKnot(int i, T v);
    inline int GetDegree();
    inline int GetNumberOfSegments();
    inline int GetN();
    inline T GetKnotA();
    inline T GetKnotB();
   
    //Static convenice functions 
    static KnotVector<T> createUniformKnotsClamped(int nControlPoints, int
                                                  degree, int &n);
    static KnotVector<T> createUniformKnotsUnclamped(int nControlPoints, int degree,
                                                     int &n);

   
    //Operators
    KnotVector<T>& operator=(const KnotVector<T>& rhs){
      deleteArrays();

      degree = rhs.degree;
      length = rhs.length;
      n = rhs.n;
      lower = rhs.lower;
      upper = rhs.upper;
      
      instanciateArrays();


      for(int i =0; i < length; i++){
          knots[i] = rhs.knots[i];
      } 
      return *this;
    };


    friend std::ostream& operator << (std::ostream& os, KnotVector<T>& kv){
       os << kv.degree << std::endl;
       os << kv.length << std::endl;
       os << kv.n << std::endl;
       os << kv.lower << std::endl;
       os << kv.upper << std::endl;
       for(int i = 0; i < kv.length; i++){
         os << kv.knots[i] << std::endl;
       }
  
       return os;
    };

    friend std::istream& operator >> (std::istream& is,KnotVector<T>& kv){
       is >> kv.degree;
       is >> kv.length; 
       is >> kv.n;
       is >> kv.lower;
       is >> kv.upper;
       kv.deleteArrays();
       kv.instanciateArrays();

       for(int i = 0; i < kv.length; i++){
         is >> kv.knots[i];
       }
       return is;

    };
  
};



//Non-inline implementations 
template <typename T>
KnotVector<T>::KnotVector(T *knots, int length, int degree) {
  this->knots = new T[length];
  for(int i=0; i < length; i++){
    this->knots[i] = knots[i];
  }
  this->length = length;
  this->degree = degree;
  lower = (int)ceil((degree+1) / 2.0);
  upper = (int)floor((degree+1) / 2.0);

  n = length - degree - 2;
  for (int i = 1; i < length; i++) {
    if (knots[i - 1] > knots[i]) {
      throw "Not a valid knot vector";                
    }
  }  
  left = new T[degree + 1];
  right = new T[degree + 1];

}


/**
 * Finds the span (Position of corresponding knot values in knot vector) a
 * given value belongs to.
 */
template <typename T>
int KnotVector<T>::FindSpan(T u) {
    if (u >= knots[n+1])
      return n;
    if(u <= knots[degree+1] )
      return degree;
    int low = degree;
    int high = n + 1;
    int mid = (low + high) / 2;
    while (u < knots[mid] || u >= knots[mid + 1]) { 
      if (u < knots[mid])
        high = mid;
      else
        low = mid;
      mid =  (low + high) / 2 ;
    }
   
    
    return mid;
}

/**
 * Gets the basis function values for the given u value. This function
 * calculates first the span which is needed in order to calculate the
 * basis functions values.
 */
template <typename T>
T *KnotVector<T>::BasisFunctions(T u) {
  return BasisFunctions(FindSpan(u), u);
}

template <typename T>
void KnotVector<T>::BasisFunctions(T u, T *res) {
  return BasisFunctions(FindSpan(u), u, res);
}

/**
 * Calculates the basis function values for the given u value, when already
 * is know in which span u lies.
 */
template <typename T>
T *KnotVector<T>::BasisFunctions(int span, T u) {
  T *res = new T[degree + 1];
  BasisFunctions(span, u, res);  
  return res;
}

template <typename T>
void KnotVector<T>::BasisFunctions(int span, T u, T* res) {
  res[0] = 1;
  for (int j = 1; j <= degree; j++) {
    left[j] = u - knots[span + 1 - j];
    right[j] = knots[span + j] - u;
    T saved = 0;
    for (int r = 0; r < j; r++) {
      T tmp = res[r] / (right[r + 1] + left[j - r]);
      res[r] = saved + right[r + 1] * tmp;
      saved = left[j - r] * tmp;
    }
    res[j] = saved;
  }


}

template <typename T>
int KnotVector<T>::GetN() {
  return n;
}

template <typename T>
int KnotVector<T>::Length(){
  return length;
}

/**
 * get the knot values as float array
 */
template <typename T>
T *KnotVector<T>::GetKnots() {
  return knots;
}

/**
 * Get the knot value at a specific index.
 */
template <typename T>
T KnotVector<T>::GetKnot(int i) {
  return knots[i];
}

template <typename T>
T KnotVector<T>::GetKnotA() {
  return knots[degree];
}

template <typename T>
T KnotVector<T>::GetKnotB() {
  return knots[n+1];
}



/**
 * Set the knot value at a specific index.
 * After this operation a call to isValid may be needed if one is not sure if the
 * KnotVector with the changed value is valid for a Nurbs.
 */
template <typename T>
void KnotVector<T>::SetKnot(int i, T val) {
  knots[i] = val;
}

template <typename T>
int KnotVector<T>::GetDegree() {
  return degree;
}

template <typename T>
int KnotVector<T>::GetNumberOfSegments(){
  int seg=0;
  T u=knots[0];
  for(int i=1; i< length; i++){
    if(u!=knots[i]){
      seg++;
      u=knots[i];
    }
  }
  return seg;
}


template <typename T>
KnotVector<T> 
KnotVector<T>::createUniformKnotsClamped(int nControlPoints, int degree, int &n){
  n = nControlPoints + degree + 1;
  T *knots =  new T[n];
  for(int i = 0; i<=degree; i++){
    knots[i] = 0;
    knots[n - 1 - i] = 1;
  }  
  int nInterior = n - 2*degree - 2;
  T step = 1.0 / (nInterior + 1 );
  for(int i = 1; i <= nInterior; i++){
    knots[degree + i] = i * step;
  }
   
  KnotVector<T> result(knots, n, degree);
  delete[] knots;
  return result;  
}



template <typename T>
KnotVector<T>
KnotVector<T>::createUniformKnotsUnclamped(int nControlPoints, int degree, int &n){
  n = nControlPoints + degree + 1;
  T *knots =  new T[ n ];
  for(int i = 0; i<n; i++){
    knots[i] = i;
  } 
  KnotVector<T> result(knots, n, degree);
  delete[] knots;
  return result;   
}




#endif


