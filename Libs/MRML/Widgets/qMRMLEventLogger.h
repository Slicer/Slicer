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

#ifndef __qMRMLEventLogger_h
#define __qMRMLEventLogger_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class qMRMLEventLoggerPrivate;
class vtkMRMLScene;
class vtkObject;

class QMRML_WIDGETS_EXPORT qMRMLEventLogger: public QObject
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  explicit qMRMLEventLogger(QObject* parent = nullptr);
  ~qMRMLEventLogger() override;

  ///
  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* scene);

  ///
  /// Return true if the corresponding event if listened by the eventLogger
  bool listeningNodeAddedEvent();
  bool listeningNodeRemovedEvent();
  bool listeningNewSceneEvent();
  bool listeningSceneClosedEvent();
  bool listeningSceneAboutToBeClosedEvent();
  bool listeningMetadataAddedEvent();
  bool listeningImportProgressFeedbackEvent();
  bool listeningSaveProgressFeedbackEvent();
  bool listeningSceneAboutToBeImportedEvent();
  bool listeningSceneImportedEvent();
  bool listeningSceneRestoredEvent();

public slots:
  ///
  /// Allow to enable or disable the listening of specific event
  void listenNodeAddedEvent(bool listen);
  void listenNodeRemovedEvent(bool listen);
  void listenNewSceneEvent(bool listen);
  void listenSceneClosedEvent(bool listen);
  void listenSceneAboutToBeClosedEvent(bool listen);
  void listenMetadataAddedEvent(bool listen);
  void listenImportProgressFeedbackEvent(bool listen);
  void listenSaveProgressFeedbackEvent(bool listen);
  void listenSceneAboutToBeImportedEvent(bool listen);
  void listenSceneImportedEvent(bool listen);
  void listenSceneRestoredEvent(bool listen);

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

  /// Enable / Disable console output
  void setConsoleOutputEnabled(bool enabled);

signals:
  ///
  /// Emitted when the associated MRML scene event is fired
  void signalNodeAddedEvent(vtkObject* calle, vtkObject* call_data);
  void signalNodeRemovedEvent(vtkObject* caller, vtkObject* call_data);
  void signalNewSceneEvent();
  void signalSceneClosedEvent();
  void signalSceneAboutToBeClosedEvent();
  void signalMetadataAddedEvent();
  void signalImportProgressFeedbackEvent();
  void signalSaveProgressFeedbackEvent();
  void signalSceneAboutToBeImportedEvent();
  void signalSceneImportedEvent();
  void signalSceneRestoredEvent();

protected:
  QScopedPointer<qMRMLEventLoggerPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLEventLogger);
  Q_DISABLE_COPY(qMRMLEventLogger);
};

#endif
