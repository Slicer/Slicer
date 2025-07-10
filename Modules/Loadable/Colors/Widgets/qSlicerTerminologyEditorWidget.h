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

#ifndef __qSlicerTerminologyEditorWidget_h
#define __qSlicerTerminologyEditorWidget_h

// Qt includes
#include <QWidget>

// Colors includes
#include "qSlicerColorsModuleWidgetsExport.h"

// Terminologies includes
#include "qSlicerTerminologyNavigatorWidget.h"

class qSlicerTerminologyEditorWidgetPrivate;

class Q_SLICER_MODULE_COLORS_WIDGETS_EXPORT qSlicerTerminologyEditorWidget : public QWidget
{
  Q_OBJECT
  // Q_PROPERTY(bool showOnlyNamedColors READ showOnlyNamedColors WRITE setShowOnlyNamedColors)
public:
  qSlicerTerminologyEditorWidget(QWidget* parent = nullptr);
  ~qSlicerTerminologyEditorWidget() override;

  /// Get selection from widget: terminology and meta-data (name, color, auto-generated flags)
  /// \param terminologyInfo Info bundle to get terminology information into
  void terminologyInfo(qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle& terminologyInfo);
  /// Set selection to widget: terminology and meta-data (name, color, auto-generated flags)
  void setTerminologyInfo(qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle& terminologyInfo);

public slots:

signals:

protected slots:
  /// Shows popup with terminology navigator when the select button is clicked.
  void onSelectFromTerminology();

protected:
  QScopedPointer<qSlicerTerminologyEditorWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTerminologyEditorWidget);
  Q_DISABLE_COPY(qSlicerTerminologyEditorWidget);
};

#endif
