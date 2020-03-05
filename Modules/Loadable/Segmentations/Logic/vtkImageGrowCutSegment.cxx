#include "vtkImageGrowCutSegment.h"

#include <iostream>
#include <vector>

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTimerLog.h>

#include "FibHeap.h"

vtkStandardNewMacro(vtkImageGrowCutSegment);

//----------------------------------------------------------------------------

const int NodeKeyValueTypeID = VTK_FLOAT;  // must match NodeKeyValueType, stores "distance" (difference in voxels)

typedef unsigned char MaskPixelType;
const int MaskPixelTypeID = VTK_UNSIGNED_CHAR;

const NodeKeyValueType DIST_INF = std::numeric_limits<NodeKeyValueType>::max();
const NodeKeyValueType DIST_EPSILON = 1e-3;

//----------------------------------------------------------------------------
class vtkImageGrowCutSegment::vtkInternal
{
public:
  vtkInternal();
  virtual ~vtkInternal();

  void Reset();

  template<typename IntensityPixelType, typename LabelPixelType>
  bool InitializationAHP(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume, vtkImageData *maskLabelVolume, double distancePenalty);

  template<typename IntensityPixelType, typename LabelPixelType>
  void DijkstraBasedClassificationAHP(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume, vtkImageData *maskLabelVolume);

  template <class SourceVolType>
  bool ExecuteGrowCut(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume, vtkImageData *maskLabelVolume,
    vtkImageData *resultLabelVolume, double distancePenalty);

  template< class SourceVolType, class SeedVolType>
  bool ExecuteGrowCut2(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume, vtkImageData *maskLabelVolume, double distancePenalty);

  // Stores the shortest distance from known labels to each point
  // If a point is set to DIST_INF then that point will modified, as a shorter distance path will be found.
  // If a point is set to DIST_EPSILON, then the distance is so small that a shorter path will not be found and so
  // the point will not be relabeled.
  vtkSmartPointer<vtkImageData> m_DistanceVolume;

  // Resulting segmentation
  vtkSmartPointer<vtkImageData> m_ResultLabelVolume;

  double m_DistancePenalty;
  NodeIndexType m_DimX;
  NodeIndexType m_DimY;
  NodeIndexType m_DimZ;

  std::vector<NodeIndexType> m_NeighborIndexOffsets;
  std::vector<double> m_NeighborDistancePenalties;
  std::vector<unsigned char> m_NumberOfNeighbors; // size of neighborhood (everywhere the same except at the image boundary)

  FibHeap *m_Heap;
  FibHeapNode *m_HeapNodes; // a node is stored for each voxel
  bool m_bSegInitialized;
};

//-----------------------------------------------------------------------------
vtkImageGrowCutSegment::vtkInternal::vtkInternal()
{
  m_DistancePenalty = 0.0;
  m_Heap = nullptr;
  m_HeapNodes = nullptr;
  m_bSegInitialized = false;
  m_DistanceVolume = vtkSmartPointer<vtkImageData>::New();
  m_ResultLabelVolume = vtkSmartPointer<vtkImageData>::New();
};

//-----------------------------------------------------------------------------
vtkImageGrowCutSegment::vtkInternal::~vtkInternal()
{
  this->Reset();
};

//-----------------------------------------------------------------------------
void vtkImageGrowCutSegment::vtkInternal::Reset()
{
  if (m_Heap != nullptr)
    {
    delete m_Heap;
    m_Heap = nullptr;
    }
  if (m_HeapNodes != nullptr)
    {
    delete[]m_HeapNodes;
    m_HeapNodes = nullptr;
    }
  m_bSegInitialized = false;
  m_DistanceVolume->Initialize();
  m_ResultLabelVolume->Initialize();
}

