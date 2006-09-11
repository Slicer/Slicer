/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiducialListNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLFiducialListNode - MRML node to represent a 3D surface model.
// .SECTION Description
// Model nodes describe polygonal data.  They indicate where the model is 
// stored on disk, and how to render it (color, opacity, etc).  Models 
// are assumed to have been constructed with the orientation and voxel 
// dimensions of the original segmented volume.

#ifndef __vtkMRMLFiducialListNode_h
#define __vtkMRMLFiducialListNode_h

#include <string>

#include "vtkMatrix4x4.h"
#include "vtkCollection.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLFiducial.h"
#include "vtkMRMLTransformableNode.h"

class VTK_MRML_EXPORT vtkMRMLFiducialListNode : public vtkMRMLTransformableNode
{
public:
  static vtkMRMLFiducialListNode *New();
  vtkTypeMacro(vtkMRMLFiducialListNode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "FiducialList";};

  // Description:
  // 
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // update display node ids
  void UpdateReferences();
  
  // Description:
  // Get/Set for Symbol scale
//  vtkSetMacro(SymbolScale,double);
  void SetSymbolScale(double scale);
  vtkGetMacro(SymbolScale,double);


  // Description:
  // Get/Set for list visibility 
  //vtkSetMacro(Visibility,int);
  void SetVisibility(int visible);
  vtkGetMacro(Visibility,int);

  // Description:
  // Get/Set for Text scale
  //vtkSetMacro(TextScale,double);
  void SetTextScale(double scale);
  vtkGetMacro(TextScale,double);
  
  // Description:
  // Get/Set for Glyph and Text color
  //vtkSetVector3Macro(Color,double);
  void SetColor(double r, double g, double b);
  void SetColor(double c[3]);
  vtkGetVectorMacro(Color,double,3);
  
  
  int GetNumberOfFiducials();
  vtkMRMLFiducial* GetNthFiducial(int n);
  int AddFiducial( );
  void RemoveFiducial(vtkMRMLFiducial *o);
  void RemoveFiducial(int i);
  int  IsFiducialPresent(vtkMRMLFiducial *o);

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //BTX
  // Description:
  // DisplayModifiedEvent is generated when display node parameters is changed
  // PolyDataModifiedEvent is generated when something else is changed
  enum
    {
      DisplayModifiedEvent = 19000,
      PolyDataModifiedEvent = 19001,
    };
//ETX

  // Description:
  // Opacity of the fiducial surface expressed as a number from 0 to 1
//  vtkSetMacro(Opacity, double);
  void SetOpacity(double opacity);
  vtkGetMacro(Opacity, double);

  // Description:
  // Ambient of the fiducial surface expressed as a number from 0 to 100
  vtkSetMacro(Ambient, double);
  vtkGetMacro(Ambient, double);
  
  // Description:
  // Diffuse of the fiducial surface expressed as a number from 0 to 100
  vtkSetMacro(Diffuse, double);
  vtkGetMacro(Diffuse, double);
  
  // Description:
  // Specular of the fiducial surface expressed as a number from 0 to 100
  vtkSetMacro(Specular, double);
  vtkGetMacro(Specular, double);

  // Description:
  // Power of the fiducial surface expressed as a number from 0 to 100
  vtkSetMacro(Power, double);
  vtkGetMacro(Power, double);

protected:
  vtkMRMLFiducialListNode();
  ~vtkMRMLFiducialListNode();
  vtkMRMLFiducialListNode(const vtkMRMLFiducialListNode&);
  void operator=(const vtkMRMLFiducialListNode&);

  double SymbolScale;
  double TextScale;
  int Visibility;
  double Color[3];

  vtkCollection *FiducialList;
  
  // Numbers relating to the display of the fiducials
  double Opacity;
  double Ambient;
  double Diffuse;
  double Specular;
  double Power;
};

#endif
