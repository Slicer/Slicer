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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerWidget_h
#define __qSlicerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

#include "qSlicerObject.h"
#include "qSlicerBaseQTGUIExport.h"

class vtkMRMLScene;
class QScrollArea;
class qSlicerWidgetPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerWidget : public QWidget, public virtual qSlicerObject
{
  Q_OBJECT
  QVTK_OBJECT
public:
  qSlicerWidget(QWidget *parent=0, Qt::WindowFlags f=0);
  virtual ~qSlicerWidget();

  ///
  /// Convenient windows to return parent widget or Null if any
  QWidget* parentWidget();

  ///
  /// If possible, set the windowsFlags of the parent container.
  /// Otherwise, set the ones of the current widget
  void setWindowFlags(Qt::WindowFlags type);

  ///
  /// Tell if the parent container is a QScrollArea
  bool isParentContainerScrollArea()const;

  ///
  /// Convenient method to Set/Get the parent container as a QScrollArea
  /// Note: Method mainly used while porting the application from KwWidget to Qt
  QScrollArea* getScrollAreaParentContainer()const;
  void setScrollAreaAsParentContainer(bool enable);

  ///
  /// If possible, set parent container geometry otherwise set widget geometry
  void setParentGeometry(int ax, int ay, int aw, int ah);

public slots:
  ///
  virtual void setParentVisible(bool visible);
  virtual void setMRMLScene(vtkMRMLScene* scene);
signals:
  void mrmlSceneChanged(vtkMRMLScene*);

protected:
  QScopedPointer<qSlicerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerWidget);
  Q_DISABLE_COPY(qSlicerWidget);
};

#endif
