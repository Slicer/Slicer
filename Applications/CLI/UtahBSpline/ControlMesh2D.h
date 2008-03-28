#ifndef CONTROLMESH2D_H
#define CONTROLMESH2D_H

template <typename T>
class ControlMesh2D{
  private:
    T **cps;
    int nU, nV;

    void deleteCPS(){
      if(cps == NULL) return;

      for(int i = 0; i < nU; i++){
         delete[] cps[i];        
       }
       delete[] cps;
    };

    void instanciateCPS(){
      cps = new T*[nU];
      for(int i = 0; i < nU; i++){
        cps[i] = new T[nV];
      }
    };



  public:
    ControlMesh2D(){
      nU = 0;
      nV = 0;
      cps = NULL;
    };

    ControlMesh2D(int u, int v){
      nU = u;
      nV = v;
      instanciateCPS();      
    };

    ~ControlMesh2D(){
      deleteCPS();
    };


    int LengthU(){
      return nU;
    };

    int LengthV(){
      return nV;
    };

    bool CheckIndex(int u, int v){
      if(u < 0 || u>=nU){
        return false;
      }
      if(v < 0 || v >=nV){
        return false;
      }
      return true;

    }

    T &Get(int u, int v){
      return cps[u][v]; 
    };


    void Set(int u, int v, const T &cp){
      cps[u][v] = cp;
    };

    //Operators
    ControlMesh2D<T>& operator=(const ControlMesh2D<T>& rhs){
      deleteCPS();

      nU = rhs.nU;
      nV = rhs.nV;
      instanciateCPS();
      for(int i =0; i < nU; i++){
         for(int j = 0; j < nV; j++){
            cps[i][j] =  rhs.cps[i][j];
         }
       } 
      return *this;
    };

    friend std::ostream& operator << (std::ostream& os, ControlMesh2D<T>& net){
       os << net.nU << std::endl;
       os << net.nV << std::endl;
       for(int i =0; i < net.nU; i++){
         //std::cout << i << std::endl;
         for(int j = 0; j < net.nV; j++){
            os << net.cps[i][j];
            //std::cout << "  " << j << std::endl;
         }
       }

      return os;

    };

    friend std::istream& operator >> (std::istream& is, ControlMesh2D<T>& net){
       net.deleteCPS();

       is >> net.nU;
       is >>  net.nV;

       net.instanciateCPS(); 

       for(int i =0; i < net.nU; i++){
         for(int j = 0; j < net.nV; j++){
            is >>  net.cps[i][j];
         }
       }   
       return is;

    };
};

#endif

