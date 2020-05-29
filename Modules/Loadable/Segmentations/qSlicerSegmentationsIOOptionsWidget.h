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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qSlicerSegmentationsIOOptionsWidget_h
#define __qSlicerSegmentationsIOOptionsWidget_h

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerIOOptionsWidget.h"

#include "qSlicerSegmentationsModuleExport.h"

class qSlicerSegmentationsIOOptionsWidgetPrivate;

/// \ingroup Slicer_QtModules_Segmentations
class Q_SLICER_QTMODULES_SEGMENTATIONS_EXPORT qSlicerSegmentationsIOOptionsWidget :
  public qSlicerIOOptionsWidget
{
  Q_OBJECT
public:
  qSlicerSegmentationsIOOptionsWidget(QWidget *parent=nullptr);
  ~qSlicerSegmentationsIOOptionsWidget() override;

protected slots:
  /// Update IO plugin properties
  void updateProperties();

private:
  Q_DECLARE_PRIVATE_D(qGetPtrHelper(qSlicerIOOptions::d_ptr), qSlicerSegmentationsIOOptionsWidget);
  Q_DISABLE_COPY(qSlicerSegmentationsIOOptionsWidget);
};

#endif
