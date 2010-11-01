/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLViewNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
///  vtkMRMLViewNode - MRML node to represent view parameters
/// 
/// View node contains view parameters

#ifndef __vtkMRMLViewNode_h
#define __vtkMRMLViewNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

class VTK_MRML_EXPORT vtkMRMLViewNode : public vtkMRMLNode
{
public:
  static vtkMRMLViewNode *New();
  vtkTypeMacro(vtkMRMLViewNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

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
  virtual const char* GetNodeTagName();

  /// 
  /// Indicates whether or not the view is active
  vtkGetMacro (Active, int );
  virtual void SetActive(int);

  /// 
  /// Indicates whether or not the view is visible (if it is not visible,
  /// then the view is not shown in any of the view layouts, but can be privately
  /// used by modules)
  vtkGetMacro(Visibility, int);
  virtual void SetVisibility ( int );

  /// 
  /// Indicates if the box is visible
  vtkGetMacro(BoxVisible, int);
  virtual void SetBoxVisible ( int );
  
  /// 
  /// Indicates if the axis labels are visible
  vtkGetMacro(AxisLabelsVisible, int);
  virtual void SetAxisLabelsVisible ( int );

  /// 
  /// Toggles visibility of fiducial points in 3D viewer
  vtkGetMacro (FiducialsVisible, int );
  virtual void SetFiducialsVisible ( int );

  vtkGetMacro (FiducialLabelsVisible, int );
  virtual void SetFiducialLabelsVisible ( int );

  /// 
  /// Field of view size
  vtkGetMacro(FieldOfView, double);
  vtkSetMacro(FieldOfView, double);

  /// 
  /// Axis label size
  vtkGetMacro(LetterSize, double);
  vtkSetMacro(LetterSize, double);

  /// 
  /// Background color
  vtkGetVector3Macro (BackgroundColor, double);
  virtual void SetBackgroundColor ( double *color );

  /// 
  /// Turn on and off animated spinning or rocking.
  vtkGetMacro (AnimationMode, int );
  virtual void SetAnimationMode ( int );

  /// 
  vtkGetMacro (ViewAxisMode, int );
  vtkSetMacro (ViewAxisMode, int );
  
  /// 
  /// Direction of animated spinning
  vtkGetMacro ( SpinDirection, int );
  vtkSetMacro ( SpinDirection, int );

  /// 
  /// Number of degrees in spin increment.
  vtkGetMacro ( SpinDegrees, double );
  vtkSetMacro ( SpinDegrees, double );

  vtkGetMacro ( RotateDegrees, double );
  vtkSetMacro ( RotateDegrees, double );
  
  /// 
  /// Amount of wait time between spin increments
  vtkGetMacro ( AnimationMs, int );
  vtkSetMacro ( AnimationMs, int );
    
  /// 
  /// Length of animated rocking
  vtkGetMacro ( RockLength, int );
  vtkSetMacro ( RockLength, int );

  /// 
  /// Increment of animated rock
  vtkGetMacro ( RockCount, int );
  vtkSetMacro ( RockCount, int );

  /// 
  /// stereo mode (including nostereo)
  vtkGetMacro ( StereoType, int );
  virtual void SetStereoType ( int );

  /// 
  /// specifies orthographic or perspective rendering
  vtkGetMacro (RenderMode, int );
  virtual void SetRenderMode ( int );
  
  //BTX
  /// Modes for automatically controlling camera 
  enum
    {
      RotateAround = 0,
      LookFrom
    };
    
  /// Rotate camera directions
  enum
    {
      PitchUp = 0,
      PitchDown,
      RollLeft,
      RollRight,
      YawLeft,
      YawRight
    };
    
  /// Stereo modes
  enum
    {
      NoStereo = 0,
      RedBlue,
      Anaglyph,
      CrystalEyes,
      Interlaced
    };

  /// render modes
  enum
    {
      Perspective = 0,
      Orthographic
    };

  /// animation mode
  enum
    {
      Off = 0,
      Spin,
      Rock
    };

  /// events
  /// Ideally, the events defined below could be handle by
  /// a MRMLViewNodeLogic
  enum
    {
      AnimationModeEvent = 19001,
      RenderModeEvent,
      StereoModeEvent,
      VisibilityEvent,
      BackgroundColorEvent,
      ActiveModifiedEvent,
      GraphicalResourcesCreatedEvent,
      PitchViewRequestedEvent,
      RollViewRequestedEvent,
      YawViewRequestedEvent,
      ZoomInRequestedEvent,
      ZoomOutRequestedEvent,
      ResetFocalPointRequestedEvent,
      LookFromAxisRequestedEvent
    };
  //ETX 


protected:
  vtkMRMLViewNode();
  ~vtkMRMLViewNode();
  vtkMRMLViewNode(const vtkMRMLViewNode&);
  void operator=(const vtkMRMLViewNode&);

  int FiducialsVisible;
  int FiducialLabelsVisible;
  int BoxVisible;
  int AxisLabelsVisible;
  double FieldOfView;
  double LetterSize;
  double BackgroundColor[3];
  
  /// 
  /// parameters of automatic spin
  int AnimationMode;
  int SpinDirection;
  double SpinDegrees;
  int AnimationMs;

  /// 
  /// parameters of automatic rock
  int RockLength;
  int RockCount;

  /// 
  /// Increment used to rotate the view
  /// once about an axis.
  double RotateDegrees;
  
  /// 
  /// parameters for stereo viewing
  int StereoType;

  /// 
  /// Specifies orthographic or perspective rendering
  int RenderMode;

  /// 
  /// Parameters for look-from or rotate-around
  /// automatic view control
  int ViewAxisMode;

  /// 
  /// Indicates whether or not the View is active
  int Active;

  /// 
  /// Indicates whether or not the View is visible
  int Visibility;
  
  /// 
  /// When a view is set Active, make other views inactive.
  virtual void RemoveActiveFlagInScene();
};

#endif
