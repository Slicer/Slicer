#include "vtkFastGrowCutSeg.h"

const unsigned short SrcDimension = 3;
typedef float DistPixelType;  // float type pixel for cost function
//typedef short SrcPixelType;
//typedef unsigned char LabPixelType;
const int VTKDistPixelType = VTK_FLOAT;

#include <iostream>
#include <vector>

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include "FibHeap.h"

vtkStandardNewMacro(vtkFastGrowCutSeg);

//----------------------------------------------------------------------------
const float  DIST_INF = std::numeric_limits<float>::max();
const float  DIST_EPSILON = 1e-3;
unsigned char NNGBH = 26; // 3x3x3 pixel neighborhood (center voxel is not included)
typedef float FPixelType;

//----------------------------------------------------------------------------

class HeapNode : public FibHeapNode
{
  float   N;
  long IndexV;

public:
  HeapNode() : FibHeapNode() { N = 0; }
  virtual void operator =(FibHeapNode& RHS);
  virtual int  operator ==(FibHeapNode& RHS);
  virtual int  operator <(FibHeapNode& RHS);
  virtual void operator =(double NewKeyVal);
  virtual void Print();
  double GetKeyValue() { return N; }
  void SetKeyValue(double n) { N = n; }
  long int GetIndexValue() { return IndexV; }
  void SetIndexValue(long int v) { IndexV = v; }
};

void HeapNode::Print()
{
  FibHeapNode::Print();
}

void HeapNode::operator =(double NewKeyVal)
{
  HeapNode Tmp;
  Tmp.N = N = NewKeyVal;
  FHN_Assign(Tmp);
}

void HeapNode::operator =(FibHeapNode& RHS)
{
  FHN_Assign(RHS);
  N = ((HeapNode&)RHS).N;
}

int  HeapNode::operator ==(FibHeapNode& RHS)
{
  if (FHN_Cmp(RHS))
  {
    return 0;
  }
  return N == ((HeapNode&)RHS).N ? 1 : 0;
}

int  HeapNode::operator <(FibHeapNode& RHS)
{
  int X;
  if ((X = FHN_Cmp(RHS)) != 0)
  {
    return X < 0 ? 1 : 0;
  }
  return N < ((HeapNode&)RHS).N ? 1 : 0;
}

//----------------------------------------------------------------------------
class vtkFastGrowCutSeg::vtkInternal
{
public:
  vtkInternal();
  virtual ~vtkInternal();

  template<typename SrcPixelType, typename LabPixelType>
  bool InitializationAHP(vtkImageData *sourceVol, vtkImageData *seedVol, vtkImageData *outData);

  template<typename SrcPixelType, typename LabPixelType>
  void DijkstraBasedClassificationAHP(vtkImageData *sourceVol, vtkImageData *seedVol, vtkImageData *outData);

  template<typename SrcPixelType, typename LabPixelType>
  bool DoSegmentation(vtkImageData *sourceVol, vtkImageData *seedVol, vtkImageData *outData);

  template <class SourceVolType>
  void ExecuteGrowCut(vtkImageData *sourceVol, vtkImageData *seedVol, vtkImageData *outData);

  template< class SourceVolType, class SeedVolType>
  void ExecuteGrowCut2(vtkImageData *sourceVol, vtkImageData *seedVol, vtkImageData *outData);

  void SetImageSize(const int imSize[3]);

  /*
  std::vector<SrcPixelType> m_imSrc; // input1
  std::vector<LabPixelType> m_imSeed; // input2
  std::vector<LabPixelType> m_imLab; // output
  std::vector<LabPixelType> m_imLabPre; // last result
  std::vector<FPixelType> m_imDistPre; // last distance map
  std::vector<FPixelType> m_imDist; // current distance map
  */
  vtkSmartPointer<vtkImageData> m_imDist;
  vtkSmartPointer<vtkImageData> m_imDistPre;
  vtkSmartPointer<vtkImageData> m_imLabPre;

  int m_imSize[3];
  long m_DIMX, m_DIMY, m_DIMZ, m_DIMXY, m_DIMXYZ;
  std::vector<int> m_indOff;
  std::vector<unsigned char>  m_NBSIZE;

  FibHeap *m_heap;
  HeapNode *m_hpNodes;
  bool m_bSegInitialized;
};

