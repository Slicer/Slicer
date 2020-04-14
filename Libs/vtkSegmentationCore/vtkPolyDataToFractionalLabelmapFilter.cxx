/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

  This file is a modified version of vtkPolyDataToImageStencil.cxx

==============================================================================*/

#include "vtkPolyDataToFractionalLabelmapFilter.h"

// SegmentationCore includes
#include <vtkSegmentationConverter.h>

// VTK includes
#include <vtkIdTypeArray.h>
#include <vtkTransform.h>
#include <vtkImageStencilData.h>
#include <vtkPolyData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkNew.h>
#include <vtkPolyDataNormals.h>
#include <vtkTriangleFilter.h>
#include <vtkStripper.h>
#include <vtkImageStencil.h>
#include <vtkImageCast.h>

// std includes
#include <map>

vtkStandardNewMacro(vtkPolyDataToFractionalLabelmapFilter);

//----------------------------------------------------------------------------
vtkPolyDataToFractionalLabelmapFilter::vtkPolyDataToFractionalLabelmapFilter()
{
  this->NumberOfOffsets = 6;

  this->LinesCache = std::map<double, vtkSmartPointer<vtkCellArray> >();
  this->SliceCache = std::map<double, vtkSmartPointer<vtkPolyData> >();
  this->PointIdsCache = std::map<double, vtkIdType*>();
  this->NptsCache = std::map<double, vtkIdType>();
  this->PointNeighborCountsCache = std::map<double,  vtkSmartPointer<vtkIdTypeArray> >();

  this->CellLocator = vtkCellLocator::New();

  this->OutputImageTransformData = vtkOrientedImageData::New();

  vtkOrientedImageData* output = vtkOrientedImageData::New();

  this->GetExecutive()->SetOutputData(0, output);

  output->ReleaseData();
  output->Delete();
}

//----------------------------------------------------------------------------
vtkPolyDataToFractionalLabelmapFilter::~vtkPolyDataToFractionalLabelmapFilter()
{
  this->OutputImageTransformData->Delete();
  this->CellLocator->Delete();
}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::SetOutput(vtkOrientedImageData* output)
{
    this->GetExecutive()->SetOutputData(0, output);
}

//----------------------------------------------------------------------------
vtkOrientedImageData* vtkPolyDataToFractionalLabelmapFilter::GetOutput()
{
  if (this->GetNumberOfOutputPorts() < 1)
    {
    return nullptr;
    }

  return vtkOrientedImageData::SafeDownCast(
    this->GetExecutive()->GetOutputData(0));
}

//----------------------------------------------------------------------------
int vtkPolyDataToFractionalLabelmapFilter::FillOutputPortInformation(
  int, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkOrientedImageData");
  return 1;
}

//----------------------------------------------------------------------------
// A helper class to quickly locate an edge, given the endpoint ids.
// It uses an stl map rather than a table partitioning scheme, since
// we have no idea how many entries there will be when we start.  So
// the performance is approximately log(n).
//
// These classes and methods are not inherited from vtkPolyDataToImageStencil,
// so they needed to be duplicated here to allow PolyDataCutter to function.
namespace {

// A Node in a linked list that contains information about one edge
class EdgeLocatorNode
{
public:
  EdgeLocatorNode()  = default;

  // Free the list that this node is the head of
  void FreeList() {
    EdgeLocatorNode *ptr = this->next;
    while (ptr)
      {
      EdgeLocatorNode *tmp = ptr;
      ptr = ptr->next;
      tmp->next = nullptr;
      delete tmp;
      }
  }

  vtkIdType ptId{-1};
  vtkIdType edgeId{-1};
  EdgeLocatorNode *next{nullptr};
};

// The EdgeLocator class itself, for keeping track of edges
class EdgeLocator
{
private:
  typedef std::map<vtkIdType, EdgeLocatorNode> MapType;
  MapType EdgeMap;

public:
  EdgeLocator() : EdgeMap() {}
  ~EdgeLocator() { this->Initialize(); }

  // Description:
  // Initialize the locator.
  void Initialize();

