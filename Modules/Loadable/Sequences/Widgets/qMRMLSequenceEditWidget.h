/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qMRMLSequenceEditWidget_h
#define __qMRMLSequenceEditWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Slicer includes
#include "qMRMLWidget.h"

#include "qSlicerSequencesModuleWidgetsExport.h"

class QListWidgetItem;
class qMRMLSequenceEditWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLSequenceNode;

class Q_SLICER_MODULE_SEQUENCES_WIDGETS_EXPORT qMRMLSequenceEditWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(
    bool candidateNodesSectionVisible READ isCandidateNodesSectionVisible WRITE setCandidateNodesSectionVisible)

public:
  typedef qMRMLWidget Superclass;
  qMRMLSequenceEditWidget(QWidget* newParent = 0);
  ~qMRMLSequenceEditWidget() override;

  bool isCandidateNodesSectionVisible();

public slots:
  /// Reimplemented from qMRMLWidget
  void setMRMLScene(vtkMRMLScene* scene) override;

  vtkMRMLSequenceNode* mrmlSequenceNode();
  void setMRMLSequenceNode(vtkMRMLSequenceNode* sequenceNode);
  void setMRMLSequenceNode(vtkMRMLNode* sequenceNode);

  void setCandidateNodesSectionVisible(bool);

protected slots:
  void updateWidgetFromMRML();
  void updateCandidateNodesWidgetFromMRML(bool forceUpdate = false);

  void candidateNodeItemClicked(QListWidgetItem*);
  void candidateNodeItemDoubleClicked(QListWidgetItem*);
  // Returns the index value of the added candidate node
  QString onAddCurrentCandidateNode();
  void onDataNodeEdited(int row, int column);
  void onIndexNameEdited();
  void onIndexUnitEdited();
  void onIndexTypeEdited(QString indexTypeString);
  void onSequenceNodeModified();

  void onAddDataNodeButtonClicked();
  void onRemoveDataNodeButtonClicked();

  /// Respond to the scene events
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
  void onMRMLSceneEndImportEvent();
  void onMRMLSceneEndRestoreEvent();
  void onMRMLSceneEndBatchProcessEvent();
  void onMRMLSceneEndCloseEvent();

protected:
  QScopedPointer<qMRMLSequenceEditWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSequenceEditWidget);
  Q_DISABLE_COPY(qMRMLSequenceEditWidget);
};

#endif
