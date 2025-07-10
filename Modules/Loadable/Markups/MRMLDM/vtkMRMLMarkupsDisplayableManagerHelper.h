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
/// .NAME vtkMRMLMarkupsDisplayableManagerHelper - utility class to manage widgets
/// .SECTION Description
/// This class defines lists that are used to manage the widgets associated with markups.
/// A markup which is managed by a displayableManager consists of
///   a) the Markups MRML Node (MarkupsNodeList)
///   b) the vtkWidget to show this markup (Widgets)
///   c) a vtkWidget to represent sliceIntersections in the slice viewers (WidgetIntersections)
///

#ifndef vtkMRMLMarkupsDisplayableManagerHelper_h
#define vtkMRMLMarkupsDisplayableManagerHelper_h

// MarkupsModule includes
#include "vtkSlicerMarkupsModuleMRMLDisplayableManagerExport.h"

// MarkupsModule/MRML includes
#include <vtkMRMLMarkupsNode.h>

// MarkupsModule/VTKWidgets includes
#include <vtkSlicerMarkupsInteractionWidget.h>
#include <vtkSlicerMarkupsWidget.h>

// VTK includes
#include <vtkSmartPointer.h>

// MRML includes
#include <vtkMRMLSliceNode.h>

// STL includes
#include <set>

class vtkMRMLMarkupsDisplayableManager;
class vtkMRMLMarkupsDisplayNode;
class vtkMRMLInteractionNode;

class VTK_SLICER_MARKUPS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLMarkupsDisplayableManagerHelper : public vtkObject
{
public:
  static vtkMRMLMarkupsDisplayableManagerHelper* New();
  vtkTypeMacro(vtkMRMLMarkupsDisplayableManagerHelper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkGetObjectMacro(DisplayableManager, vtkMRMLMarkupsDisplayableManager);
  void SetDisplayableManager(vtkMRMLMarkupsDisplayableManager*);

  /// Set all widget status to manipulate
  // void SetAllWidgetsToManipulate();

  /// Get a vtkSlicerMarkupsWidget* given a node
  vtkSlicerMarkupsWidget* GetWidget(vtkMRMLMarkupsDisplayNode* markupsDisplayNode);
  /// Get first visible widget for this markup
  vtkSlicerMarkupsWidget* GetWidget(vtkMRMLMarkupsNode* markupsNode);

  /// Get a vtkSlicerMarkupsWidget* given a node
  vtkSlicerMarkupsInteractionWidget* GetInteractionWidget(vtkMRMLMarkupsDisplayNode* markupsDisplayNode);
  /// Get first visible widget for this markup
  vtkSlicerMarkupsInteractionWidget* GetInteractionWidget(vtkMRMLMarkupsNode* markupsNode);

  /// Remove all widgets, intersection widgets, nodes
  void RemoveAllWidgetsAndNodes();

  /// Map of vtkWidget indexed using associated node ID
  typedef std::map<vtkSmartPointer<vtkMRMLMarkupsDisplayNode>, vtkSlicerMarkupsWidget*> DisplayNodeToWidgetType;
  typedef std::map<vtkSmartPointer<vtkMRMLMarkupsDisplayNode>, vtkSlicerMarkupsWidget*>::iterator DisplayNodeToWidgetIt;
  DisplayNodeToWidgetType MarkupsDisplayNodesToWidgets; // display nodes with widgets assigned

  typedef std::map<vtkSmartPointer<vtkMRMLMarkupsDisplayNode>, vtkSlicerMarkupsInteractionWidget*>
    DisplayNodeToInteractionWidgetType;
  typedef std::map<vtkSmartPointer<vtkMRMLMarkupsDisplayNode>, vtkSlicerMarkupsInteractionWidget*>::iterator
    DisplayNodeToInteractionWidgetIt;
  DisplayNodeToInteractionWidgetType MarkupsDisplayNodesToInteractionWidgets; // display nodes with widgets assigned

  typedef std::set<vtkSmartPointer<vtkMRMLMarkupsNode>> MarkupsNodesType;
  typedef std::set<vtkSmartPointer<vtkMRMLMarkupsNode>>::iterator MarkupsNodesIt;
  MarkupsNodesType MarkupsNodes; // observed markups nodes

  void AddMarkupsNode(vtkMRMLMarkupsNode* node);
  void RemoveMarkupsNode(vtkMRMLMarkupsNode* node);
  void AddDisplayNode(vtkMRMLMarkupsDisplayNode* displayNode);
  void AddWidget(vtkMRMLMarkupsDisplayNode* displayNode);
  void AddInteractionWidget(vtkMRMLMarkupsDisplayNode* displayNode);
  void RemoveDisplayNode(vtkMRMLMarkupsDisplayNode* displayNode);

  void DeleteWidget(vtkSlicerMarkupsWidget* widget);
  void DeleteInteractionWidget(vtkSlicerMarkupsInteractionWidget* widget);

  void AddObservations(vtkMRMLMarkupsNode* node);
  void RemoveObservations(vtkMRMLMarkupsNode* node);

protected:
  vtkMRMLMarkupsDisplayableManagerHelper();
  ~vtkMRMLMarkupsDisplayableManagerHelper() override;

private:
  vtkMRMLMarkupsDisplayableManagerHelper(const vtkMRMLMarkupsDisplayableManagerHelper&) = delete;
  void operator=(const vtkMRMLMarkupsDisplayableManagerHelper&) = delete;

  /// Keep a record of the current glyph type for the handles in the widget
  /// associated with this node, prevents changing them unnecessarily
  std::map<vtkMRMLNode*, std::vector<int>> NodeGlyphTypes;

  bool AddingMarkupsNode;

  std::vector<unsigned long> ObservedMarkupNodeEvents;

  vtkMRMLMarkupsDisplayableManager* DisplayableManager;
};

#endif
