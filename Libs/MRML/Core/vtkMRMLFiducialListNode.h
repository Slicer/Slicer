/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiducialListNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLFiducialListNode_h
#define __vtkMRMLFiducialListNode_h

// MRML includes
#include "vtkMRMLTransformableNode.h"
class vtkMRMLFiducial;
class vtkMRMLFiducialListStorageNode;

// VTK includes
class vtkMatrix4x4;
class vtkCollection;

// STD includes
#include <string>

///
/// a structure used when invoking an event to let others know that two
/// fiducials have swapped indices
struct FiducialListSwappedIndicesStruct
{
    int first;
    int second;
};
using FiducialListSwappedIndices = struct FiducialListSwappedIndicesStruct;

/// \brief MRML node to represent a list of points in 3D.
///
/// \deprecated Used for backward compatibility for Slicer3 fiducial lists, please use the Annotation Module MRML nodes
/// \sa vtkMRMLAnnotationNode, vtkMRMLAnnotationFiducialNode
///
/// Fiducial list nodes describe a list of points in 3d space.  They indicate
/// how to render it (color, opacity, etc).
class VTK_MRML_EXPORT vtkMRMLFiducialListNode : public vtkMRMLTransformableNode
{
public:
  /// \deprecated Used for backward compatibility for Slicer3 fiducial lists, please use the Annotation Module MRML nodes
  /// \sa vtkMRMLAnnotationNode, vtkMRMLAnnotationFiducialNode
  ///
  static vtkMRMLFiducialListNode *New();
  vtkTypeMacro(vtkMRMLFiducialListNode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///--------------------------------------------------------------------------
  /// MRMLNode methods
  ///--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;


  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "FiducialList";}

  ///
  void UpdateScene(vtkMRMLScene *scene) override;

  /// update display node ids
  void UpdateReferences() override;

  /// Get/Set for Symbol scale
///  vtkSetMacro(SymbolScale,double);
  void SetSymbolScale(double scale);
  vtkGetMacro(SymbolScale,double);


  /// Get/Set for list visibility
  ///vtkSetMacro(Visibility,int);
  void SetVisibility(int visible);
  vtkGetMacro(Visibility,int);

  /// Get/Set for Text scale
  ///vtkSetMacro(TextScale,double);
  void SetTextScale(double scale);
  vtkGetMacro(TextScale,double);

  /// Get/Set for Glyph and Text color
  ///vtkSetVector3Macro(Color,double);
  void SetColor(double r, double g, double b);
  void SetColor(double c[3]);
  vtkGetVectorMacro(Color,double,3);

  /// Get/Set for colour for when a fiducial is selected
  void SetSelectedColor(double r, double g, double b);
  void SetSelectedColor(double c[3]);
  vtkGetVectorMacro(SelectedColor,double,3);


  /// Get the number of fiducials in the list
  int GetNumberOfFiducials();

  /// Restrict access to the fiducial points, pass in a value via the list
  /// so that the appropriate events can be invoked. Returns 0 on success
  int SetNthFiducialXYZ(int n, float x, float y, float z);
  int SetNthFiducialOrientation(int n, float w, float x, float y, float z);
  int SetNthFiducialLabelText(int n, const char *text);
  int SetNthFiducialSelected(int n, int flag);
  int SetNthFiducialSelectedNoModified(int n, int flag);
  int SetNthFiducialVisibility(int n, int flag);
  int SetNthFiducialVisibilityNoModified(int n, int flag);
  int SetNthFiducialID(int n, const char *id);

  /// Set the nth fiducial to xyz transformed by the inverse of the transform
  /// to world for the list. Calls SetNthFiducialXYZ after transforming the
  /// passed in coordinates and returns the result of that call.
  int SetNthFiducialXYZWorld(int n, float x, float y, float z);

  /// Restrict access to the fiducial points, access the fiducial by id (used
  /// by the vtkSlicerFiducialListWidget). Returns 0 on success.
  int SetFiducialXYZ(std::string fiducialID, float x, float y, float z);
  /// Look through the list of fiducials for fiducialID and return the current
  /// zero based index. Useful when delete fiducials from the list, as the
  /// index will change. Returns -1 if none found.
  int GetFiducialIndex(std::string fiducialID);

  /// Set all fiducials selected state to flag
  int SetAllFiducialsSelected(int flag);

  /// Set all fiducials visible state to flag
  int SetAllFiducialsVisibility(int flag);

  /// Get the elements of the fiducial points
  /// Return a three element float holding the position
  float *GetNthFiducialXYZ(int n);

  /// Return a three element double giving the world position (any parent
  /// transform on the list applied to the return of GetNthFiducialXYZ.
  /// worldxyz is a 4 item array, xyzw
  /// returns 0 on failure, 1 on success.
  int GetNthFiducialXYZWorld(int n, double *worldxyz);
  /// get the orientation of the nth fiducial
  float *GetNthFiducialOrientation(int n);
  /// get the label text of the nth fiducial
  const char *GetNthFiducialLabelText(int n);
  /// get the selected state on the nth fiducial
  int GetNthFiducialSelected(int n);
  /// get the visible state on the nth fiducial
  int GetNthFiducialVisibility(int n);
  /// get the id of the nth fiducial
  const char *GetNthFiducialID(int n);

  /// Add a fiducial point to the list with default values
  int AddFiducial( );

  /// Add a fiducial point to the list x, y, z
  int AddFiducialWithXYZ(float x, float y, float z, int selected);
  /// Add a fiducial point to the list with a label, x,y,z, selected flag, visibility
  int AddFiducialWithLabelXYZSelectedVisibility(const char *label, float x, float y, float z, int selected, int visibility);

