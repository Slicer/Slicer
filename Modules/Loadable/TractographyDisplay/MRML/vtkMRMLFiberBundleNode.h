/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
///  vtkMRMLFiberBundleNode - MRML node to represent a fiber bundle from tractography in DTI data.
///
/// FiberBundle nodes contain trajectories ("fibers") from tractography, internally represented as vtkPolyData.
/// A FiberBundle node contains many fibers and forms the smallest logical unit of tractography
/// that MRML will manage/read/write. Each fiber has accompanying tensor data.
/// Visualization parameters for these nodes are controlled by the vtkMRMLFiberBundleDisplayNode class.
//

#ifndef __vtkMRMLFiberBundleNode_h
#define __vtkMRMLFiberBundleNode_h

#include "vtkMRMLModelNode.h"


// Tractography includes
#include "vtkSlicerTractographyDisplayModuleMRMLExport.h"

class vtkMRMLFiberBundleDisplayNode;
class vtkExtractSelectedPolyDataIds;
class vtkMRMLAnnotationNode;
class vtkIdTypeArray;
class vtkExtractPolyDataGeometry;
class vtkPlanes;
class vtkCleanPolyData;

class VTK_SLICER_TRACTOGRAPHYDISPLAY_MODULE_MRML_EXPORT vtkMRMLFiberBundleNode : public vtkMRMLModelNode
{
public:
  static vtkMRMLFiberBundleNode *New();
  vtkTypeMacro(vtkMRMLFiberBundleNode,vtkMRMLModelNode);
  //vtkTypeMacro(vtkMRMLFiberBundleNode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);


  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Read node attributes from XML (MRML) file
  virtual void ReadXMLAttributes ( const char** atts );

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML ( ostream& of, int indent );


  ///
  /// Copy the node's attributes to this object
  virtual void Copy ( vtkMRMLNode *node );

  ///
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  ///
  /// Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  ///
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "FiberBundle";};

  /// Get the subsampling ratio for the polydata
  vtkGetMacro(SubsamplingRatio, float);

  /// Set the subsampling ratio for the polydata
  //
  virtual void SetSubsamplingRatio(float);
  virtual float GetSubsamplingRatioMinValue()
    {
    return 0.;
    }
  virtual float GetSubsamplingRatioMaxValue()
    {
    return 1.;
    }

  //vtkSetClampMacro(SubsamplingRatio, float, 0, 1);

  ///
  /// Get annotation MRML object.
  vtkMRMLAnnotationNode* GetAnnotationNode ( );


  ///
  /// Set the ID annotation node for interactive selection.
  void SetAndObserveAnnotationNodeID ( const char *ID );

  ///
  /// Get ID of diffusion tensor display MRML object for fiber glyph.
  vtkGetStringMacro(AnnotationNodeID);

  //--------------------------------------------------------------------------
  /// Interactive Selection Support
  //--------------------------------------------------------------------------

  ///
  /// Enable or disable the selection with an annotation node
  virtual void SetSelectWithAnnotationNode(int);
  vtkGetMacro(SelectWithAnnotationNode, int);
  vtkBooleanMacro(SelectWithAnnotationNode, int);

  enum
  {
    PositiveAnnotationNodeSelection,
    NegativeAnnotationNodeSelection
  };


  ///
  /// Set the mode (positive or negative) of the selection with the annotation node
  vtkGetMacro(SelectionWithAnnotationNodeMode, int);
  virtual void SetSelectionWithAnnotationNodeMode(int);
  virtual void SetSelectionWithAnnotationNodeModeToPositive()
  {
    this->SetSelectionWithAnnotationNodeMode(PositiveAnnotationNodeSelection);
  }
  virtual void SetSelectionWithAnnotationNodeModeToNegative()
  {
    this->SetSelectionWithAnnotationNodeMode(NegativeAnnotationNodeSelection);
  }

  ///
  /// Reimplemented from internal reasons
#if (VTK_MAJOR_VERSION <= 5)
  virtual void SetAndObservePolyData(vtkPolyData* polyData);
#else
  virtual void SetPolyDataConnection(vtkAlgorithmOutput* inputPort);
#endif

  ///
  /// Gets the subsampled PolyData converted from the real data in the node
  virtual vtkPolyData* GetFilteredPolyData();
#if (VTK_MAJOR_VERSION > 5)
  virtual vtkAlgorithmOutput* GetFilteredPolyDataConnection();
#endif

  ///
  /// get associated line display node or NULL if not set
  vtkMRMLFiberBundleDisplayNode* GetLineDisplayNode();

  ///
  /// get associated tube display node or NULL if not set
  vtkMRMLFiberBundleDisplayNode* GetTubeDisplayNode();

  ///
  /// get associated glyph display node or NULL if not set
  vtkMRMLFiberBundleDisplayNode* GetGlyphDisplayNode();

  ///
  /// add line display node if not already present and return it
  vtkMRMLFiberBundleDisplayNode* AddLineDisplayNode();

  ///
  /// add tube display node if not already present and return it
  vtkMRMLFiberBundleDisplayNode* AddTubeDisplayNode();

  ///
  /// add glyph display node if not already present and return it
  vtkMRMLFiberBundleDisplayNode* AddGlyphDisplayNode();

  ///
  /// Create and return default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  /// Create default display nodes
  virtual void CreateDefaultDisplayNodes();

   // Description:
  // Get the maximum number of fibers to show by default when a new fiber bundle node is set
  vtkGetMacro ( MaxNumberOfFibersToShowByDefault, vtkIdType );

  // Description:
  // Set the maximum number of fibers to show by default when a new fiber bundle node is set
  vtkSetMacro ( MaxNumberOfFibersToShowByDefault, vtkIdType );

  // Description:
  // Get original cell id in the input polydata
  vtkIdType GetUnShuffledFiberID(vtkIdType shuffledIndex)
  {
    return this->ShuffledIds->GetValue(shuffledIndex);
  }

  // Description:
  // Enable, Disapble shuffle of IDs
  vtkGetMacro(EnableShuffleIDs, int);
  void SetEnableShuffleIDs(int value)
  {
    this->EnableShuffleIDs = value;
  }


protected:
  vtkMRMLFiberBundleNode();
  ~vtkMRMLFiberBundleNode();
  vtkMRMLFiberBundleNode(const vtkMRMLFiberBundleNode&);
  void operator=(const vtkMRMLFiberBundleNode&);

  void SetPolyDataToDisplayNode(vtkMRMLModelDisplayNode* modelDisplayNode);

  // Description:
  // Maximum number of fibers to show per bundle when it is loaded.
  static vtkIdType MaxNumberOfFibersToShowByDefault;
  vtkIdTypeArray* ShuffledIds;

  vtkExtractSelectedPolyDataIds* ExtractSelectedPolyDataIds;
  vtkCleanPolyData* CleanPolyDataPostSubsampling;
  vtkCleanPolyData* CleanPolyDataPostROISelection;
  float SubsamplingRatio;

  virtual void PrepareSubsampling();
  virtual void UpdateSubsampling();
  virtual void CleanSubsampling();

  /// ALL MRML nodes
  int SelectWithAnnotationNode;
  int SelectionWithAnnotationNodeMode;
  int EnableShuffleIDs;

  vtkMRMLAnnotationNode *AnnotationNode;
  char *AnnotationNodeID;
  vtkExtractPolyDataGeometry *ExtractPolyDataGeometry;
  vtkPlanes *Planes;

  virtual void PrepareROISelection();
  virtual void UpdateROISelection();
  virtual void CleanROISelection();

  virtual void SetAnnotationNodeID(const char* id);

};

#endif
