// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumePropertyStorageNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkColorTransferFunction.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <limits>
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVolumePropertyNode);

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode::vtkMRMLVolumePropertyNode()
  : EffectiveRange{0.0,-1.0}
{
  this->ObservedEvents = vtkIntArray::New();
  this->ObservedEvents->InsertNextValue(vtkCommand::StartEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::EndEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::InteractionEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::EndInteractionEvent);

  vtkVolumeProperty* property = vtkVolumeProperty::New();
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  property->SetUseClippedVoxelIntensity(true);
  // Clipped voxel intensity has to be a different enough so that it the computed gradient
  // will change primarily between the two sides of the clipping plane.
  // Default value is -10e37, which does not result in smooth clipped surface (most probably
  // due to numerical errors), but -1e10 works robustly.
  property->SetClippedVoxelIntensity(-1e10);
#endif
  vtkSetAndObserveMRMLObjectEventsMacro(this->VolumeProperty, property, this->ObservedEvents);
  property->Delete();

  // Observe the transfer functions
  this->SetColor(property->GetRGBTransferFunction());
  this->SetScalarOpacity(property->GetScalarOpacity());
  this->SetGradientOpacity(property->GetGradientOpacity());

  this->SetHideFromEditors(0);
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode::~vtkMRMLVolumePropertyNode()
{
  if(this->VolumeProperty)
    {
    vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetScalarOpacity());
    vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetGradientOpacity());
    vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetRGBTransferFunction());
    vtkSetAndObserveMRMLObjectMacro(this->VolumeProperty, nullptr);
    }
  this->ObservedEvents->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetEffectiveRange(double min, double max)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting EffectiveRange to (" << min << "," << max << ")");
  if ((this->EffectiveRange[0] != min) || (this->EffectiveRange[1] != max))
    {
    this->EffectiveRange[0] = min;
    this->EffectiveRange[1] = max;
    this->Modified();
    this->InvokeCustomModifiedEvent(EffectiveRangeModified);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetEffectiveRange(double range[2])
{
  this->SetEffectiveRange(range[0], range[1]);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLIntMacro(interpolation, InterpolationType);
  vtkMRMLWriteXMLIntMacro(shade, Shade);
  vtkMRMLWriteXMLFloatMacro(diffuse, Diffuse);
  vtkMRMLWriteXMLFloatMacro(ambient, Ambient);
  vtkMRMLWriteXMLFloatMacro(specular, Specular);
  vtkMRMLWriteXMLFloatMacro(specularPower, SpecularPower);
  vtkMRMLWriteXMLStdStringMacro(scalarOpacity, ScalarOpacityAsString);
  vtkMRMLWriteXMLStdStringMacro(gradientOpacity, GradientOpacityAsString);
  vtkMRMLWriteXMLStdStringMacro(colorTransfer, RGBTransferFunctionAsString);
  vtkMRMLWriteXMLVectorMacro(effectiveRange, EffectiveRange, double, 2);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLIntMacro(interpolation, InterpolationType);
  vtkMRMLReadXMLIntMacro(shade, Shade);
  vtkMRMLReadXMLFloatMacro(diffuse, Diffuse);
  vtkMRMLReadXMLFloatMacro(ambient, Ambient);
  vtkMRMLReadXMLFloatMacro(specular, Specular);
  vtkMRMLReadXMLFloatMacro(specularPower, SpecularPower);
  vtkMRMLReadXMLStdStringMacro(scalarOpacity, ScalarOpacityAsString);
  vtkMRMLReadXMLStdStringMacro(gradientOpacity, GradientOpacityAsString);
  vtkMRMLReadXMLStdStringMacro(colorTransfer, RGBTransferFunctionAsString);
  vtkMRMLReadXMLVectorMacro(effectiveRange, EffectiveRange, double, 2);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  this->CopyParameterSet(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::CopyParameterSet(vtkMRMLNode *anode)
{
  vtkMRMLVolumePropertyNode *node = vtkMRMLVolumePropertyNode::SafeDownCast(anode);
  if (!node)
    {
    vtkErrorMacro("CopyParameterSet: Invalid input MRML node");
    return;
    }

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyVectorMacro(EffectiveRange, double, 2);
  vtkMRMLCopyEndMacro();

  // VolumeProperty
  this->VolumeProperty->SetIndependentComponents(node->VolumeProperty->GetIndependentComponents());
  this->VolumeProperty->SetInterpolationType(node->VolumeProperty->GetInterpolationType());
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  this->VolumeProperty->SetUseClippedVoxelIntensity(node->VolumeProperty->GetUseClippedVoxelIntensity());
#endif

  for (int i=0; i<VTK_MAX_VRCOMP; i++)
    {
    this->VolumeProperty->SetComponentWeight(i,node->GetVolumeProperty()->GetComponentWeight(i));
    //TODO: No set method for GrayTransferFunction, ColorChannels, and DefaultGradientOpacity

    // Transfer functions
    vtkColorTransferFunction* rgbTransfer = vtkColorTransferFunction::New();
    rgbTransfer->DeepCopy(node->GetVolumeProperty()->GetRGBTransferFunction(i));
    this->SetColor(rgbTransfer, i);
    rgbTransfer->Delete();

    vtkPiecewiseFunction* scalar = vtkPiecewiseFunction::New();
    scalar->DeepCopy(node->GetVolumeProperty()->GetScalarOpacity(i));
    this->SetScalarOpacity(scalar, i);
    scalar->Delete();
    this->VolumeProperty->SetScalarOpacityUnitDistance(i,this->VolumeProperty->GetScalarOpacityUnitDistance(i));

    vtkPiecewiseFunction* gradient = vtkPiecewiseFunction::New();
    gradient->DeepCopy(node->GetVolumeProperty()->GetGradientOpacity(i));
    this->SetGradientOpacity(gradient, i);
    gradient->Delete();

    // Lighting
    this->VolumeProperty->SetDisableGradientOpacity(i,node->GetVolumeProperty()->GetDisableGradientOpacity(i));
    this->VolumeProperty->SetShade(i,node->GetVolumeProperty()->GetShade(i));
    this->VolumeProperty->SetAmbient(i, node->VolumeProperty->GetAmbient(i));
    this->VolumeProperty->SetDiffuse(i, node->VolumeProperty->GetDiffuse(i));
    this->VolumeProperty->SetSpecular(i, node->VolumeProperty->GetSpecular(i));
    this->VolumeProperty->SetSpecularPower(i, node->VolumeProperty->GetSpecularPower(i));
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintVectorMacro(EffectiveRange, double, 2);
  vtkMRMLPrintEndMacro();

  os << indent << "VolumeProperty: ";
  this->VolumeProperty->PrintSelf(os,indent.GetNextIndent());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::ProcessMRMLEvents( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData )
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);
  switch (event)
    {
    case vtkCommand::StartEvent:
    case vtkCommand::EndEvent:
    case vtkCommand::StartInteractionEvent:
    case vtkCommand::InteractionEvent:
    case vtkCommand::EndInteractionEvent:
      this->InvokeEvent(event);
      break;
    case vtkCommand::ModifiedEvent:
      this->Modified();
      break;
    }
}

//---------------------------------------------------------------------------
std::string vtkMRMLVolumePropertyNode::DataToString(double* data, int size)
{
  std::stringstream resultStream;
  double *it = data;
  // Write header
  resultStream << size;
  resultStream.precision(std::numeric_limits<double>::digits10);
  for (int i=0; i < size; ++i)
    {
    resultStream << " ";
    resultStream << *it;
    it++;
    }
  return resultStream.str();
}

//---------------------------------------------------------------------------
int vtkMRMLVolumePropertyNode::DataFromString(const std::string& dataString, double* &data)
{
  std::stringstream stream;
  stream << dataString;

  int size=0;
  stream >> size;
  if (size==0)
    {
    return 0;
    }
  data = new double[size];
  for(int i=0; i < size; ++i)
    {
    std::string s;
    stream >> s;
    data[i] = atof(s.c_str());
    }
  return size;
}

//---------------------------------------------------------------------------
int vtkMRMLVolumePropertyNode::NodesFromString(const std::string& dataString, double* &nodes, int nodeSize)
{
  int size = vtkMRMLVolumePropertyNode::DataFromString(dataString, nodes);
  if (size % nodeSize)
    {
    vtkGenericWarningMacro("vtkMRMLVolumePropertyNode::NodesFromString: Error parsing data string");
    return 0;
    }
  // Ensure uniqueness
  double previous = VTK_DOUBLE_MIN;
  for (int i = 0; i < size; i+= nodeSize)
    {
    nodes[i] = vtkMRMLVolumePropertyNode::HigherAndUnique(nodes[i], previous);
    }
  return size / nodeSize;
}

//---------------------------------------------------------------------------
std::string vtkMRMLVolumePropertyNode::GetPiecewiseFunctionString(vtkPiecewiseFunction* function)
{
  return vtkMRMLVolumePropertyNode::DataToString(function->GetDataPointer(), function->GetSize() * 2);
}

//---------------------------------------------------------------------------
std::string vtkMRMLVolumePropertyNode::GetColorTransferFunctionString(vtkColorTransferFunction* function)
{
  return vtkMRMLVolumePropertyNode::DataToString(function->GetDataPointer(), function->GetSize() * 4);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::GetPiecewiseFunctionFromString(
  const std::string& str,vtkPiecewiseFunction* result)
{
  double* data = nullptr;
  int size = vtkMRMLVolumePropertyNode::NodesFromString(str, data, 2);
  if (size)
    {
    result->FillFromDataPointer(size, data);
    }
  delete [] data;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::GetColorTransferFunctionFromString(
  const std::string& str, vtkColorTransferFunction* result)
{
  double* data = nullptr;
  int size = vtkMRMLVolumePropertyNode::NodesFromString(str, data, 4);
  if (size)
    {
    result->FillFromDataPointer(size, data);
    }
  delete [] data;
}

//----------------------------------------------------------------------------
double vtkMRMLVolumePropertyNode::NextHigher(double value)
{
  if (value == 0.)
    {
    // special case to avoid denormalized numbers
    return std::numeric_limits<double>::min();
    }
  // Increment the value by the smallest offset possible
  // The challenge here is to find the offset, if the value is 100000000., an
  // offset of epsilon won't work.
  typedef union {
      long long i64;
      double d64;
    } dbl_64;
  dbl_64 d;
  d.d64 = value;
  d.i64 += (value < 0.) ? -1 : 1;
  return d.d64;
}

//----------------------------------------------------------------------------
double vtkMRMLVolumePropertyNode::HigherAndUnique(double value, double &previousValue)
{
  value = std::max(value, previousValue);
  if (value == previousValue)
    {
    value = vtkMRMLVolumePropertyNode::NextHigher(value);
    }
  assert (value != previousValue);
  previousValue = value;
  return value;
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLVolumePropertyNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLVolumePropertyStorageNode"));
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetScalarOpacity(vtkPiecewiseFunction* newScalarOpacity, int component)
{
  vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetScalarOpacity(component));
  this->VolumeProperty->SetScalarOpacity(component, newScalarOpacity);
  vtkObserveMRMLObjectEventsMacro(this->VolumeProperty->GetScalarOpacity(component), this->ObservedEvents);
}

//---------------------------------------------------------------------------
vtkPiecewiseFunction* vtkMRMLVolumePropertyNode::GetScalarOpacity(int component)
{
  return this->VolumeProperty->GetScalarOpacity(component);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetGradientOpacity(
  vtkPiecewiseFunction* newGradientOpacity, int component)
{
  vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetGradientOpacity(component));
  this->VolumeProperty->SetGradientOpacity(component, newGradientOpacity);
  vtkObserveMRMLObjectEventsMacro(this->VolumeProperty->GetGradientOpacity(component), this->ObservedEvents);
}

//---------------------------------------------------------------------------
vtkPiecewiseFunction* vtkMRMLVolumePropertyNode::GetGradientOpacity(int component)
{
  return this->VolumeProperty->GetGradientOpacity(component);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetColor(vtkColorTransferFunction* newColorFunction, int component)
{
  vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetRGBTransferFunction(component));
  this->VolumeProperty->SetColor(component, newColorFunction);
  vtkObserveMRMLObjectEventsMacro(this->VolumeProperty->GetRGBTransferFunction(component), this->ObservedEvents);
}

//---------------------------------------------------------------------------
vtkColorTransferFunction* vtkMRMLVolumePropertyNode::GetColor(int component)
{
  return this->VolumeProperty->GetRGBTransferFunction(component);
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumePropertyNode::GetModifiedSinceRead()
{
  return this->Superclass::GetModifiedSinceRead() ||
    (this->VolumeProperty &&
     this->VolumeProperty->GetMTime() > this->GetStoredTime());
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumePropertyNode::CalculateEffectiveRange()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("CalculateEffectiveRange: Invalid volume property");
    return false;
    }

  vtkColorTransferFunction* colorTransferFunction = this->VolumeProperty->GetRGBTransferFunction();
  vtkPiecewiseFunction* opacityFunction = this->VolumeProperty->GetScalarOpacity();
  vtkPiecewiseFunction* gradientFunction = this->VolumeProperty->GetGradientOpacity();
  if (!colorTransferFunction || !opacityFunction || !gradientFunction)
    {
    vtkErrorMacro("CalculateEffectiveRange: Invalid transfer functions in volume property");
    return false;
    }

  double effectiveRange[2] = {0.0};

  double colorRange[2] = {0.0};
  colorTransferFunction->GetRange(colorRange);
  effectiveRange[0] = std::min(effectiveRange[0], colorRange[0]);
  effectiveRange[1] = std::max(effectiveRange[1], colorRange[1]);

  double opacityRange[2] = {0.0};
  opacityFunction->GetRange(opacityRange);
  effectiveRange[0] = std::min(effectiveRange[0], opacityRange[0]);
  effectiveRange[1] = std::max(effectiveRange[1], opacityRange[1]);

  double gradientRange[2] = {0.0};
  gradientFunction->GetRange(gradientRange);
  effectiveRange[0] = std::min(effectiveRange[0], gradientRange[0]);
  effectiveRange[1] = std::max(effectiveRange[1], gradientRange[1]);

  this->SetEffectiveRange(effectiveRange);
  return true;
}

//---------------------------------------------------------------------------
int vtkMRMLVolumePropertyNode::GetInterpolationType()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetInterpolationType: Invalid volume property");
    return 0;
    }
  return this->VolumeProperty->GetInterpolationType();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetInterpolationType(int interpolationType)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetInterpolationType: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetInterpolationType(interpolationType);
  this->Modified();
}

//---------------------------------------------------------------------------
int vtkMRMLVolumePropertyNode::GetShade()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetShade: Invalid volume property");
    return 0;
    }
  return this->VolumeProperty->GetShade();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetShade(int shade)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetShade: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetShade(shade);
  this->Modified();
}

//---------------------------------------------------------------------------
double vtkMRMLVolumePropertyNode::GetDiffuse()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetDiffuse: Invalid volume property");
    return 0.0;
    }
  return this->VolumeProperty->GetDiffuse();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetDiffuse(double diffuse)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetDiffuse: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetDiffuse(diffuse);
  this->Modified();
}

