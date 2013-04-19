/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLScalarVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLScalarVolumeDisplayNode_h
#define __vtkMRMLScalarVolumeDisplayNode_h

// MRML includes
#include "vtkMRMLVolumeDisplayNode.h"

// VTK includes
class vtkImageAccumulateDiscrete;
class vtkImageAppendComponents;
class vtkImageBimodalAnalysis;
class vtkImageCast;
class vtkImageData;
class vtkImageLogic;
class vtkImageMapToColors;
class vtkImageMapToWindowLevelColors;
class vtkImageThreshold;
class vtkImageExtractComponents;
class vtkImageMathematics;

// STD includes
#include <vector>

/// \brief MRML node for representing a volume display attributes.
///
/// vtkMRMLScalarVolumeDisplayNode nodes describe how volume is displayed.
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
  vtkBooleanMacro(AutoWindowLevel, int);
  vtkGetMacro(AutoWindowLevel, int);
  vtkSetMacro(AutoWindowLevel, int);
  
  /// 
  /// The window value to use when autoWindowLevel is 'no'
  double GetWindow();
  virtual void SetWindow(double);

  /// 
  /// The level value to use when autoWindowLevel is 'no'
  double GetLevel();
  virtual void SetLevel(double);
  
  /// Sets the window and level at once, generates only 1 modified event if
  /// needed.
  virtual void SetWindowLevel(double window, double level);
  /// Utility function that sets the window/level by passing the min and max
  /// of the window.
  void SetWindowLevelMinMax(double min, double max);

  /// Utility function that returns the minimum value of the window level
  double GetWindowLevelMin();
  /// Utility function that returns the maximum value of the window level
  double GetWindowLevelMax();

  /// 
  /// Specifies whether to apply the threshold
  vtkBooleanMacro(ApplyThreshold, int);
  vtkGetMacro(ApplyThreshold, int);
  virtual void SetApplyThreshold(int);

  /// 
  /// Specifies whether the threshold should be set automatically
  vtkBooleanMacro(AutoThreshold, int);
  vtkGetMacro(AutoThreshold, int);
  vtkSetMacro(AutoThreshold, int);

  /// 
  /// The lower threshold value to use when autoThreshold is 'no'
  /// Defaults to VTK_SHORT_MIN
  virtual double GetLowerThreshold();
  virtual void SetLowerThreshold(double lower);

  /// 
  /// The upper threshold value to use when autoThreshold is 'no'
  /// Defaults to VTK_SHORT_MAX
  virtual double GetUpperThreshold();
  virtual void SetUpperThreshold(double upper);
  
  virtual void SetThreshold(double lower, double upper);

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

  /// Set the pipeline input
  virtual void SetInputImageData(vtkImageData *imageData);

  /// Gets the pipeline input
  virtual vtkImageData* GetInputImageData();

  /// Gets the pipeline output
  virtual vtkImageData* GetOutputImageData();

  /// 
  /// Sets ImageData for background mask 
  virtual void SetBackgroundImageData(vtkImageData *imageData);
  virtual vtkImageData* GetBackgroundImageData();

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

  /// 
  /// Defines the expected range of the output data after
  /// having been mapped through the current display options
  /// If no input is set, then it searches the scene to find the associated
  /// Volume node and returns its image data scalar range.
  virtual void GetDisplayScalarRange(double range[2]);

protected:
  vtkMRMLScalarVolumeDisplayNode();
  virtual ~vtkMRMLScalarVolumeDisplayNode();
  vtkMRMLScalarVolumeDisplayNode(const vtkMRMLScalarVolumeDisplayNode&);
  void operator=(const vtkMRMLScalarVolumeDisplayNode&);

  virtual void SetColorNodeInternal(vtkMRMLColorNode* newColorNode);
  void UpdateLookupTable(vtkMRMLColorNode* newColorNode);
  void CalculateAutoLevels();

  /// Return the image data with scalar type, it can be in the middle of the
  /// pipeline, it's typically the input of the threshold/windowlevel filters
  virtual vtkImageData* GetScalarImageData();
  
  virtual void SetInputToImageDataPipeline(vtkImageData* input);

  /// 
  /// To hold preset values for window and level, so can restore this display
  /// node's window and level to ones read from DICOM files, or defined by
  /// users
  class WindowLevelPreset
  {
  public:
    double Window;
    double Level;
    WindowLevelPreset() { this->Window = 0.0; this->Level = 0.0; };
  };
  //double Window;
  //double Level;
  //double UpperThreshold;
  //double LowerThreshold;


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
  vtkImageExtractComponents *ExtractRGB;
  vtkImageExtractComponents *ExtractAlpha;
  vtkImageMathematics *MultiplyAlpha;

  /// 
  /// window level presets
  std::vector<WindowLevelPreset> WindowLevelPresets;

  /// 
  /// Used internally in CalculateScalarAutoLevels and CalculateStatisticsAutoLevels
  vtkImageAccumulateDiscrete *Accumulate;
  vtkImageBimodalAnalysis *Bimodal;
  bool IsInCalculateAutoLevels;
};

#endif
