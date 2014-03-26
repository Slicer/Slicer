/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerMultiVolumeRenderingModuleWidget_h
#define __qSlicerMultiVolumeRenderingModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerMultiVolumeRenderingModuleExport.h"

class qSlicerMultiVolumeRenderingModuleWidgetPrivate;
class vtkMRMLAnnotationROINode;
class vtkMRMLNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLViewNode;
class vtkMRMLMultiVolumeRenderingDisplayNode;
class vtkMRMLVolumePropertyNode;

/// \ingroup Slicer_QtModules_MultiVolumeRendering
class Q_SLICER_QTMODULES_MULTIVOLUMERENDERING_EXPORT qSlicerMultiVolumeRenderingModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerMultiVolumeRenderingModuleWidget(QWidget *parent=0);
  virtual ~qSlicerMultiVolumeRenderingModuleWidget();

  vtkMRMLScalarVolumeNode* getCurrentBgVolumeNode()const;
  vtkMRMLScalarVolumeNode* getCurrentFgVolumeNode()const;
  vtkMRMLScalarVolumeNode* getCurrentLabelmapVolumeNode()const;

public slots:

  void setMRMLDisplayNode(vtkMRMLNode* node);

protected slots:
  void onVisibilityBgVolumeChanged(bool);
  void onVisibilityFgVolumeChanged(bool);
  void onVisibilityLabelmapVolumeChanged(bool);

  void onCurrentBgVolumeMRMLImageNodeChanged(vtkMRMLNode* node);
  void onCurrentFgVolumeMRMLImageNodeChanged(vtkMRMLNode* node);
  void onCurrentLabelmapVolumeMRMLImageNodeChanged(vtkMRMLNode* node);

  void onCurrentMRMLDisplayNodeChanged(vtkMRMLNode* node);

  void updateRenderingFromMRMLDisplayNode();

protected:
  QScopedPointer<qSlicerMultiVolumeRenderingModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerMultiVolumeRenderingModuleWidget);
  Q_DISABLE_COPY(qSlicerMultiVolumeRenderingModuleWidget);
};

#endif
