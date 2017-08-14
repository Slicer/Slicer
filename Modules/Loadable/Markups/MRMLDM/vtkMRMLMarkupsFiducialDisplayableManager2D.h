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

#ifndef __vtkMRMLMarkupsFiducialDisplayableManager2D_h
#define __vtkMRMLMarkupsFiducialDisplayableManager2D_h

// MarkupsModule includes
#include "vtkSlicerMarkupsModuleMRMLDisplayableManagerExport.h"

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsDisplayableManager2D.h"

class vtkMRMLMarkupsFiducialNode;
class vtkSlicerViewerWidget;
class vtkMRMLMarkupsDisplayNode;
class vtkTextWidget;

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLMarkupsFiducialDisplayableManager2D :
    public vtkMRMLMarkupsDisplayableManager2D
{
public:

  static vtkMRMLMarkupsFiducialDisplayableManager2D *New();
  vtkTypeMacro(vtkMRMLMarkupsFiducialDisplayableManager2D, vtkMRMLMarkupsDisplayableManager2D);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /// Update a single seed position from the node, return true if the position changed
  virtual bool UpdateNthSeedPositionFromMRML(int n, vtkAbstractWidget *widget, vtkMRMLMarkupsNode *pointsNode) VTK_OVERRIDE;

  /// Update a single markup position from the seed widget, return true if the position changed
  virtual bool UpdateNthMarkupPositionFromWidget(int n, vtkMRMLMarkupsNode* pointsNode, vtkAbstractWidget * widget) VTK_OVERRIDE;

protected:

  vtkMRMLMarkupsFiducialDisplayableManager2D(){this->Focus="vtkMRMLMarkupsFiducialNode";}
  virtual ~vtkMRMLMarkupsFiducialDisplayableManager2D(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID) VTK_OVERRIDE;
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLMarkupsNode* node) VTK_OVERRIDE;
  /// Create new handle on widget when a new markup is added to a markups node
  virtual void OnMRMLMarkupsNodeMarkupAddedEvent(vtkMRMLMarkupsNode * markupsNode, int n) VTK_OVERRIDE;
  /// Respond to the nth markup modified event
  virtual void OnMRMLMarkupsNodeNthMarkupModifiedEvent(vtkMRMLMarkupsNode * markupsNode, int n) VTK_OVERRIDE;
  /// Respond to a markup being removed from the markups node
  virtual void OnMRMLMarkupsNodeMarkupRemovedEvent(vtkMRMLMarkupsNode * markupsNode, int n) VTK_OVERRIDE;

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLMarkupsNode * node) VTK_OVERRIDE;

  /// Update a single seed from MRML
  void SetNthSeed(int n, vtkMRMLMarkupsFiducialNode* fiducialNode, vtkSeedWidget *seedWidget);
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLMarkupsNode* node, vtkAbstractWidget * widget) VTK_OVERRIDE;

  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLMarkupsNode* node) VTK_OVERRIDE;

  /// Set up an observer on the interactor style to watch for key press events
  virtual void AdditionnalInitializeStep();
  /// Respond to the interactor style event
  virtual void OnInteractorStyleEvent(int eventid) VTK_OVERRIDE;

  /// Respond to control point modified events
  virtual void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node) VTK_OVERRIDE;

  // Clean up when scene closes
  virtual void OnMRMLSceneEndClose() VTK_OVERRIDE;

private:

  vtkMRMLMarkupsFiducialDisplayableManager2D(const vtkMRMLMarkupsFiducialDisplayableManager2D&); /// Not implemented
  void operator=(const vtkMRMLMarkupsFiducialDisplayableManager2D&); /// Not Implemented

};

#endif
