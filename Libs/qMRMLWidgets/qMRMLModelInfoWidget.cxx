/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

// QT includes

// qMRML includes
#include "qMRMLModelInfoWidget.h"
#include "ui_qMRMLModelInfoWidget.h"

// MRML includes
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLModelNode.h>

// VTK includes
#include <vtkCellData.h>
#include <vtkMassProperties.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>

//------------------------------------------------------------------------------
class qMRMLModelInfoWidgetPrivate: public Ui_qMRMLModelInfoWidget
{
  Q_DECLARE_PUBLIC(qMRMLModelInfoWidget);

protected:
  qMRMLModelInfoWidget* const q_ptr;

public:
  qMRMLModelInfoWidgetPrivate(qMRMLModelInfoWidget& object);
  void init();

  vtkMRMLModelNode* MRMLModelNode;
  vtkSmartPointer<vtkTriangleFilter> TriangleFilter;
  vtkSmartPointer<vtkMassProperties> MassProperties;
};

//------------------------------------------------------------------------------
qMRMLModelInfoWidgetPrivate::qMRMLModelInfoWidgetPrivate(qMRMLModelInfoWidget& object)
  : q_ptr(&object)
{
  this->MRMLModelNode = 0;
  this->TriangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  this->TriangleFilter->SetPassLines(0);
  this->MassProperties = vtkSmartPointer<vtkMassProperties>::New();
  this->MassProperties->SetInput( this->TriangleFilter->GetOutput() );
}

//------------------------------------------------------------------------------
void qMRMLModelInfoWidgetPrivate::init()
{
  Q_Q(qMRMLModelInfoWidget);
  this->setupUi(q);
  
  q->setEnabled(this->MRMLModelNode != 0);
}

//------------------------------------------------------------------------------
qMRMLModelInfoWidget::qMRMLModelInfoWidget(QWidget *_parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLModelInfoWidgetPrivate(*this))
{
  Q_D(qMRMLModelInfoWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLModelInfoWidget::~qMRMLModelInfoWidget()
{
}


//------------------------------------------------------------------------------
vtkMRMLModelNode* qMRMLModelInfoWidget::mrmlModelNode()const
{
  Q_D(const qMRMLModelInfoWidget);
  return d->MRMLModelNode;
}

//------------------------------------------------------------------------------
void qMRMLModelInfoWidget::setMRMLModelNode(vtkMRMLNode* node)
{
  this->setMRMLModelNode(vtkMRMLModelNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLModelInfoWidget::setMRMLModelNode(vtkMRMLModelNode* modelNode)
{
  Q_D(qMRMLModelInfoWidget);
  qvtkReconnect(d->MRMLModelNode, modelNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  //qvtkReconnect(d->MRMLModelNode, modelNode, vtkMRMLModelNode::DisplayModifiedEvent,
  //              this, SLOT(updateWidgetFromMRML()));
  d->MRMLModelNode = modelNode;
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLModelInfoWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLModelInfoWidget);
  vtkPolyData *poly = d->MRMLModelNode ? d->MRMLModelNode->GetPolyData() : 0;
  if (poly)
    {
    d->TriangleFilter->SetInput(poly);
    d->TriangleFilter->Update();
    if (d->TriangleFilter->GetOutput()->GetNumberOfCells() > 0)
      {
      d->SurfaceAreaDoubleSpinBox->setValue(d->MassProperties->GetSurfaceArea());
      d->VolumeAreaDoubleSpinBox->setValue(d->MassProperties->GetVolume());
      }
    else
      {
      d->SurfaceAreaDoubleSpinBox->setValue(0);
      d->VolumeAreaDoubleSpinBox->setValue(0);
      }

    d->NumberOfPointsSpinBox->setValue(poly->GetNumberOfPoints());
    d->NumberOfCellsSpinBox->setValue(poly->GetNumberOfCells());
    d->NumberOfPointsScalarsSpinBox->setValue(poly->GetPointData()->GetNumberOfComponents());
    d->NumberOfCellsScalarsSpinBox->setValue(poly->GetCellData()->GetNumberOfComponents());
    }
  else
    {
    d->SurfaceAreaDoubleSpinBox->setValue(0.);
    d->VolumeAreaDoubleSpinBox->setValue(0.);

    d->NumberOfPointsSpinBox->setValue(0);
    d->NumberOfCellsSpinBox->setValue(0);
    d->NumberOfPointsScalarsSpinBox->setValue(0);
    d->NumberOfCellsScalarsSpinBox->setValue(0);
    }

  vtkMRMLStorageNode *storageNode = d->MRMLModelNode ? d->MRMLModelNode->GetStorageNode() : 0;
  if (storageNode)
    {
    d->FileNameLineEdit->setText(storageNode->GetFileName());
    }
  else
    {
    d->FileNameLineEdit->setText("");
    }
  this->setEnabled(d->MRMLModelNode != 0);
}