  // Description:
  // If the edge (i0, i1) is not in the list, then it will be added and
  // given the supplied edgeId, and the return value will be false.  If
  // the edge (i0, i1) is in the list, then edgeId will be set to the
  // stored value and the return value will be true.
  bool InsertUniqueEdge(vtkIdType i0, vtkIdType i1, vtkIdType &edgeId);

  // Description:
  // A helper function for interpolating a new point along an edge.  It
  // stores the index of the interpolated point in "i", and returns true
  // if a new point was added to the locator.  The values i0, i1, v0, v1
  // are the edge endpoints and scalar values, respectively.
  bool InterpolateEdge(
    vtkPoints *inPoints, vtkPoints *outPoints,
    vtkIdType i0, vtkIdType i1, double v0, double v1,
    vtkIdType &i);
};

void EdgeLocator::Initialize()
{
  for (MapType::iterator i = this->EdgeMap.begin();
       i != this->EdgeMap.end();
       ++i)
    {
    i->second.FreeList();
    }
  this->EdgeMap.clear();
}

bool EdgeLocator::InsertUniqueEdge(
  vtkIdType i0, vtkIdType i1, vtkIdType &edgeId)
{
  // Ensure consistent ordering of edge
  if (i1 < i0)
    {
    vtkIdType tmp = i0;
    i0 = i1;
    i1 = tmp;
    }

  EdgeLocatorNode *node = &this->EdgeMap[i0];

  if (node->ptId < 0)
    {
    // Didn't find key, so add a new edge entry
    node->ptId = i1;
    node->edgeId = edgeId;
    return true;
    }

  // Search through the list for i1
  if (node->ptId == i1)
    {
    edgeId = node->edgeId;
    return false;
    }

  int i = 1;
  while (node->next != nullptr)
    {
    i++;
    node = node->next;

    if (node->ptId == i1)
      {
      edgeId = node->edgeId;
      return false;
      }
    }

  // No entry for i1, so make one and return
  node->next = new EdgeLocatorNode;
  node = node->next;
  node->ptId = i1;
  node->edgeId = edgeId;
  return true;
}

bool EdgeLocator::InterpolateEdge(
  vtkPoints *points, vtkPoints *outPoints,
  vtkIdType i0, vtkIdType i1, double v0, double v1,
  vtkIdType &i)
{
  // This swap guarantees that exactly the same point is computed
  // for both line directions, as long as the endpoints are the same.
  if (v1 > 0)
    {
    vtkIdType tmpi = i0;
    i0 = i1;
    i1 = tmpi;

    double tmp = v0;
    v0 = v1;
    v1 = tmp;
    }

  // Check to see if this point has already been computed
  i = outPoints->GetNumberOfPoints();
  if (!this->InsertUniqueEdge(i0, i1, i))
    {
    return false;
    }

  // Get the edge and interpolate the new point
  double p0[3], p1[3], p[3];
  points->GetPoint(i0, p0);
  points->GetPoint(i1, p1);

  double f = v0/(v0 - v1);
  double s = 1.0 - f;
  double t = 1.0 - s;

  p[0] = s*p0[0] + t*p1[0];
  p[1] = s*p0[1] + t*p1[1];
  p[2] = s*p0[2] + t*p1[2];

  // Add the point, store the new index in the locator
  outPoints->InsertNextPoint(p);

  return true;
}

} // end anonymous namespace

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::SetOutputImageToWorldMatrix(vtkMatrix4x4* imageToWorldMatrix)
{
  this->OutputImageTransformData->SetImageToWorldMatrix(imageToWorldMatrix);
}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::GetOutputImageToWorldMatrix(vtkMatrix4x4* imageToWorldMatrix)
{
  this->OutputImageTransformData->GetImageToWorldMatrix(imageToWorldMatrix);
}

//----------------------------------------------------------------------------
double* vtkPolyDataToFractionalLabelmapFilter::GetOutputOrigin()
{
  return this->OutputImageTransformData->GetOrigin();
}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::GetOutputOrigin(double origin[3])
{
  this->OutputImageTransformData->GetOrigin(origin);
}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::SetOutputOrigin(double origin[3])
{
  this->OutputImageTransformData->SetOrigin(origin);
}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::SetOutputOrigin(double x, double y, double z)
{
  this->OutputImageTransformData->SetOrigin(x, y, z);
}

