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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSceneViewMenu_h
#define __qMRMLSceneViewMenu_h

// Qt includes
#include <QMenu>

// CTK includes
#include <ctkVTKObject.h>

#include "qMRMLWidgetsExport.h"

class qMRMLSceneViewMenuPrivate;
class vtkMRMLScene;
class vtkMRMLNode;

class QMRML_WIDGETS_EXPORT qMRMLSceneViewMenu : public QMenu
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(QString NoSceneViewText READ noSceneViewText WRITE setNoSceneViewText)
public:
  /// Superclass typedef
  typedef QMenu Superclass;

  /// Constructors
  explicit qMRMLSceneViewMenu(QWidget* newParent = nullptr);
  ~qMRMLSceneViewMenu() override;

  /// Return a pointer on the current MRML scene
  vtkMRMLScene* mrmlScene() const;

  /// This property holds the menu's text displayed when there are no scene views
  QString noSceneViewText()const;
  void setNoSceneViewText(const QString& newText);

public slots:

  /// Set the MRML \a scene associated with the widget
  virtual void setMRMLScene(vtkMRMLScene* scene);

signals:
  void mrmlSceneChanged(vtkMRMLScene*);

protected:
  QScopedPointer<qMRMLSceneViewMenuPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSceneViewMenu);
  Q_DISABLE_COPY(qMRMLSceneViewMenu);

};

#endif
