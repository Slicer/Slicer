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
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>

// VTK includes

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
  QObject::connect( this->ColorBySolidColorCheckBox, SIGNAL(clicked()), q, SLOT(setColorBySolid()) );

  QObject::connect( this->ColorBySolidColorPicker, SIGNAL(colorChanged(QColor)), q, SLOT(onColorBySolidChanged(QColor)) );
  QObject::connect( this->ColorByScalarsColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, 
                    SLOT(setColorByCellScalarsColorTable(vtkMRMLNode*)) );

  QObject::connect( this->ColorByScalarInvariantRadioButton, SIGNAL(clicked()), q, SLOT(setColorByScalarInvariant()) );
  QObject::connect( this->ColorByScalarInvariantComboBox, SIGNAL(currentIndexChanged(int)), q,
                    SLOT(onColorByScalarInvariantChanged(int)) );  
  QObject::connect( this->ColorByScalarInvariantDisplayRange, SIGNAL(rangeChanged(double,double)), q,
                    SLOT(setColorByScalarInvariantDisplayRange(double,double)) );

  QObject::connect( this->ColorByScalarRadioButton, SIGNAL(clicked()), q, SLOT(setColorByScalar()) );
  QObject::connect( this->ColorByScalarComboBox, SIGNAL(currentIndexChanged(int)), q,
                    SLOT(onColorByScalarChanged(int)) );  

  QObject::connect( this->ColorByMeanFiberOrientationRadioButton, SIGNAL(clicked()), q, SLOT(setColorByMeanFiberOrientation()) );
  QObject::connect( this->ColorByPointFiberOrientationRadioButton, SIGNAL(clicked()), q, SLOT(setColorByPointFiberOrientation()) );

  QObject::connect( this->OpacitySlider, SIGNAL(valueChanged(double)), q, SLOT(setOpacity(double)) );
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
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayWidget::setColorByScalarInvariantDisplayRange(double min, double max)
{
  Q_D(qSlicerTractographyDisplayWidget);
  if (!d->FiberBundleDisplayNode)
    {
    return;
    }
  d->FiberBundleDisplayNode->SetScalarRange(min, max);
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
void qSlicerTractographyDisplayWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerTractographyDisplayWidget);
  if ( !d->FiberBundleNode || !d->FiberBundleDisplayNode || this->m_updating )
    {
    return;
    }
  
  d->VisibilityCheckBox->setChecked( d->FiberBundleDisplayNode->GetVisibility() );
  d->OpacitySlider->setValue( d->FiberBundleDisplayNode->GetOpacity() );
  
  d->ColorByScalarsColorTableComboBox->setCurrentNode
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
        else
         {
         d->FiberBundleDisplayNode->SetColorMode(vtkMRMLFiberBundleDisplayNode::colorModeSolid);
         }
      case vtkMRMLFiberBundleDisplayNode::colorModeSolid:
        {
        double color[3];
        d->FiberBundleDisplayNode->GetColor(color);
        d->ColorBySolidColorPicker->setColor(QColor::fromRgbF(color[0],color[1],color[2]) );
        d->ColorBySolidColorCheckBox->setChecked(1);
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
}
