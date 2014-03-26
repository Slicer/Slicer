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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerCLIModule_h
#define __qSlicerCLIModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModule.h"

#include "qSlicerBaseQTCLIExport.h"

class ModuleLogo;
class vtkMRMLCommandLineModuleNode;
class vtkSlicerCLIModuleLogic;

class qSlicerCLIModulePrivate;
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModule : public qSlicerAbstractModule
{
  Q_OBJECT
public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerCLIModule(QWidget *parent=0);
  virtual ~qSlicerCLIModule();

  ///
  /// Assign the module XML description.
  /// Note: That will also trigger the parsing of the XML structure
  void setXmlModuleDescription(const QString& xmlModuleDescription);

  /// Optionally set in the module XML description
  virtual int index() const;

  ///
  /// Return help/acknowledgement text
  virtual QString helpText() const;
  virtual QString acknowledgementText() const;

  /// Set temporary directory associated with the module
  void setTempDirectory(const QString& tempDirectory);
  QString tempDirectory() const;

  /// Set module entry point. Typically "slicer:0x012345" for loadable CLI
  /// or "/home/user/work/Slicer-Superbuild/../mycliexec" for executable CLI
  void setEntryPoint(const QString& entryPoint);
  QString entryPoint() const;

  /// SharedObjectModule for loadable modules or CommandLineModule for
  /// executable modules.
  void setModuleType(const QString& type);
  QString moduleType() const;

  /// This method allows to get a pointer to the ModuleLogic.
  /// If no moduleLogic already exists, one will be created calling
  /// 'createLogic' method.
  Q_INVOKABLE vtkSlicerCLIModuleLogic* cliModuleLogic();

  virtual QString title() const;

  /// Extracted from the "category" field
  virtual QStringList categories() const;

  /// Extracted from the "contributor" field
  virtual QStringList contributors() const;

  virtual QImage logo() const;
  void setLogo(const ModuleLogo& logo);

  /// Convert a ModuleLogo into a QIcon
  /// \todo: Find a better place for this util function
  static QImage moduleLogoToImage(const ModuleLogo& logo);
protected:
  ///
  virtual void setup();

  ///
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerCLIModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCLIModule);
  Q_DISABLE_COPY(qSlicerCLIModule);
};

#endif
