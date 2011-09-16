// QT includes

// qMRML includes
#include "qSlicerTractographyDisplayWidget.h"
#include "ui_qSlicerTractographyDisplayWidget.h"

// MRML includes
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

  std::vector<int> supportedScalarModes;
  vtkMRMLFiberBundleDisplayNode::GetSupportedColorModes(supportedScalarModes);
  for (std::vector<int>::iterator it = supportedScalarModes.begin(); it!=supportedScalarModes.end(); ++it) 
    {
    this->ColorByScalarInvariantComboBox->addItem(
        vtkMRMLDiffusionTensorDisplayPropertiesNode::GetScalarEnumAsString(*it), *it);
    }

  this->ColorBySolidColorPicker->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

  QObject::connect( this->VisibilityCheckBox, SIGNAL(clicked(bool)), q, SLOT(setVisibility(bool)) );
  QObject::connect( this->ColorByCellScalarsCheckBox, SIGNAL(clicked()), q, SLOT(setColorByCellScalars()) );
  QObject::connect( this->ColorBySolidCheckBox, SIGNAL(clicked()), q, SLOT(setColorBySolid()) );
  QObject::connect( this->ColorBySolidColorPicker, SIGNAL(colorChanged(QColor)), q, SLOT(onColorBySolidChanged(QColor)) );
  QObject::connect( this->ColorByScalarsColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, 
                    SLOT(setColorByCellScalarsColorTable(vtkMRMLNode*)) );
  QObject::connect( this->ColorByScalarInvariantCheckBox, SIGNAL(clicked()), q, SLOT(setColorByScalarInvariant()) );
  QObject::connect( this->ColorByScalarInvariantComboBox, SIGNAL(currentIndexChanged(int)), q,
                    SLOT(onColorByScalarInvariantChanged(int)) );  
  QObject::connect( this->ColorByScalarInvariantDisplayRange, SIGNAL(rangeChanged(double,double)), q,
                    SLOT(setColorByScalarInvariantDisplayRange(double,double)) );
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
  if ( !d->FiberBundleDisplayNode || this->m_updating )
    {
    return;
    }
  
  d->VisibilityCheckBox->setChecked( d->FiberBundleDisplayNode->GetVisibility() );
  d->OpacitySlider->setValue( d->FiberBundleDisplayNode->GetOpacity() );
  
  d->ColorByScalarsColorTableComboBox->setCurrentNode
    (d->FiberBundleDisplayNode->GetColorNodeID());
  switch ( d->FiberBundleDisplayNode->GetColorMode() )
    {
      case vtkMRMLFiberBundleDisplayNode::colorModeScalar:
        {
        vtkMRMLDiffusionTensorDisplayPropertiesNode *dpNode = 
          d->FiberBundleDisplayNode->GetDiffusionTensorDisplayPropertiesNode();
        d->ColorByScalarInvariantComboBox->setCurrentIndex( 
          d->ColorByScalarInvariantComboBox->findData( dpNode->GetColorGlyphBy() ));
        d->ColorByScalarInvariantCheckBox->setChecked(1);
        }
        break;
      case vtkMRMLFiberBundleDisplayNode::colorModeUseCellScalars:
        {
        d->ColorByCellScalarsCheckBox->setChecked(1);
        }
        break;
      case vtkMRMLFiberBundleDisplayNode::colorModeSolid:
        {
        double color[3];
        d->FiberBundleDisplayNode->GetColor(color);
        d->ColorBySolidColorPicker->setColor(QColor::fromRgbF(color[0],color[1],color[2]) );
        d->ColorBySolidCheckBox->setChecked(1);
        }
        break;
   }
}
