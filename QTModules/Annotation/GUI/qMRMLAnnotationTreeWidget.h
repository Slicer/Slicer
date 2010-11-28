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

#ifndef __qMRMLAnnotationTreeWidget_h
#define __qMRMLAnnotationTreeWidget_h

// Qt includes
#include <QTreeView>

// CTK includes
#include <ctkPimpl.h>

// qMRML includes
#include "qMRMLTreeWidget.h"

#include "qSlicerAnnotationModuleExport.h"


class qMRMLSortFilterProxyModel;
class qMRMLAnnotationTreeWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLScene;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT qMRMLAnnotationTreeWidget : public qMRMLTreeWidget
{
  Q_OBJECT
  Q_PROPERTY (bool listenNodeModifiedEvent READ listenNodeModifiedEvent WRITE setListenNodeModifiedEvent)
public:
  qMRMLAnnotationTreeWidget(QWidget *parent=0);
  virtual ~qMRMLAnnotationTreeWidget();

  void hideScene();

public slots:
  void setMRMLScene(vtkMRMLScene* scene);

signals:
  void currentNodeChanged(vtkMRMLNode* node);

protected slots:
  void onActivated(const QModelIndex& index);

protected:
  QScopedPointer<qMRMLAnnotationTreeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLAnnotationTreeWidget);
  Q_DISABLE_COPY(qMRMLAnnotationTreeWidget);
};

#endif
