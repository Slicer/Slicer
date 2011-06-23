/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

#ifndef __qSlicerLoadableModule_h
#define __qSlicerLoadableModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModule.h"

#include "qSlicerBaseQTGUIExport.h"

class qSlicerLoadableModulePrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerLoadableModule : public qSlicerAbstractModule
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerLoadableModule(QObject *parent=0);
  virtual ~qSlicerLoadableModule();

  ///
  /// Return help/acknowledgement text
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;

protected:
  virtual void setup();

protected:
  QScopedPointer<qSlicerLoadableModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerLoadableModule);
  Q_DISABLE_COPY(qSlicerLoadableModule);
};

Q_DECLARE_INTERFACE(qSlicerLoadableModule,
                     "org.slicer.QTModules.qSlicerLoadableModule/1.0");

#endif
