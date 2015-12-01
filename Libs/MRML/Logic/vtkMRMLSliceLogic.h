/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceLogic.h,v $
  Date:      $Date$
  Version:   $Revision: 18866

=========================================================================auto=*/

#ifndef __vtkMRMLSliceLogic_h
#define __vtkMRMLSliceLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

// STD includes
#include <vector>

class vtkMRMLDisplayNode;
class vtkMRMLLinearTransformNode;
class vtkMRMLModelDisplayNode;
class vtkMRMLModelNode;
class vtkMRMLSliceCompositeNode;
class vtkMRMLSliceLayerLogic;
class vtkMRMLSliceNode;
class vtkMRMLVolumeNode;

class vtkAlgorithmOutput;
class vtkCollection;
class vtkImageBlend;
class vtkTransform;
class vtkImageData;
class vtkImageReslice;
class vtkPolyDataCollection;
class vtkTransform;

/// \brief Slicer logic class for slice manipulation.
///
/// This class manages the logic associated with display of slice windows
/// (but not the GUI).  Features of the class include:
///  -- a back-to-front list of MrmlVolumes to be displayed
///  -- a compositing mode for each volume layer (opacity, outline, glyph, checkerboard, etc)
///  -- each layer is required to provide an RGBA image in the space defined by the vtkMRMLSliceNode
///
/// This class manages internal vtk pipelines that create an output vtkImageData
/// which can be used by the vtkSlicerSliceGUI class to display the resulting
/// composite image or it can be used as a texture map in a vtkSlicerView.
/// This class can also be used for resampling volumes for further computation.
class VTK_MRML_LOGIC_EXPORT vtkMRMLSliceLogic : public vtkMRMLAbstractLogic
{
public:
  /// The Usual VTK class functions
  static vtkMRMLSliceLogic *New();
  vtkTypeMacro(vtkMRMLSliceLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Convenient methods allowing to initialize SliceLogic given \a newSliceNode
  /// \note This method should be used when the Logic is "shared" between two widgets
  void Initialize(vtkMRMLSliceNode* newSliceNode);
  bool IsInitialized();

  /// Set / Get SliceLogic name
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);

  ///
  /// The MRML slice node for this slice logic
  vtkGetObjectMacro (SliceNode, vtkMRMLSliceNode);
  void SetSliceNode (vtkMRMLSliceNode * newSliceNode);

  ///
  /// The MRML slice node for this slice logic
  vtkGetObjectMacro (SliceCompositeNode, vtkMRMLSliceCompositeNode);
  void SetSliceCompositeNode (vtkMRMLSliceCompositeNode *SliceCompositeNode);

  ///
  /// The background slice layer
  /// TODO: this will eventually be generalized to a list of layers
  vtkGetObjectMacro (BackgroundLayer, vtkMRMLSliceLayerLogic);
  void SetBackgroundLayer (vtkMRMLSliceLayerLogic *BackgroundLayer);

  ///
  /// The forground slice layer
  /// TODO: this will eventually be generalized to a list of layers
  vtkGetObjectMacro (ForegroundLayer, vtkMRMLSliceLayerLogic);
  void SetForegroundLayer (vtkMRMLSliceLayerLogic *ForegroundLayer);

  ///
  /// The Label slice layer
  /// TODO: this will eventually be generalized to a list of layers
  vtkGetObjectMacro (LabelLayer, vtkMRMLSliceLayerLogic);
  void SetLabelLayer (vtkMRMLSliceLayerLogic *LabelLayer);

  ///
  /// Helper to set the background layer Window/Level
  void SetBackgroundWindowLevel(double window, double level);

  ///
  /// Helper to aget the background layer Window/Level
  void GetBackgroundWindowLevelAndRange(double& window, double& level,
                                      double& rangeLow, double& rangeHigh);


  ///
  /// Helper to aget the foreground layer Window/Level
  void GetForegroundWindowLevelAndRange(double& window, double& level,
                                      double& rangeLow, double& rangeHigh);
  ///
  /// Model slice plane
  vtkGetObjectMacro(SliceModelNode, vtkMRMLModelNode);

  ///
  /// Model slice plane display props
  vtkGetObjectMacro(SliceModelDisplayNode, vtkMRMLModelDisplayNode);

  ///
  /// Model slice plane transform from xy to RAS
  vtkGetObjectMacro(SliceModelTransformNode, vtkMRMLLinearTransformNode);

  ///
  /// The compositing filter
  /// TODO: this will eventually be generalized to a per-layer compositing function
  vtkGetObjectMacro(Blend, vtkImageBlend);
  vtkGetObjectMacro(BlendUVW, vtkImageBlend);

  ///
  /// The offset to the correct slice for lightbox mode
  vtkGetObjectMacro(ActiveSliceTransform, vtkTransform);

  ///
  /// All the PolyData objects to render
  vtkGetObjectMacro(PolyDataCollection, vtkPolyDataCollection);

  ///
  /// All the LookupTable objects to color the PolyData object
  vtkGetObjectMacro(LookupTableCollection, vtkCollection);

