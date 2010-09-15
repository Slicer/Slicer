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


#ifndef __qSlicerCLILoadableModule_h
#define __qSlicerCLILoadableModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerCLIModule.h"

#include "qSlicerBaseQTCLIExport.h"

class qSlicerCLILoadableModulePrivate;
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLILoadableModule : public qSlicerCLIModule
{
public:

  /// Convenient typedefs
  typedef qSlicerCLIModule Superclass;
  typedef int (*ModuleEntryPointType)(int argc, char* argv[]);
  
  qSlicerCLILoadableModule(QWidget *parent=0);
  virtual ~qSlicerCLILoadableModule();

  ///
  /// Set module entry point
  void setEntryPoint(ModuleEntryPointType _entryPoint);

  ///
  /// Get entry point as string
  virtual QString entryPoint()const;

  ///
  /// Get module type
  virtual QString moduleType()const;

protected:
  /// 
  virtual void setup();


protected:
  QScopedPointer<qSlicerCLILoadableModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCLILoadableModule);
  Q_DISABLE_COPY(qSlicerCLILoadableModule);
};

#endif
