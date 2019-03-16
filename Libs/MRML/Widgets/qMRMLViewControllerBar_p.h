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

#ifndef __qMRMLViewControllerBar_p_h
#define __qMRMLViewControllerBar_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QObject>
class QLabel;
class QToolButton;
class QHBoxLayout;
class QVBoxLayout;

// CTK includes
class ctkPopupWidget;

// qMRML includes
#include "qMRMLViewControllerBar.h"

//-----------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLViewControllerBarPrivate: public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qMRMLViewControllerBar);

protected:
  qMRMLViewControllerBar* const q_ptr;

public:
  typedef QObject Superclass;
  qMRMLViewControllerBarPrivate(qMRMLViewControllerBar& object);
  ~qMRMLViewControllerBarPrivate() override;

  virtual void init();
  virtual void setColor(QColor color);
  virtual QColor color()const;

  QToolButton*                     PinButton;
  QLabel*                          ViewLabel;
  ctkPopupWidget*                  PopupWidget;
  QWidget*                         BarWidget;
  QHBoxLayout*                     BarLayout;
  QVBoxLayout*                     ControllerLayout;
  qMRMLViewControllerBar::LayoutBehavior  LayoutBehavior;
  QColor                           BarColor;

  bool eventFilter(QObject* object, QEvent* event) override;

protected:
  virtual void setupPopupUi();
};

#endif
