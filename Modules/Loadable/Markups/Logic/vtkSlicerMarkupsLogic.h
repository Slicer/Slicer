/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkSlicerMarkupsLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerMarkupsLogic_h
#define __vtkSlicerMarkupsLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerMarkupsModuleLogicExport.h"

class vtkMRMLMarkupsNode;
class vtkMRMLMarkupsDisplayNode;

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_LOGIC_EXPORT vtkSlicerMarkupsLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerMarkupsLogic *New();
  vtkTypeMacro(vtkSlicerMarkupsLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ProcessMRMLNodesEvents(vtkObject *caller,
                                      unsigned long event,
                                      void *callData );

  /// Utility method to return the id of the selection node. Checks
  /// the mrml application logic if set, otherwise checks the scene
  /// for a singleton selection node.
  /// Returns an empty string on failure.
  std::string GetSelectionNodeID();

  /// Utility method to return the id of the active place node.
  /// Returns an empty string on failure.
  /// \sa SetActiveListID
  std::string GetActiveListID();

  /// Utility method to set the active place node from the passed markups
  /// node. Does not set the interaction mode to place.
  /// \sa GetActiveListID, StartPlaceMode
  /// \sa vtkMRMLSelectionNode::SetReferenceActivePlaceNodeClassName
  /// \sa vtkMRMLSelectionNode::SetActivePlaceNodeID
  void SetActiveListID(vtkMRMLMarkupsNode *markupsNode);

  /// Create a new display node and observe it on the markups node.
  /// On success, return the id, on failure return an empty string.
  /// If a display node already exists for this node, return the id of that
  /// node.
  std::string AddNewDisplayNodeForMarkupsNode(vtkMRMLNode *mrmlNode);

  /// Create a new markups fiducial node and associated display node, adding both to
  /// the scene. If the scene argument is null use the scene set on the logic
  /// class, and also make it the active on on the selection node, otherwise
  /// add to the passed scene.
  /// On success, return the id, on failure return an empty string.
  std::string AddNewFiducialNode(const char *name = "F", vtkMRMLScene *scene = NULL);

  /// Add a new fiducial to the currently active list at the given RAS
  /// coordinates (default 0,0,0). Will create a list is one is not active.
  /// Returns -1 on failure, index of the added fiducial
  /// on success.
  int AddFiducial(double r=0.0, double a=0.0, double s=0.0);

  /// jump the slice windows to the given coordinate
  void JumpSlicesToLocation(double x, double y, double z, bool centered);
  /// jump the slice windows to the nth markup with the mrml id id
  void JumpSlicesToNthPointInMarkup(const char *id, int n, bool centered = false);

  /// Load a markups fiducial list from fileName, return NULL on error, node ID string
  /// otherwise. Adds the appropriate storage and display nodes to the scene
  /// as well.
  char *LoadMarkupsFiducials(const char *fileName, const char *fidsName);

  /// Utility methods to operate on all markups in a markups node
  void SetAllMarkupsVisibility(vtkMRMLMarkupsNode *node, bool flag);
  void ToggleAllMarkupsVisibility(vtkMRMLMarkupsNode *node);
  void SetAllMarkupsLocked(vtkMRMLMarkupsNode *node, bool flag);
  void ToggleAllMarkupsLocked(vtkMRMLMarkupsNode *node);
  void SetAllMarkupsSelected(vtkMRMLMarkupsNode *node, bool flag);
  void ToggleAllMarkupsSelected(vtkMRMLMarkupsNode *node);

  /// set/get the default markups display node settings
  int GetDefaultMarkupsDisplayNodeGlyphType();
  void SetDefaultMarkupsDisplayNodeGlyphType(int glyphType);
  void SetDefaultMarkupsDisplayNodeGlyphTypeFromString(const char *glyphType);
  std::string GetDefaultMarkupsDisplayNodeGlyphTypeAsString();

  double GetDefaultMarkupsDisplayNodeGlyphScale();
  void SetDefaultMarkupsDisplayNodeGlyphScale(double scale);

  double GetDefaultMarkupsDisplayNodeTextScale();
  void SetDefaultMarkupsDisplayNodeTextScale(double scale);

  double GetDefaultMarkupsDisplayNodeOpacity();
  void SetDefaultMarkupsDisplayNodeOpacity(double opacity);

  double *GetDefaultMarkupsDisplayNodeColor();
  void SetDefaultMarkupsDisplayNodeColor(double *color);
  void SetDefaultMarkupsDisplayNodeColor(double r, double g, double b);

  double *GetDefaultMarkupsDisplayNodeSelectedColor();
  void SetDefaultMarkupsDisplayNodeSelectedColor(double *color);
  void SetDefaultMarkupsDisplayNodeSelectedColor(double r, double g, double b);

  int GetDefaultMarkupsDisplayNodeSliceProjection();
  void SetDefaultMarkupsDisplayNodeSliceProjection(int projection);

  double *GetDefaultMarkupsDisplayNodeSliceProjectionColor();
  void SetDefaultMarkupsDisplayNodeSliceProjectionColor(double *color);
  void SetDefaultMarkupsDisplayNodeSliceProjectionColor(double r, double g, double b);

  double GetDefaultMarkupsDisplayNodeSliceProjectionOpacity();
  void SetDefaultMarkupsDisplayNodeSliceProjectionOpacity(double opacity);

  /// utility method to set up a display node from the defaults
  void SetDisplayNodeToDefaults(vtkMRMLMarkupsDisplayNode *displayNode);

  /// utility method to copy a markup from one list to another, adding it
  /// to the end of the new list
  /// \sa vtkMRMLMarkupsNode::AddMarkup
  /// Returns true on success, false on failure
  bool CopyNthMarkupToNewList(int n, vtkMRMLMarkupsNode *markupsNode,
                              vtkMRMLMarkupsNode *newMarkupsNode);

  /// utility method to move a markup from one list to another, trying to
  /// insert it at the given new index. If the new index is larger than the
  /// number of markups in the list, adds it to the end. If new index is
  /// smaller than 0, adds it at the beginning. Otherwise inserts at
  /// that index.
  /// \sa vtkMRMLMarkupsNode::InsertMarkup
  /// Returns true on success, false on failure
  bool MoveNthMarkupToNewListAtIndex(int n, vtkMRMLMarkupsNode *markupsNode,
                                   vtkMRMLMarkupsNode *newMarkupsNode, int newIndex);

  /// Searches the scene for annotation fidicual nodes, collecting a list
  /// of annotation hierarchy nodes. Then iterates through those heirarchy nodes
  /// and moves the fiducials that are under them into new markups nodes. Leaves
  /// the top level heirarchy nodes intact as they may be parents to ruler or
  /// ROIs but deletes the 1:1 hierarchy nodes.
  void ConvertAnnotationFiducialsToMarkups();

  /// Iterate over the markups in the list and reset the markup labels using
  /// the current MarkupLabelFormat setting. Try to keep current numbering.
  /// Will work if there's a %d, %g or %f in the format string, but precision
  /// is not handled.
  void RenameAllMarkupsFromCurrentFormat(vtkMRMLMarkupsNode *markupsNode);

  /// Put the interaction node into place mode, and set the persistence of
  /// place mode according to the persistent flag.
  /// Return true on sucessfully going into place mode, false otherwise.
  /// \sa SetActiveIDList
  bool StartPlaceMode(bool persistent);

  /// Inspect all the slice composite nodes in the scene. Return 1 if all have
  /// SliceIntersectionVisibility set to true, 0 if all have it set to false,
  /// 2 if it's a combination of true and false, -1 on error
  int GetSliceIntersectionsVisibility();
  /// Set the slice intersections visbility on all the slice composite nodes
  /// in the scene
  void SetSliceIntersectionsVisibility(bool flag);

protected:
  vtkSlicerMarkupsLogic();
  virtual ~vtkSlicerMarkupsLogic();

  /// Initialize listening to MRML events
  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);
  virtual void ObserveMRMLScene();

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

private:

  vtkSlicerMarkupsLogic(const vtkSlicerMarkupsLogic&); // Not implemented
  void operator=(const vtkSlicerMarkupsLogic&);               // Not implemented

  /// keep a markups display node with default values that can be updated from
  /// the application settings
  vtkMRMLMarkupsDisplayNode *DefaultMarkupsDisplayNode;
};

#endif
