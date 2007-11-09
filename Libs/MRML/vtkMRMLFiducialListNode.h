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

  // Description:
  // Get/Set for colour for when a fiducial is selected
  void SetSelectedColor(double r, double g, double b);
  void SetSelectedColor(double c[3]);
  vtkGetVectorMacro(SelectedColor,double,3);


  // Description:
  // Get the number of fiducials in the list
  int GetNumberOfFiducials();
  
  // Description:
  // Restrict access to the fiducial points, pass in a value via the list
  // so that the appropriate events can be invoked. Returns 0 on success
  int SetNthFiducialXYZ(int n, float x, float y, float z);
  int SetNthFiducialOrientation(int n, float w, float x, float y, float z);
  int SetNthFiducialLabelText(int n, const char *text);
  int SetNthFiducialSelected(int n, int flag);
  int SetNthFiducialSelectedNoModified(int n, int flag);
  int SetNthFiducialID(int n, const char *id);

  // Description:
  // Set all fiducials selected state to flag
  int SetAllFiducialsSelected(int flag);

  // Description:
  // Get the elements of the fiducial points
  // Return a three element float holding the position
  float *GetNthFiducialXYZ(int n);
  // get the orientation of the nth fiducial
  float *GetNthFiducialOrientation(int n);
  // get the label text of the nth fiducial
  const char *GetNthFiducialLabelText(int n);
  // get the selected state on the nth fiducial
  int GetNthFiducialSelected(int n);
  // get the id of the nth fiducial
  const char *GetNthFiducialID(int n);
  
  // Description:
  // Add a fiducial point to the list with default values
  int AddFiducial( );

  // Description:
  // Add a fiducial point to the list x, y, z
  int AddFiducialWithXYZ(float x, float y, float z, int selected);
  
  void RemoveFiducial(vtkMRMLFiducial *o);
  void RemoveFiducial(int i);
  void RemoveAllFiducials();
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
      FiducialModifiedEvent = 19002,
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

  //BTX
  // Description:
  // Which kind of glyph should be used to display this fiducial?
  enum GlyphShapes
  {
    GlyphMin,
    Vertex2D = GlyphMin,
    Dash2D,
    Cross2D,
    ThickCross2D,
    Triangle2D,
    Square2D,
    Circle2D,
    Diamond2D,
    Arrow2D,
    ThickArrow2D,
    HookedArrow2D,
    StarBurst2D,
    Sphere3D,
    Diamond3D,
    GlyphMax = Diamond3D,
  };
  //ETX
  // Description:
  // The glyph type used to display this fiducial
  void SetGlyphType(int type);
  vtkGetMacro(GlyphType, int);
  // Description:
  // Returns 1 if the type is a 3d one, 0 else
  int GlyphTypeIs3D(int glyphType);
  int GlyphTypeIs3D() { return this->GlyphTypeIs3D(this->GlyphType); };

  // Description:
  // Return a string representing the glyph type
  const char* GetGlyphTypeAsString();
  const char* GetGlyphTypeAsString(int g);
  void SetGlyphTypeFromString(const char *glyphString);

protected:
  vtkMRMLFiducialListNode();
  ~vtkMRMLFiducialListNode();
  vtkMRMLFiducialListNode(const vtkMRMLFiducialListNode&);
  void operator=(const vtkMRMLFiducialListNode&);

  // Description:
  // disallow access to the fiducial points by outside classes, have them use
  // SetNthFiducial
  vtkMRMLFiducial* GetNthFiducial(int n);
  
  double SymbolScale;
  double TextScale;
  int Visibility;
  double Color[3];
  double SelectedColor[3];

  // Description:
  // The collection of fiducial points that make up this list
  vtkCollection *FiducialList;

  // Description:
  // Numbers relating to the display of the fiducials
  double Opacity;
  double Ambient;
  double Diffuse;
  double Specular;
  double Power;
  int GlyphType;
};

#endif
