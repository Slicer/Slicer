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

#ifndef __qSlicerSubjectHierarchySettingsPanel_h
#define __qSlicerSubjectHierarchySettingsPanel_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkSettingsPanel.h>

#include "qSlicerSubjectHierarchyModuleExport.h"

class QSettings;
class qSlicerSubjectHierarchySettingsPanelPrivate;

class Q_SLICER_QTMODULES_SUBJECTHIERARCHY_EXPORT qSlicerSubjectHierarchySettingsPanel
  : public ctkSettingsPanel
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkSettingsPanel Superclass;

  /// Constructor
  explicit qSlicerSubjectHierarchySettingsPanel(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerSubjectHierarchySettingsPanel() override;

public slots:

protected slots:
  void setAutoDeleteSubjectHierarchyChildrenEnabled(bool on);
  void setDisplayPatientIDEnabled(bool on);
  void setDisplayPatientBirthDateEnabled(bool on);
  void setDisplayStudyIDEnabled(bool on);
  void setDisplayStudyDateEnabled(bool on);

protected:
  QScopedPointer<qSlicerSubjectHierarchySettingsPanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchySettingsPanel);
  Q_DISABLE_COPY(qSlicerSubjectHierarchySettingsPanel);
};

#endif
