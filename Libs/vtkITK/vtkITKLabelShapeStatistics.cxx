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
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

#include "vtkITKLabelShapeStatistics.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkLongArray.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPointData.h>

// ITK includes
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkShapeLabelObject.h>
#include <itkVTKImageToImageFilter.h>

vtkStandardNewMacro(vtkITKLabelShapeStatistics);

//----------------------------------------------------------------------------
vtkITKLabelShapeStatistics::vtkITKLabelShapeStatistics()
{
  this->Directions = nullptr;

  this->ComputedStatistics.push_back(this->GetShapeStatisticAsString(Centroid));
  this->ComputedStatistics.push_back(this->GetShapeStatisticAsString(Flatness));
}

//----------------------------------------------------------------------------
vtkITKLabelShapeStatistics::~vtkITKLabelShapeStatistics()
{
  this->SetDirections(nullptr);
}

//----------------------------------------------------------------------------
void vtkITKLabelShapeStatistics::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
int vtkITKLabelShapeStatistics::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}

//----------------------------------------------------------------------------
std::string vtkITKLabelShapeStatistics::GetShapeStatisticAsString(ShapeStatistic statistic)
{
  switch (statistic)
    {
    case Centroid:
      return "Centroid";
    case OrientedBoundingBox:
      return "OrientedBoundingBox";
    case FeretDiameter:
      return "FeretDiameter";
    case Perimeter:
      return "Perimeter";
    case Roundness:
      return "Roundness";
    case Flatness:
      return "Flatness";
    case Elongation:
      return "Elongation";
    case PrincipalMoments:
      return "PrincipalMoments";
    case PrincipalAxes:
      return "PrincipalAxes";
    default:
      vtkErrorWithObjectMacro(nullptr, "GetShapeStatisticFromString: Cannot determine string for statistic: " << statistic);
      return "";
    }
}

//----------------------------------------------------------------------------
vtkITKLabelShapeStatistics::ShapeStatistic vtkITKLabelShapeStatistics::GetShapeStatisticFromString(std::string statisticName)
{
  for (int i = 0; i < ShapeStatistic_Last; ++i)
    {
    ShapeStatistic statistic = static_cast<ShapeStatistic>(i);
    if (statisticName == vtkITKLabelShapeStatistics::GetShapeStatisticAsString(statistic))
      {
      return statistic;
      }
    }
  vtkErrorWithObjectMacro(nullptr, "GetShapeStatisticFromString: Cannot determine statistic from string: " << statisticName);
  return ShapeStatistic_Last;
}

//----------------------------------------------------------------------------
void vtkITKLabelShapeStatistics::ComputeShapeStatisticOn(std::string statisticName)
{
  this->SetComputeShapeStatistic(statisticName, true);
}

//----------------------------------------------------------------------------
void vtkITKLabelShapeStatistics::ComputeShapeStatisticOff(std::string statisticName)
{
  this->SetComputeShapeStatistic(statisticName, false);
}

//----------------------------------------------------------------------------
void vtkITKLabelShapeStatistics::SetComputeShapeStatistic(std::string statisticName, bool state)
{
  std::vector<std::string>::iterator statIt = std::find(this->ComputedStatistics.begin(), this->ComputedStatistics.end(), statisticName);
  if (!state)
    {
    if (statIt != this->ComputedStatistics.end())
      {
      this->ComputedStatistics.erase(statIt);
      }
    }
  else
    {
    if (statIt == this->ComputedStatistics.end())
      {
      this->ComputedStatistics.push_back(statisticName);
      }
    }
}

