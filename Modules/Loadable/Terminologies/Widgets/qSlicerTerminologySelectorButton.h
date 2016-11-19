/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

#ifndef __qSlicerTerminologySelectorButton_h
#define __qSlicerTerminologySelectorButton_h

// Qt includes
#include <QPushButton>

#include <vtkSlicerTerminologyEntry.h>

#include "qSlicerTerminologiesModuleWidgetsExport.h"

class qSlicerTerminologySelectorButtonPrivate;

//TODO:
/// \ingroup Widgets
///
/// qSlicerTerminologySelectorButton is a QPushButton that refers to a color. The color
/// and the name of the color (i.e. &#35;FFFFFF) are displayed on the button.
/// When clicked, a color dialog pops up to select a new color
/// for the QPushButton.
class Q_SLICER_MODULE_TERMINOLOGIES_WIDGETS_EXPORT qSlicerTerminologySelectorButton : public QPushButton
{
  Q_OBJECT

  Q_PROPERTY(vtkSlicerTerminologyEntry* terminologyEntry READ terminologyEntry WRITE setTerminologyEntry NOTIFY terminologyChanged USER true)
  Q_PROPERTY(QColor color READ color WRITE setColor)

public:
  //TODO:
  /// The text will be shown on the button if
  /// displayColorName is false, otherwise the color name is shown.
  /// \sa setColor, QPushButton::setText
  explicit qSlicerTerminologySelectorButton(QWidget* parent=NULL);

  virtual ~qSlicerTerminologySelectorButton();

  /// Current selected terminology
  vtkSlicerTerminologyEntry* terminologyEntry();

  /// Current color
  QColor color();

public slots:
  /// Set a new current color without opening a dialog
  void setTerminologyEntry(vtkSlicerTerminologyEntry* newTerminology, bool modifiedEvent=true);

  /// Set color
  void setColor(QColor color);

  /// Opens a color dialog to select a new current color.
  /// If the CTK color dialog (\a UseCTKColorDialog) is used, then the color
  /// name is also set if the user selects a named color.
  /// \sa ctkColorDialog, color, colorName
  void changeTerminology();

signals:
  /// terminologyChanged is fired anytime a new color is set. Programmatically or
  /// by the user when choosing a color from the color dialog
  void terminologyChanged();

  // Invoked when the shown dialog is canceled
  void canceled();

protected slots:
  void onToggled(bool change = true);

protected:
  virtual void paintEvent(QPaintEvent* event);

  QScopedPointer<qSlicerTerminologySelectorButtonPrivate> d_ptr;
private :
  Q_DECLARE_PRIVATE(qSlicerTerminologySelectorButton);
  Q_DISABLE_COPY(qSlicerTerminologySelectorButton);
};

#endif
