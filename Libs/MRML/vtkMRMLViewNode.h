/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
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
  /// Return the color the view nodes have for the background by default.
  static double* defaultBackgroundColor();
  static double* defaultBackgroundColor2();

  /// 
  /// Indicates whether or not the view is active
  vtkGetMacro (Active, int );
  vtkSetMacro (Active, int );

  /// 
  /// Indicates whether or not the view is visible (if it is not visible,
  /// then the view is not shown in any of the view layouts, but can be privately
  /// used by modules)
  vtkGetMacro(Visibility, int);
  vtkSetMacro(Visibility, int);

  /// 
  /// Indicates if the box is visible
  vtkGetMacro(BoxVisible, int);
  vtkSetMacro(BoxVisible, int);
  
  /// 
  /// Indicates if the axis labels are visible
  vtkGetMacro(AxisLabelsVisible, int);
  vtkSetMacro(AxisLabelsVisible, int);

  /// 
  /// Toggles visibility of fiducial points in 3D viewer
  vtkGetMacro (FiducialsVisible, int );
  vtkSetMacro (FiducialsVisible, int );

  vtkGetMacro (FiducialLabelsVisible, int );
  vtkSetMacro (FiducialLabelsVisible, int );

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
  vtkSetVector3Macro (BackgroundColor, double);

  /// 
  /// Background color 2
  vtkGetVector3Macro (BackgroundColor2, double);
  vtkSetVector3Macro (BackgroundColor2, double);

  /// 
  /// Turn on and off animated spinning or rocking.
  vtkGetMacro (AnimationMode, int );
  vtkSetMacro (AnimationMode, int );

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
  vtkSetMacro ( StereoType, int );

  /// 
  /// specifies orthographic or perspective rendering
  vtkGetMacro (RenderMode, int );
  vtkSetMacro (RenderMode, int );
  
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
  enum
    {
    GraphicalResourcesCreatedEvent = 19001,
    ResetFocalPointRequestedEvent,
    };

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
  double BackgroundColor2[3];
  
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
