// QT includes

// qMRML includes
#include "qSlicerTractographyDisplayWidget.h"
#include "ui_qSlicerTractographyDisplayWidget.h"

// MRML includes
#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkMRMLFiberBundleNode.h>
#include <vtkMRMLFiberBundleDisplayNode.h>
#include <vtkMRMLFiberBundleLineDisplayNode.h>
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>

// VTK includes

#include <math.h>
//------------------------------------------------------------------------------
class qSlicerTractographyDisplayWidgetPrivate: 
  public Ui_qSlicerTractographyDisplayWidget
{
  Q_DECLARE_PUBLIC(qSlicerTractographyDisplayWidget);

protected:
  qSlicerTractographyDisplayWidget* const q_ptr;

public:
  qSlicerTractographyDisplayWidgetPrivate(qSlicerTractographyDisplayWidget& object);
  void init();
  bool centeredOrigin(double* origin)const;

  vtkMRMLFiberBundleNode* FiberBundleNode;
  vtkMRMLFiberBundleDisplayNode* FiberBundleDisplayNode;
  vtkMRMLDiffusionTensorDisplayPropertiesNode* DiffusionTensorDisplayPropertiesNode;
};

//------------------------------------------------------------------------------
qSlicerTractographyDisplayWidgetPrivate::qSlicerTractographyDisplayWidgetPrivate
                                      (qSlicerTractographyDisplayWidget& object)
  : q_ptr(&object)
{
  this->FiberBundleDisplayNode = 0;
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidgetPrivate::init()
{
  Q_Q(qSlicerTractographyDisplayWidget);
  this->setupUi(q);

  for (int i = 0; i < vtkMRMLFiberBundleDisplayNode::GetNumberOfScalarInvariants(); i++ ) 
    {
    const int scalarInvariant = vtkMRMLFiberBundleDisplayNode::GetNthScalarInvariant(i);
    this->ColorByScalarInvariantComboBox->addItem(
        vtkMRMLDiffusionTensorDisplayPropertiesNode::GetScalarEnumAsString(scalarInvariant), scalarInvariant);
    }

  this->ColorBySolidColorPicker->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

  QObject::connect( this->VisibilityCheckBox, SIGNAL(clicked(bool)), q, SLOT(setVisibility(bool)) );
  QObject::connect( this->ColorByCellScalarsRadioButton, SIGNAL(clicked()), q, SLOT(setColorByCellScalars()) );
  QObject::connect( this->ColorBySolidColorRadioButton, SIGNAL(clicked()), q, SLOT(setColorBySolid()) );
  QObject::connect( this->ColorBySolidColorPicker, SIGNAL(colorChanged(QColor)), q, SLOT(onColorBySolidChanged(QColor)) );

  QObject::connect( this->ColorByScalarsColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, 
                    SLOT(setColorByCellScalarsColorTable(vtkMRMLNode*)) );

  QObject::connect( this->ColorByScalarInvariantRadioButton, SIGNAL(clicked()), q, SLOT(setColorByScalarInvariant()) );
  QObject::connect( this->ColorByScalarInvariantComboBox, SIGNAL(currentIndexChanged(int)), q,
                    SLOT(onColorByScalarInvariantChanged(int)) );  

  QObject::connect( this->ColorByScalarRadioButton, SIGNAL(clicked()), q, SLOT(setColorByScalar()) );
  QObject::connect( this->ColorByScalarComboBox, SIGNAL(currentIndexChanged(int)), q,
                    SLOT(onColorByScalarChanged(int)) );  

  QObject::connect( this->ColorByMeanFiberOrientationRadioButton, SIGNAL(clicked()), q, SLOT(setColorByMeanFiberOrientation()) );
  QObject::connect( this->ColorByPointFiberOrientationRadioButton, SIGNAL(clicked()), q, SLOT(setColorByPointFiberOrientation()) );

  QObject::connect( this->OpacitySlider, SIGNAL(valueChanged(double)), q, SLOT(setOpacity(double)) );

  QObject::connect(this->MaterialPropertyWidget, SIGNAL(colorChanged(QColor)),
                   q, SLOT(setColor(QColor)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(opacityChanged(double)),
                   q, SLOT(setOpacity(double)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(ambientChanged(double)),
                   q, SLOT(setAmbient(double)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(diffuseChanged(double)),
                   q, SLOT(setDiffuse(double)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(specularChanged(double)),
                   q, SLOT(setSpecular(double)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(specularPowerChanged(double)),
                   q, SLOT(setSpecularPower(double)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(backfaceCullingChanged(bool)),
                   q, SLOT(setBackfaceCulling(bool)));

  QObject::connect(this->AutoWL, SIGNAL(clicked(bool)),
                  q, SLOT(setAutoWindowLevel(bool)));

  QObject::connect(this->FiberBundleColorRangeWidget, SIGNAL(valuesChanged(double, double)),
                  q, SLOT(setWindowLevel(double, double)));
  QObject::connect(this->FiberBundleColorRangeWidget, SIGNAL(rangeChanged(double, double)),
                  q, SLOT(setWindowLevelLimits(double, double)));
}



//------------------------------------------------------------------------------
qSlicerTractographyDisplayWidget::qSlicerTractographyDisplayWidget(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTractographyDisplayWidgetPrivate(*this))
{
  Q_D(qSlicerTractographyDisplayWidget);
  d->init();
  //TBD: is there some better way to avoid the updateWidgetFromMRML ringback?
  this->m_updating = 0;
}

//------------------------------------------------------------------------------
qSlicerTractographyDisplayWidget::~qSlicerTractographyDisplayWidget()
{
}

//------------------------------------------------------------------------------
vtkMRMLFiberBundleNode* qSlicerTractographyDisplayWidget::FiberBundleNode()const
{
  Q_D(const qSlicerTractographyDisplayWidget);
  return d->FiberBundleNode;
}

//------------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* qSlicerTractographyDisplayWidget::FiberBundleDisplayNode()const
{
  Q_D(const qSlicerTractographyDisplayWidget);
  return d->FiberBundleDisplayNode;
}

//------------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* qSlicerTractographyDisplayWidget::DiffusionTensorDisplayPropertiesNode()const
{
  Q_D(const qSlicerTractographyDisplayWidget);
  return d->FiberBundleDisplayNode ?
    d->FiberBundleDisplayNode->GetDiffusionTensorDisplayPropertiesNode() : 0;
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setFiberBundleNode(vtkMRMLNode* node)
{
  this->setFiberBundleNode(vtkMRMLFiberBundleNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setFiberBundleNode(vtkMRMLFiberBundleNode* fiberBundleNode)
{
  Q_D(qSlicerTractographyDisplayWidget);
  vtkMRMLFiberBundleNode *oldNode = 
    this->FiberBundleNode();
  
  d->FiberBundleNode = fiberBundleNode;

  qvtkReconnect( oldNode, this->FiberBundleNode(),
                vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()) );

  if (fiberBundleNode == 0)
    {
    setFiberBundleDisplayNode((vtkMRMLFiberBundleDisplayNode*)NULL);
    }

  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setFiberBundleDisplayNode(vtkMRMLNode* node)
{
  this->setFiberBundleDisplayNode(vtkMRMLFiberBundleDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setFiberBundleDisplayNode(vtkMRMLFiberBundleDisplayNode* FiberBundleDisplayNode)
{
  Q_D(qSlicerTractographyDisplayWidget);
  vtkMRMLFiberBundleDisplayNode *oldDisplayNode = 
    this->FiberBundleDisplayNode();
  vtkMRMLDiffusionTensorDisplayPropertiesNode *oldDisplayPropertiesNode =
    this->DiffusionTensorDisplayPropertiesNode();
  
  d->FiberBundleDisplayNode = FiberBundleDisplayNode;
  
  if (vtkMRMLFiberBundleLineDisplayNode::SafeDownCast(d->FiberBundleDisplayNode))
    {
    d->MaterialPropertyWidget->setHidden(true);
    d->MaterialPropertyGroupBox->setHidden(true);
    }
  else
    {
    d->MaterialPropertyWidget->setHidden(false);
    d->MaterialPropertyGroupBox->setHidden(false);
    }


  qvtkReconnect( oldDisplayNode, this->FiberBundleDisplayNode(),
                vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()) );
  qvtkReconnect( oldDisplayPropertiesNode, this->DiffusionTensorDisplayPropertiesNode(),
                vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()) );
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setVisibility(bool state)
{
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetVisibility(state);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::clickColorBySolid(bool checked)
{
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode) 
    {
    return;
    }

  if (checked)
  {
    d->FiberBundleDisplayNode->SetColorModeToSolid();
    d->FiberBundleDisplayNode->SetScalarVisibility(0);
  } else {
    if (d->ColorByScalarInvariantRadioButton->isChecked())
    {
      d->FiberBundleDisplayNode->SetColorModeToScalar();
    }
    else if (d->ColorByScalarRadioButton->isChecked())
    {
      d->FiberBundleDisplayNode->SetColorModeToScalarData();
    }
    else if (d->ColorByCellScalarsRadioButton->isChecked()) 
    {
      d->FiberBundleDisplayNode->SetColorModeToUseCellScalars();
    }
    else if (d->FiberBundleNode->GetPolyData()->GetPointData()->GetTensors()) 
    {
      d->FiberBundleDisplayNode->SetColorModeToScalar();
    } else {
      d->FiberBundleDisplayNode->SetColorModeToScalarData();
    }
  }
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::onColorBySolidChanged(const QColor &color)
{
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode) 
    {
    return;
    }
  d->FiberBundleDisplayNode->SetColor(color.redF(), color.greenF(), color.blueF());
}
//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setColorBySolid()
{
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetColorModeToSolid();
  d->FiberBundleDisplayNode->SetScalarVisibility(0);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setColorByScalar()
{
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetColorModeToScalarData();
  d->FiberBundleDisplayNode->SetScalarVisibility(1);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::onColorByScalarChanged(int scalarIndex)
{
  Q_D(qSlicerTractographyDisplayWidget);
  if (
      !d->FiberBundleDisplayNode || !d->FiberBundleNode ||
      !d->FiberBundleNode->GetPolyData() || !d->FiberBundleNode->GetPolyData()->GetPointData()
      )
    {
    return;
    }

  QString activeScalarName = d->ColorByScalarComboBox->itemText(scalarIndex);
  d->FiberBundleDisplayNode->SetActiveScalarName(activeScalarName.toLatin1());
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setColorByMeanFiberOrientation()
{
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetColorModeToMeanFiberOrientation(); 
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setColorByPointFiberOrientation()
{
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetColorModeToPointFiberOrientation();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setColorByScalarInvariant()
{
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetColorModeToScalar();
  d->FiberBundleDisplayNode->SetScalarVisibility(1);

  vtkMRMLDiffusionTensorDisplayPropertiesNode* displayPropertiesNode = 
                          vtkMRMLDiffusionTensorDisplayPropertiesNode::
                          SafeDownCast(d->FiberBundleDisplayNode->GetDiffusionTensorDisplayPropertiesNode());
  if (displayPropertiesNode)
    {
    if (displayPropertiesNode->GetColorGlyphBy() == vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation)
      d->ColorByScalarsColorTableComboBox->setEnabled(0);
    else
      d->ColorByScalarsColorTableComboBox->setEnabled(1);
    }
  this->updateScalarRange();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::onColorByScalarInvariantChanged(int scalarInvariantIndex)
{
  Q_D(qSlicerTractographyDisplayWidget);
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  vtkMRMLDiffusionTensorDisplayPropertiesNode* displayPropertiesNode = 
                          vtkMRMLDiffusionTensorDisplayPropertiesNode::
                          SafeDownCast(d->FiberBundleDisplayNode->GetDiffusionTensorDisplayPropertiesNode());
  if (displayPropertiesNode)
    {
    displayPropertiesNode->SetColorGlyphBy(d->ColorByScalarInvariantComboBox->itemData(scalarInvariantIndex).toInt());
    if (displayPropertiesNode->GetColorGlyphBy() == vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation)
      d->ColorByScalarsColorTableComboBox->setEnabled(0);
    else
      d->ColorByScalarsColorTableComboBox->setEnabled(1);

    this->updateScalarRange();
    }

}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setColorByCellScalars()
{
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode) 
    { 
    return;
    }
  d->FiberBundleDisplayNode->SetColorModeToUseCellScalars();
  d->FiberBundleDisplayNode->SetScalarVisibility(1);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setColorByCellScalarsColorTable(vtkMRMLNode* colortableNode)
{
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode || !colortableNode) 
    { 
    return;
    }
  Q_ASSERT(vtkMRMLColorNode::SafeDownCast(colortableNode));
  d->FiberBundleDisplayNode->SetAndObserveColorNodeID(colortableNode->GetID());
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setAutoWindowLevel(bool value)
{
  if (this->m_updating)
    return;
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode) 
    { 
    return;
    }
  d->FiberBundleDisplayNode->SetAutoScalarRange(value);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setWindowLevel(double minValue, double maxValue)
{
  if (this->m_updating)
    return;
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode) 
    { 
    return;
    }

  d->FiberBundleDisplayNode->SetScalarRange(minValue, maxValue);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setWindowLevelLimits(double minValue, double maxValue)
{
  if (this->m_updating)
    return;
  Q_D(qSlicerTractographyDisplayWidget);
  const double step = (maxValue - minValue) / 100.;
  d->FiberBundleColorRangeWidget->setSingleStep(step);
  d->FiberBundleColorRangeWidget->setDecimals(ceil(fabs(log10(step))));
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setOpacity(double opacity)
{
  Q_D(qSlicerTractographyDisplayWidget);
  
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetOpacity(opacity);
}


//------------------------------------------------------------------------------
QColor qSlicerTractographyDisplayWidget::color()const
{
  Q_D(const qSlicerTractographyDisplayWidget);
  return d->MaterialPropertyWidget->color();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setColor(const QColor& color)
{
  Q_D(qSlicerTractographyDisplayWidget);
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  // QColors loose precision in the numbers, don't reset the color if it didn't
  // "really" change.
  double* oldColor = d->FiberBundleDisplayNode->GetColor();
  if (QColor::fromRgbF(oldColor[0], oldColor[1], oldColor[2]) != color)
    {
    d->FiberBundleDisplayNode->SetColor(color.redF(), color.greenF(), color.blueF());
    }
}


//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setAmbient(double ambient)
{
  Q_D(qSlicerTractographyDisplayWidget);
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetAmbient(ambient);
}

//------------------------------------------------------------------------------
double qSlicerTractographyDisplayWidget::ambient()const
{
  Q_D(const qSlicerTractographyDisplayWidget);
  return d->MaterialPropertyWidget->ambient();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setDiffuse(double diffuse)
{
  Q_D(qSlicerTractographyDisplayWidget);
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetDiffuse(diffuse);
}

//------------------------------------------------------------------------------
double qSlicerTractographyDisplayWidget::diffuse()const
{
  Q_D(const qSlicerTractographyDisplayWidget);
  return d->MaterialPropertyWidget->diffuse();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setSpecular(double specular)
{
  Q_D(qSlicerTractographyDisplayWidget);
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetSpecular(specular);
}

//------------------------------------------------------------------------------
double qSlicerTractographyDisplayWidget::specular()const
{
  Q_D(const qSlicerTractographyDisplayWidget);
  return d->MaterialPropertyWidget->specular();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setSpecularPower(double specularPower)
{
  Q_D(qSlicerTractographyDisplayWidget);
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetPower(specularPower);
}

//------------------------------------------------------------------------------
double qSlicerTractographyDisplayWidget::specularPower()const
{
  Q_D(const qSlicerTractographyDisplayWidget);
  return d->MaterialPropertyWidget->specularPower();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setBackfaceCulling(bool backfaceCulling)
{
  Q_D(qSlicerTractographyDisplayWidget);
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetBackfaceCulling(backfaceCulling);
}

//------------------------------------------------------------------------------
bool qSlicerTractographyDisplayWidget::backfaceCulling()const
{
  Q_D(const qSlicerTractographyDisplayWidget);
  return d->MaterialPropertyWidget->backfaceCulling();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::updateScalarRange()
{
  Q_D(qSlicerTractographyDisplayWidget);

  if ( !d->FiberBundleNode || !d->FiberBundleDisplayNode || !d->FiberBundleDisplayNode->GetOutputPolyData())
    {
    return;
    }
 
 double range[2];
 bool was_updating = this->m_updating;
 this->m_updating = true;

 d->FiberBundleDisplayNode->GetOutputPolyData()->Update();
 d->FiberBundleDisplayNode->GetScalarRange(range);
 if (d->FiberBundleDisplayNode->GetAutoScalarRange())
 {
   d->FiberBundleColorRangeWidget->setMinimumValue(range[0]);
   d->FiberBundleColorRangeWidget->setMaximumValue(range[1]);
   d->FiberBundleColorRangeWidget->setRange(range[0], range[1]);
 }
 else
 {
  d->FiberBundleColorRangeWidget->setMinimumValue(range[0]);
  d->FiberBundleColorRangeWidget->setMaximumValue(range[1]);

  if ((d->FiberBundleColorRangeWidget->minimum() > range[0]) ||
      (d->FiberBundleColorRangeWidget->maximum() < range[1]))
    d->FiberBundleColorRangeWidget->setRange(range[0], range[1]);
  }
 const double step = (range[1] - range[0]) / 100.;
 d->FiberBundleColorRangeWidget->setSingleStep(step);
 d->FiberBundleColorRangeWidget->setDecimals(ceil(fabs(log10(step))));
 this->m_updating = was_updating;
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerTractographyDisplayWidget);
  if ( !d->FiberBundleNode || !d->FiberBundleDisplayNode || this->m_updating )
    {
    return;
    }
  this->m_updating = 1;

  d->VisibilityCheckBox->setChecked( d->FiberBundleDisplayNode->GetVisibility() );
  d->OpacitySlider->setValue( d->FiberBundleDisplayNode->GetOpacity() );
  
  d->ColorByScalarsColorTableComboBox->setCurrentNodeID
    (d->FiberBundleDisplayNode->GetColorNodeID());
  d->ColorByScalarComboBox->setDataSet(vtkDataSet::SafeDownCast(d->FiberBundleNode->GetPolyData()));

  bool hasTensors = false;
  if (d->FiberBundleNode && d->FiberBundleNode->GetPolyData() && 
      d->FiberBundleNode->GetPolyData()->GetPointData() &&
      d->FiberBundleNode->GetPolyData()->GetPointData()->GetTensors() )
    {
    hasTensors = true;
    }
//  const bool colorSolid = d->FiberBundleDisplayNode->GetColorMode() == vtkMRMLFiberBundleDisplayNode::colorModeSolid;

  d->ColorByScalarInvariantRadioButton->setEnabled(hasTensors);// && !colorSolid);
  d->ColorByScalarInvariantComboBox->setEnabled(hasTensors);// && !colorSolid);
//  d->ColorByScalarRadioButton->setEnabled(!colorSolid);
//  d->ColorByScalarComboBox->setEnabled(!colorSolid);
//  d->ColorByCellScalarsRadioButton->setEnabled(!colorSolid);


  switch ( d->FiberBundleDisplayNode->GetColorMode() )
    {
      case vtkMRMLFiberBundleDisplayNode::colorModeScalar:
        if (hasTensors)
          {
          vtkMRMLDiffusionTensorDisplayPropertiesNode *dpNode = 
            d->FiberBundleDisplayNode->GetDiffusionTensorDisplayPropertiesNode();

          if ( dpNode )
            {
            d->ColorByScalarInvariantComboBox->setCurrentIndex( 
              d->ColorByScalarInvariantComboBox->findData( dpNode->GetColorGlyphBy() ));

            d->ColorByScalarInvariantRadioButton->setChecked(1);
            }
          break;
          }
        else if (d->FiberBundleNode && d->FiberBundleNode->GetPolyData() &&
                 d->FiberBundleNode->GetPolyData()->GetPoints() &&
                 d->FiberBundleNode->GetPolyData()->GetPointData() )
         {
         // no tensors in a valid polydata
         d->FiberBundleDisplayNode->SetColorMode(vtkMRMLFiberBundleDisplayNode::colorModeSolid);
         }
      case vtkMRMLFiberBundleDisplayNode::colorModeSolid:
        {
        double color[3];
        d->FiberBundleDisplayNode->GetColor(color);
        d->ColorBySolidColorPicker->setColor(QColor::fromRgbF(color[0],color[1],color[2]) );
        d->ColorBySolidColorRadioButton->setChecked(1);
        }
        break;
      case vtkMRMLFiberBundleDisplayNode::colorModeUseCellScalars:
        {
        d->ColorByCellScalarsRadioButton->setChecked(1);
        }
        break;
      case vtkMRMLFiberBundleDisplayNode::colorModeScalarData:
        {

        if (d->ColorByScalarComboBox->currentText() != d->FiberBundleDisplayNode->GetActiveScalarName())
        {
          d->ColorByScalarComboBox->setCurrentIndex( 
            d->ColorByScalarComboBox->findData( d->FiberBundleDisplayNode->GetActiveScalarName() )
          );
        }

        d->ColorByScalarRadioButton->setChecked(1);

        }
        break;
   }
   if (
       d->FiberBundleDisplayNode->GetColorMode() == vtkMRMLFiberBundleDisplayNode::colorModeScalarData ||
       d->FiberBundleDisplayNode->GetColorMode() == vtkMRMLFiberBundleDisplayNode::colorModeScalar
      )
   {
     d->AutoWL->setEnabled(1);
     d->AutoWL->setChecked(d->FiberBundleDisplayNode->GetAutoScalarRange());
     if (d->FiberBundleDisplayNode->GetAutoScalarRange())
     {
       d->FiberBundleColorRangeWidget->setEnabled(0);
     }
     else
     {
       d->FiberBundleColorRangeWidget->setEnabled(1);
      }
     this->updateScalarRange();

   } else {
     d->AutoWL->setEnabled(0);
     d->FiberBundleColorRangeWidget->setEnabled(0);
   }

  d->MaterialPropertyWidget->setColor(
    QColor::fromRgbF(d->FiberBundleDisplayNode->GetColor()[0],
                     d->FiberBundleDisplayNode->GetColor()[1],
                     d->FiberBundleDisplayNode->GetColor()[2]));
  d->MaterialPropertyWidget->setOpacity(d->FiberBundleDisplayNode->GetOpacity());
  d->MaterialPropertyWidget->setAmbient(d->FiberBundleDisplayNode->GetAmbient());
  d->MaterialPropertyWidget->setDiffuse(d->FiberBundleDisplayNode->GetDiffuse());
  d->MaterialPropertyWidget->setSpecular(d->FiberBundleDisplayNode->GetSpecular());
  d->MaterialPropertyWidget->setSpecularPower(d->FiberBundleDisplayNode->GetPower());
  d->MaterialPropertyWidget->setBackfaceCulling(d->FiberBundleDisplayNode->GetBackfaceCulling());

  this->m_updating = 0;
}
