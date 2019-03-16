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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerUnitsSettingsPanel_h
#define __qSlicerUnitsSettingsPanel_h

// Qt includes
#include <QString>
#include <QStringList>

// CTK includes
#include <ctkVTKObject.h>
#include <ctkSettingsPanel.h>

// Slicer includes
class vtkMRMLNode;

// Units includes
#include "qSlicerUnitsModuleExport.h"
class qSlicerUnitsSettingsPanelPrivate;
class vtkSlicerUnitsLogic;

class Q_SLICER_QTMODULES_UNITS_EXPORT qSlicerUnitsSettingsPanel
  : public ctkSettingsPanel
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(QStringList quantities READ quantities WRITE setQuantities NOTIFY quantitiesChanged)
public:

  /// Superclass typedef
  typedef ctkSettingsPanel Superclass;

  /// Constructor
  qSlicerUnitsSettingsPanel(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerUnitsSettingsPanel() override;

  /// Set the units logic. The logic is observed to get the application
  /// scene and used to set the defaults units.
  void setUnitsLogic(vtkSlicerUnitsLogic* logic);

  /// Return the quantities for which units can be set.
  /// The quantities are saved in the settings.
  QStringList quantities();

signals:
  void quantitiesChanged(const QStringList&);

protected slots:
  void onUnitsLogicModified();
  void setQuantities(const QStringList& quantities);
  void updateFromSelectionNode();
  void showAll(bool showAll);

protected:
  QScopedPointer<qSlicerUnitsSettingsPanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerUnitsSettingsPanel);
  Q_DISABLE_COPY(qSlicerUnitsSettingsPanel);
};

#endif
