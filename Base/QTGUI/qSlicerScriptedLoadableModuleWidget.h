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

#ifndef __qSlicerScriptedLoadableModuleWidget_h
#define __qSlicerScriptedLoadableModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBaseQTGUIExport.h"

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif
class qSlicerScriptedLoadableModuleWidgetPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerScriptedLoadableModuleWidget
  :public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  Q_PROPERTY(QString moduleName READ moduleName)
public:
  typedef qSlicerAbstractModuleWidget Superclass;
  typedef qSlicerScriptedLoadableModuleWidgetPrivate Pimpl;
  qSlicerScriptedLoadableModuleWidget(QWidget * parentWidget=nullptr);
  ~qSlicerScriptedLoadableModuleWidget() override;

  QString pythonSource()const;
  bool setPythonSource(const QString& newPythonSource, const QString& className = QLatin1String(""));

  /// Convenience method allowing to retrieve the associated scripted instance
  Q_INVOKABLE PyObject* self() const;

  void enter() override;
  void exit() override;

  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;
  double nodeEditable(vtkMRMLNode* node) override;

protected:
  void setup() override;

protected:
  QScopedPointer<qSlicerScriptedLoadableModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScriptedLoadableModuleWidget);
  Q_DISABLE_COPY(qSlicerScriptedLoadableModuleWidget);
};

#endif
