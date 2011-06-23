/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiducial.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
///  vtkMRMLFiducial - MRML object to represent a 3D point.
/// 

#ifndef __vtkMRMLFiducial_h
#define __vtkMRMLFiducial_h


#include "vtkMatrix4x4.h"

#include "vtkMRML.h"

#include "vtkObject.h"

class VTK_MRML_EXPORT vtkMRMLFiducial : public vtkObject
{
public:
  static vtkMRMLFiducial *New();
  vtkTypeMacro(vtkMRMLFiducial,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  /// MRML methods
  //--------------------------------------------------------------------------

  /// 
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Set node attributes from an unparsed string of keys and values
  virtual void ReadXMLString(const char *keyValuePairs);
  
  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkObject *node);

  /// 
  /// Get/Set for Point
  vtkSetVector3Macro(XYZ,float);
  vtkGetVectorMacro(XYZ,float,3);

  /// 
  /// Get/Set for orientation 
  vtkSetVector4Macro(OrientationWXYZ,float);
  vtkGetVectorMacro(OrientationWXYZ,float,4);
  void SetOrientationWXYZFromMatrix4x4(vtkMatrix4x4 *mat);

  /// 
  /// Get/Set for LabelText
  vtkSetStringMacro(LabelText);
  vtkGetStringMacro(LabelText);

  /// 
  /// Get/Set for ID
  vtkGetStringMacro(ID);
  vtkSetStringMacro(ID);

  /// 
  /// Get/Set for Selected
  vtkGetMacro(Selected, bool);
  vtkSetMacro(Selected, bool);

  /// 
  /// Get/Set for Visibility
  vtkGetMacro(Visibility, bool);
  vtkSetMacro(Visibility, bool);
  
protected:
  vtkMRMLFiducial();
  ~vtkMRMLFiducial();
  vtkMRMLFiducial(const vtkMRMLFiducial&);
  void operator=(const vtkMRMLFiducial&);

  /// Data
  float XYZ[3];
  float OrientationWXYZ[4];
  char *LabelText;
  bool Selected;
  bool Visibility;
  char *ID;
};

#endif
