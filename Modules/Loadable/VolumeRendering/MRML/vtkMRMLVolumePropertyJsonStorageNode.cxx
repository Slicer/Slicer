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
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/

// VolumeRendering MRML includes
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumePropertyJsonStorageNode.h"

// MRML includes
#include "vtkMRMLI18N.h"
#include "vtkMRMLJsonElement.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkContourValues.h>
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkStringArray.h>
#include <vtkVolumeProperty.h>

// std includes
#include <sstream>

namespace
{
const std::string VOLUME_PROPERTY_SCHEMA =
  "https://raw.githubusercontent.com/slicer/slicer/main/Modules/Loadable/VolumeRendering/Resources/Schema/volume-property-schema-v1.0.0.json#";
// regex should be lower case
const std::string ACCEPTED_VOLUME_PROPERTY_SCHEMA_REGEX = ".*volume-property-schema-v[0-9]+\\.[0-9]+\\.[0-9]+\\.json#*$";
} // namespace

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVolumePropertyJsonStorageNode);

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyJsonStorageNode::vtkMRMLVolumePropertyJsonStorageNode()
{
  this->TypeDisplayName = vtkMRMLTr("vtkMRMLVolumePropertyJsonStorageNode", "Volume Property Json Storage");

  this->DefaultWriteFileExtension = "vp.json";
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyJsonStorageNode::~vtkMRMLVolumePropertyJsonStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyJsonStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumePropertyJsonStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLVolumePropertyNode");
}