  /// remove the passed in fiducial from the list
  void RemoveFiducial(vtkMRMLFiducial *o);
  /// remove the fiducial at index i
  void RemoveFiducial(int i);
  /// remove all fiducials from the list
  void RemoveAllFiducials();
  /// is this fiducial on the list?
  int  IsFiducialPresent(vtkMRMLFiducial *o);

  /// Process events from the MRML scene
  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData ) override;

  /// DisplayModifiedEvent is generated when display node parameters is changed
  /// PolyDataModifiedEvent is generated when something else is changed
  enum
    {
      DisplayModifiedEvent = 19000,
      PolyDataModifiedEvent = 19001,
      FiducialModifiedEvent = 19002,
      FiducialIndexModifiedEvent = 19003
    };

  /// Opacity of the fiducial surface expressed as a number from 0 to 1
  void SetOpacity(double opacity);
  vtkGetMacro(Opacity, double);

  /// Ambient of the fiducial surface expressed as a number from 0 to 1
  void SetAmbient(double val);
  vtkGetMacro(Ambient, double);

  /// Diffuse of the fiducial surface expressed as a number from 0 to 1
  void SetDiffuse(double val);
  vtkGetMacro(Diffuse, double);

  /// Specular of the fiducial surface expressed as a number from 0 to 1
  void SetSpecular(double val);
  vtkGetMacro(Specular, double);

  /// Power of the fiducial surface expressed as a number from 0 to 1
  void SetPower(double val);
  vtkGetMacro(Power, double);

  /// When fiducial lists are locked, they cannot be manipulated using the interactive widgets
  void SetLocked(int locked);
  vtkGetMacro(Locked, int);

  /// Which kind of glyph should be used to display this fiducial?
  /// Vertex2D is supposed to start at 1
  enum GlyphShapes
  {
    GlyphMin = 1,
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
    GlyphMax = Diamond3D
  };

  /// Return the min/max glyph types, for iterating over them in tcl
  int GetMinimumGlyphType() { return vtkMRMLFiducialListNode::GlyphMin; };
  int GetMaximumGlyphType() { return vtkMRMLFiducialListNode::GlyphMax; };

  /// The glyph type used to display this fiducial
  void SetGlyphType(int type);
  vtkGetMacro(GlyphType, int);
  /// Returns 1 if the type is a 3d one, 0 else
  int GlyphTypeIs3D(int glyphType);
  int GlyphTypeIs3D() { return this->GlyphTypeIs3D(this->GlyphType); };

  /// Return a string representing the glyph type, set it from a string
  const char* GetGlyphTypeAsString();
  const char* GetGlyphTypeAsString(int g);
  void SetGlyphTypeFromString(const char *glyphString);

  /// transform utility functions
  bool CanApplyNonLinearTransforms()const override;
  void ApplyTransform(vtkAbstractTransform* transform) override;

  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  /// move a fiducial point in the collection, one up/down
  /// returns -1 on failure (current index is out of bounds, the fid is already
  /// at the top or bottom of the list), the new fiducial index on success
  int MoveFiducialUp(int fidIndex);
  int MoveFiducialDown(int fidIndex);

  /// Renumber all the fiducials in the active list. It first removes any numbers
  /// from the ends of the label texts and then appends numbers starting from 0
  /// by default.
  void RenumberFiducials(int startFrom = 0);

  /// Reanme all the fiducials in the active list. It preserves any numbers
  /// already on the ends of the labels.
  void RenameFiducials(const char *newName);

  /// Flags to determine how the next fiducial added to the list is labelled
  enum NumberingSchemes
  {
      SchemeMin = 0,
      UseID = SchemeMin,
      UseIndex,
      UsePrevious,
      SchemeMax = UsePrevious
  };

  /// Flag determining how to number the next added fiducial
  vtkSetMacro(NumberingScheme, int);
  vtkGetMacro(NumberingScheme, int);

  /// Return a string representing the numbering scheme, set it from a string
  const char* GetNumberingSchemeAsString();
  const char* GetNumberingSchemeAsString(int g);
  void SetNumberingSchemeFromString(const char *schemeString);

  /// Generate the label text for a fiducial from it's id. If NumberingScheme
  /// is UseID, uses the ID (default). If NumberingScheme is UseIndex, strips the ID of any
  /// trailing numbers and appends the fiducial's index to it. If
  /// NumberingScheme is UsePrevious, checks the previous fiducial
  /// in the list for a trailing number and increments it by 1, and appends the
  /// new number. If it's the first fiducial, uses 0. If the previous fiducial
  /// has no trailing number in it's label text, will use 1.
  void SetFiducialLabelTextFromID(vtkMRMLFiducial *fid);

protected:
  vtkMRMLFiducialListNode();
  ~vtkMRMLFiducialListNode() override;
  vtkMRMLFiducialListNode(const vtkMRMLFiducialListNode&);
  void operator=(const vtkMRMLFiducialListNode&);

  /// disallow access to the fiducial points by outside classes, have them use
  /// SetNthFiducial
  vtkMRMLFiducial* GetNthFiducial(int n);

  double SymbolScale;
  double TextScale;
  int Visibility;
  double Color[3];
  double SelectedColor[3];

  /// The collection of fiducial points that make up this list
  vtkCollection *FiducialList;

  /// Numbers relating to the display of the fiducials
  double Opacity;
  double Ambient;
  double Diffuse;
  double Specular;
  double Power;
  int Locked;
  int GlyphType;

  /// How the next added fiducial will be numbered in it's LabelText field
  int NumberingScheme;
};

#endif
