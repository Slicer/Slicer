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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLLayoutWidget_h
#define __qMRMLLayoutWidget_h

// Qt includes
#include <QWidget>

// qMRML includes
#include "qMRMLWidgetsExport.h"

// MRML includes
#include <vtkMRMLLayoutNode.h>

class qMRMLLayoutWidgetPrivate;
class qMRMLLayoutManager;
class vtkMRMLScene;

class QMRML_WIDGETS_EXPORT qMRMLLayoutWidget : public QWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructors
  explicit qMRMLLayoutWidget(QWidget* widget);
  ~qMRMLLayoutWidget() override;

  /// Layout manager
  Q_INVOKABLE qMRMLLayoutManager* layoutManager()const;
  /// Set layout manager (useful for specifying a specialized layout manager class)
  Q_INVOKABLE void setLayoutManager(qMRMLLayoutManager* layoutManager);

  /// Utility function that returns the mrml scene of the layout manager
  vtkMRMLScene* mrmlScene()const;
  /// Utility function that returns the current layout of the layout manager
  int layout()const;

public slots:
  /// Set the MRML \a scene to the layout manager
  void setMRMLScene(vtkMRMLScene* scene);

  /// Propagate to the layoutmanager
  void setLayout(int);

protected:
  qMRMLLayoutWidget(qMRMLLayoutWidgetPrivate* obj, QWidget* widget);
  QScopedPointer<qMRMLLayoutWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLLayoutWidget);
  Q_DISABLE_COPY(qMRMLLayoutWidget);
};

#endif
