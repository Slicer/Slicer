/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
#ifndef __vtkPichonFastMarching_h
#define __vtkPichonFastMarching_h

#include "vtkSlicerEditorLibModuleLogicExport.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkImageAlgorithm.h>
#include <vtkVersion.h>

// STD includes
#include <vector>
#include <algorithm>

#define MAJOR_VERSION 3
#define MINOR_VERSION 1
#define DATE_VERSION "2003-1-27/20:00EST"

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/// pretty big
#define INF 1e20

/// outside margin
#define BAND_OUT 3

#define GRANULARITY_PROGRESS 20

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

typedef enum fmstatus { fmsDONE, fmsKNOWN, fmsTRIAL, fmsFAR, fmsOUT } FMstatus;
#define MASK_BIT 256

struct FMnode {
  FMstatus status;
  float T;
  int leafIndex;
};

struct FMleaf {
  int nodeIndex;
};

/// these typedef are for tclwrapper...
typedef std::vector<FMleaf> VecFMleaf;
typedef std::vector<int> VecInt;

class PichonFastMarchingPDF;

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
class VTK_SLICER_EDITORLIB_MODULE_LOGIC_EXPORT vtkPichonFastMarching
  : public vtkImageAlgorithm
{
public:
  static vtkPichonFastMarching *New();
  vtkTypeMacro(vtkPichonFastMarching,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void init(int dimX, int dimY, int dimZ, double depth, double dx, double dy, double dz);

  void setActiveLabel(int label);

  void initNewExpansion();

  int nValidSeeds();
  int nKnownPoints();

  void setNPointsEvolution( int n );

  void setInData(short* data);
  void setOutData(short* data);

  void setRAStoIJKmatrix(float m11, float m12, float m13, float m14,
             float m21, float m22, float m23, float m24,
             float m31, float m32, float m33, float m34,
             float m41, float m42, float m43, float m44);

  int addSeed( float r, float a, float s );
  int addSeedIJK( int, int, int );
  int addSeedsFromImage(vtkImageData*);

  void show(float r);

  char* cxxVersionString();
  int cxxMajorVersion();
  void tweak(char *name, double value);

protected:
  vtkPichonFastMarching();
  ~vtkPichonFastMarching() override;

  void ExecuteDataWithInformation(vtkDataObject *, vtkInformation *) override;


  friend void vtkPichonFastMarchingExecute(vtkPichonFastMarching *self,
                     vtkImageData *inData, short *inPtr,
                     vtkImageData *outData, short *outPtr,
                     int outExt[6]);

private:
  //pb wrap  vtkPichonFastMarching()(const vtkPichonFastMarching&);
  //pb wrap  void operator=(const vtkPichonFastMarching&);

  bool invalidInputs;

  double powerSpeed;

  int nNeighbors; /// =6 pb wrap, cannot be defined as constant
  int arrayShiftNeighbor[27];
  double arrayDistanceNeighbor[27];
  int tmpNeighborhood[125]; /// allocate it here so that we do not have to
  /// allocate it over and over in getMedianInhomo

  float dx;
  float dy;
  float dz;

  float invDx2;
  float invDy2;
  float invDz2;

  bool initialized;
  bool firstCall;

  FMnode *node;  /// arrival time and status for all voxels
  int *inhomo; /// inhomogeneity
  int *median; /// medican intensity

  short* outdata; /// output
  short* indata;  /// input

  /// size of the indata (=size outdata, node, inhomo)
  int dimX;
  int dimY;
  int dimZ;
  int dimXY; /// dimX*dimY
  int dimXYZ; /// dimX*dimY*dimZ
  /// coeficients of the RAS2IJK matrix
  float m11;
  float m12;
  float m13;
  float m14;

  float m21;
  float m22;
  float m23;
  float m24;

  float m31;
  float m32;
  float m33;
  float m34;

  float m41;
  float m42;
  float m43;
  float m44;

  int label;
  int depth;

  int nPointsEvolution;
  int nPointsBeforeLeakEvolution;
  int nEvolutions;

  VecInt knownPoints;
  /// vector<int> knownPoints

  VecInt seedPoints;
  /// vector<int> seedPoints

  /// minheap used by the fast marching algorithm
  VecFMleaf tree;
  ///  vector<FMleaf> tree;

  PichonFastMarchingPDF *pdfIntensityIn;
  PichonFastMarchingPDF *pdfInhomoIn;

  bool firstPassThroughShow;

  /// minheap methods
  bool emptyTree();
  void insert(const FMleaf leaf);
  FMleaf removeSmallest();
  void downTree(int index);
  void upTree(int index);

  int indexFather(int index );

  void getMedianInhomo(int index, int &median, int &inhomo );

  int shiftNeighbor(int n);
  double distanceNeighbor(int n);
  float computeT(int index );

  void setSeed(int index );

  void collectInfoSeed(int index );
  void collectInfoAll();

  float speed(int index );

  bool minHeapIsSorted();

  /* perform one step of fast marching
     return the leaf which has just been added to fmsKNOWN */
  float step();

private:
  vtkPichonFastMarching(const vtkPichonFastMarching&) = delete;
  void operator=(const vtkPichonFastMarching&) = delete;
};

#endif