//-----------------------------------------------------------------------------
template<typename IntensityPixelType, typename LabelPixelType>
bool vtkImageGrowCutSegment::vtkInternal::InitializationAHP(
    vtkImageData *vtkNotUsed(intensityVolume),
    vtkImageData *seedLabelVolume,
    vtkImageData *maskLabelVolume,
    double distancePenalty)
{
  // Release memory before reallocating
  if (m_Heap != nullptr)
    {
    delete m_Heap;
    m_Heap = nullptr;
    }
  if (m_HeapNodes != nullptr)
    {
    delete[] m_HeapNodes;
    m_HeapNodes = nullptr;
    }

  NodeIndexType dimXYZ = m_DimX * m_DimY * m_DimZ;
  if ((m_HeapNodes = new FibHeapNode[dimXYZ+1]) == nullptr)  // size is +1 for storing the zeroValueElement
    {
    vtkGenericWarningMacro("Memory allocation failed. Dimensions: " << m_DimX << "x" << m_DimY << "x" << m_DimZ);
    return false;
    }

  m_Heap = new FibHeap;
  m_Heap->SetHeapNodes(m_HeapNodes);
  LabelPixelType* seedLabelVolumePtr = nullptr;
  if (seedLabelVolume)
    {
    seedLabelVolumePtr = static_cast<LabelPixelType*>(seedLabelVolume->GetScalarPointer());
    }
  MaskPixelType* maskLabelVolumePtr = nullptr;
  if (maskLabelVolume != nullptr)
    {
    maskLabelVolumePtr = static_cast<MaskPixelType*>(maskLabelVolume->GetScalarPointer());
    }

  if (!m_bSegInitialized)
    {
    m_ResultLabelVolume->SetOrigin(seedLabelVolume->GetOrigin());
    m_ResultLabelVolume->SetSpacing(seedLabelVolume->GetSpacing());
    m_ResultLabelVolume->SetExtent(seedLabelVolume->GetExtent());
    m_ResultLabelVolume->AllocateScalars(seedLabelVolume->GetScalarType(), 1);
    m_DistanceVolume->SetOrigin(seedLabelVolume->GetOrigin());
    m_DistanceVolume->SetSpacing(seedLabelVolume->GetSpacing());
    m_DistanceVolume->SetExtent(seedLabelVolume->GetExtent());
    m_DistanceVolume->AllocateScalars(NodeKeyValueTypeID, 1);
    LabelPixelType* resultLabelVolumePtr = static_cast<LabelPixelType*>(m_ResultLabelVolume->GetScalarPointer());
    NodeKeyValueType* distanceVolumePtr = static_cast<NodeKeyValueType*>(m_DistanceVolume->GetScalarPointer());

    // Compute index offset
    m_DistancePenalty = distancePenalty;
    m_NeighborIndexOffsets.clear();
    m_NeighborDistancePenalties.clear();
    // Neighbors are traversed in the order of m_NeighborIndexOffsets,
    // therefore one would expect that the offsets should
    // be as continuous as possible (e.g., x coordinate
    // should change most quickly), but that resulted in
    // about 5-6% longer computation time. Therefore,
    // we put indices in order x1y1z1, x1y1z2, x1y1z3, etc.
    double* spacing = seedLabelVolume->GetSpacing();
    for (long ix = -1; ix <= 1; ix++)
    {
      for (long iy = -1; iy <= 1; iy++)
      {
        for (long iz = -1; iz <= 1; iz++)
        {
          if (ix == 0 && iy == 0 && iz == 0)
            {
            continue;
            }
          m_NeighborIndexOffsets.push_back(ix + long(m_DimX)*(iy + long(m_DimY)*iz));
          m_NeighborDistancePenalties.push_back(this->m_DistancePenalty * sqrt((spacing[0] * ix) * (spacing[0] * ix)
            + (spacing[1] * iy) * (spacing[1] * iy) + (spacing[2] * iz) * (spacing[2] * iz)));
          }
        }
      }

    // Determine neighborhood size for computation at each voxel.
    // The neighborhood size is everywhere the same (size of m_NeighborIndexOffsets)
    // except at the edges of the volume, where the neighborhood size is 0.
    m_NumberOfNeighbors.resize(dimXYZ);
    const unsigned char numberOfNeighbors = static_cast<unsigned char>(m_NeighborIndexOffsets.size());
    unsigned char* nbSizePtr = &(m_NumberOfNeighbors[0]);
    for (NodeIndexType z = 0; z < m_DimZ; z++)
      {
      bool zEdge = (z == 0 || z == m_DimZ - 1);
      for (NodeIndexType y = 0; y < m_DimY; y++)
        {
        bool yEdge = (y == 0 || y == m_DimY - 1);
        *(nbSizePtr++) = 0; // x == 0 (there is always padding, so we don't need to check if m_DimX>0)
        unsigned char nbSize = (zEdge || yEdge) ? 0 : numberOfNeighbors;
        for (NodeIndexType x = m_DimX-2; x > 0; x--)
          {
          *(nbSizePtr++) = nbSize;
          }
        *(nbSizePtr++) = 0; // x == m_DimX-1 (there is always padding, so we don'neighborNewDistance need to check if m_DimX>1)
        }
      }

    if (!maskLabelVolumePtr)
      {
      // no mask
      for (NodeIndexType index = 0; index < dimXYZ; index++)
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
        m_HeapNodes[index].SetIndexValue(index);
        m_Heap->Insert(&m_HeapNodes[index]);
        }
      }
    else
      {
      // with mask
      for (NodeIndexType index = 0; index < dimXYZ; index++)
        {
        if (maskLabelVolumePtr[index] != 0)
          {
          // masked region
          resultLabelVolumePtr[index] = 0;
          // small distance will prevent overwriting of masked voxels
          m_HeapNodes[index] = DIST_EPSILON;
          distanceVolumePtr[index] = DIST_EPSILON;
          // we don't add masked voxels to the heap
          // to exclude them from region growing
          }
        else
          {
          // non-masked region
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
          m_HeapNodes[index].SetIndexValue(index);
          m_Heap->Insert(&m_HeapNodes[index]);
          }
        }
      }
    }
  else
    {
    // Already initialized
    LabelPixelType* resultLabelVolumePtr = static_cast<LabelPixelType*>(m_ResultLabelVolume->GetScalarPointer());
    NodeKeyValueType* distanceVolumePtr = static_cast<NodeKeyValueType*>(m_DistanceVolume->GetScalarPointer());
    for (NodeIndexType index = 0; index < dimXYZ; index++)
      {
      if (seedLabelVolumePtr[index] != 0)
        {
        // Only grow from new/changed seeds
        if (resultLabelVolumePtr[index] != seedLabelVolumePtr[index] // changed seed
          || distanceVolumePtr[index] > DIST_EPSILON // new seed
          )
          {
          m_HeapNodes[index] = DIST_EPSILON;
          distanceVolumePtr[index] = DIST_EPSILON;
          resultLabelVolumePtr[index] = seedLabelVolumePtr[index];
          m_HeapNodes[index].SetIndexValue(index);
          m_Heap->Insert(&m_HeapNodes[index]);
          }
        // Old seeds will be completely ignored in updates, as their labels have been already propagated
        // and their value cannot changed (because their value is prescribed).
        }
      else
        {
        m_HeapNodes[index] = DIST_INF;
        m_HeapNodes[index].SetIndexValue(index);
        m_Heap->Insert(&m_HeapNodes[index]);
        }
      }
    }

  // Insert 0 then extract it, which will balance heap
  NodeIndexType zeroValueElementIndex = dimXYZ;
  m_HeapNodes[zeroValueElementIndex] = 0;
  m_HeapNodes[zeroValueElementIndex].SetIndexValue(zeroValueElementIndex);
  m_Heap->Insert(&m_HeapNodes[zeroValueElementIndex]);
  m_Heap->ExtractMin();

  return true;
}

