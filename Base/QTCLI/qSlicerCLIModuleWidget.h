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

#ifndef __qSlicerCLIModuleWidget_h
#define __qSlicerCLIModuleWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBaseQTCLIExport.h"

class ModuleDescription;
class vtkMRMLNode; 
class qSlicerCLIModuleWidgetPrivate;

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCLIModuleWidget(ModuleDescription* desc, QWidget *parent=0);
  virtual ~qSlicerCLIModuleWidget();

  ///
  /// Set the \a entryPoint associated with the module
  void setModuleEntryPoint(const QString& entryPoint);

public slots:
  ///
  /// Set the current \a commandLineModuleNode
  void setCurrentCommandLineModuleNode(vtkMRMLNode* commandLineModuleNode);

  void apply();
  void cancel();
  void reset();

protected:
  /// 
  virtual void setup();

protected:
  QScopedPointer<qSlicerCLIModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCLIModuleWidget);
  Q_DISABLE_COPY(qSlicerCLIModuleWidget);
};

#endif