//----------------------------------------------------------------------------
int vtkMRMLVolumePropertyJsonStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  vtkMRMLVolumePropertyNode* vpNode = vtkMRMLVolumePropertyNode::SafeDownCast(refNode);

  const char* filePath = this->GetFileName();
  if (!filePath)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::ReadDataInternal", "Reading volume property node file failed: invalid filename.");
    return 0;
  }

  int result = 1;

  vtkNew<vtkMRMLJsonReader> jsonReader;
  vtkSmartPointer<vtkMRMLJsonElement> jsonElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonReader->ReadFromFile(filePath));
  if (!jsonElement)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::ReadDataInternal", jsonReader->GetUserMessages()->GetAllMessagesAsString());
    return 0;
  }

  vtkSmartPointer<vtkMRMLJsonElement> volumeProperties = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonElement->GetArrayProperty("volumeProperties"));
  if (!volumeProperties)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::ReadDataInternal", "Reading volume property node file failed: 'volumeProperties' array not found.");
    return 0;
  }

  if (volumeProperties->GetArraySize() < 1)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::ReadDataInternal", "Reading volume property node file failed: 'volumeProperties' array is empty.");
    return 0;
  }

  vtkSmartPointer<vtkMRMLJsonElement> volumePropertyElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(volumeProperties->GetArrayItem(0));
  if (!this->ReadVolumePropertyNode(vpNode, volumePropertyElement))
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::ReadDataInternal", "Reading volume property node file failed: unable to read volume property node.");
    return 0;
  }

  return 1;
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumePropertyJsonStorageNode::ReadVolumePropertyNode(vtkMRMLVolumePropertyNode* volumePropertyNode, vtkMRMLJsonElement* volumePropertyElement)
{
  if (!volumePropertyNode || !volumePropertyElement)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::ReadVolumePropertyNodeFromJsonElement", "Reading volume property node failed: invalid node or element.");
    return false;
  }

  bool success = true;
  double effectiveRange[2] = { 0.0, 1.0 };
  if (volumePropertyElement->GetVectorProperty("effectiveRange", effectiveRange, 2))
  {
    volumePropertyNode->SetEffectiveRange(effectiveRange);
  }

  vtkVolumeProperty* volumeProperty = volumePropertyNode->GetVolumeProperty();

  vtkSmartPointer<vtkDoubleArray> isoSurfaceValues = vtkSmartPointer<vtkDoubleArray>::Take(volumePropertyElement->GetDoubleArrayProperty("isoSurfaceValues"));
  if (isoSurfaceValues)
  {
    volumeProperty->GetIsoSurfaceValues()->SetNumberOfContours(isoSurfaceValues->GetNumberOfValues());
    for (int i = 0; i < isoSurfaceValues->GetNumberOfValues(); ++i)
    {
      volumeProperty->GetIsoSurfaceValues()->SetValue(i, isoSurfaceValues->GetValue(i));
    }
  }

  bool independentComponents = true;
  if (volumePropertyElement->GetBoolProperty("independentComponents", independentComponents))
  {
    volumeProperty->SetIndependentComponents(independentComponents);
  }

  std::string interpolationTypeString = vtkMRMLVolumePropertyJsonStorageNode::GetInterpolationTypeAsString(VTK_LINEAR_INTERPOLATION);
  volumePropertyElement->GetStringProperty("interpolationType", interpolationTypeString);
  volumeProperty->SetInterpolationType(vtkMRMLVolumePropertyJsonStorageNode::GetInterpolationTypeFromString(interpolationTypeString));

  bool useClippedVoxelIntensity = false;
  volumePropertyElement->GetBoolProperty("useClippedVoxelIntensity", useClippedVoxelIntensity);
  volumeProperty->SetUseClippedVoxelIntensity(useClippedVoxelIntensity);

  double clippedVoxelIntensity = 0.0;
  volumePropertyElement->GetDoubleProperty("clippedVoxelIntensity", clippedVoxelIntensity);
  volumeProperty->SetClippedVoxelIntensity(clippedVoxelIntensity);

  double scatteringAnisotropy = 0.0;
  volumePropertyElement->GetDoubleProperty("scatteringAnisotropy", scatteringAnisotropy);
  volumeProperty->SetScatteringAnisotropy(scatteringAnisotropy);

  vtkSmartPointer<vtkMRMLJsonElement> componentsElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(volumePropertyElement->GetArrayProperty("components"));
  if (componentsElement)
  {
    int numberOfComponents = componentsElement->GetArraySize();
    volumePropertyNode->SetNumberOfIndependentComponents(numberOfComponents);
    if (numberOfComponents > VTK_MAX_VRCOMP)
    {
      vtkWarningMacro("vtkMRMLVolumePropertyJsonStorageNode::ReadVolumeProperty: "
                      "Number of components exceeds maximum ("
                      << VTK_MAX_VRCOMP
                      << "). "
                         "Only the first "
                      << VTK_MAX_VRCOMP << " components will be read.");
    }

    for (int i = 0; i < numberOfComponents && i < VTK_MAX_VRCOMP; ++i)
    {
      vtkSmartPointer<vtkMRMLJsonElement> componentElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(componentsElement->GetArrayItem(i));
      if (!componentElement)
      {
        vtkErrorToMessageCollectionMacro(
          this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::ReadVolumeProperty", "Reading volume property failed: component element is null.");
        success = false;
        continue;
      }

      double componentWeight = 1.0;
      if (componentElement->GetDoubleProperty("componentWeight", componentWeight))
      {
        volumeProperty->SetComponentWeight(i, componentWeight);
      }

      bool shade = true;
      if (componentElement->GetBoolProperty("shade", shade))
      {
        volumeProperty->SetShade(i, shade);
      }

      vtkSmartPointer<vtkMRMLJsonElement> lightingElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(componentElement->GetObjectProperty("lighting"));
      if (lightingElement)
      {
        double ambient = 0.1;
        lightingElement->GetDoubleProperty("ambient", ambient);
        volumeProperty->SetAmbient(i, ambient);

        double diffuse = 0.7;
        lightingElement->GetDoubleProperty("diffuse", diffuse);
        volumeProperty->SetDiffuse(i, diffuse);

        double specular = 0.2;
        lightingElement->GetDoubleProperty("specular", specular);
        volumeProperty->SetSpecular(i, specular);

        double specularPower = 10.0;
        lightingElement->GetDoubleProperty("specularPower", specularPower);
        volumeProperty->SetSpecularPower(i, specularPower);
      }

      bool disableGradientOpacity = false;
      componentElement->GetBoolProperty("disableGradientOpacity", disableGradientOpacity);
      volumeProperty->SetDisableGradientOpacity(i, disableGradientOpacity);

      double scalarOpacityUnitDistance = 1.0;
      componentElement->GetDoubleProperty("scalarOpacityUnitDistance", scalarOpacityUnitDistance);
      volumeProperty->SetScalarOpacityUnitDistance(i, scalarOpacityUnitDistance);

      if (componentElement->HasMember("rgbTransferFunction"))
      {
        vtkSmartPointer<vtkMRMLJsonElement> rgbTransferFunctionElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(componentElement->GetObjectProperty("rgbTransferFunction"));
        if (rgbTransferFunctionElement)
        {
          success &= this->ReadTransferFunction(volumeProperty->GetRGBTransferFunction(i), rgbTransferFunctionElement);
        }
      }

      vtkSmartPointer<vtkMRMLJsonElement> scalarOpacityElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(componentElement->GetObjectProperty("scalarOpacity"));
      if (scalarOpacityElement)
      {
        vtkNew<vtkPiecewiseFunction> scalarOpacity;
        success &= this->ReadTransferFunction(scalarOpacity, scalarOpacityElement);
        volumeProperty->SetScalarOpacity(i, scalarOpacity);
      }

      vtkSmartPointer<vtkMRMLJsonElement> gradientOpacityElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(componentElement->GetObjectProperty("gradientOpacity"));
      if (gradientOpacityElement)
      {
        vtkNew<vtkPiecewiseFunction> gradientOpacity;
        success &= this->ReadTransferFunction(gradientOpacity, gradientOpacityElement);
        volumeProperty->SetGradientOpacity(i, gradientOpacity);
      }

      // 2D transfer function support could be implemented here in the future by calling ReadTransferFunction2D.
      // this->ReadTransferFunction2D(transferFunction2D, "transferFunction2D");
    }
  }

  return success;
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumePropertyJsonStorageNode::ReadTransferFunction(vtkObject* transferFunction, vtkMRMLJsonElement* transferFunctionElement)
{
  if (!transferFunctionElement)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::ReadTransferFunction", "Reading transfer function failed: transfer function element is null.");
    return false;
  }

  vtkPiecewiseFunction* piecewiseFunction = vtkPiecewiseFunction::SafeDownCast(transferFunction);
  vtkColorTransferFunction* colorTransferFunction = vtkColorTransferFunction::SafeDownCast(transferFunction);
  if (!piecewiseFunction && !colorTransferFunction)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumePropertyJsonStorageNode::ReadTransferFunction",
                                     "Reading transfer function failed: transfer function is neither a piecewise function nor a color transfer function.");
    return false;
  }

  if (piecewiseFunction)
  {
    piecewiseFunction->RemoveAllPoints();
  }
  if (colorTransferFunction)
  {
    colorTransferFunction->RemoveAllPoints();
  }

  vtkSmartPointer<vtkMRMLJsonElement> pointsElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(transferFunctionElement->GetArrayProperty("points"));
  for (int i = 0; i < pointsElement->GetArraySize(); ++i)
  {
    vtkSmartPointer<vtkMRMLJsonElement> pointElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(pointsElement->GetArrayItem(i));
    if (!pointElement)
    {
      vtkErrorToMessageCollectionMacro(
        this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::ReadTransferFunction", "Reading transfer function failed: point element is null.");
      return false;
    }

    double x = 0.0;
    pointElement->GetDoubleProperty("x", x);
    double midpoint = 0.5;
    pointElement->GetDoubleProperty("midpoint", midpoint);
    double sharpness = 0.0;
    pointElement->GetDoubleProperty("sharpness", sharpness);
    if (piecewiseFunction)
    {
      double y = 0.0;
      pointElement->GetDoubleProperty("y", y);
      double values[4] = { x, y, midpoint, sharpness };
      piecewiseFunction->AddPoint(x, y, midpoint, sharpness);
    }
    else if (colorTransferFunction)
    {
      double color[3] = { 0.0, 0.0, 0.0 };
      pointElement->GetVectorProperty("color", color, 3);
      colorTransferFunction->AddRGBPoint(x, color[0], color[1], color[2], midpoint, sharpness);
    }
  }

  return true;
}