//-----------------------------------------------------------------------------
template<typename IntensityPixelType, typename LabelPixelType>
void vtkImageGrowCutSegment::vtkInternal::DijkstraBasedClassificationAHP(
    vtkImageData *intensityVolume,
    vtkImageData *vtkNotUsed(seedLabelVolume),
    vtkImageData *vtkNotUsed(maskLabelVolume))
{
  if (m_Heap == nullptr || m_HeapNodes == nullptr)
    {
    return;
    }

  LabelPixelType* resultLabelVolumePtr = static_cast<LabelPixelType*>(m_ResultLabelVolume->GetScalarPointer());
  IntensityPixelType* imSrc = static_cast<IntensityPixelType*>(intensityVolume->GetScalarPointer());

  if (!m_bSegInitialized)
    {
    // Full computation
    NodeKeyValueType* distanceVolumePtr = static_cast<NodeKeyValueType*>(m_DistanceVolume->GetScalarPointer());
    LabelPixelType* resultLabelVolumePtr = static_cast<LabelPixelType*>(m_ResultLabelVolume->GetScalarPointer());

    // Normal Dijkstra (to be used in initializing the segmenter for the current image)
    while (!m_Heap->IsEmpty())
      {
      FibHeapNode* hnMin = m_Heap->ExtractMin();
      NodeIndexType index = hnMin->GetIndexValue();
      NodeKeyValueType currentDistance = hnMin->GetKeyValue();
      LabelPixelType currentLabel = resultLabelVolumePtr[index];

      // Update neighbors
      NodeKeyValueType pixCenter = imSrc[index];
      unsigned char nbSize = m_NumberOfNeighbors[index];
      for (unsigned char i = 0; i < nbSize; i++)
        {
        NodeIndexType indexNgbh = index + m_NeighborIndexOffsets[i];
        NodeKeyValueType neighborCurrentDistance = distanceVolumePtr[indexNgbh];
        NodeKeyValueType neighborNewDistance = fabs(pixCenter - imSrc[indexNgbh]) + currentDistance + m_NeighborDistancePenalties[i];
        if (neighborCurrentDistance > neighborNewDistance)
          {
          distanceVolumePtr[indexNgbh] = neighborNewDistance;
          resultLabelVolumePtr[indexNgbh] = currentLabel;
          m_Heap->DecreaseKey(&m_HeapNodes[indexNgbh], neighborNewDistance);
          }
        }
      }
    }
  else
    {
    // Quick update

    // Adaptive Dijkstra
    NodeKeyValueType* distanceVolumePtr = static_cast<NodeKeyValueType*>(m_DistanceVolume->GetScalarPointer());
    while (!m_Heap->IsEmpty())
      {
      FibHeapNode* hnMin = m_Heap->ExtractMin();
      NodeKeyValueType currentDistance = hnMin->GetKeyValue();

      // Stop if minimum value is infinite (it means there are no more voxels to propagate labels from)
      if (currentDistance == DIST_INF)
        {
        break;
        }

      NodeIndexType index = hnMin->GetIndexValue();
      LabelPixelType currentLabel = resultLabelVolumePtr[index];

      // Update neighbors
      NodeKeyValueType pixCenter = imSrc[index];
      unsigned char nbSize = m_NumberOfNeighbors[index];
      for (unsigned char i = 0; i < nbSize; i++)
        {
        NodeIndexType indexNgbh = index + m_NeighborIndexOffsets[i];
        NodeKeyValueType neighborCurrentDistance = distanceVolumePtr[indexNgbh];
        NodeKeyValueType neighborNewDistance = fabs(pixCenter - imSrc[indexNgbh]) + currentDistance + m_NeighborDistancePenalties[i];
        if (neighborCurrentDistance > neighborNewDistance)
          {
          distanceVolumePtr[indexNgbh] = neighborNewDistance;
          resultLabelVolumePtr[indexNgbh] = currentLabel;

          m_Heap->DecreaseKey(&m_HeapNodes[indexNgbh], neighborNewDistance);
          }
        }
      }
    }

  m_bSegInitialized = true;

  // Release memory
  delete m_Heap;
  m_Heap = nullptr;
  delete[] m_HeapNodes;
  m_HeapNodes = nullptr;
}

