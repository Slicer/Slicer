/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkDataDef.h,v $
  Date:      $Date: 2006/12/08 23:28:24 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
#ifndef __vtkDataDef_h
#define __vtkDataDef_h


#include "vtkEMSegment.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

/// ----------------------------------------------------
/// Stucture and function needed for convolution
/// ----------------------------------------------------

//
//BTX - begin tcl exclude
//
/// From Simon convolution.cxx

typedef struct {
  float *input;
  int nrow;
  int ncol;
  int nslice;
  float *filter;
  int *indexes;
  int numindexes;
  int M1;
  int M2;
  int N1;
  int N2;
  int O1;
  int O2;
  float *output;
  int startindex;
  int endindex;   /* Process voxels in the range [startindex, endindex) */
} convolution_filter_work;

// ----------------------------------------------------------------------------------------------
/// Definitions for 3D float array EMVolume
/// ----------------------------------------------------------------------------------------------/ 

/// Kilian turn around dimension so it is y,x,z like in matlab ! 
class VTK_EMSEGMENT_EXPORT EMVolume {
public:
  //static EMVolume *New() {return (new vtkDataDef);}
  EMVolume(){this->Data  = NULL;this->MaxX = this->MaxY = this->MaxZ = this->MaxXY = this->MaxXYZ = 0;}
  EMVolume(int initZ,int initY, int initX) {this->allocate(initZ,initY, initX);}
  ~EMVolume() {this->deallocate();}

  /// Convolution in all three direction 
  /// Can be made using less memory but then it will be probably be slower
  void Conv(float *v,int vLen) {
    this->ConvY(v,vLen);
    this->ConvX(v,vLen);
    this->ConvZ(v,vLen);
  }
  void Resize(int DimZ,int DimY,int DimX) {
    if ((this->MaxX == DimX) && (this->MaxY == DimY) && (this->MaxZ == DimZ)) return;
    this->deallocate();this->allocate(DimZ,DimY,DimX);
  }

  float& operator () (int z,int y, int x) {return this->Data[x+this->MaxX*y + this->MaxXY*z];}
  const float& operator () (int z,int y, int x) const {return this->Data[x+this->MaxX*y + this->MaxXY*z];}

  EMVolume & operator = (const EMVolume &trg) {
    if ( this->Data == trg.Data ) return *this;
    /// Has to be of the same dimension
    if ((this->MaxX == trg.MaxX) && (this->MaxY == trg.MaxY) && (this->MaxZ == trg.MaxZ))
      {
      memcpy(this->Data,trg.Data,sizeof(float)*this->MaxXYZ);
      }
    else
      {
      std::cerr << "operator=: dimensions are not the same." << std::endl;
      }
    return *this;
  }

  /// Kilian : Just to be compatible with older version
  void Conv(double *v,int vLen);
  /// End -  Multi Thread Function
  void ConvY(float *v, int vLen);
  void ConvX(float *v, int vLen);
  /// Same as above only sorce and target Volume are different => Slower 
  void ConvX(EMVolume &src,float v[], int vLen);
  void ConvZ(float *v, int vLen);
  /// Same as above only sorce and target Volume are different => Slower 
  void ConvZ(EMVolume &src,float v[], int vLen);
  void Print(char name[]);
  void Test(int Vdim);
  void SetValue(float val) {
    if (val) {for (int i = 0; i < this->MaxXYZ; i++ ) this->Data[i] = val;}
    else { memset(this->Data, 0,sizeof(float)*this->MaxXYZ);}
  }
  float* GetData() {return this->Data;}