//---------------------------------------------------------------------------
double vtkMRMLVolumePropertyNode::GetAmbient()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetAmbient: Invalid volume property");
    return 0.0;
    }
  return this->VolumeProperty->GetAmbient();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetAmbient(double ambient)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetAmbient: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetAmbient(ambient);
  this->Modified();
}

//---------------------------------------------------------------------------
double vtkMRMLVolumePropertyNode::GetSpecular()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetSpecular: Invalid volume property");
    return 0.0;
    }
  return this->VolumeProperty->GetSpecular();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetSpecular(double specular)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetSpecular: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetSpecular(specular);
  this->Modified();
}

//---------------------------------------------------------------------------
double vtkMRMLVolumePropertyNode::GetSpecularPower()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetSpecularPower: Invalid volume property");
    return 0.0;
    }
  return this->VolumeProperty->GetSpecularPower();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetSpecularPower(double specularPower)
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("SetSpecularPower: Invalid volume property");
    return;
    }
  this->VolumeProperty->SetSpecularPower(specularPower);
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkMRMLVolumePropertyNode::GetScalarOpacityAsString()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetScalarOpacityAsString: Invalid volume property");
    return "";
    }
  return this->GetPiecewiseFunctionString(this->VolumeProperty->GetScalarOpacity());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetScalarOpacityAsString(std::string scalarOpacityFunctionStr)
{
  vtkPiecewiseFunction* scalarOpacity = vtkPiecewiseFunction::New();
  this->GetPiecewiseFunctionFromString(scalarOpacityFunctionStr.c_str(), scalarOpacity);
  this->SetScalarOpacity(scalarOpacity);
  scalarOpacity->Delete();
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkMRMLVolumePropertyNode::GetGradientOpacityAsString()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetGradientOpacityAsString: Invalid volume property");
    return "";
    }
  return this->GetPiecewiseFunctionString(this->VolumeProperty->GetGradientOpacity());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetGradientOpacityAsString(std::string gradientOpacityFunctionStr)
{
  vtkPiecewiseFunction* gradientOpacity = vtkPiecewiseFunction::New();
  this->GetPiecewiseFunctionFromString(gradientOpacityFunctionStr.c_str(), gradientOpacity);
  this->SetGradientOpacity(gradientOpacity);
  gradientOpacity->Delete();
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkMRMLVolumePropertyNode::GetRGBTransferFunctionAsString()
{
  if (!this->VolumeProperty)
    {
    vtkErrorMacro("GetRGBTransferFunctionAsString: Invalid volume property");
    return "";
    }
  return this->GetColorTransferFunctionString(this->VolumeProperty->GetRGBTransferFunction());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::SetRGBTransferFunctionAsString(std::string rgbTransferFunctionStr)
{
  vtkColorTransferFunction* colorTransfer = vtkColorTransferFunction::New();
  this->GetColorTransferFunctionFromString(rgbTransferFunctionStr.c_str(), colorTransfer);
  this->SetColor(colorTransfer);
  colorTransfer->Delete();
  this->Modified();
}
