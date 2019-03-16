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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLCaptureToolBar_h
#define __qMRMLCaptureToolBar_h

// Qt includes
//#include <QSignalMapper>
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>
// no ui begin
#include <ctkVTKObject.h>
// no ui end

// qMRMLWidget includes
#include "qMRMLWidget.h"
#include "qMRMLWidgetsExport.h"

class qMRMLCaptureToolBarPrivate;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLViewNode;

class QMRML_WIDGETS_EXPORT qMRMLCaptureToolBar : public QToolBar
{
  Q_OBJECT
  QVTK_OBJECT

  Q_PROPERTY(bool popupsTimeOut READ popupsTimeOut WRITE setPopupsTimeOut)

public:
  typedef QToolBar Superclass;

  /// Constructor
  /// Title is the name of the toolbar (can appear using right click on the toolbar area)
  qMRMLCaptureToolBar(const QString& title, QWidget* parent = nullptr);
  qMRMLCaptureToolBar(QWidget* parent = nullptr);
  ~qMRMLCaptureToolBar() override;

  // Get popupsTimeOut setting
  bool popupsTimeOut() const;

public slots:
  virtual void setMRMLScene(vtkMRMLScene* newScene);
  void setActiveMRMLThreeDViewNode(vtkMRMLViewNode * newActiveMRMLThreeDViewNode);

  /// Set flag to time out pop ups, set from the qSlicerAppMainWindow according to the
  /// AA_EnableTesting attribute
  void setPopupsTimeOut(bool flag);

signals:
  void screenshotButtonClicked();
  void sceneViewButtonClicked();
  void mrmlSceneChanged(vtkMRMLScene*);

protected:
  QScopedPointer<qMRMLCaptureToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLCaptureToolBar);
  Q_DISABLE_COPY(qMRMLCaptureToolBar);
};

#endif
