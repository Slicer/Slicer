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
  virtual ~qMRMLLayoutWidget();

  /// Layout manager
  qMRMLLayoutManager* layoutManager()const;
  /// Utility function that returns the mrml scene of the layout manager
  vtkMRMLScene* mrmlScene()const;
  /// Utility function that returns the current layout of the layout manager
  int layout()const;

public slots:
  ///
  /// Set the MRML \a scene to the layout manager
  void setMRMLScene(vtkMRMLScene* scene);

  /// Switch to the different layout
  /// propagate to the layout manager
  void switchToConventionalView();
  void switchToOneUp3DView();
  void switchToOneUpRedSliceView();
  void switchToOneUpYellowSliceView();
  void switchToOneUpGreenSliceView();
  void switchToOneUpSliceView(const QString& sliceViewName);
  void switchToFourUpView();
  void switchToTabbed3DView();
  void switchToTabbedSliceView();
  void switchToLightboxView();
  void switchToCompareView();
  void switchToCompareWidescreenView();
  void switchToThreeOverThreeView();
  void switchToFourOverFourView();
  void switchToSideBySideLightboxView();
  void switchToDual3DView();
  void switchToNone();

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