//----------------------------------------------------------------------------
bool vtkITKLabelShapeStatistics::GetComputeShapeStatistic(std::string statisticName)
{
  std::vector<std::string>::iterator statIt = std::find(this->ComputedStatistics.begin(), this->ComputedStatistics.end(), statisticName);
  if (statIt != this->ComputedStatistics.end())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
// Note: local function not method - conforms to signature in itkCommand.h
void vtkITKLabelShapeStatisticsHandleProgressEvent (itk::Object *caller,
                                          const itk::EventObject& vtkNotUsed(eventObject),
                                          void *clientdata)
{
  itk::ProcessObject *itkFilter = dynamic_cast<itk::ProcessObject*>(caller);
  vtkAlgorithm *vtkFilter = reinterpret_cast<vtkAlgorithm*>(clientdata);
  if ( itkFilter && vtkFilter )
    {
    vtkFilter->UpdateProgress ( itkFilter->GetProgress() );
    }
};

//----------------------------------------------------------------------------
template <class T>
T* GetArray(vtkTable* table, std::string name, int numberOfComponents, std::vector<std::string>* componentNames = nullptr)
{
  vtkSmartPointer<T> array = T::SafeDownCast(table->GetColumnByName(name.c_str()));
  if (!array)
    {
    array = vtkSmartPointer<T>::New();
    array->SetName(name.c_str());
    array->SetNumberOfComponents(numberOfComponents);
    array->SetNumberOfTuples(table->GetNumberOfRows());
    table->AddColumn(array);
    int componentIndex = 0;
    if (componentNames)
      {
      if (numberOfComponents != static_cast<int>(componentNames->size()))
        {
        vtkErrorWithObjectMacro(nullptr, "vtkITKLabelShapeStatistics: GetArray - Number of components and component names do not match!");
        }
      else
        {
        for (std::string componentName : *componentNames)
          {
          array->SetComponentName(componentIndex, componentName.c_str());
          }
        }
      }
    }
  return array.GetPointer();
}

//----------------------------------------------------------------------------
template <class T>
void vtkITKLabelShapeStatisticsExecute(vtkITKLabelShapeStatistics* self, vtkImageData* input, vtkTable* output,
  vtkMatrix4x4* directionMatrix, T* vtkNotUsed(inPtr))
{
  if (!self || !input || !output)
    {
    return;
    }

  // Clear current results
  output->Initialize();

  // Wrap VTK image into an ITK image
  using ImageType = itk::Image<T, 3> ;
  using VTKToITKFilterType = itk::VTKImageToImageFilter<ImageType>;
  typename VTKToITKFilterType::Pointer vtkToITKFilter = VTKToITKFilterType::New();
  vtkToITKFilter->SetInput(input);
  vtkToITKFilter->Update();
  ImageType* inImage = vtkToITKFilter->GetOutput();

  // TODO: When vtkImageData is updated to include direction, this should be updated
  if (directionMatrix)
    {
    typename ImageType::DirectionType gridDirectionMatrix;
    for (unsigned int row = 0; row < 3; row++)
      {
      for (unsigned int column = 0; column < 3; column++)
        {
        gridDirectionMatrix(row, column) = directionMatrix->GetElement(row, column);
        }
      }
    inImage->SetDirection(gridDirectionMatrix);
    }

  // Set up the progress callback
  itk::CStyleCommand::Pointer progressCommand = itk::CStyleCommand::New();
  progressCommand->SetClientData(static_cast<void*>(self));
  progressCommand->SetCallback(vtkITKLabelShapeStatisticsHandleProgressEvent);

  using ShapeLabelObjectType = itk::ShapeLabelObject<T, 3>;
  using LabelMapType = itk::LabelMap<ShapeLabelObjectType>;
  using LableShapeFilterType = itk::LabelImageToShapeLabelMapFilter<ImageType, LabelMapType>;

  bool computeFeretDiameter = self->GetComputeShapeStatistic(self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::ShapeStatistic::FeretDiameter));
  bool computePerimeter = self->GetComputeShapeStatistic(self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::ShapeStatistic::Perimeter)) ||
    self->GetComputeShapeStatistic(self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::ShapeStatistic::Roundness));
  bool computeOrientedBoundingBox =
  self->GetComputeShapeStatistic(self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::ShapeStatistic::OrientedBoundingBox));

  typename LableShapeFilterType::Pointer labelFilter = LableShapeFilterType::New();
  labelFilter->AddObserver(itk::ProgressEvent(), progressCommand);
  labelFilter->SetInput(inImage);
  labelFilter->SetComputeFeretDiameter(computeFeretDiameter);
  labelFilter->SetComputePerimeter(computePerimeter);
  labelFilter->SetComputeOrientedBoundingBox(computeOrientedBoundingBox);
  labelFilter->Update();

  typename LabelMapType::Pointer labelmapObject = labelFilter->GetOutput();
  const std::vector<typename ShapeLabelObjectType::LabelType> labelValues = labelmapObject->GetLabels();

  // Number of rows in the table is equal to the number of label values
  output->SetNumberOfRows(labelValues.size());

  int rowIndex = -1;
  for (unsigned int i = 0; i < labelValues.size(); ++i)
    {
    rowIndex++;
    int labelValue = labelValues[i];

    typename ShapeLabelObjectType::ShapeLabelObject::Pointer shapeObject = labelmapObject->GetLabelObject(labelValue);
    if (!shapeObject)
      {
      continue;
      }

    vtkLongArray* array = GetArray<vtkLongArray>(output, "LabelValue", 1);
    array->InsertTuple1(rowIndex, labelValue);

    for (std::string statisticName : self->GetComputedStatistics())
      {
      if (statisticName == self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::Centroid))
        {
        typename ShapeLabelObjectType::CentroidType centroidObject = shapeObject->GetCentroid();
        vtkDoubleArray* array = GetArray<vtkDoubleArray>(output, statisticName, 3);
        array->InsertTuple3(rowIndex, centroidObject[0], centroidObject[1], centroidObject[2]);
        }
      else if (statisticName == self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::Roundness))
        {
        double roundness = shapeObject->GetRoundness();
        vtkDoubleArray* array = GetArray<vtkDoubleArray>(output, statisticName, 1);
        array->InsertTuple1(rowIndex, roundness);
        }
      else if (statisticName == self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::Flatness))
        {
        double flatness = shapeObject->GetFlatness();
        vtkDoubleArray* array = GetArray<vtkDoubleArray>(output, statisticName, 1);
        array->InsertTuple1(rowIndex, flatness);
        }
      else if (statisticName == self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::Elongation))
        {
        double elongation = shapeObject->GetElongation();
        vtkDoubleArray* array = GetArray<vtkDoubleArray>(output, statisticName, 1);
        array->InsertTuple1(rowIndex, elongation);
        }
      else if (statisticName == self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::FeretDiameter))
        {
        double feretDiameter = shapeObject->GetFeretDiameter();
        vtkDoubleArray* array = GetArray<vtkDoubleArray>(output, statisticName, 1);
        array->InsertTuple1(rowIndex, feretDiameter);
        }
      else if (statisticName == self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::Perimeter))
        {
        double perimeter = shapeObject->GetPerimeter();
        vtkDoubleArray* array = GetArray<vtkDoubleArray>(output, statisticName, 1);
        array->InsertTuple1(rowIndex, perimeter);
        }
      else if (statisticName == self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::OrientedBoundingBox))
        {
        typename ShapeLabelObjectType::OrientedBoundingBoxPointType boundingBoxOrigin = shapeObject->GetOrientedBoundingBoxOrigin();
        vtkDoubleArray* obbOriginArray = GetArray<vtkDoubleArray>(output, "OrientedBoundingBoxOrigin", 3);
        obbOriginArray->InsertTuple3(rowIndex, boundingBoxOrigin[0], boundingBoxOrigin[1], boundingBoxOrigin[2]);

        typename ShapeLabelObjectType::OrientedBoundingBoxPointType boundingBoxSize = shapeObject->GetOrientedBoundingBoxSize();
        std::vector<std::string> componentNames = { "x", "y", "z" };
        vtkDoubleArray* obbSizeArray = GetArray<vtkDoubleArray>(output, "OrientedBoundingBoxSize", 3, &componentNames);
        obbSizeArray->InsertTuple3(rowIndex, boundingBoxSize[0], boundingBoxSize[1], boundingBoxSize[2]);

        typename ShapeLabelObjectType::OrientedBoundingBoxDirectionType boundingBoxDirections = shapeObject->GetOrientedBoundingBoxDirection();
        vtkDoubleArray* obbDirectionXArray = GetArray<vtkDoubleArray>(output, "OrientedBoundingBoxDirectionX", 3);
        obbDirectionXArray->InsertTuple3(rowIndex, boundingBoxDirections(0, 0), boundingBoxDirections(0, 1), boundingBoxDirections(0, 2));
        vtkDoubleArray* obbDirectionYArray = GetArray<vtkDoubleArray>(output, "OrientedBoundingBoxDirectionY", 3);
        obbDirectionYArray->InsertTuple3(rowIndex, boundingBoxDirections(1, 0), boundingBoxDirections(1, 1), boundingBoxDirections(1, 2));
        vtkDoubleArray* obbDirectionZArray = GetArray<vtkDoubleArray>(output, "OrientedBoundingBoxDirectionZ", 3);
        obbDirectionZArray->InsertTuple3(rowIndex, boundingBoxDirections(2, 0), boundingBoxDirections(2, 1), boundingBoxDirections(2, 2));
        }
      else if (statisticName == self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::PrincipalMoments))
        {
        typename ShapeLabelObjectType::VectorType principalMoments = shapeObject->GetPrincipalMoments();
        vtkDoubleArray* principalMomentsArray = GetArray<vtkDoubleArray>(output, statisticName, 3);
        principalMomentsArray->InsertTuple3(rowIndex, principalMoments[0], principalMoments[1], principalMoments[2]);
        }
      else if (statisticName == self->GetShapeStatisticAsString(vtkITKLabelShapeStatistics::PrincipalAxes))
        {
        typename ShapeLabelObjectType::MatrixType principalAxes = shapeObject->GetPrincipalAxes();
        vtkDoubleArray* principalAxisXArray = GetArray<vtkDoubleArray>(output, "PrincipalAxisX", 3);
        principalAxisXArray->InsertTuple3(rowIndex, principalAxes(0, 0), principalAxes(0, 1), principalAxes(0, 2));
        vtkDoubleArray* principalAxisYArray = GetArray<vtkDoubleArray>(output, "PrincipalAxisY", 3);
        principalAxisYArray->InsertTuple3(rowIndex, principalAxes(1, 0), principalAxes(1, 1), principalAxes(1, 2));
        vtkDoubleArray* principalAxisZArray = GetArray<vtkDoubleArray>(output, "PrincipalAxisZ", 3);
        principalAxisZArray->InsertTuple3(rowIndex, principalAxes(2, 0), principalAxes(2, 1), principalAxes(2, 2));
        }
      }
    }
}

