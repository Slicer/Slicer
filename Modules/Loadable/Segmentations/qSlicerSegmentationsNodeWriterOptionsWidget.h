/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __qSlicerSegmentationsNodeWriterOptionsWidget_h
#define __qSlicerSegmentationsNodeWriterOptionsWidget_h

// Slicer includes
#include "qSlicerGenericIOOptionsWidget.h"
#include "qSlicerSegmentationsModuleExport.h"
#include "vtkSlicerSegmentationsNodeWriter.h"

/// Options widget of vtkSlicerSegmentationsNodeWriter.
///
/// \deprecated Options are described by vtkSlicerSegmentationsNodeWriter (see vtkMRMLFileIOHandler::GetOptionsDescription)
/// and displayed by qSlicerGenericIOOptionsWidget.
class Q_SLICER_QTMODULES_SEGMENTATIONS_EXPORT qSlicerSegmentationsNodeWriterOptionsWidget : public qSlicerGenericIOOptionsWidget
{
  Q_OBJECT
public:
  typedef qSlicerGenericIOOptionsWidget Superclass;
  explicit qSlicerSegmentationsNodeWriterOptionsWidget(QWidget* parent = nullptr)
    : Superclass(parent)
  {
    this->setIOHandler(qSlicerGenericIOOptionsWidget::findOrCreateIOHandler<vtkSlicerSegmentationsNodeWriter>());
  }
};

#endif