//---------------------------------------------------------------------------
std::string vtkMRMLVolumePropertyJsonStorageNode::GetInterpolationTypeAsString(int interpolationType)
{
  switch (interpolationType)
  {
    case VTK_NEAREST_INTERPOLATION: return "nearest";
    case VTK_LINEAR_INTERPOLATION: return "linear";
    case VTK_CUBIC_INTERPOLATION: return "cubic";
    default: break;
  }
  return "Unknown";
}

//---------------------------------------------------------------------------
int vtkMRMLVolumePropertyJsonStorageNode::GetInterpolationTypeFromString(const std::string& interpolationTypeString)
{
  if (interpolationTypeString == "nearest")
  {
    return VTK_NEAREST_INTERPOLATION;
  }
  else if (interpolationTypeString == "linear")
  {
    return VTK_LINEAR_INTERPOLATION;
  }
  else if (interpolationTypeString == "cubic")
  {
    return VTK_CUBIC_INTERPOLATION;
  }
  return VTK_NEAREST_INTERPOLATION;
}

//----------------------------------------------------------------------------
int vtkMRMLVolumePropertyJsonStorageNode::WriteDataInternal(vtkMRMLNode* refNode)
{
  vtkMRMLVolumePropertyNode* vpNode = vtkMRMLVolumePropertyNode::SafeDownCast(refNode);
  if (!vpNode)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::WriteDataInternal", "Writing volume property node file failed: invalid node.");
    return 0;
  }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::WriteDataInternal", "Writing markups node file failed: file name not specified.");
    return 0;
  }

  vtkNew<vtkMRMLJsonWriter> writer;
  if (!writer->WriteToFileBegin(fullName.c_str(), VOLUME_PROPERTY_SCHEMA.c_str()))
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumePropertyJsonStorageNode::WriteDataInternal",
                                     "Writing markups node file failed: unable to open file '" << fullName << "' for writing.");
    return 0;
  }

  int success = 1;
  writer->WriteArrayPropertyStart("volumeProperties");
  if (!this->WriteVolumePropertyNode(writer, vpNode))
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumePropertyJsonStorageNode::WriteDataInternal",
                                     "Writing markups node file failed: unable to write markups node '" << (vpNode->GetName() ? vpNode->GetName() : "") << "'.");
    success = 0;
  }
  writer->WriteArrayPropertyEnd();

  if (!writer->WriteToFileEnd())
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::WriteDataInternal", "Writing volume property node file failed for '" << fullName << "'");
    success = 0;
  }

  return success;
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumePropertyJsonStorageNode::WriteVolumePropertyNode(vtkMRMLJsonWriter* writer, vtkMRMLVolumePropertyNode* volumePropertyNode)
{
  if (!writer)
  {
    vtkErrorMacro("vtkMRMLVolumePropertyJsonStorageNode::WriteVolumePropertyNode: "
                  "Writer is null. Cannot write volume property.");
    return false;
  }
  if (!volumePropertyNode)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::WriteVolumePropertyNode", "Writing volume property node failed: volume property node is null.");
    return false;
  }

  writer->WriteObjectStart();
  writer->WriteVectorProperty("effectiveRange", volumePropertyNode->GetEffectiveRange(), 2);
  this->WriteVolumeProperty(writer, volumePropertyNode->GetVolumeProperty(), volumePropertyNode->GetNumberOfIndependentComponents());
  writer->WriteObjectEnd();
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumePropertyJsonStorageNode::WriteVolumeProperty(vtkMRMLJsonWriter* writer, vtkVolumeProperty* volumeProperty, int numberOfComponents /*=1*/)
{
  if (!writer)
  {
    vtkErrorMacro("vtkMRMLVolumePropertyJsonStorageNode::WriteVolumeProperty: "
                  "Writer is null. Cannot write volume property.");
    return false;
  }
  if (!volumeProperty)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::WriteVolumeProperty", "Writing volume property failed: volume property is null.");
    return false;
  }

  bool success = true;
  success &= this->WriteContourValues(writer, "isoSurfaceValues", volumeProperty->GetIsoSurfaceValues());

  writer->WriteBoolProperty("independentComponents", volumeProperty->GetIndependentComponents());
  writer->WriteStringProperty("interpolationType", vtkMRMLVolumePropertyJsonStorageNode::GetInterpolationTypeAsString(volumeProperty->GetInterpolationType()));
  writer->WriteBoolProperty("useClippedVoxelIntensity", volumeProperty->GetUseClippedVoxelIntensity());
  writer->WriteDoubleProperty("clippedVoxelIntensity", volumeProperty->GetClippedVoxelIntensity());
  writer->WriteDoubleProperty("scatteringAnisotropy", volumeProperty->GetScatteringAnisotropy());

  writer->WriteArrayPropertyStart("components");
  for (int i = 0; i < numberOfComponents; ++i)
  {
    writer->WriteObjectStart();

    writer->WriteDoubleProperty("componentWeight", volumeProperty->GetComponentWeight(i));
    writer->WriteBoolProperty("shade", volumeProperty->GetShade(i));

    writer->WriteObjectPropertyStart("lighting");
    writer->WriteDoubleProperty("diffuse", volumeProperty->GetDiffuse(i));
    writer->WriteDoubleProperty("ambient", volumeProperty->GetAmbient(i));
    writer->WriteDoubleProperty("specular", volumeProperty->GetSpecular(i));
    writer->WriteDoubleProperty("specularPower", volumeProperty->GetSpecularPower(i));
    writer->WriteObjectEnd();

    writer->WriteBoolProperty("disableGradientOpacity", volumeProperty->GetDisableGradientOpacity(i));
    writer->WriteDoubleProperty("scalarOpacityUnitDistance", volumeProperty->GetScalarOpacityUnitDistance(i));

    success &= this->WriteTransferFunction(writer, "rgbTransferFunction", volumeProperty->GetRGBTransferFunction(i));
    success &= this->WriteTransferFunction(writer, "scalarOpacity", volumeProperty->GetScalarOpacity(i));
    success &= this->WriteTransferFunction(writer, "gradientOpacity", volumeProperty->GetStoredGradientOpacity(i));

    // 2D transfer function support could be implemented here in the future by calling WriteTransferFunction2D.
    // this->WriteTransferFunction2D(writer, volumeProperty->GetTransferFunction2D(i), "transferFunction2D");

    writer->WriteObjectEnd();
  }
  writer->WriteArrayPropertyEnd();

  return success;
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumePropertyJsonStorageNode::WriteContourValues(vtkMRMLJsonWriter* writer, const char* name, vtkContourValues* contourValues)
{
  if (!writer)
  {
    vtkErrorMacro("vtkMRMLVolumePropertyJsonStorageNode::WriteContourValues: "
                  "Writer is null. Cannot write iso-surface values.");
    return false;
  }

  if (!name || std::string(name).empty())
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::WriteContourValues", "Writing iso-surface values failed: name is null or empty.") return false;
  }

  if (!contourValues)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::WriteContourValues", "Writing iso-surface values failed: contour values is null.") return false;
  }

  double* values = contourValues->GetValues();
  vtkNew<vtkDoubleArray> contourValuesArray;
  contourValuesArray->SetNumberOfValues(contourValues->GetNumberOfContours());
  for (int i = 0; i < contourValues->GetNumberOfContours(); ++i)
  {
    contourValuesArray->SetValue(i, contourValues->GetValue(i));
  }
  writer->WriteDoubleArrayProperty("isoSurfaceValues", contourValuesArray);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumePropertyJsonStorageNode::WriteTransferFunction(vtkMRMLJsonWriter* writer, const char* name, vtkObject* transferFunction)
{
  if (!writer)
  {
    vtkErrorMacro("vtkMRMLVolumePropertyJsonStorageNode::WriteTransferFunction: "
                  "Writer is null. Cannot write transfer function.");
    return false;
  }

  if (!name || std::string(name).empty())
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::WriteTransferFunction", "Writing transfer function failed: name is null or empty.");
    return false;
  }

  vtkPiecewiseFunction* piecewiseFunction = vtkPiecewiseFunction::SafeDownCast(transferFunction);
  vtkColorTransferFunction* colorTransferFunction = vtkColorTransferFunction::SafeDownCast(transferFunction);
  if (!piecewiseFunction && !colorTransferFunction)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumePropertyJsonStorageNode::WriteTransferFunction",
                                     "Writing transfer function failed: transfer function is neither a piecewise function nor a color transfer function.");
    return false;
  }

  writer->WriteObjectPropertyStart(name);

  int numberOfNodes = 0;
  std::string type = "unknown";
  if (piecewiseFunction)
  {
    type = "piecewiseLinearFunction";
    numberOfNodes = piecewiseFunction->GetSize();
  }
  else if (colorTransferFunction)
  {
    type = "colorTransferFunction";
    numberOfNodes = colorTransferFunction->GetSize();
  }
  writer->WriteStringProperty("type", type);

  writer->WriteArrayPropertyStart("points");
  for (int i = 0; i < numberOfNodes; ++i)
  {
    writer->WriteObjectStart();

    if (piecewiseFunction)
    {
      double values[4] = { 0, 0, 0, 0 };
      piecewiseFunction->GetNodeValue(i, values);
      writer->WriteDoubleProperty("x", values[0]);
      writer->WriteDoubleProperty("y", values[1]);
      writer->WriteDoubleProperty("midpoint", values[2]);
      writer->WriteDoubleProperty("sharpness", values[3]);
    }

    if (colorTransferFunction)
    {
      double values[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
      colorTransferFunction->GetNodeValue(i, values);
      writer->WriteDoubleProperty("x", values[0]);
      writer->WriteVectorProperty("color", &values[1], 3); // rgb values
      writer->WriteDoubleProperty("midpoint", values[4]);
      writer->WriteDoubleProperty("sharpness", values[5]);
    }

    writer->WriteObjectEnd();
  }
  writer->WriteArrayPropertyEnd();

  writer->WriteObjectPropertyEnd();
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyJsonStorageNode::InitializeSupportedReadFileTypes()
{
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLVolumePropertyJsonStorageNode", "MRML Volume Property") + " (.vp.json)");
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyJsonStorageNode::InitializeSupportedWriteFileTypes()
{
  //: File format name
  this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLVolumePropertyJsonStorageNode", "MRML Volume Property") + " (.vp.json)");
}

//----------------------------------------------------------------------------
int vtkMRMLVolumePropertyJsonStorageNode::GetNumberOfVolumePropertiesInFile(const char* filePath)
{
  if (!filePath)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::GetNumberOfVolumePropertiesInFile", "Getting number of volume properties in file failed: invalid filename.");
    return 0;
  }

  vtkNew<vtkMRMLJsonReader> jsonReader;
  vtkSmartPointer<vtkMRMLJsonElement> jsonElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonReader->ReadFromFile(filePath));
  if (!jsonElement)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::GetNumberOfVolumePropertiesInFile", jsonReader->GetUserMessages()->GetAllMessagesAsString());
    return 0;
  }

  vtkSmartPointer<vtkMRMLJsonElement> volumeProperties = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonElement->GetArrayProperty("volumeProperties"));
  if (!volumeProperties)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumePropertyJsonStorageNode::GetNumberOfVolumePropertiesInFile",
                                     "Getting number of volume properties in file failed: 'volumeProperties' array not found.");
    return 0;
  }

  return volumeProperties->GetArraySize();
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkMRMLVolumePropertyJsonStorageNode::AddNewVolumePropertyNodeFromFile(const char* filePath, const char* nodeName /*=nullptr*/, int vpIndex /*=0*/)
{
  if (!filePath)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::AddNewVolumePropertyNodeFromFile", "Reading volume property node file failed: invalid filename.");
    return nullptr;
  }

  std::string newNodeName;
  if (nodeName && strlen(nodeName) > 0)
  {
    newNodeName = this->GetScene()->GetUniqueNameByString(nodeName);
  }
  else
  {
    newNodeName = this->GetScene()->GetUniqueNameByString(this->GetFileNameWithoutExtension(filePath).c_str());
  }

  vtkNew<vtkMRMLJsonReader> jsonReader;
  vtkSmartPointer<vtkMRMLJsonElement> jsonElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonReader->ReadFromFile(filePath));
  if (!jsonElement)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumePropertyJsonStorageNode::AddNewVolumePropertyNodeFromFile", jsonReader->GetUserMessages()->GetAllMessagesAsString());
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLJsonElement> volumeProperties = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonElement->GetArrayProperty("volumeProperties"));
  if (!volumeProperties)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumePropertyJsonStorageNode::AddNewVolumePropertyNodeFromFile",
                                     "Reading volume property node file failed: 'volumeProperties' array not found.");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLJsonElement> volumePropertyElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(volumeProperties->GetArrayItem(vpIndex));
  if (!volumePropertyElement)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumePropertyJsonStorageNode::AddNewVolumePropertyNodeFromFile",
                                     "Reading volume property node file failed: unable to read volume property node.");
    return nullptr;
  }
  vtkMRMLVolumePropertyNode* volumePropertyNode = vtkMRMLVolumePropertyNode::SafeDownCast(this->GetScene()->AddNewNodeByClass("vtkMRMLVolumePropertyNode", newNodeName));
  if (!volumePropertyNode)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumePropertyJsonStorageNode::AddNewVolumePropertyNodeFromFile",
                                     "Adding new volume property node failed: unable to create volume property node.");
    return nullptr;
  }

  if (!this->ReadVolumePropertyNode(volumePropertyNode, volumePropertyElement))
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumePropertyJsonStorageNode::AddNewVolumePropertyNodeFromFile",
                                     "Adding new volume property node failed: unable to read volume property node from file.");
    return nullptr;
  }

  return volumePropertyNode;
}
