/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: ,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

  =========================================================================auto=*/

#ifndef __vtkMRMLGlyphableVolumeDisplayPropertiesNode_h
#define __vtkMRMLGlyphableVolumeDisplayPropertiesNode_h

#include "vtkMRMLColorTableNode.h"
class vtkPolyData;

/// \brief MRML node for display of a diffusion tensor.
///
/// This node describes display properties at the (conceptual) single-tensor level.
/// A tensor can be displayed using various scalar invariants and glyphs.
/// This class is used by classes (vtkMRMLFiberBundleDisplayNode,
/// vtkMRMLGlyphableVolumeVolumeDisplayNode) that handle higher-level display
/// concepts for many diffusion tensors, such as choosing between scalars/glyphs/etc.
/// for specific display needs.
/// This class inherits from the vtkMRMLColorNode->vtkMRMLColorTableNode superclasses,
/// used for vtkMRMLModelNodes and vtkMRMLVolumeNodes, in order to
/// provide specific lookup tables for the scalar invariant display.
class VTK_MRML_EXPORT vtkMRMLGlyphableVolumeDisplayPropertiesNode : public vtkMRMLColorTableNode
{
 public:
  static vtkMRMLGlyphableVolumeDisplayPropertiesNode *New();
  vtkTypeMacro(vtkMRMLGlyphableVolumeDisplayPropertiesNode,vtkMRMLColorTableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Read node attributes from a MRML file in XML format.
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object.
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "GlyphableVolumeDisplayProperties";};

  //--------------------------------------------------------------------------
  /// Display Information: Parameters of glyph geometry
  //--------------------------------------------------------------------------

  ///
  /// Get the scale factor applied to the glyphs.
  vtkGetMacro(GlyphScaleFactor, double);

  ///
  /// Set the scale factor applied to the glyphs.
  vtkSetMacro(GlyphScaleFactor, double);


  /// TO DO: add the rest of the scalars

  //--------------------------------------------------------------------------
  /// Convenience functions to get an appropriate glyph source
  //--------------------------------------------------------------------------

  ///
  /// Get a polydata object according to current glyph display settings
  /// (so a line, sphere, or tube) to use as a source for a glyphing filter.
  vtkGetObjectMacro( GlyphSource, vtkPolyData );
  virtual void UpdateGlyphSource();

 protected:
  vtkMRMLGlyphableVolumeDisplayPropertiesNode();
  ~vtkMRMLGlyphableVolumeDisplayPropertiesNode();
  vtkMRMLGlyphableVolumeDisplayPropertiesNode(const vtkMRMLGlyphableVolumeDisplayPropertiesNode&);
  void operator=(const vtkMRMLGlyphableVolumeDisplayPropertiesNode&);


  double GlyphScaleFactor;
  /// ---- VTK objects for display --- //
  vtkPolyData * GlyphSource;

  /// This is used internally to set a pointer to this polydata
  /// and reference count it.
  /// TO DO: is this causing an extra modified event?
  virtual void SetGlyphSource(vtkPolyData* polyData);

  /// TO DO: add specific lookup tables ranging from 0..1 for or -1 1
  /// for scalar invariants with those ranges

  /// TO DO: read/write MRML for all parameters

};

#endif

