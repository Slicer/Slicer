/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// CTK includes
//#include <ctkModelTester.h>

#include "qSlicerVolumesModuleWidget.h"
#include "ui_qSlicerVolumesModule.h"

//-----------------------------------------------------------------------------
class qSlicerVolumesModuleWidgetPrivate: public Ui_qSlicerVolumesModule
{
public:
};

//-----------------------------------------------------------------------------
qSlicerVolumesModuleWidget::qSlicerVolumesModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumesModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumesModuleWidget::~qSlicerVolumesModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::setup()
{
  Q_D(qSlicerVolumesModuleWidget);
  d->setupUi(this);

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, 
                                                SLOT(setActiveVolumeNode(vtkMRMLNode*)));
  //ctkModelTester* tester = new ctkModelTester(this);
  //tester->setModel(d->ActiveVolumeNodeSelector->model());
}

//-----------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::setActiveVolumeNode(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumesModuleWidget);
  d->VolumeDisplayWidget->setMRMLVolumeNode(node);
}
