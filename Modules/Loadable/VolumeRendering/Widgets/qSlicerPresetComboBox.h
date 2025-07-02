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

#ifndef __qSlicerPresetComboBox_h
#define __qSlicerPresetComboBox_h

// MRMLWidgets includes
#include <qMRMLNodeComboBox.h>

// Slicer includes
#include "qSlicerVolumeRenderingModuleWidgetsExport.h"

class vtkMRMLNode;
class qSlicerPresetComboBoxPrivate;

class Q_SLICER_MODULE_VOLUMERENDERING_WIDGETS_EXPORT qSlicerPresetComboBox : public qMRMLNodeComboBox
{
  Q_OBJECT
  /// Show icons in the combobox and popup window.
  Q_PROPERTY(bool showIcons READ showIcons WRITE setShowIcons)
  /// Show label in the popup window.
  Q_PROPERTY(bool showLabelsInPopup READ showLabelsInPopup WRITE setShowLabelsInPopup)
  /// Icon size in popup.
  Q_PROPERTY(QSize iconSizeInPopup READ iconSizeInPopup WRITE setIconSizeInPopup)

public:
  /// Constructors
  typedef qMRMLNodeComboBox Superclass;
  explicit qSlicerPresetComboBox(QWidget* parent = nullptr);
  ~qSlicerPresetComboBox() override;

  bool showIcons() const;
  bool showLabelsInPopup() const;
  QSize iconSizeInPopup() const;

public slots:
  void setIconToPreset(vtkMRMLNode* presetNode);
  void setShowIcons(bool show);
  void setShowLabelsInPopup(bool show);
  void setIconSizeInPopup(const QSize& size);

protected slots:
  void updateComboBoxTitleAndIcon(vtkMRMLNode* presetNode);

protected:
  QScopedPointer<qSlicerPresetComboBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPresetComboBox);
  Q_DISABLE_COPY(qSlicerPresetComboBox);
};

#endif
