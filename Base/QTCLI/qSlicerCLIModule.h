/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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

class qSlicerCLIModulePrivate;
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModule : public qSlicerAbstractModule
{
public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerCLIModule(QWidget *parent=0);
  virtual ~qSlicerCLIModule();

  /// 
  /// Assign the module XML description.
  /// Note: That will also trigger the parsing of the XML structure
  void setXmlModuleDescription(const char* xmlModuleDescription);

  virtual QString title()const;
  virtual QString category()const;
  virtual QString contributor()const;

  /// 
  /// Return help/acknowledgement text
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;

  /// 
  /// Set temporary directory associated with the module
  void setTempDirectory(const QString& tempDirectory);

  ///
  /// Get entry point as string
  virtual QString entryPoint()const = 0;

  ///
  /// Get module type
  virtual QString moduleType()const = 0;

protected:
  /// 
  virtual void setup();

  /// 
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// 
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

protected:
  QScopedPointer<qSlicerCLIModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCLIModule);
  Q_DISABLE_COPY(qSlicerCLIModule);
};

#endif
