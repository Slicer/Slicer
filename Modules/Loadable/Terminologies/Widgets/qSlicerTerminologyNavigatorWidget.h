/*==============================================================================

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

==============================================================================*/

#ifndef __qSlicerTerminologyNavigatorWidget_h
#define __qSlicerTerminologyNavigatorWidget_h

// MRMLWidgets includes
#include "qMRMLWidget.h"

#include "qSlicerTerminologiesModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class qSlicerTerminologyNavigatorWidgetPrivate;

class QTableWidgetItem;
class QColor;
class vtkSlicerTerminologyEntry;

/// \brief Qt widget for browsing a terminology dictionary.
///   DICOM properties of the selected entry can also be set if enabled.
/// \ingroup SlicerRt_QtModules_Terminologies_Widgets
class Q_SLICER_MODULE_TERMINOLOGIES_WIDGETS_EXPORT qSlicerTerminologyNavigatorWidget : public qMRMLWidget
{
  Q_OBJECT
  Q_PROPERTY(bool anatomicRegionSectionVisible READ anatomicRegionSectionVisible WRITE setAnatomicRegionSectionVisible)

public:
  /// Constructor
  explicit qSlicerTerminologyNavigatorWidget(QWidget* parent = 0);
  /// Destructor
  virtual ~qSlicerTerminologyNavigatorWidget();

  /// Populate terminology entry from terminology and anatomy selection
  /// \return Success flag (e.g. fail if no type is selected)
  bool terminologyEntry(vtkSlicerTerminologyEntry* entry);

  /// Update terminology and anatomy widgets and selections from terminology entry
  /// \return Success flag (e.g. fail if no type is specified in entry)
  bool setTerminologyEntry(vtkSlicerTerminologyEntry* entry);

  /// Get whether anatomic region section are visible
  bool anatomicRegionSectionVisible() const;

  /// Convert terminology entry VTK object to string list with the contained code meanings.
  /// The string list contains the following strings:
  ///   [ terminologyContextName, categoryCodeMeaning, typeCodeMeaning, typeModifierCodeMeaning,
  ///     anatomicContextName, anatomicRegionCodeMeaning, anatomicRegionModifierCodeMeaning ]
  static QStringList codeMeaningsFromTerminologyEntry(vtkSlicerTerminologyEntry* entry);

  /// Populate terminology entry VTK object with code meanings in a string list
  /// The string list contains the following strings:
  ///   [ terminologyContextName, categoryCodeMeaning, typeCodeMeaning, typeModifierCodeMeaning,
  ///     anatomicContextName, anatomicRegionCodeMeaning, anatomicRegionModifierCodeMeaning ]
  ///  \return Success flag
  static bool terminologyEntryFromCodeMeanings(QStringList codeMeanings, vtkSlicerTerminologyEntry* entry);

  /// Get recommended color from type (or type modifier if any) of the selected terminology entry
  static QColor recommendedColorFromTerminology(vtkSlicerTerminologyEntry* entry);

public slots:
  /// Show/hide anatomic region section section
  void setAnatomicRegionSectionVisible(bool);

protected:
  /// Populate terminology combobox based on current selection
  void populateTerminologyComboBox();
  /// Populate category table based on selected terminology and category search term
  void populateCategoryTable();
  /// Populate type table based on selected category and type search term
  void populateTypeTable();
  /// Populate type modifier combobox based on current selection
  void populateTypeModifierComboBox();

  /// Populate anatomic region context combobox based on current selection
  void populateAnatomicContextComboBox();
  /// Populate region table based on selected anatomic region context and type search term
  void populateRegionTable();
  /// Populate region modifier combobox based on current selection
  void populateRegionModifierComboBox();

protected slots:
  void onTerminologySelectionChanged(int);
  void onCategoryClicked(QTableWidgetItem*);
  void onTypeClicked(QTableWidgetItem*);
  void onTypeModifierSelectionChanged(int);
  void onCategorySearchTextChanged(QString);
  void onTypeSearchTextChanged(QString);

  void onAnatomicContextSelectionChanged(int);
  void onRegionClicked(QTableWidgetItem*);
  void onRegionModifierSelectionChanged(int);
  void onRegionSearchTextChanged(QString);

  void onRecommendedColorChanged(QColor);

protected:
  QScopedPointer<qSlicerTerminologyNavigatorWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTerminologyNavigatorWidget);
  Q_DISABLE_COPY(qSlicerTerminologyNavigatorWidget);
};

#endif