//----------------------------------------------------------------------------
double* vtkPolyDataToFractionalLabelmapFilter::GetOutputSpacing()
{
  return this->OutputImageTransformData->GetSpacing();
}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::GetOutputSpacing(double spacing[3])
{
  this->OutputImageTransformData->GetSpacing(spacing);
}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::SetOutputSpacing(double spacing[3])
{
  this->OutputImageTransformData->SetSpacing(spacing);
}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::SetOutputSpacing(double x, double y, double z)
{
  this->OutputImageTransformData->SetSpacing(x, y, z);
}


//----------------------------------------------------------------------------
vtkOrientedImageData* vtkPolyDataToFractionalLabelmapFilter::AllocateOutputData(
  vtkDataObject *out, int* uExt)
{
  vtkOrientedImageData *outputData = vtkOrientedImageData::SafeDownCast(out);
  if (!outputData)
    {
    vtkWarningMacro("Call to AllocateOutputData with non vtkOrientedImageData"
                    " output");
    return nullptr;
    }

  // Allocate output image data
  outputData->SetExtent(uExt);
  outputData->AllocateScalars(VTK_FRACTIONAL_DATA_TYPE, 1);

  // Set-up fractional labelmap
  void* fractionalLabelMapVoxelsPointer = outputData->GetScalarPointerForExtent(outputData->GetExtent());
  if (!fractionalLabelMapVoxelsPointer)
    {
    vtkErrorMacro("Convert: Failed to allocate memory for output labelmap image!");
    return nullptr;
    }
  else
    {
    int extent[6];
    outputData->GetExtent(extent);
    memset(fractionalLabelMapVoxelsPointer, FRACTIONAL_MIN, ((extent[1]-extent[0]+1)*(extent[3]-extent[2]+1)*(extent[5]-extent[4]+1) * outputData->GetScalarSize() * outputData->GetNumberOfScalarComponents()));
    }

  return outputData;
}

