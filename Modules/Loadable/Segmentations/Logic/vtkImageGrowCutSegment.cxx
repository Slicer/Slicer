#include "vtkImageGrowCutSegment.h"

#include <iostream>
#include <vector>

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkLoggingMacros.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTimerLog.h>

#include "FibHeap.h"

vtkStandardNewMacro(vtkImageGrowCutSegment);

//----------------------------------------------------------------------------
typedef float DistancePixelType;  // type for cost function
const int DistancePixelTypeID = VTK_FLOAT;
const DistancePixelType DIST_INF = std::numeric_limits<DistancePixelType>::max();
const DistancePixelType DIST_EPSILON = 1e-3;

//----------------------------------------------------------------------------
class HeapNode : public FibHeapNode
{
public:
  HeapNode()
  : FibHeapNode()
  , m_Key(0)
  , m_Index(-1)
  {
  }

  virtual void operator =(FibHeapNode& RHS)
  {
    FHN_Assign(RHS);
    m_Key = ((HeapNode&)RHS).m_Key;
  }

  virtual int operator ==(FibHeapNode& RHS)
  {
    if (FHN_Cmp(RHS))
      {
      return 0;
      }
    return m_Key == ((HeapNode&)RHS).m_Key ? 1 : 0;
  }

  virtual int operator <(FibHeapNode& RHS)
  {
    int x = FHN_Cmp(RHS);
    if (x != 0)
      {
      return x < 0 ? 1 : 0;
      }
    return m_Key < ((HeapNode&)RHS).m_Key ? 1 : 0;
  }

  virtual void operator =(DistancePixelType newKeyVal)
  {
    HeapNode tmp;
    tmp.m_Key = m_Key = newKeyVal;
    FHN_Assign(tmp);
  }

  inline DistancePixelType GetKeyValue() { return m_Key; }
  inline void SetKeyValue(DistancePixelType keyValue) { m_Key = keyValue; }
  inline long int GetIndexValue() { return m_Index; }
  inline void SetIndexValue(long int indexValue) { m_Index = indexValue; }

protected:
  DistancePixelType m_Key;
  long m_Index;
};

//----------------------------------------------------------------------------
class vtkImageGrowCutSegment::vtkInternal
{
public:
  vtkInternal();
  virtual ~vtkInternal();

  void Reset();

  template<typename IntensityPixelType, typename LabelPixelType>
  bool InitializationAHP(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume);

  template<typename IntensityPixelType, typename LabelPixelType>
  void DijkstraBasedClassificationAHP(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume);

  template <class SourceVolType>
  bool ExecuteGrowCut(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume, vtkImageData *resultLabelVolume);

  template< class SourceVolType, class SeedVolType>
  bool ExecuteGrowCut2(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume);

  vtkSmartPointer<vtkImageData> m_DistanceVolume;
  vtkSmartPointer<vtkImageData> m_DistanceVolumePre;
  vtkSmartPointer<vtkImageData> m_ResultLabelVolume;
  vtkSmartPointer<vtkImageData> m_ResultLabelVolumePre;

  long m_DimX;
  long m_DimY;
  long m_DimZ;
  std::vector<long> m_NeighborIndexOffsets;
  std::vector<unsigned char> m_NumberOfNeighbors;

  FibHeap *m_Heap;
  HeapNode *m_HeapNodes;
  bool m_bSegInitialized;
};

//-----------------------------------------------------------------------------
vtkImageGrowCutSegment::vtkInternal::vtkInternal()
{
  m_Heap = NULL;
  m_HeapNodes = NULL;
  m_bSegInitialized = false;
  m_DistanceVolume = vtkSmartPointer<vtkImageData>::New();
  m_DistanceVolumePre = vtkSmartPointer<vtkImageData>::New();
  m_ResultLabelVolume = vtkSmartPointer<vtkImageData>::New();
  m_ResultLabelVolumePre = vtkSmartPointer<vtkImageData>::New();
};

//-----------------------------------------------------------------------------
vtkImageGrowCutSegment::vtkInternal::~vtkInternal()
{
  this->Reset();
};

//-----------------------------------------------------------------------------
void vtkImageGrowCutSegment::vtkInternal::Reset()
{
  if (m_Heap != NULL)
    {
    delete m_Heap;
    m_Heap = NULL;
    }
  if (m_HeapNodes != NULL)
    {
    delete[]m_HeapNodes;
    m_HeapNodes = NULL;
    }
  m_bSegInitialized = false;
  m_DistanceVolume->Initialize();
  m_DistanceVolumePre->Initialize();
  m_ResultLabelVolume->Initialize();
  m_ResultLabelVolumePre->Initialize();
}

