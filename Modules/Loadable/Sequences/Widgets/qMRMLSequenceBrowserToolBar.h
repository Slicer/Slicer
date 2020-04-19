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

==============================================================================*/

#ifndef __qMRMLSequenceBrowserToolBar_h
#define __qMRMLSequenceBrowserToolBar_h

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
#include "qSlicerSequencesModuleWidgetsExport.h"

class qMRMLSequenceBrowserToolBarPrivate;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLSequenceBrowserNode;

class Q_SLICER_MODULE_SEQUENCES_WIDGETS_EXPORT qMRMLSequenceBrowserToolBar : public QToolBar
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QToolBar Superclass;

  /// Constructor
  /// Title is the name of the toolbar (can appear using right click on the toolbar area)
  qMRMLSequenceBrowserToolBar(const QString& title, QWidget* parent = 0);
  qMRMLSequenceBrowserToolBar(QWidget* parent = 0);
  ~qMRMLSequenceBrowserToolBar() override;

public slots:
  virtual void setMRMLScene(vtkMRMLScene* newScene);
  void setActiveBrowserNode(vtkMRMLSequenceBrowserNode* newActiveBrowserNode);

signals:
  void mrmlSceneChanged(vtkMRMLScene*);
  void activeBrowserNodeChanged(vtkMRMLNode* activeBrowserNode);

protected:
  QScopedPointer<qMRMLSequenceBrowserToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSequenceBrowserToolBar);
  Q_DISABLE_COPY(qMRMLSequenceBrowserToolBar);
};

#endif
