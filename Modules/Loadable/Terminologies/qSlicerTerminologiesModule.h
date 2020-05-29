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

#ifndef __qSlicerTerminologiesModule_h
#define __qSlicerTerminologiesModule_h

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerTerminologiesModuleExport.h"

class qSlicerTerminologiesModulePrivate;

/// \ingroup SlicerRt_QtModules_DicomRtImport
class Q_SLICER_QTMODULES_TERMINOLOGIES_EXPORT qSlicerTerminologiesModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerTerminologiesModule(QObject *parent=nullptr);
  ~qSlicerTerminologiesModule() override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Help to use the module
  QString helpText()const override;

  /// Return acknowledgments
  QString acknowledgementText()const override;

  /// Return the authors of the module
  QStringList contributors()const override;

  /// Return the categories for the module
  QStringList categories()const override;

  /// List dependencies
  QStringList dependencies()const override;

protected:

  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation* createWidgetRepresentation() override;

  /// Create and return the logic associated to this module (will return only import logic!)
  vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerTerminologiesModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTerminologiesModule);
  Q_DISABLE_COPY(qSlicerTerminologiesModule);
};

#endif
