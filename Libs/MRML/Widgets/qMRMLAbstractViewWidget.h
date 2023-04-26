/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __qMRMLAbstractViewWidget_h
#define __qMRMLAbstractViewWidget_h

// Qt includes
class QWidget;

// qMRMLWidget includes
#include "qMRMLWidget.h"
class qMRMLViewControllerBar;

// MRML includes
class vtkMRMLAbstractViewNode;

// MRMLLogic includes
class vtkMRMLAbstractLogic;

class QMRML_WIDGETS_EXPORT qMRMLAbstractViewWidget : public qMRMLWidget
{
  Q_OBJECT

  Q_PROPERTY(QString viewLabel READ viewLabel WRITE setViewLabel)
  Q_PROPERTY(QColor viewColor READ viewColor WRITE setViewColor)

public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLAbstractViewWidget(QWidget* parent = nullptr);
  ~qMRMLAbstractViewWidget() override = default;

  /// Get slice controller
  virtual Q_INVOKABLE qMRMLViewControllerBar* controllerWidget() const = 0;

  /// Get the View node observed by view.
  virtual Q_INVOKABLE vtkMRMLAbstractViewNode* mrmlAbstractViewNode() const = 0;

  /// \sa qMRMLSliceControllerWidget::viewLogic()
  virtual Q_INVOKABLE vtkMRMLAbstractLogic* logic() const;

  /// Get a reference to the underlying view widget.
  /// Be careful if you change the viewWidget, you might
  /// unsynchronize the view from the nodes/logics.
  virtual Q_INVOKABLE QWidget* viewWidget() const = 0;

  /// Get the text displayed in the controlled widget label.
  /// \sa setViewLabel()
  virtual Q_INVOKABLE QString viewLabel() const;

  /// Set the text displayed in the controlled widget label.
  /// \sa viewLabel()
  virtual Q_INVOKABLE void setViewLabel(const QString& newViewLabel);

  /// Get the color displayed in the controlled widget.
  /// \sa setViewColor()
  virtual Q_INVOKABLE QColor viewColor() const;

  /// Set the color displayed in the controlled widget.
  /// \sa viewColor()
  virtual Q_INVOKABLE void setViewColor(const QColor& newViewColor);

public slots:
  /// Set the current \a viewNode to observe
  virtual void setMRMLAbstractViewNode(vtkMRMLAbstractViewNode* newViewNode) = 0;

  /// Calls pauseRender() if pause is true or resumeRender() if pause is false
  /// \sa pauseRender(), resumeRender()
  virtual void setRenderPaused(bool pause);
  /// Increments the pause render count
  /// \sa setPauseRender()
  virtual void pauseRender();
  /// De-increments the pause render count and calls scheduleRender() if one is currently pending
  /// \sa setPauseRender()
  virtual void resumeRender();

private:
  Q_DISABLE_COPY(qMRMLAbstractViewWidget);
};

#endif
