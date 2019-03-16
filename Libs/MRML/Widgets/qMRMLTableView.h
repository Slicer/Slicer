/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef __qMRMLTableView_h
#define __qMRMLTableView_h

// Qt includes
#include <QTableView>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class QSortFilterProxyModel;
class qMRMLTableViewPrivate;
class qMRMLTableModel;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLTableNode;
class vtkMRMLTableViewNode;

/// \brief Spreadsheet view for table nodes.
/// Allow view/edit of a vtkMRMLTableNode.
class QMRML_WIDGETS_EXPORT qMRMLTableView : public QTableView
{
  Q_OBJECT
  Q_PROPERTY(bool transposed READ transposed WRITE setTransposed)
  Q_PROPERTY(bool firstRowLocked READ firstRowLocked WRITE setFirstRowLocked)
  Q_PROPERTY(bool firstColumnLocked READ firstColumnLocked WRITE setFirstColumnLocked)
public:
  qMRMLTableView(QWidget *parent=nullptr);
  ~qMRMLTableView() override;

  /// Return a pointer on the current MRML scene
  Q_INVOKABLE vtkMRMLScene* mrmlScene() const;

  /// Get the TableView node observed by view.
  Q_INVOKABLE vtkMRMLTableViewNode* mrmlTableViewNode()const;

  Q_INVOKABLE vtkMRMLTableNode* mrmlTableNode()const;

  Q_INVOKABLE qMRMLTableModel* tableModel()const;
  Q_INVOKABLE QSortFilterProxyModel* sortFilterProxyModel()const;

  bool transposed()const;
  bool firstRowLocked()const;
  bool firstColumnLocked()const;

  QList<int> selectedMRMLTableColumnIndices()const;

public slots:
  /// Set the MRML \a scene that should be listened for events.
  /// If scene is set then MRMLTableViewNode has to be set, too.
  /// If scene is set then scene has to be set before MRMLTableNode is set.
  void setMRMLScene(vtkMRMLScene* newScene);

  /// Set the current \a viewNode to observe. If nullptr then view properties are not stored in the scene.
  void setMRMLTableViewNode(vtkMRMLTableViewNode* newTableViewNode);

  void setMRMLTableNode(vtkMRMLTableNode* tableNode);
  /// Utility function to simply connect signals/slots with Qt Designer
  void setMRMLTableNode(vtkMRMLNode* tableNode);

  /// Set transposed flag.
  /// If transposed is true then columns of the MRML table are added as rows in the model.
  /// This affects only this particular view, the settings is not stored in MRML.
  void setTransposed(bool transposed);

  void setFirstRowLocked(bool locked);
  void setFirstColumnLocked(bool locked);

  void copySelection();
  void pasteSelection();
  void plotSelection();

  void insertRow();
  void insertColumn();

  void deleteRow();
  void deleteColumn();

signals:

  /// When designing custom qMRMLWidget in the designer, you can connect the
  /// mrmlSceneChanged signal directly to the aggregated MRML widgets that
  /// have a setMRMLScene slot.
  void mrmlSceneChanged(vtkMRMLScene*);

  /// Emitted when a different table node is selected or different cells are selected.
  void selectionChanged();

protected:
  void keyPressEvent(QKeyEvent* event) override;

  QScopedPointer<qMRMLTableViewPrivate> d_ptr;

  void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected) override;

private:
  Q_DECLARE_PRIVATE(qMRMLTableView);
  Q_DISABLE_COPY(qMRMLTableView);
};

#endif