  ///
  /// An image reslice instance to pull a single slice from the volume that
  /// represents the filmsheet display output
  vtkGetObjectMacro(ExtractModelTexture, vtkImageReslice);

  ///
  /// the tail of the pipeline
  /// -- returns NULL if none of the inputs exist
  vtkAlgorithmOutput *GetImageDataConnection();

  ///
  /// update the pipeline to reflect the current state of the nodes
  void UpdatePipeline();

  /// Internally used by UpdatePipeline
  void UpdateImageData();

  /// Reimplemented to avoir calling ProcessMRMLSceneEvents when we are added the
  /// MRMLModelNode into the scene
  virtual bool EnterMRMLCallback()const;

  ///
  /// Manage and syncronise the SliceNode
  void UpdateSliceNode();

  ///
  /// Update slicer node given a layout name
  void UpdateSliceNodeFromLayout();

  ///
  /// Manage and syncronise the SliceCompositeNode
  void UpdateSliceCompositeNode();

  ///
  /// Get the volume node corresponding to layer
  /// (0=background, 1=foreground, 2=label)
  vtkMRMLVolumeNode *GetLayerVolumeNode(int layer);

  ///
  /// Get the size of the volume, transformed to RAS space
  static void GetVolumeRASBox(vtkMRMLVolumeNode *volumeNode, double rasDimensions[3], double rasCenter[3]);

  ///
  /// Get the size of the volume, transformed to slice space
  void GetVolumeSliceDimensions(vtkMRMLVolumeNode *volumeNode, double sliceDimensions[3], double sliceCenter[3]);

  ///
  /// Get the spacing of the volume, transformed to slice space
  /// - to be used, for example, to set the slice increment for stepping a single
  ///   voxel relative to the current slice view
  double *GetVolumeSliceSpacing(vtkMRMLVolumeNode *volumeNode);

  ///
  /// Get the min/max bounds of the volume
  /// - note these are not translated by the current slice offset so they can
  ///   be used to calculate the range (e.g. of a slider) that operates in slice space
  void GetVolumeSliceBounds(vtkMRMLVolumeNode *volumeNode, double sliceBounds[6]);

  ///
  /// adjust the node's field of view to match the extent of current background volume
  void FitSliceToVolume(vtkMRMLVolumeNode *volumeNode, int width, int height);

  ///
  /// Get the size of the volume, transformed to RAS space
  void GetBackgroundRASBox(double rasDimensions[3], double rasCenter[3]);

  ///
  /// Get the size of the volume, transformed to slice space
  void GetBackgroundSliceDimensions(double sliceDimensions[3], double sliceCenter[3]);

  ///
  /// Get the spacing of the volume, transformed to slice space
  /// - to be used, for example, to set the slice increment for stepping a single
  ///   voxel relative to the current slice view
  double *GetBackgroundSliceSpacing();

  ///
  /// Get the min/max bounds of the volume
  /// - note these are not translated by the current slice offset so they can
  ///   be used to calculate the range (e.g. of a slider) that operates in slice space
  void GetBackgroundSliceBounds(double sliceBounds[6]);

  ///
  /// adjust the node's field of view to match the extent of current background volume
  void FitSliceToBackground(int width, int height);

  ///
  /// adjust the node's field of view to match the extent of all volume layers
  ///  (fits to first non-null layer)
  void FitSliceToAll(int width = -1, int height = -1);

  /// adjust the node's field of view to match the FOV
  /// the value fov will be applied to the smallest slice window dimension
  void FitFOVToBackground(double fov);

  /// Adjust dimensions and fov based on the new viewport size.
  /// The size should be the viewport size (typically vtkRenderWindow), not the
  /// size of the renderers (important if it's in a lightbox mode).
  /// It must be called each time the renderwindow size is modified and each
  /// time the lightbox configuration is changed.
  void ResizeSliceNode(double newWidth, double newHeight);

  ///
  /// Get the spacing of the volume, transformed to slice space
  /// - to be used, for example, to set the slice increment for stepping a single
  ///   voxel relative to the current slice view
  /// - returns first non-null layer
  double *GetLowestVolumeSliceSpacing();

  ///
  /// Get the min/max bounds of the volume
  /// - note these are not translated by the current slice offset so they can
  ///   be used to calculate the range (e.g. of a slider) that operates in slice space
  /// - returns first non-null layer
  void GetLowestVolumeSliceBounds(double sliceBounds[6]);

  ///
  /// Get/Set the current distance from the origin to the slice plane
  double GetSliceOffset();
  void SetSliceOffset(double offset);

  ///
  /// Get the largest slice bounding box for all volumes in layers
  void GetSliceBounds(double sliceBounds[6]);

  ///
  /// Set slice extents to all layers
  void SetSliceExtentsToSliceNode();

  /// Indicate an interaction with the slice node is beginning. The
  /// parameters of the slice node being manipulated are passed as a
  /// bitmask. See vtkMRMLSliceNode::InteractionFlagType.
  void StartSliceNodeInteraction(unsigned int parameters);

  /// Indicate an interaction with the slice node has been completed
  void EndSliceNodeInteraction();

