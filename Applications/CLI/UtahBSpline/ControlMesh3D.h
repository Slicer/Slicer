#ifndef CONTROLMESH3D_H
#define CONTROLMESH3D_H

template <typename T>
class ControlMesh3D{
  private:
    T ***cps;
    int nU, nV, nW;

    void deleteCPS(){
      if(cps == NULL) return;

      for(int i = 0; i < nU; i++){
         for(int j = 0; j < nV; j++){
          delete[] cps[i][j];
         } 
         delete[] cps[i];        
       }
       delete[] cps;
    };

    void instanciateCPS(){
      cps = new T**[nU];
      for(int i = 0; i < nU; i++){
        cps[i] = new T*[nV];
        for(int j=0; j<nV; j++){
          cps[i][j] = new T[nW];
        }
      }
    };



  public:
    ControlMesh3D(){
      nU = 0;
      nV = 0;
      nW = 0;
      cps = NULL;
    };

    ControlMesh3D(int u, int v, int w){
      nU = u;
      nV = v;
      nW =w;
      instanciateCPS();      
    };

    ~ControlMesh3D(){
      deleteCPS();
    };


    int LengthU(){
      return nU;
    };

    int LengthV(){
      return nV;
    };

    int LengthW(){
      return nW;
    };


    bool CheckIndex(int u, int v, int w){
      if(u < 0 || u>=nU){
        return false;
      }
      if(v < 0 || v >=nV){
        return false;
      }
      if(w < 0 || w >=nW){
        return false;
      }

      return true;

    }

    T &Get(int u, int v, int w){
      return cps[u][v][w]; 
    };


    void Set(int u, int v, int w, const T &cp){
      cps[u][v][w] = cp;
    };

    //Operators
    ControlMesh3D<T>& operator=(const ControlMesh3D<T>& rhs){
      deleteCPS();

      nU = rhs.nU;
      nV = rhs.nV;
      nW = rhs.nW;

      instanciateCPS();
      for(int i =0; i < nU; i++){
         for(int j = 0; j < nV; j++){
           for(int k = 0; k < nW; k++){
              cps[i][j][k] =  rhs.cps[i][j][k];
           }
         }
       } 
      return *this;
    };

    friend std::ostream& operator << (std::ostream& os, ControlMesh3D<T>& vol){
       os << vol.nU << std::endl;
       os << vol.nV << std::endl;
       os << vol.nW << std::endl;

       for(int i =0; i < vol.nU; i++){
         //std::cout << i << std::endl;
         for(int j = 0; j < vol.nV; j++){
           for(int k = 0; k < vol.nW; k++){
              os << vol.cps[i][j][k];
            }
            //std::cout << "  " << j << std::endl;
         }
       }
      return os;
    };

    friend std::istream& operator >> (std::istream& is, ControlMesh3D<T>& vol){
       vol.deleteCPS();

       is >> vol.nU;
       is >> vol.nV;
       is >> vol.nW;

       vol.instanciateCPS(); 

       for(int i =0; i < vol.nU; i++){
         for(int j = 0; j < vol.nV; j++){
           for(int k = 0; k < vol.nW; k++){
             is >>  vol.cps[i][j][k];
           }
         }
       }   
       return is;

    };
};

#endif