//-----------------------------------------------------------------------------
vtkFastGrowCutSeg::vtkInternal::vtkInternal()
{
  m_heap = NULL;
  m_hpNodes = NULL;
  m_bSegInitialized = false;
  m_imDist = vtkSmartPointer<vtkImageData>::New();
  m_imDistPre = vtkSmartPointer<vtkImageData>::New();
  m_imLabPre = vtkSmartPointer<vtkImageData>::New();
};

//-----------------------------------------------------------------------------
vtkFastGrowCutSeg::vtkInternal::~vtkInternal()
{
  if (m_heap != NULL)
  {
    delete m_heap;
    m_heap = NULL;
  }
  if (m_hpNodes != NULL)
  {
    delete[]m_hpNodes;
    m_hpNodes = NULL;
  }
};

//-----------------------------------------------------------------------------
void vtkFastGrowCutSeg::vtkInternal::SetImageSize(const int imSize[3])
{
  m_imSize[0] = imSize[0];
  m_imSize[1] = imSize[1];
  m_imSize[2] = imSize[2];
};

//-----------------------------------------------------------------------------
template<typename SrcPixelType, typename LabPixelType>
bool vtkFastGrowCutSeg::vtkInternal::DoSegmentation(vtkImageData *sourceVol, vtkImageData *seedVol, vtkImageData *outData)
{
  if (!InitializationAHP<SrcPixelType, LabPixelType>(sourceVol, seedVol, outData))
  {
    return false;
  }
  DijkstraBasedClassificationAHP<SrcPixelType, LabPixelType>(sourceVol, seedVol, outData);
  return true;
};

