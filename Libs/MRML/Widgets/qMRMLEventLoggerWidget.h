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

#ifndef __qMRMLEventLoggerWidget_h
#define __qMRMLEventLoggerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class qMRMLEventLoggerWidgetPrivate;
class vtkMRMLScene;
class vtkObject;

class QMRML_WIDGETS_EXPORT qMRMLEventLoggerWidget: public QWidget
{
  Q_OBJECT
public:
  typedef QWidget Superclass;
  explicit qMRMLEventLoggerWidget(QWidget *parent = nullptr);
  ~qMRMLEventLoggerWidget() override;

public slots:

  ///
  /// Set the MRML scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* scene);

  /// Enable / Disable console output
  void setConsoleOutputEnabled(bool enabled);

protected slots:

  virtual void onNodeAddedEvent(vtkObject* caller, vtkObject* call_data);
  virtual void onNodeRemovedEvent(vtkObject* caller, vtkObject* call_data);
  virtual void onNewSceneEvent();
  virtual void onSceneClosedEvent();
  virtual void onSceneAboutToBeClosedEvent();
  virtual void onMetadataAddedEvent();
  virtual void onImportProgressFeedbackEvent();
  virtual void onSaveProgressFeedbackEvent();
  virtual void onSceneAboutToBeImportedEvent();
  virtual void onSceneImportedEvent();
  virtual void onSceneRestoredEvent();

protected:
  QScopedPointer<qMRMLEventLoggerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLEventLoggerWidget);
  Q_DISABLE_COPY(qMRMLEventLoggerWidget);
};

#endif