//-----------------------------------------------------------------------------
template<typename IntensityPixelType, typename LabelPixelType>
bool vtkImageGrowCutSegment::vtkInternal::InitializationAHP(
    vtkImageData *vtkNotUsed(intensityVolume),
    vtkImageData *seedLabelVolume)
{
  m_Heap = new FibHeap;
  long dimXYZ = m_DimX * m_DimY * m_DimZ;
  if ((m_HeapNodes = new HeapNode[dimXYZ + 1]) == NULL)
    {
    vtkGenericWarningMacro("Memory allocation failed. Dimensions: " << m_DimX << "x" << m_DimY << "x" << m_DimZ);
    return false;
    }
  LabelPixelType* seedLabelVolumePtr = static_cast<LabelPixelType*>(seedLabelVolume->GetScalarPointer());

  if (!m_bSegInitialized)
    {
    m_ResultLabelVolume->SetOrigin(seedLabelVolume->GetOrigin());
    m_ResultLabelVolume->SetSpacing(seedLabelVolume->GetSpacing());
    m_ResultLabelVolume->SetExtent(seedLabelVolume->GetExtent());
    m_ResultLabelVolume->AllocateScalars(seedLabelVolume->GetScalarType(), 1);
    m_DistanceVolume->SetOrigin(seedLabelVolume->GetOrigin());
    m_DistanceVolume->SetSpacing(seedLabelVolume->GetSpacing());
    m_DistanceVolume->SetExtent(seedLabelVolume->GetExtent());
    m_DistanceVolume->AllocateScalars(DistancePixelTypeID, 1);
    m_ResultLabelVolumePre->SetExtent(0, -1, 0, -1, 0, -1);
    m_ResultLabelVolumePre->AllocateScalars(seedLabelVolume->GetScalarType(), 1);
    m_DistanceVolumePre->SetExtent(0, -1, 0, -1, 0, -1);
    m_DistanceVolumePre->AllocateScalars(DistancePixelTypeID, 1);
    LabelPixelType* resultLabelVolumePtr = static_cast<LabelPixelType*>(m_ResultLabelVolume->GetScalarPointer());
    DistancePixelType* distanceVolumePtr = static_cast<DistancePixelType*>(m_DistanceVolume->GetScalarPointer());

    // Compute index offset
    m_NeighborIndexOffsets.clear();
    // Neighbors are traversed in the order of m_NeighborIndexOffsets,
    // therefore one would expect that the offsets should
    // be as continuous as possible (e.g., x coordinate
    // should change most quickly), but that resulted in
    // about 5-6% longer computation time. Therefore,
    // we put indices in order x1y1z1, x1y1z2, x1y1z3, etc.
    for (int ix = -1; ix <= 1; ix++)
      {
      for (int iy = -1; iy <= 1; iy++)
        {
        for (int iz = -1; iz <= 1; iz++)
          {
          if (ix == 0 && iy == 0 && iz == 0)
            {
            continue;
            }
          m_NeighborIndexOffsets.push_back(long(ix) + m_DimX*(long(iy) + m_DimY*long(iz)));
          }
        }
      }

    // Determine neighborhood size for computation at each voxel.
    // The neighborhood size is everwhere the same (size of m_NeighborIndexOffsets)
    // except at the edges of the volume, where the neighborhood size is 0.
    m_NumberOfNeighbors.resize(dimXYZ);
    const unsigned char numberOfNeighbors = m_NeighborIndexOffsets.size();
    unsigned char* nbSizePtr = &(m_NumberOfNeighbors[0]);
    for (int z = 0; z < m_DimZ; z++)
      {
      bool zEdge = (z == 0 || z == m_DimZ - 1);
      for (int y = 0; y < m_DimY; y++)
        {
        bool yEdge = (y == 0 || y == m_DimY - 1);
        *(nbSizePtr++) = 0; // x == 0 (there is always padding, so we don'neighborNewDistance need to check if m_DimX>0)
        unsigned char nbSize = (zEdge || yEdge) ? 0 : numberOfNeighbors;
        for (int x = m_DimX-2; x > 0; x--)
          {
          *(nbSizePtr++) = nbSize;
          }
        *(nbSizePtr++) = 0; // x == m_DimX-1 (there is always padding, so we don'neighborNewDistance need to check if m_DimX>1)
        }
      }

    for (long index = 0; index < dimXYZ; index++)
      {
      LabelPixelType seedValue = seedLabelVolumePtr[index];
      resultLabelVolumePtr[index] = seedValue;
      if (seedValue == 0)
        {
        m_HeapNodes[index] = DIST_INF;
        distanceVolumePtr[index] = DIST_INF;
        }
      else
        {
        m_HeapNodes[index] = DIST_EPSILON;
        distanceVolumePtr[index] = DIST_EPSILON;
        }
      m_Heap->Insert(&m_HeapNodes[index]);
      m_HeapNodes[index].SetIndexValue(index);
      }
    }
  else
    {
    // Already initialized
    LabelPixelType* resultLabelVolumePtr = static_cast<LabelPixelType*>(m_ResultLabelVolume->GetScalarPointer());
    DistancePixelType* distanceVolumePtr = static_cast<DistancePixelType*>(m_DistanceVolume->GetScalarPointer());

    for (long index = 0; index < dimXYZ; index++)
      {
      if (seedLabelVolumePtr[index] != 0)
        {
        // Only grow from new/changed seeds
        if (resultLabelVolumePtr[index] != seedLabelVolumePtr[index])
          {
          m_HeapNodes[index] = DIST_EPSILON;
          distanceVolumePtr[index] = DIST_EPSILON;
          resultLabelVolumePtr[index] = seedLabelVolumePtr[index];
          m_Heap->Insert(&m_HeapNodes[index]);
          m_HeapNodes[index].SetIndexValue(index);
          }
        }
      else
        {
        m_HeapNodes[index] = DIST_INF;
        distanceVolumePtr[index] = DIST_INF;
        resultLabelVolumePtr[index] = 0;
        m_Heap->Insert(&m_HeapNodes[index]);
        m_HeapNodes[index].SetIndexValue(index);
        }
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
template<typename IntensityPixelType, typename LabelPixelType>
void vtkImageGrowCutSegment::vtkInternal::DijkstraBasedClassificationAHP(
    vtkImageData *intensityVolume,
    vtkImageData *vtkNotUsed(seedLabelVolume))
{
  LabelPixelType* resultLabelVolumePtr = static_cast<LabelPixelType*>(m_ResultLabelVolume->GetScalarPointer());
  IntensityPixelType* imSrc = static_cast<IntensityPixelType*>(intensityVolume->GetScalarPointer());

  // Insert 0 then extract it, which will balance heap
  HeapNode hnZero;
  m_Heap->Insert(&hnZero);
  m_Heap->ExtractMin();

  if (!m_bSegInitialized)
    {
    // Full computation
    DistancePixelType* distanceVolumePtr = static_cast<DistancePixelType*>(m_DistanceVolume->GetScalarPointer());
    LabelPixelType* resultLabelVolumePtr = static_cast<LabelPixelType*>(m_ResultLabelVolume->GetScalarPointer());

    // Normal Dijkstra (to be used in initializing the segmenter for the current image)
    HeapNode hnTmp;
    while (!m_Heap->IsEmpty())
      {
      HeapNode* hnMin = (HeapNode *)m_Heap->ExtractMin();
      long index = hnMin->GetIndexValue();
      DistancePixelType currentDistance = hnMin->GetKeyValue();
      LabelPixelType currentLabel = resultLabelVolumePtr[index];
      distanceVolumePtr[index] = currentDistance;

      // Update neighbors
      DistancePixelType pixCenter = imSrc[index];
      unsigned char nbSize = m_NumberOfNeighbors[index];
      for (unsigned char i = 0; i < nbSize; i++)
        {
        long indexNgbh = index + m_NeighborIndexOffsets[i];
        DistancePixelType neighborCurrentDistance = distanceVolumePtr[indexNgbh];
        DistancePixelType neighborNewDistance = fabs(pixCenter - imSrc[indexNgbh]) + currentDistance;
        if (neighborCurrentDistance > neighborNewDistance)
          {
          distanceVolumePtr[indexNgbh] = neighborNewDistance;
          resultLabelVolumePtr[indexNgbh] = currentLabel;

          hnTmp = m_HeapNodes[indexNgbh];
          hnTmp.SetKeyValue(neighborNewDistance);
          m_Heap->DecreaseKey(&m_HeapNodes[indexNgbh], hnTmp);
          }
        }
      }
    }
  else
    {
    // Quick update

    LabelPixelType* resultLabelVolumePrePtr = static_cast<LabelPixelType*>(m_ResultLabelVolumePre->GetScalarPointer());

    // Adaptive Dijkstra
    HeapNode hnTmp;
    long dimXYZ = m_DimX * m_DimY * m_DimZ;
    while (!m_Heap->IsEmpty())
      {
      DistancePixelType* distanceVolumePrePtr = static_cast<DistancePixelType*>(m_DistanceVolumePre->GetScalarPointer());
      DistancePixelType* distanceVolumePtr = static_cast<DistancePixelType*>(m_DistanceVolume->GetScalarPointer());

      HeapNode* hnMin = (HeapNode *)m_Heap->ExtractMin();
      DistancePixelType currentDistance = hnMin->GetKeyValue();

      // Stop of minimum value is infinite
      if (currentDistance == DIST_INF)
        {
        for (long index = 0; index < dimXYZ; index++)
          {
          if (resultLabelVolumePtr[index] == 0)
            {
            resultLabelVolumePtr[index] = resultLabelVolumePrePtr[index];
            distanceVolumePtr[index] = distanceVolumePrePtr[index];
            }
          }
        break;
        }

      // Stop propagation when the new distance is larger than the previous one
      long index = hnMin->GetIndexValue();
      if (currentDistance > distanceVolumePrePtr[index])
        {
        distanceVolumePtr[index] = distanceVolumePrePtr[index];
        resultLabelVolumePtr[index] = resultLabelVolumePrePtr[index];
        continue;
        }

      LabelPixelType currentLabel = resultLabelVolumePtr[index];
      distanceVolumePtr[index] = currentDistance;

      // Update neighbors
      DistancePixelType pixCenter = imSrc[index];
      unsigned char nbSize = m_NumberOfNeighbors[index];
      for (unsigned char i = 0; i < nbSize; i++)
        {
        long indexNgbh = index + m_NeighborIndexOffsets[i];
        DistancePixelType neighborCurrentDistance = distanceVolumePtr[indexNgbh];
        DistancePixelType neighborNewDistance = fabs(pixCenter - imSrc[indexNgbh]) + currentDistance;
        if (neighborCurrentDistance > neighborNewDistance)
          {
          distanceVolumePtr[indexNgbh] = neighborNewDistance;
          resultLabelVolumePtr[indexNgbh] = currentLabel;

          hnTmp = m_HeapNodes[indexNgbh];
          hnTmp.SetKeyValue(neighborNewDistance);
          m_Heap->DecreaseKey(&m_HeapNodes[indexNgbh], hnTmp);
          }
        }
      }
    }

  // Update previous labels and distance information
  m_ResultLabelVolumePre->DeepCopy(m_ResultLabelVolume);
  m_DistanceVolumePre->DeepCopy(m_DistanceVolume);
  m_bSegInitialized = true;

  // Release memory
  if (m_Heap != NULL)
    {
    delete m_Heap;
    m_Heap = NULL;
    }
  //m_HeapNodes.clear();
  if (m_HeapNodes != NULL)
    {
    delete[] m_HeapNodes;
    m_HeapNodes = NULL;
    }
}

//-----------------------------------------------------------------------------
template< class IntensityPixelType, class LabelPixelType>
bool vtkImageGrowCutSegment::vtkInternal::ExecuteGrowCut2(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume)
{
  int* imSize = intensityVolume->GetDimensions();
  m_DimX = imSize[0];
  m_DimY = imSize[1];
  m_DimZ = imSize[2];

  if (m_DimX <= 2 || m_DimY <= 2 || m_DimZ <= 2)
    {
    // image is too small (there should be space for at least one voxel padding around the image)
    vtkGenericWarningMacro("vtkImageGrowCutSegment: image size is too small");
    return false;
    }

  if (!InitializationAHP<IntensityPixelType, LabelPixelType>(intensityVolume, seedLabelVolume))
    {
    return false;
    }

  DijkstraBasedClassificationAHP<IntensityPixelType, LabelPixelType>(intensityVolume, seedLabelVolume);
  return true;
}

//----------------------------------------------------------------------------
template <class SourceVolType>
bool vtkImageGrowCutSegment::vtkInternal::ExecuteGrowCut(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume, vtkImageData *resultLabelVolume)
{
  const double compareTolerance = 1e-6;
  int* extent = intensityVolume->GetExtent();
  double* spacing = intensityVolume->GetSpacing();
  double* origin = intensityVolume->GetOrigin();
  int* seedExtent = seedLabelVolume->GetExtent();
  double* seedSpacing = seedLabelVolume->GetSpacing();
  double* seedOrigin = seedLabelVolume->GetOrigin();

  // Return with error if intensity volume geometry differs from seed label volume geometry
  if (seedExtent[0] != extent[0] || seedExtent[1] != extent[1]
    || seedExtent[2] != extent[2] || seedExtent[3] != extent[3]
    || seedExtent[4] != extent[4] || seedExtent[5] != extent[5]
    || fabs(seedOrigin[0] - origin[0]) > compareTolerance
    || fabs(seedOrigin[1] - origin[1]) > compareTolerance
    || fabs(seedOrigin[2] - origin[2]) > compareTolerance
    || fabs(seedSpacing[0] - spacing[0]) > compareTolerance
    || fabs(seedSpacing[1] - spacing[1]) > compareTolerance
    || fabs(seedSpacing[2] - spacing[2]) > compareTolerance)
    {
    vtkGenericWarningMacro("vtkImageGrowCutSegment: Seed label volume geometry does not match intensity volume geometry");
    return false;
    }

  // Restart growcut from scratch if image size is changed (then cached buffers cannot be reused)
  int* outExtent = m_ResultLabelVolume->GetExtent();
  double* outSpacing = m_ResultLabelVolume->GetSpacing();
  double* outOrigin = m_ResultLabelVolume->GetOrigin();
  if (outExtent[0] != extent[0] || outExtent[1] != extent[1]
    || outExtent[2] != extent[2] || outExtent[3] != extent[3]
    || outExtent[4] != extent[4] || outExtent[5] != extent[5]
    || fabs(outOrigin[0] - origin[0]) > compareTolerance
    || fabs(outOrigin[1] - origin[1]) > compareTolerance
    || fabs(outOrigin[2] - origin[2]) > compareTolerance
    || fabs(outSpacing[0] - spacing[0]) > compareTolerance
    || fabs(outSpacing[1] - spacing[1]) > compareTolerance
    || fabs(outSpacing[2] - spacing[2]) > compareTolerance)
    {
    this->Reset();
    }
  else if (m_ResultLabelVolume->GetScalarType() != seedLabelVolume->GetScalarType())
    {
    this->Reset();
    }

  bool success = false;
  switch (seedLabelVolume->GetScalarType())
  {
    vtkTemplateMacro((success = ExecuteGrowCut2<SourceVolType, VTK_TT>(intensityVolume, seedLabelVolume)));
  default:
    vtkGenericWarningMacro("vtkOrientedImageDataResample::MergeImage: Unknown ScalarType");
  }

  if (success)
    {
    resultLabelVolume->ShallowCopy(this->m_ResultLabelVolume);
    }
  else
    {
    resultLabelVolume->Initialize();
    }
  return success;
}

//-----------------------------------------------------------------------------
vtkImageGrowCutSegment::vtkImageGrowCutSegment()
{
  this->Internal = new vtkInternal();
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
vtkImageGrowCutSegment::~vtkImageGrowCutSegment()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkImageGrowCutSegment::ExecuteDataWithInformation(
  vtkDataObject *resultLabelVolumeDataObject, vtkInformation* vtkNotUsed(resultLabelVolumeInfo))
{
  vtkImageData *intensityVolume = vtkImageData::SafeDownCast(this->GetInput(0));
  vtkImageData *seedLabelVolume = vtkImageData::SafeDownCast(this->GetInput(1));
  vtkImageData *resultLabelVolume = vtkImageData::SafeDownCast(resultLabelVolumeDataObject);

  vtkNew<vtkTimerLog> logger;
  logger->StartTimer();

  switch (intensityVolume->GetScalarType())
    {
    vtkTemplateMacro(this->Internal->ExecuteGrowCut<VTK_TT>(intensityVolume, seedLabelVolume, resultLabelVolume));
    break;
    }
  logger->StopTimer();
  vtkDebugMacro(<< "vtkImageGrowCutSegment execution time: " << logger->GetElapsedTime());
}

//-----------------------------------------------------------------------------
int vtkImageGrowCutSegment::RequestInformation(
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

void vtkImageGrowCutSegment::Reset()
{
  this->Internal->Reset();
}

void vtkImageGrowCutSegment::PrintSelf(ostream &os, vtkIndent indent)
{
  std::cout << "This function has been found" << std::endl;
}