  /// Indicate an interaction with the slice composite node is
  /// beginning. The parameters of the slice node being manipulated
  /// are passed as a bitmask. See vtkMRMLSliceNode::InteractionFlagType.
  void StartSliceCompositeNodeInteraction(unsigned int parameters);

  /// Indicate an interaction with the slice composite node has been completed
  void EndSliceCompositeNodeInteraction();

  /// Indicate the slice offset value is starting to change
  void StartSliceOffsetInteraction();

  /// Indicate the slice offset value has completed its change
  void EndSliceOffsetInteraction();

  ///
  /// Set the current distance so that it corresponds to the closest center of
  /// a voxel in IJK space (integer value)
  void SnapSliceOffsetToIJK();

  static const int SLICE_INDEX_ROTATED;
  static const int SLICE_INDEX_OUT_OF_VOLUME;
  static const int SLICE_INDEX_NO_VOLUME;

  /// Get the DICOM slice index (1-based) from slice offset (distance from the origin to the slice plane).
  /// If the return value is negative then then no slice index can be determined:
  /// SLICE_INDEX_ROTATED=the slice is rotated compared to the volume planes,
  /// SLICE_INDEX_OUT_OF_VOLUME=the slice plane is out of the volume
  /// SLICE_INDEX_NO_VOLUME=the specified volume is not available
  int GetSliceIndexFromOffset(double sliceOffset, vtkMRMLVolumeNode *volumeNode);

  /// Get the DICOM slice index (1-based) from slice offset (distance from the origin to the slice plane).
  /// Slice index is computed for the first available volume (the search order is
  /// background, foreground, label volume).
  /// If the return value is negative then then no slice index can be determined for the
  /// first available volume:
  /// SLICE_INDEX_ROTATED=the slice is rotated compared to the volume planes,
  /// SLICE_INDEX_OUT_OF_VOLUME=the slice plane is out of the volume
  /// SLICE_INDEX_NO_VOLUME=no volume is available
  int GetSliceIndexFromOffset(double sliceOffset);

  ///
  /// Make a slice model with the current configuration
  void CreateSliceModel();
  void DeleteSliceModel();

  ///
  /// Get  all slice displaynodes creating PolyData models like glyphs etc.
  std::vector< vtkMRMLDisplayNode*> GetPolyDataDisplayNodes();
  /// Return the associated slicerlayer nodes
  static vtkMRMLSliceCompositeNode* GetSliceCompositeNode(vtkMRMLSliceNode* node);

  /// Default node name suffix for use with volume slice models to distinguish them
  /// as built in models rather than user accessible.
  /// \sa IsSliceModelNode
  static const std::string SLICE_MODEL_NODE_NAME_SUFFIX;

  /// Return true if the node is a model node that has the default volume slice
  /// node name suffix, false otherwise
  /// \sa SLICE_MODEL_NODE_NAME_SUFFIX
  static bool IsSliceModelNode(vtkMRMLNode *mrmlNode);
  /// Return true if the display node is a volume slice node display node
  /// by checking the attribute SliceLogic.IsSliceModelDiplayNode
  /// Returns false if the attribute is not present, true if the attribute
  /// is present and not equal to zero
  static bool IsSliceModelDisplayNode(vtkMRMLDisplayNode *mrmlDisplayNode);

protected:

  vtkMRMLSliceLogic();
  virtual ~vtkMRMLSliceLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);

  ///
  /// process logic events
  virtual void ProcessMRMLLogicsEvents(vtkObject * caller,
                                       unsigned long event,
                                       void * callData);
  void ProcessMRMLLogicsEvents();

  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneStartClose();
  virtual void OnMRMLSceneEndImport();
  virtual void OnMRMLSceneEndRestore();

  void UpdateSliceNodes();
  void SetupCrosshairNode();

  virtual void OnMRMLNodeModified(vtkMRMLNode* node);
  static vtkMRMLSliceCompositeNode* GetSliceCompositeNode(vtkMRMLScene* scene,
                                                          const char* layoutName);

  bool                        AddingSliceModelNodes;
  bool                        Initialized;

  char *                      Name;
  vtkMRMLSliceNode *          SliceNode;
  vtkMRMLSliceCompositeNode * SliceCompositeNode;
  vtkMRMLSliceLayerLogic *    BackgroundLayer;
  vtkMRMLSliceLayerLogic *    ForegroundLayer;
  vtkMRMLSliceLayerLogic *    LabelLayer;


  vtkImageBlend *   Blend;
  vtkImageBlend *   BlendUVW;
  vtkImageReslice * ExtractModelTexture;
  vtkAlgorithmOutput *    ImageDataConnection;
  vtkTransform *    ActiveSliceTransform;

  vtkPolyDataCollection * PolyDataCollection;
  vtkCollection *         LookupTableCollection;

  vtkMRMLModelNode *            SliceModelNode;
  vtkMRMLModelDisplayNode *     SliceModelDisplayNode;
  vtkMRMLLinearTransformNode *  SliceModelTransformNode;
  double                        SliceSpacing[3];

private:

  vtkMRMLSliceLogic(const vtkMRMLSliceLogic&);
  void operator=(const vtkMRMLSliceLogic&);

};

#endif
