/*=========================================================================

  Program: 3D Slicer

  Copyright (c) Seattle Children's Hospital d/b/a Seattle Children's Research Institute.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, EBATINCA, S.L.
  and was funded by by Murat Maga (Seattle Children's Research Institute).

=========================================================================*/

#ifndef __qSlicerTerminologyEditorButton_h
#define __qSlicerTerminologyEditorButton_h

// Qt includes
#include <QPushButton>

// Colors includes
#include "qSlicerColorsModuleWidgetsExport.h"

#include "qSlicerTerminologyEditorWidget.h"

// Terminologies includes
#include "qSlicerTerminologyNavigatorWidget.h"
#include "vtkSlicerTerminologyEntry.h"

class qSlicerTerminologyEditorButtonPrivate;

/// \brief Button that opens terminology selector dialog
class Q_SLICER_MODULE_COLORS_WIDGETS_EXPORT qSlicerTerminologyEditorButton : public QPushButton
{
  Q_OBJECT

public:
  explicit qSlicerTerminologyEditorButton(QWidget* parent = nullptr);
  ~qSlicerTerminologyEditorButton() override;

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

  QScopedPointer<qSlicerTerminologyEditorButtonPrivate> d_ptr;
private :
  Q_DECLARE_PRIVATE(qSlicerTerminologyEditorButton);
  Q_DISABLE_COPY(qSlicerTerminologyEditorButton);
};

#endif
