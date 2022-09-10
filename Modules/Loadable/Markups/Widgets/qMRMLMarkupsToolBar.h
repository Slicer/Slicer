/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qMRMLMarkupsToolBar_h
#define __qMRMLMarkupsToolBar_h

// Qt includes
#include <QSignalMapper>
#include <QToolBar>
#include <QMenu>
#include <QCheckBox>


// CTK includes
#include <ctkPimpl.h>
// no ui begin
#include <ctkVTKObject.h>
// no ui end

// qMRMLWidget includes
#include "qMRMLWidget.h"
#include "qSlicerMarkupsModuleWidgetsExport.h"

// MRML includes
#include <vtkMRMLScene.h>

// MRML includes
#include <vtkSlicerMarkupsLogic.h>

class qMRMLMarkupsToolBarPrivate;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLMarkupsNode;
class vtkMRMLInteractionNode;
class vtkMRMLSelectionNode;
class vtkSlicerApplicationLogic;

class QAction;
class QActionGroup;
class QToolButton;

class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT qMRMLMarkupsToolBar : public QToolBar
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef QToolBar Superclass;

  /// Constructor
  /// Title is the name of the toolbar (can appear using right click on the toolbar area)
  qMRMLMarkupsToolBar(const QString& title, QWidget* parent = 0);
  qMRMLMarkupsToolBar(QWidget* parent = 0);
  ~qMRMLMarkupsToolBar() override;

  vtkMRMLMarkupsNode* activeMarkupsNode();
  Q_INVOKABLE vtkMRMLInteractionNode* interactionNode()const;
  Q_INVOKABLE vtkMRMLSelectionNode* selectionNode()const;

  void initializeToolBarLayout();
  void addCreateNodeShortcut(QString keySequence);
  void addTogglePersistenceShortcut(QString keySequence);
  void addPlacePointShortcut(QString keySequence);

public slots:
  void setApplicationLogic(vtkSlicerApplicationLogic* logic);
  virtual void setMRMLScene(vtkMRMLScene* newScene);
  void updateToolBarLayout();

  /// Set the currently selected markups node.
  void setActiveMarkupsNode(vtkMRMLMarkupsNode* newActiveMarkupsNode);
  void setPersistence(bool persistent);
  void setInteractionNode(vtkMRMLInteractionNode* interactionNode);
  void interactionModeActionTriggered(bool);
  void setSelectionNode(vtkMRMLSelectionNode* selectionNode);

    /// Create markup by class.
  void onAddNewMarkupsNodeByClass(const QString& className);

  // Keyboard shortcuts for Markups node interactions
  void onCreateNodeShortcut();
  void onTogglePersistenceShortcut();
  void onPlacePointShortcut();

signals:
  void mrmlSceneChanged(vtkMRMLScene*);
  void activeMarkupsNodeChanged(vtkMRMLNode* activeMarkupsNode);
  void activeMarkupsPlaceModeChanged(bool enabled);

protected slots:
  /// Update the widget when a different markups node is selected by the combo box.
  void onMarkupsNodeChanged(vtkMRMLNode*);

protected:
  QScopedPointer<qMRMLMarkupsToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLMarkupsToolBar);
  Q_DISABLE_COPY(qMRMLMarkupsToolBar);
};

#endif
