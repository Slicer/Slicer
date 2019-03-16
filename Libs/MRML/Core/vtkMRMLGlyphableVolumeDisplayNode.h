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
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "GlyphableVolumeDisplay";}

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

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
  void SetSceneReferences() override;

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Finds the storage node and read the data
  void UpdateScene(vtkMRMLScene *scene) override;

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
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;
  ///
  /// set gray colormap
  void SetDefaultColorMap(/*int isLabelMap*/) override;

  ///
  /// get associated slice glyph display node or nullptr if not set
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
  void GetDisplayScalarRange(double range[2]) override
    {
    this->Superclass::GetDisplayScalarRange(range);
    }

protected:
  vtkMRMLGlyphableVolumeDisplayNode();
  ~vtkMRMLGlyphableVolumeDisplayNode() override;
  vtkMRMLGlyphableVolumeDisplayNode(const vtkMRMLGlyphableVolumeDisplayNode&);
  void operator=(const vtkMRMLGlyphableVolumeDisplayNode&);

  char *GlyphColorNodeID;

  void SetGlyphColorNodeID(const char* id);

  vtkMRMLColorNode *GlyphColorNode;

  int VisualizationMode;

};

#endif