//-----------------------------------------------------------------------------
template<typename SrcPixelType, typename LabPixelType>
bool vtkFastGrowCutSeg::vtkInternal::InitializationAHP(vtkImageData *sourceVol, vtkImageData *seedVol, vtkImageData *outData)
{
  m_DIMX = m_imSize[0];
  m_DIMY = m_imSize[1];
  m_DIMZ = m_imSize[2];
  m_DIMXY = m_DIMX*m_DIMY;
  m_DIMXYZ = m_DIMXY*m_DIMZ;

  std::cout << "DIM = [" << m_DIMX << ", " << m_DIMY << ", " << m_DIMZ << "]" << std::endl;


  if ((m_heap = new FibHeap) == NULL || (m_hpNodes = new HeapNode[m_DIMXYZ + 1]) == NULL)
  {
    std::cerr << "Memory allocation failed-- ABORTING.\n";
    return false;
  }
  m_heap->ClearHeapOwnership();

  SrcPixelType* imSrc = static_cast<SrcPixelType*>(sourceVol->GetScalarPointer());
  LabPixelType* imSeed = static_cast<LabPixelType*>(seedVol->GetScalarPointer());
  LabPixelType* imLab = static_cast<LabPixelType*>(outData->GetScalarPointer());

  long  i, j, k, index;
  if (!m_bSegInitialized)
  {
    m_imDist->SetExtent(seedVol->GetExtent());
    m_imDist->AllocateScalars(VTKDistPixelType, 1);
    m_imLabPre->SetExtent(0, -1, 0, -1, 0, -1);
    m_imLabPre->AllocateScalars(seedVol->GetScalarType(), 1);
    m_imDistPre->SetExtent(0, -1, 0, -1, 0, -1);
    m_imDistPre->AllocateScalars(VTKDistPixelType, 1);
    DistPixelType* imDist = static_cast<DistPixelType*>(m_imDist->GetScalarPointer());

    // Compute index offset
    m_indOff.clear();
    for (int ix = -1; ix <= 1; ix++)
    {
      for (int iy = -1; iy <= 1; iy++)
      {
        for (int iz = -1; iz <= 1; iz++)
        {
          if (!(ix == 0 && iy == 0 && iz == 0))
          {
            m_indOff.push_back(ix + iy*m_DIMX + iz*m_DIMXY);
          }
        }
      }
    }

    // Determine neighborhood size at each vertex
    m_NBSIZE = std::vector<unsigned char>(m_DIMXYZ, 0);
    for (i = 1; i < m_DIMX - 1; i++)
    {
      for (j = 1; j < m_DIMY - 1; j++)
      {
        for (k = 1; k < m_DIMZ - 1; k++)
        {
          index = i + j*m_DIMX + k*m_DIMXY;
          m_NBSIZE[index] = NNGBH;
        }
      }
    }

    for (index = 0; index < m_DIMXYZ; index++)
    {
      imLab[index] = imSeed[index];
      if (imLab[index] == 0)
      {
        m_hpNodes[index] = (float)DIST_INF;
        imDist[index] = DIST_INF;
      }
      else
      {
        m_hpNodes[index] = (float)DIST_EPSILON;
        imDist[index] = DIST_EPSILON;
      }

      m_heap->Insert(&m_hpNodes[index]);
      m_hpNodes[index].SetIndexValue(index);
    }
  }
  else
  {
    // Already initialized
    LabPixelType* imLabPre = static_cast<LabPixelType*>(m_imLabPre->GetScalarPointer());
    DistPixelType* imDist = static_cast<DistPixelType*>(m_imDist->GetScalarPointer());

    for (index = 0; index < m_DIMXYZ; index++)
    {
      if (imSeed[index] != 0 && imSeed[index] != imLabPre[index])
      {
        // if(imSeed[index] != 0 && (imDistPre[index] != 0 ||  (imDistPre[index] == 0 && imSeed[index] != imLabPre[index]))) {
        m_hpNodes[index] = (float)DIST_EPSILON;
        imDist[index] = DIST_EPSILON;
        imLab[index] = imSeed[index];
      }
      else
      {
        m_hpNodes[index] = (float)DIST_INF;
        imDist[index] = DIST_INF;
        imLab[index] = 0;
      }
      m_heap->Insert(&m_hpNodes[index]);
      m_hpNodes[index].SetIndexValue(index);
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
template<typename SrcPixelType, typename LabPixelType>
void vtkFastGrowCutSeg::vtkInternal::DijkstraBasedClassificationAHP(vtkImageData *sourceVol, vtkImageData *seedVol, vtkImageData *outData)
{
  HeapNode *hnMin, hnTmp;
  float t, tOri, tSrc;
  long i, index, indexNgbh;
  LabPixelType labSrc;
  SrcPixelType pixCenter;

  LabPixelType* imLab = static_cast<LabPixelType*>(outData->GetScalarPointer());
  SrcPixelType* imSrc = static_cast<SrcPixelType*>(sourceVol->GetScalarPointer());

  // Insert 0 then extract it, which will balance heap
  m_heap->Insert(&hnTmp); m_heap->ExtractMin();

  long k = 0; // it could be used for early termination of segmentation

  if (m_bSegInitialized)
  {
    LabPixelType* imLabPre = static_cast<LabPixelType*>(m_imLabPre->GetScalarPointer());

    // Adaptive Dijkstra
    while (!m_heap->IsEmpty())
    {
      DistPixelType* imDistPre = static_cast<DistPixelType*>(m_imDistPre->GetScalarPointer());
      DistPixelType* imDist = static_cast<DistPixelType*>(m_imDist->GetScalarPointer());

      hnMin = (HeapNode *)m_heap->ExtractMin();
      index = hnMin->GetIndexValue();
      tSrc = hnMin->GetKeyValue();

      // stop propagation when the new distance is larger than the previous one
      if (tSrc == DIST_INF) break;
      if (tSrc > imDistPre[index])
      {
        imDist[index] = imDistPre[index];
        imLab[index] = imLabPre[index];
        continue;
      }

      labSrc = imLab[index];
      imDist[index] = tSrc;

      // Update neighbors
      pixCenter = imSrc[index];
      for (i = 0; i < m_NBSIZE[index]; i++)
      {
        indexNgbh = index + m_indOff[i];
        tOri = imDist[indexNgbh];
        t = (pixCenter > imSrc[indexNgbh] ? pixCenter - imSrc[indexNgbh] : imSrc[indexNgbh] - pixCenter) + tSrc;
        if (tOri > t)
        {
          imDist[indexNgbh] = t;
          imLab[indexNgbh] = labSrc;

          hnTmp = m_hpNodes[indexNgbh];
          hnTmp.SetKeyValue(t);
          m_heap->DecreaseKey(&m_hpNodes[indexNgbh], hnTmp);
        }

      }

      k++;
    }
  }
  else
  {
    DistPixelType* imDist = static_cast<DistPixelType*>(m_imDist->GetScalarPointer());
    LabPixelType* imLab = static_cast<LabPixelType*>(outData->GetScalarPointer());

    // Normal Dijkstra (to be used in initializing the segmenter for the current image)
    while (!m_heap->IsEmpty())
    {
      hnMin = (HeapNode *)m_heap->ExtractMin();
      index = hnMin->GetIndexValue();
      tSrc = hnMin->GetKeyValue();
      labSrc = imLab[index];
      imDist[index] = tSrc;

      // Update neighbors
      pixCenter = imSrc[index];
      for (i = 0; i < m_NBSIZE[index]; i++)
      {
        indexNgbh = index + m_indOff[i];
        tOri = imDist[indexNgbh];
        t = (pixCenter > imSrc[indexNgbh] ? pixCenter - imSrc[indexNgbh] : imSrc[indexNgbh] - pixCenter) + tSrc;
        if (tOri > t)
        {
          imDist[indexNgbh] = t;
          imLab[indexNgbh] = labSrc;

          hnTmp = m_hpNodes[indexNgbh];
          hnTmp.SetKeyValue(t);
          m_heap->DecreaseKey(&m_hpNodes[indexNgbh], hnTmp);
        }
      }
      k++;
    }
  }

  // Update previous labels and distance information
  m_imLabPre->DeepCopy(outData);
  m_imDistPre->DeepCopy(m_imDist);

  // Release memory
  if (m_heap != NULL)
  {
    delete m_heap;
    m_heap = NULL;
  }
  if (m_hpNodes != NULL)
  {
    delete[]m_hpNodes;
    m_hpNodes = NULL;
  }
}

//-----------------------------------------------------------------------------
template< class SourceVolType, class SeedVolType>
void vtkFastGrowCutSeg::vtkInternal::ExecuteGrowCut2(vtkImageData *sourceVol, vtkImageData *seedVol, vtkImageData *outData)
{
  int* dims = sourceVol->GetDimensions();
  // TODO: Check image size. If it's different then maybe reallocate.
  this->SetImageSize(dims);
  m_bSegInitialized = false;

  // Do Segmentation
  this->DoSegmentation<SourceVolType, SeedVolType>(sourceVol, seedVol, outData);
}

//----------------------------------------------------------------------------
template <class SourceVolType>
void vtkFastGrowCutSeg::vtkInternal::ExecuteGrowCut(vtkImageData *sourceVol, vtkImageData *seedVol, vtkImageData *outData)
{
  switch (seedVol->GetScalarType())
  {
    vtkTemplateMacro((ExecuteGrowCut2<SourceVolType, VTK_TT>(sourceVol, seedVol, outData)));
  default:
    vtkGenericWarningMacro("vtkOrientedImageDataResample::MergeImage: Unknown ScalarType");
  }
}

//-----------------------------------------------------------------------------
vtkFastGrowCutSeg::vtkFastGrowCutSeg()
{
  this->Internal = new vtkInternal();
  SourceVol = NULL;
  SeedVol = NULL;
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
vtkFastGrowCutSeg::~vtkFastGrowCutSeg()
{
  //these functions decrement reference count on the vtkImageData's (incremented by the SetMacros)
  if (this->SourceVol)
  {
    this->SetSourceVol(NULL);
  }

  if (this->SeedVol)
  {
    this->SetSeedVol(NULL);
  }
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkFastGrowCutSeg::ExecuteDataWithInformation(
  vtkDataObject *outData, vtkInformation* outInfo)
{
  vtkImageData *sourceVol = vtkImageData::SafeDownCast(GetInput(0));
  vtkImageData *seedVol = vtkImageData::SafeDownCast(GetInput(1));
  vtkImageData *outVol = vtkImageData::SafeDownCast(outData);

  int outExt[6];
  double spacing[3], origin[3];

  sourceVol->GetOrigin(origin);
  sourceVol->GetSpacing(spacing);
  sourceVol->GetExtent(outExt);

  //TODO: check if source and seed volumes have the same size, origin, spacing

  outVol->SetOrigin(origin);
  outVol->SetSpacing(spacing);
  outVol->SetExtent(outExt);
  outVol->AllocateScalars(outInfo);

  vtkFastGrowCutSeg::vtkInternal *self = this->Internal;
  switch (sourceVol->GetScalarType())
  {
    vtkTemplateMacro(this->Internal->ExecuteGrowCut<VTK_TT>(sourceVol, seedVol, outVol));
    break;
  }
}

//-----------------------------------------------------------------------------
int vtkFastGrowCutSeg::RequestInformation(
  vtkInformation * request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(1);

  if (inInfo != NULL)
  {
    this->Superclass::RequestInformation(request, inputVector, outputVector);
  }
  return 1;
}

void vtkFastGrowCutSeg::Initialization()
{
}


void vtkFastGrowCutSeg::RunFGC()
{

}

void vtkFastGrowCutSeg::PrintSelf(ostream &os, vtkIndent indent)
{
  std::cout << "This function has been found" << std::endl;
}
