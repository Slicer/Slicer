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

#ifndef __qSlicerVolumesIOOptionsWidget_h
#define __qSlicerVolumesIOOptionsWidget_h

// Slicer includes
#include "qSlicerGenericIOOptionsWidget.h"
#include "qSlicerVolumesModuleExport.h"
#include "vtkSlicerVolumesReader.h"

/// Options widget of vtkSlicerVolumesReader.
///
/// \deprecated Options are described by vtkSlicerVolumesReader (see vtkMRMLFileIOHandler::GetOptionsDescription)
/// and displayed by qSlicerGenericIOOptionsWidget.
class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerVolumesIOOptionsWidget : public qSlicerGenericIOOptionsWidget
{
  Q_OBJECT
public:
  typedef qSlicerGenericIOOptionsWidget Superclass;
  explicit qSlicerVolumesIOOptionsWidget(QWidget* parent = nullptr)
    : Superclass(parent)
  {
    this->setIOHandler(qSlicerGenericIOOptionsWidget::findOrCreateIOHandler<vtkSlicerVolumesReader>());
  }
};

#endif
