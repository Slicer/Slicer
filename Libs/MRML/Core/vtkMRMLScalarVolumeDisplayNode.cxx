/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLVolumeNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageHistogramStatistics.h>
#include <vtkImageLogic.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageStencil.h>
#include <vtkImageThreshold.h>
#include <vtkObjectFactory.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkVersion.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLScalarVolumeDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeDisplayNode::vtkMRMLScalarVolumeDisplayNode()
{
  // Strings
  this->Interpolate = 1;
  this->AutoWindowLevel = 1;
  this->AutoThreshold = 0;
  this->ApplyThreshold = 0;
  this->InvertDisplayScalarRange = 0;

  // try setting a default grayscale color map
  // this->SetDefaultColorMap(0);

  // create and set visualization pipeline
  this->AlphaLogic = vtkImageLogic::New();
  this->MapToColors = vtkImageMapToColors::New();
  this->Threshold = vtkImageThreshold::New();
  this->AppendComponents = vtkImageAppendComponents::New();

  this->ExtractRGB = vtkImageExtractComponents::New();
  this->ExtractAlpha = vtkImageExtractComponents::New();
  this->MultiplyAlpha = vtkImageStencil::New();

  this->MapToWindowLevelColors = vtkImageMapToWindowLevelColors::New();
  this->MapToWindowLevelColors->SetOutputFormatToLuminance();
  this->MapToWindowLevelColors->SetWindow(256.);
  this->MapToWindowLevelColors->SetLevel(128.);

  this->MapToColors->SetOutputFormatToRGBA();
  // This input may be changed later if ScalarRangeFlag is modified.
  this->MapToColors->SetInputConnection(this->MapToWindowLevelColors->GetOutputPort());

  this->ExtractRGB->SetInputConnection(this->MapToColors->GetOutputPort());
  this->ExtractRGB->SetComponents(0, 1, 2);

  this->ExtractAlpha->SetInputConnection(this->MapToColors->GetOutputPort());
  this->ExtractAlpha->SetComponents(3);

  this->Threshold->ReplaceInOn();
  this->Threshold->SetInValue(255);
  this->Threshold->ReplaceOutOn();
  this->Threshold->SetOutValue(255);
  this->Threshold->SetOutputScalarTypeToUnsignedChar();
  this->Threshold->ThresholdBetween(VTK_SHORT_MIN, VTK_SHORT_MAX);

  this->MultiplyAlpha->SetInputConnection(0, this->ExtractAlpha->GetOutputPort());
  this->MultiplyAlpha->SetBackgroundValue(0);

  this->AlphaLogic->SetOperationToAnd();
  this->AlphaLogic->SetOutputTrueValue(255);

  this->AlphaLogic->SetInputConnection(0, this->Threshold->GetOutputPort());
  // this->AlphaLogic->SetInputConnection(1, this->Threshold->GetOutputPort() );
  this->AlphaLogic->SetInputConnection(1, this->MultiplyAlpha->GetOutputPort());

  this->AppendComponents->RemoveAllInputs();
  this->AppendComponents->AddInputConnection(0, this->ExtractRGB->GetOutputPort());
  this->AppendComponents->AddInputConnection(0, this->AlphaLogic->GetOutputPort());

  this->HistogramStatistics = nullptr;
  this->IsInCalculateAutoLevels = false;

  this->MRMLObserverManager->ObserveObject(this, 10000.);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeDisplayNode::~vtkMRMLScalarVolumeDisplayNode()
{
  this->SetAndObserveColorNodeID(nullptr);

  this->AlphaLogic->Delete();
  this->MapToColors->Delete();
  this->Threshold->Delete();
  this->AppendComponents->Delete();
  this->MapToWindowLevelColors->Delete();
  this->ExtractRGB->Delete();
  this->ExtractAlpha->Delete();
  this->MultiplyAlpha->Delete();

  if (this->HistogramStatistics)
  {
    this->HistogramStatistics->Delete();
    this->HistogramStatistics = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetDefaultColorMap()
{
  this->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetInputImageDataConnection(vtkAlgorithmOutput* imageDataConnection)
{
  if (this->GetInputImageDataConnection() == imageDataConnection)
  {
    return;
  }
  vtkAlgorithm* oldInputImageDataAlgorithm = this->GetInputImageDataConnection() ? this->GetInputImageDataConnection()->GetProducer() : nullptr;

  if (oldInputImageDataAlgorithm != nullptr)
  {
    vtkUnObserveMRMLObjectMacro(oldInputImageDataAlgorithm);
  }

  this->Superclass::SetInputImageDataConnection(imageDataConnection);

  vtkAlgorithm* inputImageDataAlgorithm = this->GetInputImageDataConnection() ? this->GetInputImageDataConnection()->GetProducer() : nullptr;
  if (inputImageDataAlgorithm != nullptr)
  {
    vtkObserveMRMLObjectMacro(inputImageDataAlgorithm);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetInputToImageDataPipeline(vtkAlgorithmOutput* imageDataConnection)
{
  this->Threshold->SetInputConnection(imageDataConnection);

  if (this->GetScalarRangeFlag() == vtkMRMLDisplayNode::UseDirectMapping)
  {
    // Bypass window/level to show table scalar range directly
    this->MapToColors->SetInputConnection(imageDataConnection);
  }
  else
  {
    // Reconnection is expensive operation, therefore only do it if needed
    if (this->MapToColors->GetInputConnection(0, 0) != this->MapToWindowLevelColors->GetOutputPort())
    {
      this->MapToColors->SetInputConnection(this->MapToWindowLevelColors->GetOutputPort());
    }
  }
  this->MapToWindowLevelColors->SetInputConnection(imageDataConnection);
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetBackgroundImageStencilDataConnection(vtkAlgorithmOutput* imageDataConnection)
{
  this->MultiplyAlpha->SetStencilConnection(imageDataConnection);
}
//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLScalarVolumeDisplayNode::GetBackgroundImageStencilDataConnection()
{
  // Input ports:
  // 0 = foreground image, 1 = background image, 2 = stencil
  const int stencilInputPort = 2;
  return this->MultiplyAlpha->GetNumberOfInputConnections(0) > stencilInputPort ? this->MultiplyAlpha->GetInputConnection(0, stencilInputPort) : nullptr;
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLScalarVolumeDisplayNode::GetInputImageDataConnection()
{
  return this->MapToWindowLevelColors->GetNumberOfInputConnections(0) ? this->MapToWindowLevelColors->GetInputConnection(0, 0) : nullptr;
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLScalarVolumeDisplayNode::GetOutputImageDataConnection()
{
  return this->AppendComponents->GetOutputPort();
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  {
    std::stringstream ss;
    ss << this->GetWindow();
    of << " window=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->GetLevel();
    of << " level=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->GetUpperThreshold();
    of << " upperThreshold=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->GetLowerThreshold();
    of << " lowerThreshold=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->Interpolate;
    of << " interpolate=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->InvertDisplayScalarRange;
    of << " invertDisplayScalarRange=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->AutoWindowLevel;
    of << " autoWindowLevel=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->ApplyThreshold;
    of << " applyThreshold=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->AutoThreshold;
    of << " autoThreshold=\"" << ss.str() << "\"";
  }
  if (this->WindowLevelPresets.size() > 0)
  {
    for (int p = 0; p < this->GetNumberOfWindowLevelPresets(); p++)
    {
      of << " windowLevelPreset" << p << "=\""
         << vtkMRMLScalarVolumeDisplayNode::GetWindowLevelAsPresetString(this->WindowLevelPresets[p].Window, this->WindowLevelPresets[p].Level) << "\"";
    }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  std::vector<WindowLevelPreset> windowLevelPresets;

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
  {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "window"))
    {
      std::stringstream ss;
      ss << attValue;
      double window;
      ss >> window;
      this->SetWindow(window);
    }
    else if (!strcmp(attName, "level"))
    {
      std::stringstream ss;
      ss << attValue;
      double level;
      ss >> level;
      this->SetLevel(level);
    }
    else if (!strcmp(attName, "upperThreshold"))
    {
      std::stringstream ss;
      ss << attValue;
      double threshold;
      ss >> threshold;
      this->SetUpperThreshold(threshold);
    }
    else if (!strcmp(attName, "lowerThreshold"))
    {
      std::stringstream ss;
      ss << attValue;
      double threshold;
      ss >> threshold;
      this->SetLowerThreshold(threshold);
    }
    else if (!strcmp(attName, "interpolate"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Interpolate;
    }
    else if (!strcmp(attName, "invertDisplayScalarRange"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->InvertDisplayScalarRange;
    }
    else if (!strcmp(attName, "autoWindowLevel"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->AutoWindowLevel;
    }
    else if (!strcmp(attName, "applyThreshold"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ApplyThreshold;
    }
    else if (!strcmp(attName, "autoThreshold"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->AutoThreshold;
    }
    else if (!strncmp(attName, "windowLevelPreset", 17))
    {
      double window = 0.0;
      double level = 0.0;
      if (!vtkMRMLScalarVolumeDisplayNode::GetWindowLevelFromPresetString(attValue, window, level))
      {
        continue;
      }
      windowLevelPresets.push_back(WindowLevelPreset(window, level));
    }
  }

  this->SetWindowLevelPresets(windowLevelPresets);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);

  // CopyContent updates the color table, so we need to set the display range first
  vtkMRMLScalarVolumeDisplayNode* node = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(anode);
  if (node)
  {
    this->SetAutoWindowLevel(node->GetAutoWindowLevel());
    this->SetWindowLevel(node->GetWindow(), node->GetLevel());
    this->SetAutoThreshold(node->GetAutoThreshold()); // don't want to run CalculateAutoLevel
    this->SetApplyThreshold(node->GetApplyThreshold());
    this->SetThreshold(node->GetLowerThreshold(), node->GetUpperThreshold());
    this->SetInterpolate(node->Interpolate);
    this->SetInvertDisplayScalarRange(node->GetInvertDisplayScalarRange());
    this->SetWindowLevelPresets(node->WindowLevelPresets);
  }

  Superclass::CopyContent(anode, deepCopy);
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os, indent);

  os << indent << "AutoWindowLevel:   " << this->AutoWindowLevel << "\n";
  os << indent << "Window:            " << this->GetWindow() << "\n";
  os << indent << "Level:             " << this->GetLevel() << "\n";
  os << indent << "Window Level Presets:\n";
  for (int p = 0; p < this->GetNumberOfWindowLevelPresets(); p++)
  {
    os << indent.GetNextIndent() << p << " Window: " << this->GetWindowPreset(p) << " | Level: " << this->GetLevelPreset(p) << "\n";
  }
  os << indent << "AutoThreshold:     " << this->AutoThreshold << "\n";
  os << indent << "ApplyThreshold:    " << this->GetApplyThreshold() << "\n";
  os << indent << "UpperThreshold:    " << this->GetUpperThreshold() << "\n";
  os << indent << "LowerThreshold:    " << this->GetLowerThreshold() << "\n";
  os << indent << "Interpolate:       " << this->Interpolate << "\n";
  os << indent << "InvertDisplayScalarRange: " << this->InvertDisplayScalarRange << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLColorNode* cnode = vtkMRMLColorNode::SafeDownCast(caller);
  if (cnode && event == vtkCommand::ModifiedEvent)
  {
    this->UpdateLookupTable(cnode);
  }
  if (vtkAlgorithmOutput::SafeDownCast(caller) == this->GetScalarImageDataConnection() && //
      this->GetScalarImageDataConnection() &&                                             //
      event == vtkCommand::ModifiedEvent)
  {
    this->CalculateAutoLevels();
  }
  if (caller == this && event == vtkCommand::ModifiedEvent && //
      !this->IsInCalculateAutoLevels)
  {
    int wasModifying = this->GetDisableModifiedEvent();
    this->SetDisableModifiedEvent(1);
    this->CalculateAutoLevels();
    // TODO: Reset the pending event counter.
    this->SetDisableModifiedEvent(wasModifying);
  }
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
double vtkMRMLScalarVolumeDisplayNode::GetWindow()
{
  return this->MapToWindowLevelColors->GetWindow();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetWindow(double window)
{
  if (this->MapToWindowLevelColors->GetWindow() == window)
  {
    return;
  }

  this->MapToWindowLevelColors->SetWindow(window);
  this->Modified();
}

//---------------------------------------------------------------------------
double vtkMRMLScalarVolumeDisplayNode::GetLevel()
{
  return this->MapToWindowLevelColors->GetLevel();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetLevel(double level)
{
  if (this->MapToWindowLevelColors->GetLevel() == level)
  {
    return;
  }

  this->MapToWindowLevelColors->SetLevel(level);
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetWindowLevel(double window, double level)
{
  if (this->MapToWindowLevelColors->GetWindow() == window && //
      this->MapToWindowLevelColors->GetLevel() == level)
  {
    return;
  }

  this->MapToWindowLevelColors->SetWindow(window);
  this->MapToWindowLevelColors->SetLevel(level);
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetWindowLevelMinMax(double min, double max)
{
  double window = max - min;
  double level = 0.5 * (min + max);
  this->SetWindowLevel(window, level);
}

//---------------------------------------------------------------------------
double vtkMRMLScalarVolumeDisplayNode::GetWindowLevelMin()
{
  const double window = this->GetWindow();
  const double level = this->GetLevel();
  return level - 0.5 * window;
}

//---------------------------------------------------------------------------
double vtkMRMLScalarVolumeDisplayNode::GetWindowLevelMax()
{
  const double window = this->GetWindow();
  const double level = this->GetLevel();
  return level + 0.5 * window;
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetApplyThreshold(int apply)
{
  if (this->ApplyThreshold == apply)
  {
    return;
  }
  this->ApplyThreshold = apply;
  this->Threshold->SetOutValue(apply ? 0 : 255);
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetLowerThreshold(double threshold)
{
  this->SetThreshold(threshold, this->GetUpperThreshold());
}

//---------------------------------------------------------------------------
double vtkMRMLScalarVolumeDisplayNode::GetLowerThreshold()
{
  return this->Threshold->GetLowerThreshold();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetUpperThreshold(double threshold)
{
  this->SetThreshold(this->GetLowerThreshold(), threshold);
}

//---------------------------------------------------------------------------
double vtkMRMLScalarVolumeDisplayNode::GetUpperThreshold()
{
  return this->Threshold->GetUpperThreshold();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetThreshold(double lowerThreshold, double upperThreshold)
{
  if (this->GetLowerThreshold() == lowerThreshold && //
      this->GetUpperThreshold() == upperThreshold)
  {
    return;
  }
  this->Threshold->ThresholdBetween(lowerThreshold, upperThreshold);
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetColorNodeInternal(vtkMRMLColorNode* newColorNode)
{
  this->Superclass::SetColorNodeInternal(newColorNode);
  this->UpdateLookupTable(newColorNode);
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::UpdateLookupTable(vtkMRMLColorNode* newColorNode)
{
  vtkSmartPointer<vtkScalarsToColors> lookupTable = nullptr;
  if (newColorNode)
  {
    lookupTable = newColorNode->GetLookupTable();
    if (lookupTable == nullptr)
    {
      if (vtkMRMLProceduralColorNode::SafeDownCast(newColorNode) != nullptr)
      {
        vtkDebugMacro("UpdateImageDataPipeline: getting color transfer function");
        lookupTable = vtkMRMLProceduralColorNode::SafeDownCast(newColorNode)->GetColorTransferFunction();
      }
    }
  }

  if (lookupTable && this->GetScalarRangeFlag() != vtkMRMLDisplayNode::UseDirectMapping //
      && (lookupTable->GetRange()[0] != 0.0 || lookupTable->GetRange()[1] != 255.0 || this->GetInvertDisplayScalarRange()))
  {
    // Convert table range to 0, 255 to match the output from MapToWindowLevelColors
    vtkSmartPointer<vtkScalarsToColors> newLookupTable = vtkSmartPointer<vtkScalarsToColors>::Take(lookupTable->NewInstance());
    newLookupTable->DeepCopy(lookupTable);
    newLookupTable->SetRange(0, 255);

    // If invert is enabled, invert the lookup table
    if (this->GetInvertDisplayScalarRange())
    {
      if (vtkColorTransferFunction* ctf = vtkColorTransferFunction::SafeDownCast(newLookupTable))
      {
        // For continuous transfer functions, invert the positions
        vtkNew<vtkColorTransferFunction> invertedCtf;
        double range[2] = { 0, 255 };
        ctf->GetRange(range);
        for (int i = 0; i < ctf->GetSize(); i++)
        {
          double val[6];
          ctf->GetNodeValue(i, val);
          // Invert the position while keeping the color
          val[0] = range[0] + (range[1] - val[0]);
          invertedCtf->AddRGBPoint(val[0], val[1], val[2], val[3], val[4], val[5]);
        }
        newLookupTable = invertedCtf;
      }
      else if (vtkLookupTable* lut = vtkLookupTable::SafeDownCast(newLookupTable))
      {
        // For discrete lookup tables, invert the table entries
        vtkNew<vtkLookupTable> invertedLut;
        invertedLut->SetNumberOfTableValues(lut->GetNumberOfTableValues());
        invertedLut->SetRange(0, 255);
        for (int i = 0; i < lut->GetNumberOfTableValues(); i++)
        {
          double rgba[4];
          lut->GetTableValue(i, rgba);
          invertedLut->SetTableValue(lut->GetNumberOfTableValues() - 1 - i, rgba);
        }
        newLookupTable = invertedLut;
      }
    }
    lookupTable = newLookupTable;
  }

  this->MapToColors->SetLookupTable(lookupTable);
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetWindowLevelPresets(const std::vector<WindowLevelPreset>& srcWindowLevelPresets)
{
  // Compare with a tolerance
  static const double WINDOW_LEVEL_PRESET_TOLERANCE = 1.e-6;
  if (srcWindowLevelPresets.size() == this->GetNumberOfWindowLevelPresets())
  {
    bool presetsAreEqual = true;
    for (int presetIndex = 0; presetIndex < this->WindowLevelPresets.size(); presetIndex++)
    {
      WindowLevelPreset& preset = this->WindowLevelPresets[presetIndex];
      const WindowLevelPreset& srcPreset = srcWindowLevelPresets[presetIndex];
      if (fabs(preset.Window - srcPreset.Window) > WINDOW_LEVEL_PRESET_TOLERANCE || //
          fabs(preset.Level - srcPreset.Level) > WINDOW_LEVEL_PRESET_TOLERANCE)
      {
        presetsAreEqual = false;
        break;
      }
    }
    if (presetsAreEqual)
    {
      // no change
      return;
    }
  }

  // Presets are different, overwrite
  this->WindowLevelPresets = srcWindowLevelPresets;
  this->Modified();
}

//---------------------------------------------------------------------------
bool vtkMRMLScalarVolumeDisplayNode::GetWindowLevelFromPresetString(const char* preset, double& window, double& level)
{
  if (preset == nullptr)
  {
    vtkGenericWarningMacro("GetWindowLevelFromPresetString: null input string");
    return false;
  }
  // parse the string, the string is double|double
  std::vector<std::string> parts = vtksys::SystemTools::SplitString(preset, '|');
  if (parts.size() != 2)
  {
    return false;
  }
  window = atof(parts[0].c_str());
  level = atof(parts[1].c_str());
  return true;
}

//---------------------------------------------------------------------------
std::string vtkMRMLScalarVolumeDisplayNode::GetWindowLevelAsPresetString(double window, double level)
{
  std::stringstream ss;
  ss << window << "|" << level;
  return ss.str();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::AddWindowLevelPresetFromString(const char* preset)
{
  double window = 0.0;
  double level = 0.0;
  if (vtkMRMLScalarVolumeDisplayNode::GetWindowLevelFromPresetString(preset, window, level))
  {
    this->AddWindowLevelPreset(window, level);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::AddWindowLevelPreset(double window, double level)
{
  vtkMRMLScalarVolumeDisplayNode::WindowLevelPreset preset(window, level);
  this->WindowLevelPresets.push_back(preset);
  this->Modified();
}

//---------------------------------------------------------------------------
int vtkMRMLScalarVolumeDisplayNode::GetNumberOfWindowLevelPresets()
{
  return static_cast<int>(this->WindowLevelPresets.size());
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetWindowLevelFromPreset(int p)
{
  if (p < 0 || p >= this->GetNumberOfWindowLevelPresets())
  {
    vtkErrorMacro("SetWindowLevelFromPreset: index " << p << " out of range 0 to " << this->GetNumberOfWindowLevelPresets() - 1);
    return;
  }
  // TODO: change this flag to a regular int showing which preset is being used
  this->SetAutoWindowLevel(0);
  this->SetWindowLevel(this->GetWindowPreset(p), this->GetLevelPreset(p));
}

//---------------------------------------------------------------------------
double vtkMRMLScalarVolumeDisplayNode::GetWindowPreset(int p)
{
  if (p < 0 || p >= this->GetNumberOfWindowLevelPresets())
  {
    vtkErrorMacro("GetWindowPreset: index " << p << " out of range 0 to " << this->GetNumberOfWindowLevelPresets() - 1);
    return 0.0;
  }

  return this->WindowLevelPresets[p].Window;
}

//---------------------------------------------------------------------------
double vtkMRMLScalarVolumeDisplayNode::GetLevelPreset(int p)
{
  if (p < 0 || p >= this->GetNumberOfWindowLevelPresets())
  {
    vtkErrorMacro("GetLevelPreset: index " << p << " out of range 0 to " << this->GetNumberOfWindowLevelPresets() - 1);
    return 0.0;
  }

  return this->WindowLevelPresets[p].Level;
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::ResetWindowLevelPresets()
{
  this->WindowLevelPresets.clear();
  this->Modified();
}

//---------------------------------------------------------------------------
vtkImageData* vtkMRMLScalarVolumeDisplayNode::GetScalarImageData()
{
  vtkAlgorithm* producer = this->GetScalarImageDataConnection() ? this->GetScalarImageDataConnection()->GetProducer() : nullptr;
  return vtkImageData::SafeDownCast(producer ? producer->GetOutputDataObject(0) : nullptr);
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLScalarVolumeDisplayNode::GetScalarImageDataConnection()
{
  return this->GetInputImageDataConnection();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::GetDisplayScalarRange(double range[2])
{
  range[0] = 0;
  range[1] = 255.;

  vtkImageData* imageData = this->GetScalarImageData();
  if (!imageData || !this->GetInputImageData())
  {
    // it's a problem if the volume node has an image data but the display node
    // doesn't. It's ok if the display node is not yet in the scene: being
    // loaded (vtkMRMLScene::LoadIntoScene)or stored
    // (vtkMRMLSceneViewNode::StoreScene).
    assert(!this->GetVolumeNode() || !this->GetVolumeNode()->GetImageData() || //
           !this->GetScene() || this->GetScene()->GetNodeByID(this->GetID()) != this);
    vtkDebugMacro(<< "No valid image data, returning default values [0, 255]");
    return;
  }
  this->GetScalarImageDataConnection()->GetProducer()->Update();
  imageData->GetScalarRange(range);
  if (imageData->GetNumberOfScalarComponents() >= 3 && //
      fabs(range[0]) < 0.000001 && fabs(range[1]) < 0.000001)
  {
    range[0] = 0;
    range[1] = 255;
  }
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::CalculateAutoLevels()
{
  if (!this->GetAutoWindowLevel() && !this->GetAutoThreshold())
  {
    vtkDebugMacro("CalculateScalarAutoLevels: " << (this->GetID() == nullptr ? "nullid" : this->GetID()) << ": Auto window level not turned on, returning.");
    return;
  }

  vtkImageData* imageDataScalar = this->GetScalarImageData();
  if (!imageDataScalar)
  {
    vtkDebugMacro("CalculateScalarAutoLevels: input image data is null");
    return;
  }
  // Make sure the point data is up to date.
  // Remember, the display node pipeline is not connected to a consumer (volume
  // display nodes are cloned by the slice logic) therefore no-one has run the
  // filters.
  if (this->GetInputImageData())
  {
    this->GetScalarImageDataConnection()->GetProducer()->Update();
  }

  if (!(imageDataScalar->GetPointData()) || //
      !(imageDataScalar->GetPointData()->GetScalars()))
  {
    vtkDebugMacro("CalculateScalarAutoLevels: input image data is null");
    return;
  }

  if (this->HistogramStatistics == nullptr)
  {
    this->HistogramStatistics = vtkImageHistogramStatistics::New();

    // Set automatic window/level to include the entire intensity range
    // (except top/bottom 0.1%, to not let a very thin tail of the intensity
    // distribution to decrease the image contrast too much).
    // While in CT and sometimes in MRI, there may be a large empty area
    // outside the reconstructed image, which could be suppressed
    // by a larger lower percentile value, it would make the method
    // too specific to particular imaging modalities and could lead to
    // suboptimal results for other types of images.
    // Therefore, we choose small, symmetric percentile values here
    // and maybe add modality-specific methods later (e.g., for CT
    // images we could set lower value to -1000HU).
    this->HistogramStatistics->SetAutoRangePercentiles(0.1, 99.9);

    // Percentiles are very low (0.1%), so there is no need for
    // range expansion.
    this->HistogramStatistics->SetAutoRangeExpansionFactors(0.0, 0.0);
  }

  this->IsInCalculateAutoLevels = true;
  this->HistogramStatistics->SetInputData(imageDataScalar);
  this->HistogramStatistics->Update();
  double* intensityRange = this->HistogramStatistics->GetAutoRange();
  vtkDebugMacro("CalculateScalarAutoLevels:" << " lower: " << intensityRange[0] << " upper: " << intensityRange[1]);

  int disabledModify = this->StartModify();
  if (this->GetAutoWindowLevel())
  {
    this->SetWindowLevelMinMax(intensityRange[0], intensityRange[1]);
  }
  if (this->GetAutoThreshold())
  {
    this->SetThreshold(intensityRange[0], intensityRange[1]);
  }
  this->EndModify(disabledModify);
  this->IsInCalculateAutoLevels = false;
}

//---------------------------------------------------------------------------
vtkScalarsToColors* vtkMRMLScalarVolumeDisplayNode::GetLookupTable()
{
  return this->MapToColors->GetLookupTable();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarVolumeDisplayNode::SetInvertDisplayScalarRange(int invert)
{
  if (this->InvertDisplayScalarRange == invert)
  {
    // no change
    return;
  }
  this->InvertDisplayScalarRange = invert;
  this->UpdateLookupTable(this->GetColorNode());
  this->Modified();
}
