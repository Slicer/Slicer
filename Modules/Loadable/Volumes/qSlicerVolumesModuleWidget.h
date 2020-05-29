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

#ifndef __qSlicerVolumesModuleWidget_h
#define __qSlicerVolumesModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVolumesModuleExport.h"

class qSlicerVolumesModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_Volumes
class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerVolumesModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVolumesModuleWidget(QWidget *parent=nullptr);
  ~qSlicerVolumesModuleWidget() override;

  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;

protected:
  void setup() override;

protected slots:
  void nodeSelectionChanged(vtkMRMLNode*);
  void updateWidgetFromMRML();
  void convertVolume();

protected:
  QScopedPointer<qSlicerVolumesModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumesModuleWidget);
  Q_DISABLE_COPY(qSlicerVolumesModuleWidget);
};

#endif
