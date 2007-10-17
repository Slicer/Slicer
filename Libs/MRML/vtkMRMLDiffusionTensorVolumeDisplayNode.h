/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDiffusionTensorVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLDiffusionTensorVolumeDisplayNode - MRML node for representing a volume (image stack).
// .SECTION Description
// Volume nodes describe data sets that can be thought of as stacks of 2D 
// images that form a 3D volume.  Volume nodes describe where the images 
// are stored on disk, how to render the data (window and level), and how 
// to read the files.  This information is extracted from the image 
// headers (if they exist) at the time the MRML file is generated.  
// Consequently, MRML files isolate MRML browsers from understanding how 
// to read the myriad of file formats for medical data. 

#ifndef __vtkMRMLDiffusionTensorVolumeDisplayNode_h
#define __vtkMRMLDiffusionTensorVolumeDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLVolumeGlyphDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

class vtkDiffusionTensorMathematicsSimple;
class vtkDiffusionTensorGlyph;
class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLDiffusionTensorVolumeDisplayNode : public vtkMRMLVolumeGlyphDisplayNode
{
  public:
  static vtkMRMLDiffusionTensorVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLDiffusionTensorVolumeDisplayNode,vtkMRMLVolumeGlyphDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  virtual const char* GetNodeTagName() {return "DiffusionTensorVolumeDisplay";};

  virtual vtkPolyData* ExecuteGlyphPipeLineAndGetPolyData( vtkImageData* );

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );

  //--------------------------------------------------------------------------
  // Display Information
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // MRML nodes that are observed
  //--------------------------------------------------------------------------

  // Description:
  // Get diffusion tensor display MRML object for fiber line.
  vtkMRMLDiffusionTensorDisplayPropertiesNode* GetDiffusionTensorDisplayPropertiesNode ( );

  // Description:
  // Set diffusion tensor display MRML object for fiber line.
  void SetAndObserveDiffusionTensorDisplayPropertiesNodeID ( const char *ID );

  // Description:
  // Get ID of diffusion tensor display MRML object for fiber line.
  vtkGetStringMacro(DiffusionTensorDisplayPropertiesNodeID);

    // Description:
  // Sets vtkImageData to be converted to displayable vtkImageData
  virtual void SetImageData(vtkImageData *imageData);

  virtual void UpdateImageDataPipeline();


protected:
  vtkMRMLDiffusionTensorVolumeDisplayNode();
  ~vtkMRMLDiffusionTensorVolumeDisplayNode();
  vtkMRMLDiffusionTensorVolumeDisplayNode(const vtkMRMLDiffusionTensorVolumeDisplayNode&);
  void operator=(const vtkMRMLDiffusionTensorVolumeDisplayNode&);
  
  vtkMRMLDiffusionTensorDisplayPropertiesNode *DiffusionTensorDisplayPropertiesNode;
  char *DiffusionTensorDisplayPropertiesNodeID;

  vtkSetReferenceStringMacro(DiffusionTensorDisplayPropertiesNodeID);

  vtkDiffusionTensorGlyph* DiffusionTensorGlyphFilter;

  vtkDiffusionTensorMathematicsSimple *DTIMathematics;

};

#endif

