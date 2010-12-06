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

#ifndef __qSlicerScriptedLoadableModule_h
#define __qSlicerScriptedLoadableModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerBaseQTGUIExport.h"

class qSlicerScriptedLoadableModulePrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerScriptedLoadableModule : public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PROPERTY(QString title READ title WRITE setTitle)
  Q_PROPERTY(QString category READ category WRITE setCategory)
  Q_PROPERTY(QString contributor READ contributor WRITE setContributor)
  Q_PROPERTY(QString helpText READ helpText WRITE setHelpText)
  Q_PROPERTY(QString acknowledgementText READ acknowledgementText WRITE setAcknowledgementText)
  
public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerScriptedLoadableModule(QObject *parent=0);
  virtual ~qSlicerScriptedLoadableModule();
  
  bool setPythonSource(const QString& newPythonSource);

  /// Convenient method allowing to retrieve the associated scripted instance
  /// Q_INVOKABLE PyObject* pythonInstance() const;
  
  virtual QString title()const ;
  void setTitle(const QString& newTitle);
  
  virtual QString category()const;
  void setCategory(const QString& newCategory);
  
  virtual QString contributor()const;
  void setContributor(const QString& newContributor);
  
  virtual QString helpText()const;
  void setHelpText(const QString& newHelpText);
  
  virtual QString acknowledgementText()const;
  void setAcknowledgementText(const QString& newAcknowledgementText);

protected:

  virtual void setup();

  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();

  virtual vtkSlicerLogic* createLogic();

protected:
  QScopedPointer<qSlicerScriptedLoadableModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScriptedLoadableModule);
  Q_DISABLE_COPY(qSlicerScriptedLoadableModule);
};

#endif
