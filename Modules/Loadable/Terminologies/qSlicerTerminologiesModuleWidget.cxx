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

// Slicer includes
#include "qSlicerTerminologiesModuleWidget.h"
#include "ui_qSlicerTerminologiesModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Terminologies
class qSlicerTerminologiesModuleWidgetPrivate: public Ui_qSlicerTerminologiesModule
{
public:
  qSlicerTerminologiesModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTerminologiesModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerTerminologiesModuleWidgetPrivate::qSlicerTerminologiesModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerTerminologiesModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTerminologiesModuleWidget::qSlicerTerminologiesModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerTerminologiesModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerTerminologiesModuleWidget::~qSlicerTerminologiesModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerTerminologiesModuleWidget::setup()
{
  Q_D(qSlicerTerminologiesModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
