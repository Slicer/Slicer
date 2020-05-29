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
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __qSlicerTextsModuleWidget_h
#define __qSlicerTextsModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

// Texts includes
#include "qSlicerTextsModuleExport.h"

class qSlicerTextsModuleWidgetPrivate;

class Q_SLICER_QTMODULES_TEXTS_EXPORT qSlicerTextsModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTextsModuleWidget(QWidget *parent=nullptr);
  ~qSlicerTextsModuleWidget() override;

  /// Support of node editing. Selects node in user interface that the user wants to edit
  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;

protected:

  void setup() override;

protected:
  QScopedPointer<qSlicerTextsModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTextsModuleWidget);
  Q_DISABLE_COPY(qSlicerTextsModuleWidget);
};

#endif
