#ifndef __vtkSlicerAnnotationModuleLogic_h
#define __vtkSlicerAnnotationModuleLogic_h

// Slicer Logic includes
#include "vtkSlicerAnnotationsModuleLogicExport.h"
#include "vtkSlicerModuleLogic.h"

// MRML includes
class vtkMRMLAnnotationHierarchyNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkMRMLAnnotationNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationTextDisplayNode;

// VTK includes
#include <vtkStdString.h>

// STD includes
#include <string>

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_LOGIC_EXPORT vtkSlicerAnnotationModuleLogic
  :public vtkSlicerModuleLogic
{
public:
  enum Events{
    RefreshRequestEvent = vtkCommand::UserEvent,
    HierarchyNodeAddedEvent
  };
  static vtkSlicerAnnotationModuleLogic *New();
  vtkTypeMacro(vtkSlicerAnnotationModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Start the place mode for annotations.
  /// By default, the singleton interaction node is updated.
  void StartPlaceMode(bool persistent=false, vtkMRMLInteractionNode* interactionNode = nullptr);

  /// Exit the place mode for annotations.
  /// By default, the singleton interaction node is updated.
  void StopPlaceMode(bool persistent=false, vtkMRMLInteractionNode* interactionNode = nullptr);

  // Start adding a new annotation Node
  void AddAnnotationNode(const char * nodeDescriptor, bool persistent=false);

  // After a node was added, propagate to widget
  void AddNodeCompleted(vtkMRMLAnnotationNode* annotationNode);

  /// Cancel the current annotation placement or remove last annotation node.
  /// By default, the singleton interaction node is updated.
  void CancelCurrentOrRemoveLastAddedAnnotationNode(vtkMRMLInteractionNode* interactionNode = nullptr);

  /// Remove an AnnotationNode and also its 1-1 IS-A hierarchyNode, if found.
  void RemoveAnnotationNode(vtkMRMLAnnotationNode* annotationNode);

  //
  // Annotation Properties (interface to MRML)
  //
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;

  /// Check if node id corresponds to an annotaton node
  bool IsAnnotationNode(const char* id);

  bool IsAnnotationHierarchyNode(const char* id);

  /// Return the text display node for the annotation mrml node with this id,
  /// null if not a valid node, not an annotation node, or doesn't have a text
  /// display node
  vtkMRMLAnnotationTextDisplayNode *GetTextDisplayNode(const char *id);
  /// Return the point display node for the annotation mrml node with this id,
  /// null if not a valid node, not an annotation node, or doesn't have a point
  /// display node
  vtkMRMLAnnotationPointDisplayNode *GetPointDisplayNode(const char *id);
  /// Return the line display node for the annotation mrml node with this id,
  /// null if not a valid node, not an annotation node, or doesn't have a line
  /// display node
  vtkMRMLAnnotationLineDisplayNode *GetLineDisplayNode(const char *id);

  /// Get the name of an Annotation MRML node
  const char * GetAnnotationName(const char * id);

  /// Return the text of an Annotation MRML node
  vtkStdString GetAnnotationText(const char* id);
  /// Set the text of an Annotation MRML node
  void SetAnnotationText(const char* id, const char * newtext);

  /// Get the text scale of an Annotation MRML node
  double GetAnnotationTextScale(const char* id);
  /// Set the text scale of an Annotation MRML node
  void SetAnnotationTextScale(const char* id, double textScale);

  /// Get the selected text color of an Annotation MRML node
  double * GetAnnotationTextSelectedColor(const char* id);
  /// Set the selected text color of an Annotation MRML node
  void SetAnnotationTextSelectedColor(const char* id, double * color);

  /// Get the text color of an Annotation MRML node
  double * GetAnnotationTextUnselectedColor(const char* id);
  /// Set the text color of an Annotation MRML node
  void SetAnnotationTextUnselectedColor(const char* id, double * color);

  /// Get the color of an annotation mrml node, returns null if can't find it
  double * GetAnnotationColor(const char *id);
  /// Set the color of an annotation mrml node
  void SetAnnotationColor(const char *id, double *color);

  /// Get the unselected color of an annotation mrml node, returns null if can't find it
  double * GetAnnotationUnselectedColor(const char *id);
  /// Set the unselected color of an annotation mrml node
  void SetAnnotationUnselectedColor(const char *id, double *color);

  /// Get the point color of an annotation mrml node, returns null if can't find it
  double * GetAnnotationPointColor(const char *id);
  /// Set the point color of an annotation mrml node
  void SetAnnotationPointColor(const char *id, double *color);

  /// Get the unselected point color of an annotation mrml node, returns null if can't find it
  double * GetAnnotationPointUnselectedColor(const char *id);
  /// Set the unselected point color of an annotation mrml node
  void SetAnnotationPointUnselectedColor(const char *id, double *color);

  /// Get the point glyph type of the annotation mrml node as a string,
  /// returns null if can't find it
  const char * GetAnnotationPointGlyphTypeAsString(const char *id);
  /// Get the point glyph type of the annotation mrml node,
  int GetAnnotationPointGlyphType(const char *id);
  /// Set the point glyph type of the annotation mrml node from a string
  void SetAnnotationPointGlyphTypeFromString(const char *id, const char *glyphType);
  /// Set the point glyph type of the annotation mrml node
  void SetAnnotationPointGlyphType(const char *id, int glyphType);

  /// Get the line color of an annotation mrml node, returns null if can't find it
  double * GetAnnotationLineColor(const char *id);
  /// Set the line color of an annotation mrml node
  void SetAnnotationLineColor(const char *id, double *color);

  /// Get the unselected line color of an annotation mrml node, returns null if can't find it
  double * GetAnnotationLineUnselectedColor(const char *id);
  /// Set the unselected line color of an annotation mrml node
  void SetAnnotationLineUnselectedColor(const char *id, double *color);


  /// Get the measurement value of an Annotation MRML node
  const char * GetAnnotationMeasurement(const char * id, bool showUnits);

  /// Get the icon name of an Annotation MRML node
  const char * GetAnnotationIcon(const char * id);
  const char * GetAnnotationIcon(vtkMRMLNode* mrmlNode);

  /// Get the lock flag of an Annotation MRML node
  int GetAnnotationLockedUnlocked(const char * id);
  /// Toggle the lock flag of an Annotation MRML node
  void SetAnnotationLockedUnlocked(const char * id);

  /// Get the visibility flag of an Annotation MRML node
  int GetAnnotationVisibility(const char * id);
  /// Toggle the visibility flag of an Annotation MRML node
  void SetAnnotationVisibility(const char * id);

  /// Set the selected flag of an Annotation MRML node
  void SetAnnotationSelected(const char * id, bool selected);
  /// Set the selected flag of all annotation mrml nodes
  void SetAllAnnotationsSelected(bool selected);

  /// Backup an Annotation MRML node
  void BackupAnnotationNode(const char * id);
  /// Restore a backup of an Annotation MRML node
  void RestoreAnnotationNode(const char * id);
  /// Deletes a backup of an Annotation MRML node
  void DeleteBackupNodes(const char * id);

  /// Jump 2d Slices to the location of the first control point of an Annotation MRML node
  void JumpSlicesToAnnotationCoordinate(const char* id);

  const char * MoveAnnotationUp(const char* id);
  const char * MoveAnnotationDown(const char* id);

  //
  // SnapShot functionality
  //
  /// Create a snapShot.
  void CreateSnapShot(const char* name, const char* description, int screenshotType, double scaleFactor, vtkImageData* screenshot);

  /// Modify an existing snapShot.
  void ModifySnapShot(vtkStdString id, const char* name, const char* description, int screenshotType, double scaleFactor, vtkImageData* screenshot);

  /// Return the name of an existing annotation snapShot.
  vtkStdString GetSnapShotName(const char* id);

  /// Return the description of an existing annotation snapShot.
  vtkStdString GetSnapShotDescription(const char* id);

  /// Return the screenshotType of an existing annotation snapShot.
  int GetSnapShotScreenshotType(const char* id);

  /// Return the scaleFactor of an existing annotation snapShot.
  double GetSnapShotScaleFactor(const char* id);

  /// Return the screenshot of an existing annotation snapShot.
  vtkImageData* GetSnapShotScreenshot(const char* id);

  /// Check if node id corresponds to a snapShot node
  bool IsSnapshotNode(const char* id);

  //
  // Hierarchy functionality
  //
  /// Add a new visible annotation hierarchy.
  /// The active hierarchy node will be the parent. If there is no
  /// active hierarchy node, use the top-level annotation hierarchy node as the parent.
  /// If there is no top-level annotation hierarchy node, create additionally a top-level hierarchy node which serves as
  /// a parent to the new hierarchy node. The newly added hierarchy node will be the
  /// active hierarchy node. Return true on success, false on failure.
  bool AddHierarchy();

  /// Return the toplevel Annotation hierarchy node ID or create one and add it to the scene if there is none:
  /// If an optional annotationNode is given, insert the new toplevel hierarchy before it. If not,
  /// just add the new toplevel hierarchy node.
  char * GetTopLevelHierarchyNodeID(vtkMRMLNode* node=nullptr);

  /// Return the top level annotation hierarchy node for this node's class, adding one under the top level annotation hierarchy if there is none. If the passed in node is null, return null
  char * GetTopLevelHierarchyNodeIDForNodeClass(vtkMRMLAnnotationNode *annotationNode);

  /// Get the active hierarchy node which will be used as a parent for new annotations
  vtkMRMLAnnotationHierarchyNode *GetActiveHierarchyNode();

  /// get/set the id of the active hierarchy node
  vtkGetStringMacro(ActiveHierarchyNodeID);
  vtkSetStringMacro(ActiveHierarchyNodeID);

  /// Set the visible flag on all annotations in the hierarchy. If hierarchy is null, use active hierarchy
  void SetHierarchyAnnotationsVisibleFlag(vtkMRMLAnnotationHierarchyNode* hierarchyNode, bool flag);
  /// Set the lock flag on all annotations in the hierarchy. If hierarchy is null, use active hierarchy
  void SetHierarchyAnnotationsLockFlag(vtkMRMLAnnotationHierarchyNode* hierarchyNode, bool flag);

  //
  // Place Annotations programmatically
  //

  //
  // Report functionality
  //
  /// Return HTML markup for a specific annotation node
  const char* GetHTMLRepresentation(vtkMRMLAnnotationNode* annotationNode, int level);
  /// Return HTML markup for a specific hierarchy node
  const char* GetHTMLRepresentation(vtkMRMLAnnotationHierarchyNode* hierarchyNode, int level);


  /// Add a model display node to the scene for a passed in hierarchy node, if
  /// it doesn't already have one, return the ID, nullptr on failure
  const char* AddDisplayNodeForHierarchyNode(vtkMRMLAnnotationHierarchyNode *hnode);

  /// Legacy support: load a Slicer3 fiducial list from file. Uses FiducialsLogic to load into a legacy node first, then translates into annotation nodes
  /// returns a comma separated list of the annot nodes loaded
  char *LoadFiducialList(const char *filename);

  /// Load an annotation from file, return nullptr on error, node ID string
  /// otherwise. Adds the appropriate storage and display nodes to the scene
  /// as well. fileType is from this class's enum
  char *LoadAnnotation(const char *filename, const char *name, int fileType);
  /// Enumeration listing valid file types to load
  enum
  {
    None = 0,
    Fiducial,
    Ruler,
    ROI,
  };

  /// Add a new fiducial to the currently active hierarchy. Places the
  /// fiducial at the given RAS coordinates (default 0,0,0) with the
  /// given label (if nullptr, uses default naming convention).
  /// Returns the ID of the newly added node.
  char *AddFiducial(double r=0.0, double a=0.0, double s=0.0, const char *label=nullptr);

protected:

  vtkSlicerAnnotationModuleLogic();

  ~vtkSlicerAnnotationModuleLogic() override;

  // Initialize listening to MRML events
  void SetMRMLSceneInternal(vtkMRMLScene * newScene) override;
  void ObserveMRMLScene() override;

  // MRML events
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneEndClose() override;
  void ProcessMRMLNodesEvents(vtkObject *caller,
                                      unsigned long event,
                                      void *callData ) override;
  virtual void OnMRMLAnnotationNodeModifiedEvent(vtkMRMLNode* node);

private:

  vtkMRMLAnnotationNode* m_LastAddedAnnotationNode;

  std::string m_StringHolder;

  char* m_MeasurementFormat;
  char* m_CoordinateFormat;

  // id of the currently active hierarchy node
  char *ActiveHierarchyNodeID;

  //
  // Private hierarchy functionality.
  //
  /// Add a new annotation hierarchy node for a given annotationNode.
  /// If there is an optional annotationNode, insert the new hierarchy node before it else just add it.
  /// The active hierarchy node will be the parent. If there is no
  /// active hierarchy node, use the top-level annotation hierarchy node as the parent.
  /// If there is no top-level annotation hierarchy node, create additionally a top-level hierarchy node which serves as
  /// a parent to the new hierarchy node. Return true on success, false on failure.
  bool AddHierarchyNodeForAnnotation(vtkMRMLAnnotationNode* annotationNode=nullptr);

private:
  vtkSlicerAnnotationModuleLogic(const vtkSlicerAnnotationModuleLogic&) = delete;
  void operator=(const vtkSlicerAnnotationModuleLogic&) = delete;
};

#endif
