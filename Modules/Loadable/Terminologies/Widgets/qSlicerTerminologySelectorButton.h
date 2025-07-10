/*=========================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

=========================================================================*/

#ifndef __qSlicerTerminologySelectorButton_h
#define __qSlicerTerminologySelectorButton_h

// Qt includes
#include <QPushButton>

// Terminologies includes
#include "qSlicerTerminologiesModuleWidgetsExport.h"

#include "qSlicerTerminologyNavigatorWidget.h"
#include "vtkSlicerTerminologyEntry.h"

class qSlicerTerminologySelectorButtonPrivate;

/// \brief Button that opens terminology selector dialog
class Q_SLICER_MODULE_TERMINOLOGIES_WIDGETS_EXPORT qSlicerTerminologySelectorButton : public QPushButton
{
  Q_OBJECT

public:
  explicit qSlicerTerminologySelectorButton(QWidget* parent = nullptr);
  ~qSlicerTerminologySelectorButton() override;

#ifndef __VTK_WRAP__
  /// Get selected terminology and other metadata (name, color, auto-generated flags) into given info bundle object
  void terminologyInfo(qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle& terminologyInfo);
#endif

public slots:

#ifndef __VTK_WRAP__
  /// Set terminology and other metadata (name, color, auto-generated flags)
  void setTerminologyInfo(qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle& terminologyInfo);
#endif

  /// Opens a terminology dialog to select a new terminology.
  void changeTerminology();

signals:
  void terminologyChanged();
  void canceled();

  /// Emitted when the user selects a custom name or color
  void userSetCustomNameOrColor();

protected slots:
  void onToggled(bool toggled = true);

protected:
  void paintEvent(QPaintEvent* event) override;

  QScopedPointer<qSlicerTerminologySelectorButtonPrivate> d_ptr;
private:
  Q_DECLARE_PRIVATE(qSlicerTerminologySelectorButton);
  Q_DISABLE_COPY(qSlicerTerminologySelectorButton);
};

#endif
