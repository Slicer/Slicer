/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeDisplayNode - MRML node for representing a volume display attributes
// .SECTION Description
// vtkMRMLVolumeDisplayNode nodes describe how volume is displayed.

#ifndef __vtkMRMLVolumeDisplayNode_h
#define __vtkMRMLVolumeDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLColorNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLVolumeDisplayNode : public vtkMRMLNode
{
  public:
  static vtkMRMLVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLVolumeDisplayNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  virtual const char* GetNodeTagName() {return "VolumeDisplay";};

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  //--------------------------------------------------------------------------
  // Display Information
  //--------------------------------------------------------------------------
  
  // Description:
  // Specifies whether windowing and leveling are to be performed automatically
  vtkBooleanMacro(AutoWindowLevel, int);
  vtkGetMacro(AutoWindowLevel, int);
  vtkSetMacro(AutoWindowLevel, int);

  // Description:
  // The window value to use when autoWindowLevel is 'no'
  vtkGetMacro(Window, double);
  vtkSetMacro(Window, double);

  // Description:
  // The level value to use when autoWindowLevel is 'no'
  vtkGetMacro(Level, double);
  vtkSetMacro(Level, double);

  // Description:
  // Specifies whether to apply the threshold
  vtkBooleanMacro(ApplyThreshold, int);
  vtkGetMacro(ApplyThreshold, int);
  vtkSetMacro(ApplyThreshold, int);

  // Description:
  // Specifies whether the threshold should be set automatically
  vtkBooleanMacro(AutoThreshold, int);
  vtkGetMacro(AutoThreshold, int);
  vtkSetMacro(AutoThreshold, int);

  // Description:
  // The upper threshold value to use when autoThreshold is 'no'
  vtkGetMacro(UpperThreshold, double);
  vtkSetMacro(UpperThreshold, double);

  // Description:
  // The lower threshold value to use when autoThreshold is 'no'
  vtkGetMacro(LowerThreshold, double);
  vtkSetMacro(LowerThreshold, double);

  // Description:
  // Set/Get interpolate reformated slices
  vtkGetMacro(Interpolate, int);
  vtkSetMacro(Interpolate, int);
  vtkBooleanMacro(Interpolate, int);

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);
  
  // Description:
  // String ID of the color MRML node
  void SetAndObserveColorNodeID(const char *ColorNodeID);
  vtkGetStringMacro(ColorNodeID);

  // Description:
  // Get associated color MRML node
  vtkMRMLColorNode* GetColorNode();

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
 
protected:
  vtkMRMLVolumeDisplayNode();
  ~vtkMRMLVolumeDisplayNode();
  vtkMRMLVolumeDisplayNode(const vtkMRMLVolumeDisplayNode&);
  void operator=(const vtkMRMLVolumeDisplayNode&);

  char *ColorNodeID;

  vtkSetStringMacro(ColorNodeID);

  vtkMRMLColorNode *ColorNode;

  double Window;
  double Level;
  double UpperThreshold;
  double LowerThreshold;


  // Booleans
  int Interpolate;
  int AutoWindowLevel;
  int ApplyThreshold;
  int AutoThreshold;

};

#endif

