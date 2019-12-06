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
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __qSlicerSegmentationsNodeWriterOptionsWidget_h
#define __qSlicerSegmentationsNodeWriterOptionsWidget_h

/// QtCore includes
#include "qSlicerSegmentationsModuleExport.h"
#include "qSlicerNodeWriterOptionsWidget.h"

class qSlicerSegmentationsNodeWriterOptionsWidgetPrivate;

class Q_SLICER_QTMODULES_SEGMENTATIONS_EXPORT qSlicerSegmentationsNodeWriterOptionsWidget
  : public qSlicerNodeWriterOptionsWidget
{
  Q_OBJECT

public:
  typedef qSlicerNodeWriterOptionsWidget Superclass;
  explicit qSlicerSegmentationsNodeWriterOptionsWidget(QWidget* parent = nullptr);
  ~qSlicerSegmentationsNodeWriterOptionsWidget() override;

public slots:
  void setObject(vtkObject* object) override;

protected slots:
  virtual void setCropToMinimumExtent(bool crop);

private:
  Q_DECLARE_PRIVATE_D(qGetPtrHelper(qSlicerIOOptions::d_ptr), qSlicerSegmentationsNodeWriterOptionsWidget);
};

#endif