//-----------------------------------------------------------------------------
template< class IntensityPixelType, class LabelPixelType>
bool vtkImageGrowCutSegment::vtkInternal::ExecuteGrowCut2(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume,
  vtkImageData *maskLabelVolume, double distancePenalty)
{
  int* imSize = intensityVolume->GetDimensions();

  vtkIdType numberOfVoxels = imSize[0] * imSize[1] * imSize[2];
  vtkIdType maxNumberOfVoxels = std::numeric_limits<NodeIndexType>::max();
  if (numberOfVoxels >= maxNumberOfVoxels)
    {
    // we use unsigned int as index type to reduce memory usage, which limits number of voxels to 2^32,
    // but this is not a practical limitation, as images containing more than 2^32 voxels would take too
    // much time an memory to grow-cut anyway
    vtkGenericWarningMacro("vtkImageGrowCutSegment: image size is too large (" << numberOfVoxels << " voxels)."
      << " Maximum number of voxels is " << maxNumberOfVoxels - 1 << ".");
    return false;
    }

  m_DimX = vtkMath::ClampValue(imSize[0], 0, VTK_INT_MAX);
  m_DimY = vtkMath::ClampValue(imSize[1], 0, VTK_INT_MAX);
  m_DimZ = vtkMath::ClampValue(imSize[2], 0, VTK_INT_MAX);

  if (m_DimX <= 2 || m_DimY <= 2 || m_DimZ <= 2)
    {
    // image is too small (there should be space for at least one voxel padding around the image)
    vtkGenericWarningMacro("vtkImageGrowCutSegment: image size is too small. Minimum size along each dimension is 3.");
    return false;
    }

  if (!InitializationAHP<IntensityPixelType, LabelPixelType>(intensityVolume, seedLabelVolume, maskLabelVolume, distancePenalty))
    {
    return false;
    }

  DijkstraBasedClassificationAHP<IntensityPixelType, LabelPixelType>(intensityVolume, seedLabelVolume, maskLabelVolume);
  return true;
}

