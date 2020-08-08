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
#include <deque>

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
class vtkTransform;

struct SliceLayerInfo;
struct BlendPipeline;

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
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// CompositeModifiedEvent is generated when slice composite node is modified
  enum
    {
    CompositeModifiedEvent = 18000
    };

  enum
    {
    LayerNone = -1,
    LayerBackground = 0,
    LayerForeground = 1,
    LayerLabel = 2
    };

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
  /// The foreground slice layer
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
  /// Helper to get the background layer Window/Level, intensity range and
  /// status of automatic Window/Level setting
  void GetBackgroundWindowLevelAndRange(double& window, double& level,
                                      double& rangeLow, double& rangeHigh, bool& autoWindowLevel);

  ///
  /// Helper to get the background layer Window/Level and intensity range
  void GetBackgroundWindowLevelAndRange(double& window, double& level,
                                      double& rangeLow, double& rangeHigh);

  ///
  /// Helper to set the foreground layer Window/Level
  void SetForegroundWindowLevel(double window, double level);

  ///
  /// Helper to get the foreground layer Window/Level, intensity range and
  /// status of automatic Window/Level setting
  void GetForegroundWindowLevelAndRange(double& window, double& level,
                                      double& rangeLow, double& rangeHigh, bool& autoWindowLevel);

  ///
  /// Helper to get the foreground layer Window/Level and intensity range
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
  vtkImageBlend* GetBlend();
  vtkImageBlend* GetBlendUVW();

  ///
  /// An image reslice instance to pull a single slice from the volume that
  /// represents the filmsheet display output
  vtkGetObjectMacro(ExtractModelTexture, vtkImageReslice);

  ///
  /// the tail of the pipeline
  /// -- returns nullptr if none of the inputs exist
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
  /// Manage and synchronise the SliceNode
  void UpdateSliceNode();

  ///
  /// Update slicer node given a layout name
  void UpdateSliceNodeFromLayout();

  ///
  /// Manage and synchronise the SliceCompositeNode
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
  /// If useVoxelCenter is set to false (default) then bounds of voxel sides are returned
  /// (otherwise then bounds of voxels centers are returned).
  void GetVolumeSliceBounds(vtkMRMLVolumeNode *volumeNode, double sliceBounds[6], bool useVoxelCenter=false);

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
  /// Get the spacing of the lowest volume layer (background, foreground, label),
  /// transformed to slice space
  /// - to be used, for example, to set the slice increment for stepping a single
  ///   voxel relative to the current slice view
  /// - returns first non-null layer
  double *GetLowestVolumeSliceSpacing();

  ///
  /// Get the min/max bounds of the lowest volume layer (background, foreground, label)
  /// - note these are not translated by the current slice offset so they can
  ///   be used to calculate the range (e.g. of a slider) that operates in slice space
  /// - returns first non-null layer
  /// If useVoxelCenter is set to false (default) then bounds of voxel sides are returned
  /// (otherwise then bounds of voxels centers are returned).
  void GetLowestVolumeSliceBounds(double sliceBounds[6], bool useVoxelCenter=false);

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
  /// Return the associated slice node
  static vtkMRMLSliceNode* GetSliceNode(vtkMRMLSliceCompositeNode* node);

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
  ~vtkMRMLSliceLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene * newScene) override;

  ///
  /// process logic events
  void ProcessMRMLLogicsEvents(vtkObject * caller,
                                       unsigned long event,
                                       void * callData) override;
  void ProcessMRMLLogicsEvents();

  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneStartClose() override;
  void OnMRMLSceneEndImport() override;
  void OnMRMLSceneEndRestore() override;

  void UpdateSliceNodes();
  void SetupCrosshairNode();

  void OnMRMLNodeModified(vtkMRMLNode* node) override;
  static vtkMRMLSliceCompositeNode* GetSliceCompositeNode(vtkMRMLScene* scene,
                                                          const char* layoutName);
  static vtkMRMLSliceNode* GetSliceNode(vtkMRMLScene* scene,
    const char* layoutName);

  ///
  /// Helper to set Window/Level in any layer
  void SetWindowLevel(double window, double level, int layer);

  /// Helper to update input of blend filter from a set of layers.
  /// It minimizes changes to the imaging pipeline (does not remove and
  /// re-add an input if it is not changed) because rebuilding of the pipeline
  /// is a relatively expensive operation.
  bool UpdateBlendLayers(vtkImageBlend* blend, const std::deque<SliceLayerInfo> &layers);

  bool                        AddingSliceModelNodes;
  bool                        Initialized;

  char *                      Name;
  vtkMRMLSliceNode *          SliceNode;
  vtkMRMLSliceCompositeNode * SliceCompositeNode;
  vtkMRMLSliceLayerLogic *    BackgroundLayer;
  vtkMRMLSliceLayerLogic *    ForegroundLayer;
  vtkMRMLSliceLayerLogic *    LabelLayer;

  BlendPipeline* Pipeline;
  BlendPipeline* PipelineUVW;
  vtkImageReslice * ExtractModelTexture;
  vtkAlgorithmOutput *    ImageDataConnection;
  vtkTransform *    ActiveSliceTransform;

  vtkMRMLModelNode *            SliceModelNode;
  vtkMRMLModelDisplayNode *     SliceModelDisplayNode;
  vtkMRMLLinearTransformNode *  SliceModelTransformNode;
  double                        SliceSpacing[3];

private:

  vtkMRMLSliceLogic(const vtkMRMLSliceLogic&) = delete;
  void operator=(const vtkMRMLSliceLogic&) = delete;

};

#endif
