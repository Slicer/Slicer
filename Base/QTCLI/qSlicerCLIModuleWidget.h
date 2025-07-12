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

#ifndef __qSlicerCLIModuleWidget_h
#define __qSlicerCLIModuleWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBaseQTCLIExport.h"

class ModuleDescription;
class vtkMRMLNode;
class vtkMRMLCommandLineModuleNode;
class qSlicerCLIModuleWidgetPrivate;

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCLIModuleWidget(QWidget* parent = nullptr);
  ~qSlicerCLIModuleWidget() override;

  /// Get the current \a commandLineModuleNode
  Q_INVOKABLE vtkMRMLCommandLineModuleNode* currentCommandLineModuleNode() const;

  // Node editing
  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;
  double nodeEditable(vtkMRMLNode* node) override;

  /// Translate string from source language to current application language
  QString translate(const std::string& sourceText) const;

public slots:
  /// Set the current \a commandLineModuleNode
  void setCurrentCommandLineModuleNode(vtkMRMLNode* commandLineModuleNode);

  void apply(bool wait = false);
  void cancel();
  void reset();

  void setAutoRun(bool enable);
  void setAutoRunWhenParameterChanged(bool enable);
  void setAutoRunWhenInputModified(bool enable);
  void setAutoRunOnOtherInputEvents(bool enable);
  void setAutoRunCancelsRunningProcess(bool enable);

protected:
  ///
  void setup() override;

  /// Set up the GUI from mrml when entering
  void enter() override;

  /// Run a command line module given \a parameterNode
  /// If \a waitForCompletion is True, the call will return only upon completion of
  /// the module execution.
  void run(vtkMRMLCommandLineModuleNode* parameterNode, bool waitForCompletion = false);

  /// Abort the execution of the module associated with \a node
  void cancel(vtkMRMLCommandLineModuleNode* node);

protected:
  QScopedPointer<qSlicerCLIModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCLIModuleWidget);
  Q_DISABLE_COPY(qSlicerCLIModuleWidget);
};

#endif
