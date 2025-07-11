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

#ifndef __qMRMLViewControllerBar_h
#define __qMRMLViewControllerBar_h

// Qt includes
class QLabel;
class QLayout;
class QToolButton;

// CTK includes
#include <ctkVTKObject.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"
#include "qMRMLWidgetsExport.h"
class qMRMLViewControllerBarPrivate;
class vtkMRMLAbstractViewNode;

/// qMRMLViewControllerBar is the base class of all the bars over views.
/// A controller bar typically contains a pin button, a view label to uniquely
/// define a view, a popup widget to control the view and a unique color per
/// type of view. The popup widget can be made occupy space in the
/// widget and is then displayed below the bar.
//
// Widget layout:
//   VBoxLayout (ControllerLayout)
//      Widget (BarWidget)
//        HBoxLayout (BarLayout)
//      Optional PopupWidget (can be statically displayed under BarWidget if ControllerBar is a panel)
//
//
// To add widgets to the "bar" section, add them to the barLayout().
// To add widgets to the "controller" section (when not using a
// popup), add them to the layout().
//
class QMRML_WIDGETS_EXPORT qMRMLViewControllerBar : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool showMaximizeViewButton READ showMaximizeViewButton WRITE setShowMaximizeViewButton)

public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLViewControllerBar(QWidget* parent = nullptr);
  ~qMRMLViewControllerBar() override;

  enum LayoutBehavior
  {
    Popup = 0,
    Panel
  };

  /// Set the behavior of the controller, i.e. should it be a popup or
  /// should it occupy space within the widget? (not certain that this
  /// method can be called multiple times to toggle between the behaviors)
  Q_INVOKABLE void setLayoutBehavior(LayoutBehavior behavior);

  /// Get the layout for the "bar" in the view controller.
  /// This layout is an HBoxLayout. It is packed in a VBoxLayout that
  /// contains the "bar" and other controllers.
  Q_INVOKABLE QLayout* barLayout();

  /// Get the widget for the "bar" in the view controller.
  /// This is the part of the controller that is visible
  /// even when view controller is not pinned.
  Q_INVOKABLE QWidget* barWidget();

  /// Push-pin icon that shows additional options when clicked.
  Q_INVOKABLE QToolButton* pinButton();

  /// Label that displays the view's name.
  Q_INVOKABLE QLabel* viewLabel();

  bool showMaximizeViewButton() const;

public slots:
  void maximizeView();

  void setShowMaximizeViewButton(bool show);

protected slots:
  virtual void updateWidgetFromMRMLView();

protected:
  QScopedPointer<qMRMLViewControllerBarPrivate> d_ptr;
  qMRMLViewControllerBar(qMRMLViewControllerBarPrivate* pimpl, QWidget* parent = nullptr);

  virtual void setMRMLViewNode(vtkMRMLAbstractViewNode* viewNode);
  virtual vtkMRMLAbstractViewNode* mrmlViewNode() const;

private:
  Q_DECLARE_PRIVATE(qMRMLViewControllerBar);
  Q_DISABLE_COPY(qMRMLViewControllerBar);
};

#endif
