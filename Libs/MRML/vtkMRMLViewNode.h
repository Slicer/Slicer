/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLViewNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLViewNode - MRML node to represent view parameters
// .SECTION Description
// View node contains view parameters

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
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "View";};

  // Description:
  // When a view is set Active, make other views inactive.
  virtual void MakeOthersInActive();

  // Description:
  // Indicates whether or not the view is active
  vtkGetMacro (Active, int );
  vtkSetMacro (Active, int );
  
  // Description:
  // Indicates if the box is visible
  vtkGetMacro(BoxVisible, int);
  virtual void SetBoxVisible ( int );
  //vtkSetMacro (BoxVisible, int );
  
  // Description:
  // Indicates if the axis labels are visible
  vtkGetMacro(AxisLabelsVisible, int);
  virtual void SetAxisLabelsVisible ( int );
//  vtkSetMacro(AxisLabelsVisible, int);

  // Description:
  // Toggles visibility of fiducial points in 3D viewer
  vtkGetMacro (FiducialsVisible, int );
  virtual void SetFiducialsVisible ( int );
//  vtkSetMacro (FiducialsVisible, int);

  vtkGetMacro (FiducialLabelsVisible, int );
  virtual void SetFiducialLabelsVisible ( int );

  // Description:
  // Field of view size
  vtkGetMacro(FieldOfView, double);
  vtkSetMacro(FieldOfView, double);

  // Description:
  // Axis label size
  vtkGetMacro(LetterSize, double);
  vtkSetMacro(LetterSize, double);

  // Description:
  // Background color
  vtkGetVector3Macro (BackgroundColor, double);
  virtual void SetBackgroundColor ( double *color );
//  vtkSetVector3Macro (BackgroundColor, double);

  // Description:
  // Turn on and off animated spinning or rocking.
  vtkGetMacro (AnimationMode, int );
  virtual void SetAnimationMode ( int );
//  vtkSetMacro (AnimationMode, int );

  // Description:
  vtkGetMacro (ViewAxisMode, int );
  vtkSetMacro (ViewAxisMode, int );
  
  // Description:
  // Direction of animated spinning
  vtkGetMacro ( SpinDirection, int );
  vtkSetMacro ( SpinDirection, int );

  // Description:
  // Number of degrees in spin increment.
  vtkGetMacro ( SpinDegrees, double );
  vtkSetMacro ( SpinDegrees, double );

  vtkGetMacro ( RotateDegrees, double );
  vtkSetMacro ( RotateDegrees, double );
  
  // Description:
  // Amount of wait time between spin increments
  vtkGetMacro ( AnimationMs, int );
  vtkSetMacro ( AnimationMs, int );
    
  // Description:
  // Length of animated rocking
  vtkGetMacro ( RockLength, int );
  vtkSetMacro ( RockLength, int );

  // Description:
  // Increment of animated rock
  vtkGetMacro ( RockCount, int );
  vtkSetMacro ( RockCount, int );

  // Description:
  // stereo mode (including nostereo)
  vtkGetMacro ( StereoType, int );
  virtual void SetStereoType ( int );
//  vtkSetMacro ( StereoType, int );

  // Description:
  // specifies orthographic or perspective rendering
  vtkGetMacro (RenderMode, int );
  virtual void SetRenderMode ( int );
//  vtkSetMacro (RenderMode, int );
  
  //BTX
  // Modes for automatically controlling camera 
  enum
    {
      RotateAround = 0,
      LookFrom
    };
    
  // Rotate camera directions
  enum
    {
      PitchUp = 0,
      PitchDown,
      RollLeft,
      RollRight,
      YawLeft,
      YawRight
    };
    
  // Stereo modes
  enum
    {
      NoStereo = 0,
      RedBlue,
      Anaglyph,
      CrystalEyes,
      Interlaced
    };

  // render modes
  enum
    {
      Perspective = 0,
      Orthographic
    };

  // animation mode
  enum
    {
      Off = 0,
      Spin,
      Rock
    };

  // events
  enum
    {
      AnimationModeEvent = 19001,
      RenderModeEvent,
      StereoModeEvent,
      VisibilityEvent,
      BackgroundColorEvent,
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
  
  // Description:
  // parameters of automatic spin
  int AnimationMode;
  int SpinDirection;
  double SpinDegrees;
  int AnimationMs;

  // Description:
  // parameters of automatic rock
  int RockLength;
  int RockCount;

  // Description:
  // Increment used to rotate the view
  // once about an axis.
  double RotateDegrees;
  
  // Description:
  // parameters for stereo viewing
  int StereoType;

  // Description:
  // Specifies orthographic or perspective rendering
  int RenderMode;

  // Description:
  // Parameters for look-from or rotate-around
  // automatic view control
  int ViewAxisMode;

  // Description:
  // Indicates whether or not the View is active
  int Active;
  
};

#endif
