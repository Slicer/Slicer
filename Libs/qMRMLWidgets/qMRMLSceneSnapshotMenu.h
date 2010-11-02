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

#ifndef __qMRMLSceneSnapshotMenu_h
#define __qMRMLSceneSnapshotMenu_h

// Qt includes
#include <QMenu>

// CTK includes
#include <ctkVTKObject.h>

#include "qMRMLWidgetsExport.h"

class qMRMLSceneSnapshotMenuPrivate;
class vtkMRMLScene;
class vtkMRMLNode;

class QMRML_WIDGETS_EXPORT qMRMLSceneSnapshotMenu : public QMenu
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(QString NoSnapshotText READ noSnapshotText WRITE setNoSnapshotText)
public:
  /// Superclass typedef
  typedef QMenu Superclass;
  
  /// Constructors
  explicit qMRMLSceneSnapshotMenu(QWidget* newParent = 0);
  virtual ~qMRMLSceneSnapshotMenu();

  /// Return a pointer on the current MRML scene
  vtkMRMLScene* mrmlScene() const;

  /// This property holds the menu's text displayed when there is no snapshots.
  QString noSnapshotText()const;
  void setNoSnapshotText(const QString& newText);
  
public slots:

  /// Set the MRML \a scene associated with the widget
  virtual void setMRMLScene(vtkMRMLScene* scene);

signals:
  void mrmlSceneChanged(vtkMRMLScene*);

protected:
  QScopedPointer<qMRMLSceneSnapshotMenuPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSceneSnapshotMenu);
  Q_DISABLE_COPY(qMRMLSceneSnapshotMenu);

};

#endif