//----------------------------------------------------------------------------
int vtkPolyDataToFractionalLabelmapFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkOrientedImageData *outputData = vtkOrientedImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

    this->AllocateOutputData(
    outputData,
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT()));

  vtkInformation *inputInfo = inputVector[0]->GetInformationObject(0);
  vtkPolyData *inputData = vtkPolyData::SafeDownCast(
    inputInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkMatrix4x4> outputLabelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->OutputImageTransformData->GetImageToWorldMatrix(outputLabelmapImageToWorldMatrix);
  outputData->SetImageToWorldMatrix(outputLabelmapImageToWorldMatrix);
  outputData->SetExtent(this->OutputWholeExtent);

  vtkSmartPointer<vtkTransform> inverseOutputLabelmapGeometryTransform = vtkSmartPointer<vtkTransform>::New();
  inverseOutputLabelmapGeometryTransform->SetMatrix(outputLabelmapImageToWorldMatrix);
  inverseOutputLabelmapGeometryTransform->Inverse();

  // Transform the polydata from RAS to IJK space
  vtkSmartPointer<vtkTransformPolyDataFilter> transformPolyDataFilter =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformPolyDataFilter->SetInputData(inputData);
  transformPolyDataFilter->SetTransform(inverseOutputLabelmapGeometryTransform);

  // Compute polydata normals
  vtkNew<vtkPolyDataNormals> normalFilter;
  normalFilter->SetInputConnection(transformPolyDataFilter->GetOutputPort());
  normalFilter->ConsistencyOn();

  // Make sure that we have a clean triangle polydata
  vtkNew<vtkTriangleFilter> triangle;
  triangle->SetInputConnection(normalFilter->GetOutputPort());

  // Convert to triangle strip
  vtkSmartPointer<vtkStripper> stripper = vtkSmartPointer<vtkStripper>::New();
  stripper->SetInputConnection(triangle->GetOutputPort());
  stripper->Update();

  // PolyData of the closed surface in IJK space
  vtkSmartPointer<vtkPolyData> transformedClosedSurface = stripper->GetOutput();

  this->CellLocator->SetDataSet(transformedClosedSurface);
  this->CellLocator->BuildLocator();

  int extent[6];
  outputData->GetExtent(extent);

  vtkSmartPointer<vtkImageData> emptyImageData = vtkSmartPointer<vtkImageData>::New();
  emptyImageData->SetExtent(extent);
  emptyImageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  void* emptyImageDataPointer = emptyImageData->GetScalarPointerForExtent(emptyImageData->GetExtent());
  if (!emptyImageDataPointer)
  {
    vtkErrorMacro("Convert: Failed to allocate memory for output labelmap image!");
    return false;
  }
  else
  {
    memset(emptyImageDataPointer, 0, ((extent[1]-extent[0]+1)*(extent[3]-extent[2]+1)*(extent[5]-extent[4]+1) * emptyImageData->GetScalarSize() * emptyImageData->GetNumberOfScalarComponents()));
  }


  // The magnitude of the offset step size ( n-1 / 2n )
  double offsetStepSize = (double)(this->NumberOfOffsets-1.0)/(2 * this->NumberOfOffsets);

  vtkSmartPointer<vtkImageStencilData> imageStencilData = vtkSmartPointer<vtkImageStencilData>::New();
  imageStencilData->SetExtent(extent);
  imageStencilData->SetSpacing(1.0, 1.0, 1.0);

  vtkNew<vtkImageStencil> imageStencil;
  imageStencil->SetInputData(emptyImageData);
  imageStencil->SetStencilData(imageStencilData);
  imageStencil->ReverseStencilOn();
  imageStencil->SetBackgroundValue(1); // General foreground value is 1 (background value because of reverse stencil)

  vtkNew<vtkImageCast> imageCast;
  imageCast->SetInputConnection(imageStencil->GetOutputPort());
  imageCast->SetOutputScalarTypeToUnsignedChar();

  // Iterate through "NumberOfOffsets" in each of the dimensions and create a binary labelmap at each offset
  for (int k = 0; k < this->NumberOfOffsets; ++k)
  {
    double kOffset = ( (double) k / this->NumberOfOffsets - offsetStepSize );

    for (int j = 0; j < this->NumberOfOffsets; ++j)
    {
      double jOffset = ( (double) j / this->NumberOfOffsets - offsetStepSize );

      for (int i = 0; i < this->NumberOfOffsets; ++i)
      {
        double iOffset = ( (double) i / this->NumberOfOffsets - offsetStepSize );

        // Create stencil for the current binary labelmap offset
        imageStencilData->AllocateExtents();
        imageStencilData->SetOrigin(iOffset, jOffset, kOffset);
        this->FillImageStencilData(imageStencilData, transformedClosedSurface, extent);

        // Save result to output
        imageCast->Update();
        this->AddBinaryLabelMapToFractionalLabelMap(imageCast->GetOutput(), outputData);

        this->UpdateProgress(((i+1)*(j+1)*(k+1))/(this->NumberOfOffsets*this->NumberOfOffsets*this->NumberOfOffsets));

      } // i
    } // j
  } // k

  return 1;
}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::AddBinaryLabelMapToFractionalLabelMap(vtkImageData* binaryLabelMap, vtkImageData* fractionalLabelMap)
{

  if (!binaryLabelMap)
  {
    vtkErrorMacro("AddBinaryLabelMapToFractionalLabelMap: Invalid vtkImageData!");
    return;
  }

  if (!fractionalLabelMap)
  {
    vtkErrorMacro("AddBinaryLabelMapToFractionalLabelMap: Invalid vtkImageData!");
    return;
  }

  int binaryExtent[6] = {0,-1,0,-1,0,-1};
  binaryLabelMap->GetExtent(binaryExtent);

  int fractionalExtent[6] = {0,-1,0,-1,0,-1};
  fractionalLabelMap->GetExtent(fractionalExtent);

  // Get points to the extent in both the binary and fractional labelmaps
  char* binaryLabelMapPointer = (char*)binaryLabelMap->GetScalarPointerForExtent(binaryExtent);
  FRACTIONAL_DATA_TYPE* fractionalLabelMapPointer = (FRACTIONAL_DATA_TYPE*)fractionalLabelMap->GetScalarPointerForExtent(fractionalExtent);

  int dimensions[6] = {0,0,0};
  fractionalLabelMap->GetDimensions(dimensions);

  int numberOfVoxels = dimensions[0]*dimensions[1]*dimensions[2];

  for (int i = 0; i < numberOfVoxels; ++i)
  {
    (*fractionalLabelMapPointer) += (*binaryLabelMapPointer) * FRACTIONAL_STEP_SIZE;
    ++binaryLabelMapPointer;
    ++fractionalLabelMapPointer;
  }

}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::FillImageStencilData(
  vtkImageStencilData *data, vtkPolyData* closedSurface,
  int extent[6])
{
  // Description of algorithm:
  // 1) cut the polydata at each z slice to create polylines
  // 2) find all "loose ends" and connect them to make polygons
  //    (if the input polydata is closed, there will be no loose ends)
  // 3) go through all line segments, and for each integer y value on
  //    a line segment, store the x value at that point in a bucket
  // 4) for each z integer index, find all the stored x values
  //    and use them to create one z slice of the vtkStencilData

  // the spacing and origin of the generated stencil
  double *spacing = data->GetSpacing();
  double *origin = data->GetOrigin();

  // if we have no data then return
  if (!this->GetInput()->GetNumberOfPoints())
    {
    return;
    }

  // Only divide once
  double invspacing[3];
  invspacing[0] = 1.0/spacing[0];
  invspacing[1] = 1.0/spacing[1];
  invspacing[2] = 1.0/spacing[2];

  // get the input data
  vtkPolyData *input = closedSurface;

  // the output produced by cutting the polydata with the Z plane
  vtkSmartPointer<vtkPolyData> slice;

  // This raster stores all line segments by recording all "x"
  // positions on the surface for each y integer position.
  vtkImageStencilRaster raster(&extent[2]);
  raster.SetTolerance(this->Tolerance);

  // The extent for one slice of the image
  int sliceExtent[6];
  sliceExtent[0] = extent[0]; sliceExtent[1] = extent[1];
  sliceExtent[2] = extent[2]; sliceExtent[3] = extent[3];
  sliceExtent[4] = extent[4]; sliceExtent[5] = extent[4];

  // Loop through the slices
  for (int idxZ = extent[4]; idxZ <= extent[5]; idxZ++)
    {

    double z = idxZ*spacing[2] + origin[2];

    raster.PrepareForNewData();

    if ( this->SliceCache.count(z) == 0 )
      {

      slice = vtkSmartPointer<vtkPolyData>::New();

      // Step 1: Cut the data into slices
      if (input->GetNumberOfPolys() > 0 || input->GetNumberOfStrips() > 0)
        {

        this->PolyDataCutter(input, slice, z);
        }
      else
        {
        // if no polys, select polylines instead
        this->PolyDataSelector(input, slice, z, spacing[2]);
        }

      if (!slice->GetNumberOfLines())
        {
        continue;
        }

      this->SliceCache.insert(std::pair<double, vtkPolyData*>(z, slice));

      }

    slice = this->SliceCache[z];

    // convert to structured coords via origin and spacing
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->DeepCopy(slice->GetPoints());
    vtkIdType numberOfPoints = points->GetNumberOfPoints();

    for (vtkIdType j = 0; j < numberOfPoints; j++)
      {
      double tempPoint[3];
      points->GetPoint(j, tempPoint);
      tempPoint[0] = (tempPoint[0] - origin[0])*invspacing[0];
      tempPoint[1] = (tempPoint[1] - origin[1])*invspacing[1];
      tempPoint[2] = (tempPoint[2] - origin[2])*invspacing[2];
      points->SetPoint(j, tempPoint);
      }

    if (this->LinesCache.count(z) == 0)
    {

      // Step 2: Find and connect all the loose ends
      std::vector<vtkIdType> pointNeighbors(numberOfPoints);
      vtkSmartPointer<vtkIdTypeArray> pointNeighborCountsArray = vtkSmartPointer<vtkIdTypeArray>::New();
      pointNeighborCountsArray->Allocate(numberOfPoints, 1);
      vtkIdType* pointNeighborCounts = pointNeighborCountsArray->GetPointer(0);
      memset(pointNeighborCounts, 0, numberOfPoints*sizeof(vtkIdType));

      // get the connectivity count for each point
      vtkSmartPointer<vtkCellArray> lines = slice->GetLines();
      vtkIdType npts = 0;
      vtkIdType *pointIds = nullptr;
      vtkIdType count = lines->GetNumberOfConnectivityEntries();
      for (vtkIdType loc = 0; loc < count; loc += npts + 1)
        {
        lines->GetCell(loc, npts, pointIds);
        if (npts > 0)
          {
          pointNeighborCounts[pointIds[0]] += 1;
          for (vtkIdType j = 1; j < npts-1; j++)
            {
            pointNeighborCounts[pointIds[j]] += 2;
            }
          pointNeighborCounts[pointIds[npts-1]] += 1;
          if (pointIds[0] != pointIds[npts-1])
            {
            // store the neighbors for end points, because these are
            // potentially loose ends that will have to be dealt with later
            pointNeighbors[pointIds[0]] = pointIds[1];
            pointNeighbors[pointIds[npts-1]] = pointIds[npts-2];
            }
          }
        }

      // use connectivity count to identify loose ends and branch points
      std::vector<vtkIdType> looseEndIds;
      std::vector<vtkIdType> branchIds;

      for (vtkIdType j = 0; j < numberOfPoints; j++)
        {
        if (pointNeighborCounts[j] == 1)
          {
          looseEndIds.push_back(j);
          }
        else if (pointNeighborCounts[j] > 2)
          {
          branchIds.push_back(j);
          }
        }

      // remove any spurs
      for (size_t b = 0; b < branchIds.size(); b++)
        {
        for (size_t i = 0; i < looseEndIds.size(); i++)
          {
          if (pointNeighbors[looseEndIds[i]] == branchIds[b])
            {
            // mark this pointId as removed
            pointNeighborCounts[looseEndIds[i]] = 0;
            looseEndIds.erase(looseEndIds.begin() + i);
            i--;
            if (--pointNeighborCounts[branchIds[b]] <= 2)
              {
              break;
              }
            }
          }
        }

      // join any loose ends
      while (looseEndIds.size() >= 2)
        {
        size_t n = looseEndIds.size();

        // search for the two closest loose ends
        double maxval = -VTK_FLOAT_MAX;
        vtkIdType firstIndex = 0;
        vtkIdType secondIndex = 1;
        bool isCoincident = false;
        bool isOnHull = false;

        for (size_t i = 0; i < n && !isCoincident; i++)
          {
          // first loose end
          vtkIdType firstLooseEndId = looseEndIds[i];
          vtkIdType neighborId = pointNeighbors[firstLooseEndId];

          double firstLooseEnd[3];
          slice->GetPoint(firstLooseEndId, firstLooseEnd);
          double neighbor[3];
          slice->GetPoint(neighborId, neighbor);

          for (size_t j = i+1; j < n; j++)
            {
            vtkIdType secondLooseEndId = looseEndIds[j];
            if (secondLooseEndId != neighborId)
              {
              double currentLooseEnd[3];
              slice->GetPoint(secondLooseEndId, currentLooseEnd);

              // When connecting loose ends, use dot product to favor
              // continuing in same direction as the line already
              // connected to the loose end, but also favour short
              // distances by dividing dotprod by square of distance.
              double v1[2], v2[2];
              v1[0] = firstLooseEnd[0] - neighbor[0];
              v1[1] = firstLooseEnd[1] - neighbor[1];
              v2[0] = currentLooseEnd[0] - firstLooseEnd[0];
              v2[1] = currentLooseEnd[1] - firstLooseEnd[1];
              double dotprod = v1[0]*v2[0] + v1[1]*v2[1];
              double distance2 = v2[0]*v2[0] + v2[1]*v2[1];

              // check if points are coincident
              if (distance2 == 0)
                {
                firstIndex = i;
                secondIndex = j;
                isCoincident = true;
                break;
                }

              // prefer adding segments that lie on hull
              double midpoint[2], normal[2];
              midpoint[0] = 0.5*(currentLooseEnd[0] + firstLooseEnd[0]);
              midpoint[1] = 0.5*(currentLooseEnd[1] + firstLooseEnd[1]);
              normal[0] = currentLooseEnd[1] - firstLooseEnd[1];
              normal[1] = -(currentLooseEnd[0] - firstLooseEnd[0]);
              double sidecheck = 0.0;
              bool checkOnHull = true;
              for (size_t k = 0; k < n; k++)
                {
                if (k != i && k != j)
                  {
                  double checkEnd[3];
                  slice->GetPoint(looseEndIds[k], checkEnd);
                  double dotprod2 = ((checkEnd[0] - midpoint[0])*normal[0] +
                                     (checkEnd[1] - midpoint[1])*normal[1]);
                  if (dotprod2*sidecheck < 0)
                    {
                    checkOnHull = false;
                    }
                  sidecheck = dotprod2;
                  }
                }

              // check if new candidate is better than previous one
              if ((checkOnHull && !isOnHull) ||
                  (checkOnHull == isOnHull && dotprod > maxval*distance2))
                {
                firstIndex = i;
                secondIndex = j;
                isOnHull |= checkOnHull;
                maxval = dotprod/distance2;
                }
              }
            }
          }

        // get info about the two loose ends and their neighbors
        vtkIdType firstLooseEndId = looseEndIds[firstIndex];
        vtkIdType neighborId = pointNeighbors[firstLooseEndId];
        double firstLooseEnd[3];
        slice->GetPoint(firstLooseEndId, firstLooseEnd);
        double neighbor[3];
        slice->GetPoint(neighborId, neighbor);

        vtkIdType secondLooseEndId = looseEndIds[secondIndex];
        vtkIdType secondNeighborId = pointNeighbors[secondLooseEndId];
        double secondLooseEnd[3];
        slice->GetPoint(secondLooseEndId, secondLooseEnd);
        double secondNeighbor[3];
        slice->GetPoint(secondNeighborId, secondNeighbor);

        // remove these loose ends from the list
        looseEndIds.erase(looseEndIds.begin() + secondIndex);
        looseEndIds.erase(looseEndIds.begin() + firstIndex);

        if (!isCoincident)
          {
          // create a new line segment by connecting these two points
          lines->InsertNextCell(2);
          lines->InsertCellPoint(firstLooseEndId);
          lines->InsertCellPoint(secondLooseEndId);
          }
        }

        this->LinesCache.insert(std::pair<double, vtkCellArray*>(z, lines));
        this->NptsCache.insert(std::pair<double, vtkIdType>(z, npts));
        this->PointNeighborCountsCache.insert(std::pair<double, vtkSmartPointer<vtkIdTypeArray> >(z, pointNeighborCountsArray));

      }

   if (this->LinesCache.count(z) == 0)
    {
     continue;
    }

    vtkCellArray* lines = this->LinesCache[z];
    vtkIdType count = lines->GetNumberOfConnectivityEntries();
    vtkIdType* pointIds = this->PointIdsCache[z];
    vtkIdType npts = this->NptsCache[z];
    vtkIdTypeArray* pointNeighborCountsArray = this->PointNeighborCountsCache[z];
    vtkIdType* pointNeighborCounts = pointNeighborCountsArray->GetPointer(0);

    // Step 3: Go through all the line segments for this slice,
    // and for each integer y position on the line segment,
    // drop the corresponding x position into the y raster line.
    for (vtkIdType loc = 0; loc < count; loc += npts + 1)
      {
      lines->GetCell(loc, npts, pointIds);
      if (npts > 0)
        {
        vtkIdType pointId0 = pointIds[0];
        double point0[3];
        points->GetPoint(pointId0, point0);
        for (vtkIdType j = 1; j < npts; j++)
          {
          vtkIdType pointId1 = pointIds[j];
          double point1[3];
          points->GetPoint(pointId1, point1);

          // make sure points aren't flagged for removal
          if (pointNeighborCounts[pointId0] > 0 &&
              pointNeighborCounts[pointId1] > 0)
            {
            raster.InsertLine(point0, point1);
            }

          pointId0 = pointId1;
          point0[0] = point1[0];
          point0[1] = point1[1];
          point0[2] = point1[2];
          }
        }
      }

    // Step 4: Use the x values stored in the xy raster to create
    // one z slice of the vtkStencilData
    sliceExtent[4] = idxZ;
    sliceExtent[5] = idxZ;
    raster.FillStencilData(data, sliceExtent);

    }

}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::PolyDataCutter(
  vtkPolyData *input, vtkPolyData *output, double z)
{
  vtkPoints *points = input->GetPoints();
  vtkPoints *newPoints = vtkPoints::New();
  newPoints->SetDataType(points->GetDataType());
  newPoints->Allocate(333);
  vtkCellArray *newLines = vtkCellArray::New();
  newLines->Allocate(1000);

  // An edge locator to avoid point duplication while clipping
  EdgeLocator edgeLocator;

  vtkSmartPointer<vtkIdList> cells = vtkSmartPointer<vtkIdList>::New();
  cells->Initialize();

  double bounds[6] = {0,0,0,0,0,0};
  input->GetBounds(bounds);
  bounds[4] = z;
  bounds[5] = z;

  // Find cells that intersect with the current slice.
  this->CellLocator->FindCellsWithinBounds(bounds, cells);

  // Go through all cells and clip them.
  vtkIdType numCells = cells->GetNumberOfIds();


  vtkIdType loc = 0;
  for (vtkIdType cellId = 0; cellId < numCells; cellId++)
    {

    vtkIdType id = cells->GetId(cellId);

    if (input->GetCellType(id) != VTK_TRIANGLE &&
        input->GetCellType(id) != VTK_TRIANGLE_STRIP)
      {
        continue;
      }

    vtkIdType npts, *ptIds;
    input->GetCellPoints(id, npts, ptIds);
    loc += npts + 1;

    vtkIdType numSubCells = 1;
    if (input->GetCellType(id) == VTK_TRIANGLE_STRIP)
      {
      numSubCells = npts - 2;
      npts = 3;
      }

    for (vtkIdType subId = 0; subId < numSubCells; subId++)
      {
      vtkIdType i1 = ptIds[npts-1];
      double point[3];
      points->GetPoint(i1, point);
      double v1 = point[2] - z;
      bool c1 = (v1 > 0);
      bool odd = ((subId & 1) != 0);

      // To store the ids of the contour line
      vtkIdType linePts[2];
      linePts[0] = 0;
      linePts[1] = 0;

      for (vtkIdType i = 0; i < npts; i++)
        {
        // Save previous point info
        vtkIdType i0 = i1;
        double v0 = v1;
        bool c0 = c1;

        // Generate new point info
        i1 = ptIds[i];
        points->GetPoint(i1, point);
        v1 = point[2] - z;
        c1 = (v1 > 0);

        // If at least one edge end point wasn't clipped
        if ( (c0 | c1) )
          {
          // If only one end was clipped, interpolate new point
          if ( (c0 ^ c1) )
            {
            edgeLocator.InterpolateEdge(
              points, newPoints, i0, i1, v0, v1, linePts[c0 ^ odd]);
            }
          }
        }

      // Insert the contour line if one was created
      if (linePts[0] != linePts[1])
        {
        newLines->InsertNextCell(2, linePts);
        }

      // Increment to get to the next triangle, if cell is a strip
      ptIds++;
      }
    }

  output->SetPoints(newPoints);
  output->SetLines(newLines);
  newPoints->Delete();
  newLines->Delete();
}

//----------------------------------------------------------------------------
void vtkPolyDataToFractionalLabelmapFilter::DeleteCache()
{

  this->SliceCache.clear();
  this->LinesCache.clear();
  this->NptsCache.clear();
  this->PointIdsCache.clear();
  this->PointNeighborCountsCache.clear();

}