  void SaveDataToFile(char *FileName) {
    char VolumeFileName[1024];
    sprintf(VolumeFileName,"%s.img",FileName); 
    FILE *File= fopen(VolumeFileName, "wb");
    /// Could not open file
    if (File == NULL)
      {
      std::cerr << "SaveDataToFile: unable to open file " << VolumeFileName << " for writing." << std::endl;
      return;
      }
    size_t written = fwrite(this->Data, sizeof(float),this->MaxXYZ, File);
    if (written != (size_t)(this->MaxXYZ))
      {
      std::cerr << "SaveDataToFile: failed to write " << this->MaxXYZ << " to file " << VolumeFileName << ", instead wrote " << written << std::endl;
      }
    fflush(File);
    fclose(File);

#ifndef _WIN32
    /// Compress file 
    char command[1024];
    sprintf(command,"gzip --fast -f \"%s\"",VolumeFileName);
    if (system(command) != 0)
      {
      std::cout << "Error runing system command " << command << std::endl;
      }
#endif
  }
 

 void ReadDataFromFile(char *FileName) {
    char VolumeFileName[1024];

#ifndef _WIN32
    /// First uncompress file
    char command[1024];
    sprintf(command,"gunzip --fast -f \"%s.img.gz\"",FileName);
    /// Do not quit here bc could have been unzipped before - if file does not exist next check will catch it 
    int returnVal = system(command);
    if (returnVal != 0)
      {
      std::cout <<  "ReadDataFromFile: error executing system command " << command << std::endl;
      }
#endif

    sprintf(VolumeFileName,"%s.img",FileName);
    FILE *File= fopen(VolumeFileName, "rb");
    /// Could not open file
    if (File == NULL)
      {
      std::cerr << "ReadDataFromFile: unable to open file " << VolumeFileName << std::endl;
      return;
      }
    size_t numRead = fread (this->Data,sizeof(float),this->MaxXYZ,File);
    if (numRead != (size_t)(this->MaxXYZ))
      {
      std::cerr << "ReadDataFromFile: error reading " << VolumeFileName << ", expected to read " << this->MaxXYZ << ", instead got " << numRead << std::endl;
      }
    /// terminate
    fclose (File);

#ifndef _WIN32
    /// Compress it back 
    sprintf(command,"gzip --fast -f \"%s\"",VolumeFileName);
    returnVal = system(command);
    if (returnVal != 0)
      {
      std::cout << "ReadDataFromFile: error executing system command " << command << std::endl;
      }
#endif

 }

 void EraseDataFile(char *FileName)
 {
    char VolumeFileName[1024];
#ifndef _WIN32
    /// Compress it back 
    sprintf(VolumeFileName,"%s.img.gz",FileName);
#else 
    sprintf(VolumeFileName,"%s.img",FileName);
#endif
    if (remove(VolumeFileName) != 0)
      {
      std::cerr << "EraseDataFile: error erasing file " << VolumeFileName << std::endl;
      }
 }

 int GetMaxX() {return this->MaxX;} 
 int GetMaxY() {return this->MaxY;} 
 int GetMaxZ() {return this->MaxZ;} 


protected :
  float *Data;
  int MaxX, MaxY, MaxZ, MaxXY, MaxXYZ;

  void allocate (int initZ,int initY, int initX) {
    this->MaxX  = initX;this->MaxY  = initY;this->MaxZ  = initZ;
    this->MaxXY = initX*initY; this->MaxXYZ = this->MaxXY*this->MaxZ;
    this->Data = new float[this->MaxXYZ];
  } 

  void deallocate () {
    if (this->Data) delete[] this->Data;
    this->Data  = NULL;this->MaxX = this->MaxY = this->MaxZ  =  this->MaxXY = this->MaxXYZ = 0;
  }


}; 

/// ----------------------------------------------------------------------------------------------
/// Definitions for 5D float array EMTriVolume (lower triangle)
/// ----------------------------------------------------------------------------------------------/ 
/// It is a 5 dimensional Volume where m[t1][t2][z][y][x] t1>= t2 is only defined 
/// Lower Traingular matrix - or a symmetric matrix where you only save the lower triangle
class VTK_EMSEGMENT_EXPORT EMTriVolume {
protected :
  EMVolume **TriVolume;
  int Dim;

