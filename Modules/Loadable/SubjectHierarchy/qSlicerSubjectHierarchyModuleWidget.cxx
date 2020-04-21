/*==============================================================================

  Program: 3D Slicer

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

// Subject Hierarchy includes
#include "qSlicerSubjectHierarchyModuleWidget.h"
#include "ui_qSlicerSubjectHierarchyModule.h"

#include "qSlicerSubjectHierarchyPluginLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qSlicerSubjectHierarchyModuleWidgetPrivate: public Ui_qSlicerSubjectHierarchyModule
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyModuleWidget);
protected:
  qSlicerSubjectHierarchyModuleWidget* const q_ptr;
public:
  qSlicerSubjectHierarchyModuleWidgetPrivate(qSlicerSubjectHierarchyModuleWidget& object);
  ~qSlicerSubjectHierarchyModuleWidgetPrivate();
public:
  /// Subject hierarchy plugin logic
  qSlicerSubjectHierarchyPluginLogic* PluginLogic;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidgetPrivate::qSlicerSubjectHierarchyModuleWidgetPrivate(qSlicerSubjectHierarchyModuleWidget& object)
  : q_ptr(&object)
  , PluginLogic(nullptr)
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidgetPrivate::~qSlicerSubjectHierarchyModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidget::qSlicerSubjectHierarchyModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerSubjectHierarchyModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidget::~qSlicerSubjectHierarchyModuleWidget() = default;

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginLogic* qSlicerSubjectHierarchyModuleWidget::pluginLogic()
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);
  return d->PluginLogic;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setPluginLogic(qSlicerSubjectHierarchyPluginLogic* pluginLogic)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);
  d->PluginLogic = pluginLogic;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setup()
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