//----------------------------------------------------------------------------
// This is the superclasses style of Execute method.  Convert it into
// an imaging style Execute method.
int vtkITKLabelShapeStatistics::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the data object
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkTable* output = vtkTable::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkImageData* input = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDebugMacro(<< "Executing label shape statistics");

  //
  // Initialize and check input
  //
  vtkPointData *pd = input->GetPointData();
  if (pd ==nullptr)
    {
    vtkErrorMacro(<<"PointData is NULL");
    return 0;
    }
  vtkDataArray *inScalars=pd->GetScalars();
  if ( inScalars == nullptr )
    {
    vtkErrorMacro(<<"Scalars must be defined for island math");
    return 0;
    }

  if (inScalars->GetNumberOfComponents() == 1 )
    {

////////// These types are not defined in itk ////////////
#undef VTK_TYPE_USE_LONG_LONG
#undef VTK_TYPE_USE___INT64

#define CALL  vtkITKLabelShapeStatisticsExecute(this, input, output, this->Directions, static_cast<VTK_TT *>(inPtr));

    void* inPtr = input->GetScalarPointer();
    switch (inScalars->GetDataType())
      {
      vtkTemplateMacroCase(VTK_LONG, long, CALL);                               \
      vtkTemplateMacroCase(VTK_UNSIGNED_LONG, unsigned long, CALL);             \
      vtkTemplateMacroCase(VTK_INT, int, CALL);                                 \
      vtkTemplateMacroCase(VTK_UNSIGNED_INT, unsigned int, CALL);               \
      vtkTemplateMacroCase(VTK_SHORT, short, CALL);                             \
      vtkTemplateMacroCase(VTK_UNSIGNED_SHORT, unsigned short, CALL);           \
      vtkTemplateMacroCase(VTK_CHAR, char, CALL);                               \
      vtkTemplateMacroCase(VTK_SIGNED_CHAR, signed char, CALL);                 \
      vtkTemplateMacroCase(VTK_UNSIGNED_CHAR, unsigned char, CALL);             \
      default:
        {
        vtkErrorMacro(<< "Incompatible data type for this version of ITK.");
        return 0;
        }
      } //switch
    }
  else
    {
    vtkErrorMacro(<< "Only single component images supported.");
    return 0;
    }
  return 1;
}