  void allocate (int initDim, int initZ,int initY, int initX) {
    this->Dim = initDim;
    int x,y;
    this->TriVolume = new EMVolume*[Dim];
    for (y=0; y < initDim; y++) {
      this->TriVolume[y] = new EMVolume[y+1];
      for (x = 0; x <= y ; x++) this->TriVolume[y][x].Resize(initZ,initY,initX);
    }
  }
  void deallocate () {
    if (this->TriVolume) {
      for (int y = 0; y < this->Dim; y++) delete[] this->TriVolume[y];
      delete[] this->TriVolume;
    }
    this->TriVolume  = NULL;this->Dim  = 0;
  }
public:
  EMTriVolume(){this->TriVolume  = NULL;this->Dim  = 0;}
  EMTriVolume(int initDim,int initZ,int initY, int initX) {this->allocate(initDim,initZ,initY, initX);}
  ~EMTriVolume() {this->deallocate();}

  void Resize(int initDim, int DimZ,int DimY,int DimX) {
    this->deallocate();this->allocate(initDim,DimZ,DimY,DimX);
  }

  float& operator () (int t1, int t2, int z,int y, int x) {return this->TriVolume[t1][t2](z,y,x);}
  const float& operator () (int t1, int t2, int z,int y, int x) const {return this->TriVolume[t1][t2](z,y,x);}

  EMTriVolume & operator = (const EMTriVolume &trg) {
    if ( this->TriVolume == trg.TriVolume) return *this;
    /// Has to be of the same dimension
    if (this->Dim == trg.Dim)
      {
      for (int y=0; y < this->Dim; y++)
        {  
        for (int x = 0; x <= y ; x++)
          {
          this->TriVolume[y][x] = trg.TriVolume[y][x];
          }
        }
      }
    else
      {
      std::cerr << "operator=: dimensions are not the same " << this->Dim << " != " << trg.Dim << std::endl;
      }
    return *this;
  }

  /// Kilian : Just to be compliant with old version
  void Conv(double *v,int vLen) {
    float *v_f = new float[vLen];
    for (int i = 0; i < vLen; i++) v_f[i] = float(v[i]);
    this->Conv(v_f,vLen);  
    delete[] v_f;
  }

  void Conv(float *v,int vLen) {
    int x,y;
    for (y=0 ; y < this->Dim; y++) { 
      for (x = 0; x <= y; x++) this->TriVolume[y][x].Conv(v,vLen);
    }
  }
  void SetValue(float val) {
    int x,y;
    for (y=0 ; y < this->Dim; y++) { 
      for (x = 0; x <= y; x++) this->TriVolume[y][x].SetValue(val);
    }
  }

  void SaveDataToFile(char *FileName) {
    char VolumeFileName[1024];
    for (int y=0; y < this->Dim; y++) {  
      for (int x = 0; x <= y ; x++) {
    sprintf(VolumeFileName,"%s_%d_%d",FileName,y,x); 
    this->TriVolume[y][x].SaveDataToFile(VolumeFileName);
      }
    }
  }

  void ReadDataFromFile(char *FileName) {
    char VolumeFileName[1024];
    for (int y=0; y < this->Dim; y++) {  
      for (int x = 0; x <= y ; x++) {
    sprintf(VolumeFileName,"%s_%d_%d",FileName,y,x); 
    this->TriVolume[y][x].ReadDataFromFile(VolumeFileName);
      }
    }
  }

void EraseDataFile(char *FileName) {
    char VolumeFileName[1024];
    for (int y=0; y < this->Dim; y++) {  
      for (int x = 0; x <= y ; x++) {
    sprintf(VolumeFileName,"%s_%d_%d",FileName,y,x); 
    this->TriVolume[y][x].EraseDataFile(VolumeFileName);
      }
    }
}
};


/// ----------------------------------------------------------------------------------------------
/// Dummy class 
/// ----------------------------------------------------------------------------------------------/ 
class VTK_EMSEGMENT_EXPORT vtkDataDef { 
public:
  static vtkDataDef *New() {return (new vtkDataDef);}
protected:

};

//
//ETX - end tcl exclude
//

#endif
