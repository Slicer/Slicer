/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCrosshairNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLCrosshairNode - MRML node for storing a crosshair through RAS space
/// 
/// This node stores the information about a crosshair (position,
/// style, attributes)
//

#ifndef __vtkMRMLCrosshairNode_h
#define __vtkMRMLCrosshairNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

class VTK_MRML_EXPORT vtkMRMLCrosshairNode : public vtkMRMLNode
{
  public:
  static vtkMRMLCrosshairNode *New();
  vtkTypeMacro(vtkMRMLCrosshairNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Crosshair";};

  /// 
  /// configures the crosshair appearance and behavior
  vtkGetMacro (CrosshairMode, int );
  vtkSetMacro (CrosshairMode, int );  
  vtkGetMacro (CrosshairBehavior, int );
  vtkSetMacro (CrosshairBehavior, int );  
  vtkSetClampMacro (CrosshairThickness, int, 1, 3);
  vtkGetMacro (CrosshairThickness, int);
  void SetCrosshairToFine() { this->SetCrosshairThickness(1); }
  void SetCrosshairToMedium() { this->SetCrosshairThickness(2); }
  void SetCrosshairToThick() { this->SetCrosshairThickness(3); }
  
  /// 
  /// Set crosshair position
  vtkSetVector3Macro(CrosshairRAS, double);
  vtkGetVector3Macro(CrosshairRAS, double);
  
  /// 
  /// Is the crosshair to be used for navigation or as just a cursor
  vtkSetMacro(Navigation, int);
  vtkGetMacro(Navigation, int);
  vtkBooleanMacro(Navigation, int);

  /// 
  /// Name of the layout
  void SetCrosshairName(const char *name) {
    this->SetSingletonTag(name);
  }
  char *GetCrosshairName() {
    return this->GetSingletonTag();
  }

  //BTX
  /// Modes for crosshair display and behavior
  enum
    {
      NoCrosshair = 0,
      ShowBasic,
      ShowIntersection,
      ShowHashmarks,
      ShowAll,
      ShowSmallBasic,
      ShowSmallIntersection
    };
  enum
    {
      Fine = 1,
      Medium,
      Thick
    };
  enum
    {
      Normal = 0,
      JumpSlice
    };
  //ETX

protected:
  vtkMRMLCrosshairNode();
  ~vtkMRMLCrosshairNode();
  vtkMRMLCrosshairNode(const vtkMRMLCrosshairNode&);
  void operator=(const vtkMRMLCrosshairNode&);

  int CrosshairMode;
  int CrosshairThickness;
  int CrosshairBehavior;

  int Navigation;

  double CrosshairRAS[3];

};

#endif

