/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLTreeWidget_h
#define __qMRMLTreeWidget_h

// Qt includes
#include <QTreeView>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class qMRMLSortFilterProxyModel;
class qMRMLTreeWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLScene;

class QMRML_WIDGETS_EXPORT qMRMLTreeWidget : public QTreeView
{
  Q_OBJECT
  Q_PROPERTY (bool listenNodeModifiedEvent READ listenNodeModifiedEvent WRITE setListenNodeModifiedEvent)
public:
  qMRMLTreeWidget(QWidget *parent=0);
  virtual ~qMRMLTreeWidget();

  vtkMRMLScene* mrmlScene()const;

  /// \sa qMRMLSceneModel::setListenNodeModifiedEvent
  void setListenNodeModifiedEvent(bool listen);
  /// \sa qMRMLSceneModel::listenNodeModifiedEvent
  bool listenNodeModifiedEvent()const;

  ///
  /// Retrieve the sortFilterProxyModel used to filter/sort
  /// the nodes
  qMRMLSortFilterProxyModel* sortFilterProxyModel()const;

public slots:
  void setMRMLScene(vtkMRMLScene* scene);

signals:
  void currentNodeChanged(vtkMRMLNode* node);

protected slots:
  void onActivated(const QModelIndex& index);
protected:
  virtual void updateGeometries();
protected:
  QScopedPointer<qMRMLTreeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLTreeWidget);
  Q_DISABLE_COPY(qMRMLTreeWidget);
};

#endif
