/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLGlyphableVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLGlyphableVolumeDisplayNode_h
#define __vtkMRMLGlyphableVolumeDisplayNode_h

// MRML includes
#include "vtkMRMLScalarVolumeDisplayNode.h"
class vtkMRMLColorNode;
class vtkMRMLGlyphableVolumeSliceDisplayNode;
class vtkMRMLVolumeNode;

// STD includes
#include <vector>

/// \brief MRML node for representing a volume display attributes.
///
/// vtkMRMLGlyphableVolumeDisplayNode nodes describe how volume is displayed.
class VTK_MRML_EXPORT vtkMRMLGlyphableVolumeDisplayNode : public vtkMRMLScalarVolumeDisplayNode
{
  public:
  static vtkMRMLGlyphableVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLGlyphableVolumeDisplayNode,vtkMRMLScalarVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  virtual const char* GetNodeTagName() {return "GlyphableVolumeDisplay";};

  ///
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  //--------------------------------------------------------------------------
  /// Display Information
  //--------------------------------------------------------------------------

  /// Set/Get visualization Mode
  enum
    {
    visModeScalar = 0,
    visModeGlyph = 1,
    visModeBoth = 2
    };

  vtkGetMacro(VisualizationMode, int);
  vtkSetMacro(VisualizationMode, int);

  //virtual vtkPolyData* ExecuteGlyphPipeLineAndGetPolyData( vtkImageData* );

  void SetVisualizationModeToScalarVolume() {
    this->SetVisualizationMode(this->visModeScalar);
  };
  void SetVisualizationModeToGlyphs() {
    this->SetVisualizationMode(this->visModeGlyph);
  };
  void SetVisualizationModeToBoth() {
    this->SetVisualizationMode(this->visModeBoth);
  };

  /// Set Glyph color node ID as reference to the scene
  virtual void SetSceneReferences();

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  ///
  /// Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  ///
  /// String ID of the color MRML node
  void SetAndObserveGlyphColorNodeID(const char *GlyphColorNodeID);
  void SetAndObserveGlyphColorNodeID(std::string GlyphColorNodeID);
  vtkGetStringMacro(GlyphColorNodeID);

  ///
  /// Get associated color MRML node
  vtkMRMLColorNode* GetGlyphColorNode();

  ///
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );
  ///
  /// set gray colormap
  void SetDefaultColorMap(/*int isLabelMap*/);

  ///
  /// get associated slice glyph display node or NULL if not set
  virtual std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*>
    GetSliceGlyphDisplayNodes( vtkMRMLVolumeNode* vtkNotUsed(node) )
    {
    vtkErrorMacro("Shouldn't be calling this");
    return std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*>();
    }


  ///
  /// add slice glyph display nodes if not already present and return it
  virtual void AddSliceGlyphDisplayNodes( vtkMRMLVolumeNode* vtkNotUsed(node) )
    {
    vtkErrorMacro("Shouldn't be calling this");
    }

  ///
  /// Defines the expected range of the output data for given imageData after
  /// having been mapped through the current display options
  virtual void GetDisplayScalarRange(double range[2])
    {
    this->Superclass::GetDisplayScalarRange(range);
    };

protected:
  vtkMRMLGlyphableVolumeDisplayNode();
  ~vtkMRMLGlyphableVolumeDisplayNode();
  vtkMRMLGlyphableVolumeDisplayNode(const vtkMRMLGlyphableVolumeDisplayNode&);
  void operator=(const vtkMRMLGlyphableVolumeDisplayNode&);

  char *GlyphColorNodeID;

  void SetGlyphColorNodeID(const char* id);

  vtkMRMLColorNode *GlyphColorNode;

  int VisualizationMode;

};

#endif