//----------------------------------------------------------------------------
template <class SourceVolType>
bool vtkImageGrowCutSegment::vtkInternal::ExecuteGrowCut(vtkImageData *intensityVolume, vtkImageData *seedLabelVolume,
  vtkImageData *maskLabelVolume, vtkImageData *resultLabelVolume, double distancePenalty)
{
  int* extent = intensityVolume->GetExtent();
  double* spacing = intensityVolume->GetSpacing();
  double* origin = intensityVolume->GetOrigin();
  int* seedExtent = seedLabelVolume->GetExtent();
  double* seedSpacing = seedLabelVolume->GetSpacing();
  double* seedOrigin = seedLabelVolume->GetOrigin();
  const double compareTolerance = (spacing[0]+spacing[1]+spacing[2])/3.0 * 0.01;

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

  // Return with error if intensity volume geometry differs from mask label volume geometry
  if (maskLabelVolume)
    {
    int* maskExtent = maskLabelVolume->GetExtent();
    double* maskSpacing = maskLabelVolume->GetSpacing();
    double* maskOrigin = maskLabelVolume->GetOrigin();
    if (maskExtent[0] != extent[0] || maskExtent[1] != extent[1]
      || maskExtent[2] != extent[2] || maskExtent[3] != extent[3]
      || maskExtent[4] != extent[4] || maskExtent[5] != extent[5]
      || fabs(maskOrigin[0] - origin[0]) > compareTolerance
      || fabs(maskOrigin[1] - origin[1]) > compareTolerance
      || fabs(maskOrigin[2] - origin[2]) > compareTolerance
      || fabs(maskSpacing[0] - spacing[0]) > compareTolerance
      || fabs(maskSpacing[1] - spacing[1]) > compareTolerance
      || fabs(maskSpacing[2] - spacing[2]) > compareTolerance)
      {
      vtkGenericWarningMacro("vtkImageGrowCutSegment: Mask label volume geometry does not match intensity volume geometry");
      return false;
      }
    if (maskLabelVolume->GetScalarType() != MaskPixelTypeID || maskLabelVolume->GetNumberOfScalarComponents() != 1)
      {
      vtkGenericWarningMacro("vtkImageGrowCutSegment: Mask label volume scalar must be single-component unsigned char");
      return false;
      }
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
    || fabs(outSpacing[2] - spacing[2]) > compareTolerance
    || fabs(distancePenalty - m_DistancePenalty) > compareTolerance)
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
    vtkTemplateMacro((success = ExecuteGrowCut2<SourceVolType, VTK_TT>(intensityVolume, seedLabelVolume, maskLabelVolume, distancePenalty)));
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
  this->SetNumberOfInputPorts(3);
  this->SetNumberOfOutputPorts(1);
  this->DistancePenalty = 0.0;
}

//-----------------------------------------------------------------------------
vtkImageGrowCutSegment::~vtkImageGrowCutSegment()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
int vtkImageGrowCutSegment::FillInputPortInformation(int port, vtkInformation * info)
{
  vtkImageAlgorithm::FillInputPortInformation(port, info);
  if (port == 2)
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkImageGrowCutSegment::ExecuteDataWithInformation(
  vtkDataObject *resultLabelVolumeDataObject, vtkInformation* vtkNotUsed(resultLabelVolumeInfo))
{
  vtkImageData *intensityVolume = vtkImageData::SafeDownCast(this->GetInput(0));
  vtkImageData *seedLabelVolume = vtkImageData::SafeDownCast(this->GetInput(1));
  vtkImageData *maskLabelVolume = vtkImageData::SafeDownCast(this->GetInput(2));
  vtkImageData *resultLabelVolume = vtkImageData::SafeDownCast(resultLabelVolumeDataObject);

  vtkNew<vtkTimerLog> logger;
  logger->StartTimer();

  switch (intensityVolume->GetScalarType())
    {
    vtkTemplateMacro(this->Internal->ExecuteGrowCut<VTK_TT>(intensityVolume, seedLabelVolume, maskLabelVolume, resultLabelVolume, this->DistancePenalty));
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
  if (inInfo != nullptr)
    {
    this->Superclass::RequestInformation(request, inputVector, outputVector);
    }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkImageGrowCutSegment::Reset()
{
  this->Internal->Reset();
}

//-----------------------------------------------------------------------------
void vtkImageGrowCutSegment::PrintSelf(ostream &os, vtkIndent indent)
{
  // XXX Implement this function
  this->Superclass::PrintSelf(os, indent);
}
