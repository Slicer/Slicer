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

#ifndef __vtkMRMLMarkupsNode_h
#define __vtkMRMLMarkupsNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkVector.h>

class vtkStringArray;
class vtkMatrix4x4;

/// see doxygen enabled comment in class description
typedef struct
{
  std::string ID;
  std::string Label;
  std::string Description;
  std::string AssociatedNodeID;
  std::vector < vtkVector3d> points;
  double OrientationWXYZ[4];
  bool Selected;
  bool Locked;
  bool Visibility;
} Markup;

/// \brief MRML node to represent a list of markups
/// Markups nodes contains a list of markups that each contain a list of points.
/// Visualization parameters for these nodes are controlled by the
/// vtkMRMLMarkupsDisplayNode class.
/// Each markup has a unique ID.
/// Each markup is defined by a certain number of RAS points,
/// 1 for fiducials, 2 for rulers, 3 for angles, etc.
/// Each markup has an orientation defined by a quaternion. It's represented
/// by a 4 element vector: [0] = the angle of rotation, [1,2,3] = the axis of
/// rotation. Default is 0.0, 0.0, 0.0, 1.0
/// Each markup also has an associated node id, set when the markup
/// is placed on a data set to link the markup to the volume or model.
/// Each markup can also be individually un/selected, un/locked, in/visibile,
/// and have a label (short, shown in the viewers) and description (longer,
/// shown in the GUI).
/// \sa vtkMRMLMarkupsDisplayNode
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsNode : public vtkMRMLDisplayableNode
{
  /// Make the storage node a friend so that ReadDataInternal can set the markup
  /// ids
  friend class vtkMRMLMarkupsStorageNode;
  friend class vtkMRMLMarkupsFiducialStorageNode;

public:
  static vtkMRMLMarkupsNode *New();
  vtkTypeMacro(vtkMRMLMarkupsNode,vtkMRMLDisplayableNode);

  void PrintMarkup(ostream&  os, vtkIndent indent, Markup *markup);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char* GetIcon() {return "";};

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Markups";};

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Write this node's information to a vector of strings for passing to a CLI,
  /// precede each datum with the prefix if not an empty string
  /// coordinateSystemFlag = 0 for RAS, 1 for LPS
  /// multipleFlag = 1 for the whole list, 1 for the first selected markup
  virtual void WriteCLI(std::vector<std::string>& commandLine,
                        std::string prefix, int coordinateSystem = 0,
                        int multipleFlag = 1);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Currently only calls superclass UpdateScene
  void UpdateScene(vtkMRMLScene *scene);

  /// Alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );


  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  /// Access to a VTK string array, not currently used
  int AddText(const char *newText);
  void SetText(int id, const char *newText);
  vtkStdString GetText(int id);
  int DeleteText(int id);
  int GetNumberOfTexts();
  void RemoveAllTexts();

  /// Invoke events when markups change, passing the markup index if applicable.
  /// Invoke the lock modified event when a markup's lock status is changed.
  /// Invoke the label format modified event when markup label format changes.
  /// Invoke the point start interaction event when starting interacting with a point.
  /// Invoke the point modified event when a markup's location changes.
  /// Invoke the point end interaction event when an interaction process finishes.
  /// Invoke the point clicked event when user clicked a markup
  /// (pressed and released the moues button without moving the mouse cursor).
  /// Invoke the NthMarkupModifiedEvent event when a markup's non location value.
  /// Invoke the markup added event when adding a new markup to a markups node.
  /// Invoke the markup removed event when removing one or all markups from a node
  /// (caught by the displayable manager to make sure the widgets match the node).
  enum
  {
    LockModifiedEvent = 19000,
    LabelFormatModifiedEvent,
    PointModifiedEvent,
    PointStartInteractionEvent,
    PointEndInteractionEvent,
    PointClickedEvent,
    NthMarkupModifiedEvent,
    MarkupAddedEvent,
    MarkupRemovedEvent,
  };

  /// Clear out the node of all markups
  virtual void RemoveAllMarkups();

  /// Get the Locked property on the markup node/list of markups.
  vtkGetMacro(Locked, int);
  /// Set the Locked property on the markup node/list of markups
  /// If set to 1 then parameters should not be changed, and dragging the
  /// markups is disabled in 2d and 3d.
  /// Overrides the Locked flag on individual Markups in that when the node is
  /// set to be locked, all the markups in the list are locked. When the node
  /// is unlocked, use the locked flag on the individual markups to determine
  /// their locked state.
  void SetLocked(int locked);
  /// Get/Set the Locked property on the markup.
  /// If set to 1 then parameters should not be changed
  vtkBooleanMacro(Locked, int);

  /// Return true if n is a valid markup, false otherwise
  bool MarkupExists(int n);
  /// Return the number of markups that are stored in this node
  int GetNumberOfMarkups();
  /// Return true if p is a valid point in a valid markup n, false otherwise
  bool PointExistsInMarkup(int p, int n);
  /// Return the number of points in a markup, 0 if n is invalid
  int GetNumberOfPointsInNthMarkup(int n);
  /// Return a pointer to the nth markup stored in this node, null if n is out of bounds
  Markup * GetNthMarkup(int n);
  /// Initialise a markup to default values
  void InitMarkup(Markup *markup);
  /// Add a markup to the end of the list. Return index
  /// of new markup, -1 on failure.
  int AddMarkup(Markup markup);
  /// Create a new markup with n points.
  /// If point is specified then all markup positions will be initialized to that position,
  /// otherwise markup positions are initialized to (0,0,0).
  /// Return index of new markup, -1 on failure.
  int AddMarkupWithNPoints(int n, std::string label = std::string(), vtkVector3d* point = NULL);
  /// Create a new markup with one point.
  /// Return index of new markup, -1 on failure.
  int AddPointToNewMarkup(vtkVector3d point, std::string label = std::string());
  /// Create a new markup with one point, defined in the world coordinate system.
  /// Return index of new markup, -1 on failure.
  int AddPointWorldToNewMarkup(vtkVector3d point, std::string label = std::string());
  /// Add a point to the nth markup, returning the point index
  int AddPointToNthMarkup(vtkVector3d point, int n);

  /// Get the position of the pointIndex'th point in markupIndex markup,
  /// returning it as a vtkVector3d
  vtkVector3d GetMarkupPointVector(int markupIndex, int pointIndex);
  /// Get the position of the pointIndex'th point in markupIndex markup,
  /// setting the elements of point
  void GetMarkupPoint(int markupIndex, int pointIndex, double point[3]);
  /// Get points in LPS coordinate system
  void GetMarkupPointLPS(int markupIndex, int pointIndex, double point[3]);
  /// Return a three element double giving the world position (any parent
  /// transform on the markup applied to the return of GetMarkupPoint.
  /// Returns 0 on failure, 1 on success.
  int GetMarkupPointWorld(int markupIndex, int pointIndex, double worldxyz[4]);

  /// Remove a markup
  void RemoveMarkup(int m);

  /// Insert a markup in this list at targetIndex.
  /// If targetIndex is < 0, insert at the start of the list.
  /// If targetIndex is > list size - 1, append to end of list.
  /// Returns true on success, false on failure.
  bool InsertMarkup(Markup m, int targetIndex);

  /// Copy settings from source markup to target markup
  void CopyMarkup(Markup *source, Markup *target);

  /// Swap the position of two markups
  void SwapMarkups(int m1, int m2);

  /// Set a point in a markup from a pointer to an array
  /// \sa SetMarkupPoint
  void SetMarkupPointFromPointer(const int markupIndex, const int pointIndex, const double * pos);
  /// Set a point in a markup from an array
  /// \sa SetMarkupPoint
  void SetMarkupPointFromArray(const int markupIndex, const int pointIndex, const double pos[3]);
  /// Set a point in a markup from coordinates
  /// \sa SetMarkupPointFromPointer, SetMarkupPointFromArray
  void SetMarkupPoint(const int markupIndex, const int pointIndex, const double x, const double y, const double z);
  /// Set a point in a markup using LPS coordinate system, converting to RAS
  /// \sa SetMarkupPoint
  void SetMarkupPointLPS(const int markupIndex, const int pointIndex, const double x, const double y, const double z);
  /// Set the markupIndex markup's point pointIndex to xyz transformed
  /// by the inverse of the transform to world for the node.
  /// Calls SetMarkupPoint after transforming the passed in coordinate
  /// \sa SetMarkupPoint
  void SetMarkupPointWorld(const int markupIndex, const int pointIndex, const double x, const double y, const double z);

  /// Set the orientation for a markup from a pointer to a double array
  void SetNthMarkupOrientationFromPointer(int n, const double *orientation);
  /// Set the orientation for a markup from a double array
  void SetNthMarkupOrientationFromArray(int n, const double orientation[4]);
  /// Set the orientation for a markup from passed parameters
  void SetNthMarkupOrientation(int n, double w, double x, double y, double z);
  /// Get the orientation quaternion for a markup
  void GetNthMarkupOrientation(int n, double orientation[4]);

  /// Get/Set the associated node id for the nth markup
  std::string GetNthMarkupAssociatedNodeID(int n = 0);
  void SetNthMarkupAssociatedNodeID(int n, std::string id);

  /// Get the id for the nth markup
  std::string GetNthMarkupID(int n = 0);
  /// Get Markup index based on it's ID
  int GetMarkupIndexByID(const char* markupID);
  /// Get Markup based on it's ID
  Markup* GetMarkupByID(const char* markupID);

  /// Get the Selected flag on the nth markup, returns false if markup doesn't
  /// exist
  bool GetNthMarkupSelected(int n = 0);
  /// Set the Selected flag on the Nth markup
  /// \sa vtkMRMLNode::SetSelected
  void SetNthMarkupSelected(int n, bool flag);
  /// Get the Locked flag on the Nth markup, returns false if markup doesn't
  /// exist
  bool GetNthMarkupLocked(int n = 0);
  /// Set Locked property on Nth markup. If locked is set to
  /// true on the node/list as a whole, the nth markup locked flag is used to
  /// determine if it is locked. If the locked flag is set to false on the node
  /// as a whole, all markups are locked but keep this value for when the
  /// list as a whole is turned unlocked.
  /// \sa vtMRMLMarkupsNode::SetLocked
  void SetNthMarkupLocked(int n, bool flag);
  /// Get the Visibility flag of the Nth markup, returns false if markup doesn't
  /// exist
  bool GetNthMarkupVisibility(int n = 0);
  /// Set Visibility property on Nth markup. If the visibility is set to
  /// true on the node/list as a whole, the nth markup visibility is used to
  /// determine if it is visible. If the visibility is set to false on the node
  /// as a whole, all markups are hidden but keep this value for when the
  /// list as a whole is turned visible.
  /// \sa vtkMRMLDisplayableNode::SetDisplayVisibility
  /// \sa vtkMRMLDisplayNode::SetVisibility
  void SetNthMarkupVisibility(int n, bool flag);
  /// Get the Label on the nth markup, returns an empty string if the
  /// markup doesn't exist
  std::string GetNthMarkupLabel(int n = 0);
  /// Set the Label on the nth markup
  void SetNthMarkupLabel(int n, std::string label);
  /// Get the Description on the nth markup, returns an empty string if the
  /// markup doesn't exist
  std::string GetNthMarkupDescription(int n = 0);
  /// Set the Description on the nth markup
  void SetNthMarkupDescription(int n, std::string description);

  // Transform utility functions

  /// Returns true since can apply non linear transforms
  /// \sa ApplyTransform
  virtual bool CanApplyNonLinearTransforms()const;
  /// Apply the passed transformation to all of the markup points
  /// \sa CanApplyNonLinearTransforms
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  /// Get the markup label format string that defines the markup names.
  /// \sa SetMarkupLabelFormat
  std::string GetMarkupLabelFormat();
  /// Set the markup label format strign that defines the markup names,
  /// then invoke the LabelFormatModifedEvent
  /// In standard printf notation, with the addition of %N being replaced
  /// by the list name.
  /// %d will resolve to the highest not yet used list index integer.
  /// Character strings will otherwise pass through
  /// Defaults to %N-%d which will yield markup names of Name-0, Name-1,
  /// Name-2
  /// \sa GetMarkupLabelFormat
  void SetMarkupLabelFormat(std::string format);

  /// If the MarkupLabelFormat contains the string %N, return a string
  /// in which that has been replaced with the list name. If the list name is
  /// NULL, replace it with an empty string. If the MarkupLabelFormat doesn't
  /// contain %N, return MarkupLabelFormat
  std::string ReplaceListNameInMarkupLabelFormat();

  /// Reimplemented to take into account the modified time of the markups
  /// Returns true if the node (default behavior) or the markups are modified
  /// since read/written.
  /// Note: The MTime of the markups node is used to know if it has been modified.
  /// So if you invoke class specific modified events without calling Modified() on the
  /// markups, GetModifiedSinceRead() won't return true.
  /// \sa vtkMRMLStorableNode::GetModifiedSinceRead()
  virtual bool GetModifiedSinceRead();

  /// Reset the id of the nth markup according to the local policy
  /// Called after an already initialised markup has been added to the
  /// scene. Returns false if n out of bounds, true on success.
  bool ResetNthMarkupID(int n);

protected:
  vtkMRMLMarkupsNode();
  ~vtkMRMLMarkupsNode();
  vtkMRMLMarkupsNode(const vtkMRMLMarkupsNode&);
  void operator=(const vtkMRMLMarkupsNode&);

  vtkStringArray *TextList;

  /// Set the id of the nth markup.
  /// The goal is to keep this ID unique, so it's
  /// managed by the markups node.
  void SetNthMarkupID(int n, std::string id);

  /// Generate a scene unique ID for a markup. If the scene is not set,
  /// returns a number based on the max number of markups that
  /// have been in this list
  std::string GenerateUniqueMarkupID();;

private:
  /// Vector of point sets, each markup can have N markups of the same type
  /// saved in the vector.
  std::vector < Markup > Markups;

  int Locked;

  std::string MarkupLabelFormat;

  // Keep track of the number of markups that were added to the list, always
  // incrementing, not decreasing when they're removed. Used to help create
  // unique names and ids. Reset to 0 when \sa RemoveAllMarkups called
  int MaximumNumberOfMarkups;
};

#endif
