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

#include <string>

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
  // Indicates if the box is visible
  vtkGetMacro(BoxVisible, int);
  vtkSetMacro(BoxVisible, int); 
  
  // Description:
  // Indicates if the axis labels are visible
  vtkGetMacro(AxisLabelsVisible, int);
  vtkSetMacro(AxisLabelsVisible, int);

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
  vtkSetVector3Macro (BackgroundColor, double);

  // Description:
  // Turn on and off animated spinning
  vtkGetMacro (Spin, int );
  vtkSetMacro (Spin, int );

  // Description:
  // Direction of animated spinning
  vtkGetMacro ( SpinDirection, int );
  vtkSetMacro ( SpinDirection, int );

  // Description:
  // Number of degrees in spin increment.
  vtkGetMacro ( SpinDegrees, double );
  vtkSetMacro ( SpinDegrees, double );

  // Description:
  // Amount of wait time between spin increments
  vtkGetMacro ( SpinMs, int );
  vtkSetMacro ( SpinMs, int );
    
  // Description:
  // Turn on and off animated rocking
  vtkGetMacro ( Rock, int );
  vtkSetMacro ( Rock, int );

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
  vtkSetMacro ( StereoType, int );

  // Description:
  // specifies orthographic or perspective rendering
  vtkGetMacro (RenderMode, int );
  vtkSetMacro (RenderMode, int );
  
  //BTX
  // Modes for controlling camera by clicking axes
  enum
    {
      RotateAround = 0,
      LookFrom
    };
    
  // Rotate camera directions
  enum
    {
      Up = 0,
      Down,
      Left,
      Right
    };
    
  // Stereo modes
  enum
    {
      NoStereo = 0,
      RedBlue,
      CrystalEyes,
      Interlaced
    };

  // render modes
  enum
    {
      Perspective = 0,
      Orthographic
    };
  //ETX 

protected:
  vtkMRMLViewNode();
  ~vtkMRMLViewNode();
  vtkMRMLViewNode(const vtkMRMLViewNode&);
  void operator=(const vtkMRMLViewNode&);


  int BoxVisible;
  int AxisLabelsVisible;
  double FieldOfView;
  double LetterSize;
  double BackgroundColor[3];
  
  // Description:
  // parameters of automatic spin
  int Spin;
  int SpinDirection;
  double SpinDegrees;
  int SpinMs;

  // Description:
  // parameters of automatic rock
  int Rock;
  int RockLength;
  int RockCount;

  // Description:
  // parameters for stereo viewing
  int StereoType;

  // Description:
  // Specifies orthographic or perspective rendering
  int RenderMode;
  
};

#endif
