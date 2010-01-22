/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLScalarVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLScalarVolumeDisplayNode - MRML node for representing a volume display attributes
/// 
/// vtkMRMLScalarVolumeDisplayNode nodes describe how volume is displayed.

#ifndef __vtkMRMLScalarVolumeDisplayNode_h
#define __vtkMRMLScalarVolumeDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLVolumeDisplayNode.h"

#include "vtkImageData.h"
#include "vtkImageCast.h"
#include "vtkImageLogic.h"
#include "vtkImageMapToColors.h"
#include "vtkImageThreshold.h"
#include "vtkImageAppendComponents.h"
#include "vtkImageMapToWindowLevelColors.h"



class VTK_MRML_EXPORT vtkMRMLScalarVolumeDisplayNode : public vtkMRMLVolumeDisplayNode
{
  public:
  static vtkMRMLScalarVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLScalarVolumeDisplayNode,vtkMRMLVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "VolumeDisplay";};


  //--------------------------------------------------------------------------
  /// Display Information
  //--------------------------------------------------------------------------
  
  /// 
  /// Specifies whether windowing and leveling are to be performed automatically
  //vtkBooleanMacro(AutoWindowLevel, int);
  vtkGetMacro(AutoWindowLevel, int);
  //vtkSetMacro(AutoWindowLevel, int);
  void SetAutoWindowLevel(int);
  void AutoWindowLevelOn() { this->SetAutoWindowLevel(1); };
  void AutoWindowLevelOff() { this->SetAutoWindowLevel(0); };

  
  /// 
  /// The window value to use when autoWindowLevel is 'no'
  vtkGetMacro(Window, double);
  vtkSetMacro(Window, double);

  /// 
  /// The level value to use when autoWindowLevel is 'no'
  vtkGetMacro(Level, double);
  vtkSetMacro(Level, double);

  /// 
  /// Specifies whether to apply the threshold
  vtkBooleanMacro(ApplyThreshold, int);
  vtkGetMacro(ApplyThreshold, int);
  vtkSetMacro(ApplyThreshold, int);

  /// 
  /// Specifies whether the threshold should be set automatically
  vtkBooleanMacro(AutoThreshold, int);
  vtkGetMacro(AutoThreshold, int);
  vtkSetMacro(AutoThreshold, int);

  /// 
  /// The upper threshold value to use when autoThreshold is 'no'
  vtkGetMacro(UpperThreshold, double);
  vtkSetMacro(UpperThreshold, double);

  /// 
  /// The lower threshold value to use when autoThreshold is 'no'
  vtkGetMacro(LowerThreshold, double);
  vtkSetMacro(LowerThreshold, double);

  /// 
  /// Set/Get interpolate reformated slices
  vtkGetMacro(Interpolate, int);
  vtkSetMacro(Interpolate, int);
  vtkBooleanMacro(Interpolate, int);

  virtual void SetDefaultColorMap();

  /// 
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
  /// 
  /// Sets vtkImageData to be converted to displayable vtkImageData
  virtual void SetImageData(vtkImageData *imageData)
    {
    this->Threshold->SetInput( imageData);
    this->MapToWindowLevelColors->SetInput( imageData);
    };

  /// 
  /// Sets ImageData for background mask 
  virtual void SetBackgroundImageData(vtkImageData *imageData)
    {
    this->ResliceAlphaCast->SetInput(imageData);
    };

  /// 
  /// Gets ImageData converted from the real data in the node
  virtual vtkImageData* GetImageData() 
    {
    this->UpdateImageDataPipeline();
    if (this->Threshold->GetInput() == NULL)
      {
      return NULL;
      }
    this->AppendComponents->Update();
    return this->AppendComponents->GetOutput();
    };

  virtual void UpdateImageDataPipeline();

  /// 
  /// Parse a string with window and level as double|double, and add a preset 
  void AddWindowLevelPresetFromString(const char *preset);
  /// 
  /// Add a window level preset
  void AddWindowLevelPreset(double window, double level);

  /// 
  /// Remove all presets
  void ResetWindowLevelPresets();

  /// 
  /// Set Window and Level from preset p
  void SetWindowLevelFromPreset(int p);

  /// 
  /// Get the number of window/level presets
  int GetNumberOfWindowLevelPresets();

  /// 
  /// Return a specific preset, returns 0 if p out of range
  double GetWindowPreset(int p);
  double GetLevelPreset(int p);
  
protected:
  vtkMRMLScalarVolumeDisplayNode();
  ~vtkMRMLScalarVolumeDisplayNode();
  vtkMRMLScalarVolumeDisplayNode(const vtkMRMLScalarVolumeDisplayNode&);
  void operator=(const vtkMRMLScalarVolumeDisplayNode&);

  /// 
  /// To hold preset values for window and level, so can restore this display
  /// node's window and level to ones read from DICOM files, or defined by
  /// users
  //BTX
  class WindowLevelPreset
  {
  public:
    double Window;
    double Level;
    WindowLevelPreset() { this->Window = 0.0; this->Level = 0.0; };
  };
  //ETX
  double Window;
  double Level;
  double UpperThreshold;
  double LowerThreshold;


  /// Booleans
  int Interpolate;
  int AutoWindowLevel;
  int ApplyThreshold;
  int AutoThreshold;

  vtkImageCast *ResliceAlphaCast;
  vtkImageLogic *AlphaLogic;
  vtkImageMapToColors *MapToColors;
  vtkImageThreshold *Threshold;
  vtkImageAppendComponents *AppendComponents;
  vtkImageMapToWindowLevelColors *MapToWindowLevelColors;

  /// 
  /// window level presets
  //BTX
  std::vector<WindowLevelPreset> WindowLevelPresets;
  //ETX

};

#endif

