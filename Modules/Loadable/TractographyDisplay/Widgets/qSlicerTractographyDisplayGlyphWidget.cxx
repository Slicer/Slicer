// QT includes

// qMRML includes
#include "qSlicerTractographyDisplayGlyphWidget.h"
#include "ui_qSlicerTractographyDisplayGlyphWidget.h"

// MRML includes
#include <vtkMRMLFiberBundleDisplayNode.h>
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>

// VTK includes

//------------------------------------------------------------------------------
class qSlicerTractographyDisplayGlyphWidgetPrivate:
  public Ui_qSlicerTractographyDisplayGlyphWidget
{
  Q_DECLARE_PUBLIC(qSlicerTractographyDisplayGlyphWidget);

protected:
  qSlicerTractographyDisplayGlyphWidget* const q_ptr;

public:
  qSlicerTractographyDisplayGlyphWidgetPrivate(qSlicerTractographyDisplayGlyphWidget& object);
  void init();
  bool centeredOrigin(double* origin)const;

  vtkMRMLFiberBundleDisplayNode* FiberBundleDisplayNode;
  vtkMRMLDiffusionTensorDisplayPropertiesNode* DiffusionTensorDisplayPropertiesNode;
};

//------------------------------------------------------------------------------
qSlicerTractographyDisplayGlyphWidgetPrivate::qSlicerTractographyDisplayGlyphWidgetPrivate
                                      (qSlicerTractographyDisplayGlyphWidget& object)
  : q_ptr(&object)
{
  this->FiberBundleDisplayNode = 0;
  this->DiffusionTensorDisplayPropertiesNode = 0;
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidgetPrivate::init()
{
  Q_Q(qSlicerTractographyDisplayGlyphWidget);
  this->setupUi(q);

  QObject::connect( this->GlyphTypeSelector, SIGNAL(currentIndexChanged(int)), q,
                    SLOT(setGlyphType(int)) );
  QObject::connect( this->GlyphEigenvectorSelector, SIGNAL(currentIndexChanged(int)), q,
                    SLOT(setGlyphEigenvector(int)) );
  QObject::connect( this->ScaleFactorSlider, SIGNAL(valueChanged(double)), q, SLOT(setGlyphScaleFactor(double)) );
  QObject::connect( this->SpacingSlider, SIGNAL(valueChanged(double)), q, SLOT(setGlyphSpacing(double)) );
  QObject::connect( this->GlyphSidesSlider, SIGNAL(valueChanged(double)), q, SLOT(setTubeGlyphNumberOfSides(double)) );
  QObject::connect( this->GlyphRadiusSlider, SIGNAL(valueChanged(double)), q, SLOT(setTubeGlyphRadius(double)) );
}



//------------------------------------------------------------------------------
qSlicerTractographyDisplayGlyphWidget::qSlicerTractographyDisplayGlyphWidget(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTractographyDisplayGlyphWidgetPrivate(*this))
{
  Q_D(qSlicerTractographyDisplayGlyphWidget);
  d->init();
}

//------------------------------------------------------------------------------
qSlicerTractographyDisplayGlyphWidget::~qSlicerTractographyDisplayGlyphWidget()
{
}

//------------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* qSlicerTractographyDisplayGlyphWidget::fiberBundleDisplayNode()const
{
  Q_D(const qSlicerTractographyDisplayGlyphWidget);
  return d->FiberBundleDisplayNode;
}

//------------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* qSlicerTractographyDisplayGlyphWidget::diffusionTensorDisplayPropertiesNode()const
{
  Q_D(const qSlicerTractographyDisplayGlyphWidget);
  return d->FiberBundleDisplayNode ?
    d->FiberBundleDisplayNode->GetDiffusionTensorDisplayPropertiesNode() : 0;
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::setFiberBundleDisplayNode(vtkMRMLNode* node)
{
  this->setFiberBundleDisplayNode(vtkMRMLFiberBundleDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::
  setFiberBundleDisplayNode
  (vtkMRMLFiberBundleDisplayNode* fiberBundleDisplayNode)
{
  Q_D(qSlicerTractographyDisplayGlyphWidget);
  vtkMRMLFiberBundleDisplayNode *oldDisplayNode = d->FiberBundleDisplayNode;

  d->FiberBundleDisplayNode = fiberBundleDisplayNode;

  qvtkReconnect( oldDisplayNode, d->FiberBundleDisplayNode,
                vtkCommand::ModifiedEvent, this,
                SLOT(updateWidgetFromMRMLDisplayNode()) );
  this->updateWidgetFromMRMLDisplayNode();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::
  setDiffusionTensorDisplayPropertiesNode(vtkMRMLNode* node)
{
  this->setDiffusionTensorDisplayPropertiesNode
    (vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::
  setDiffusionTensorDisplayPropertiesNode
  (vtkMRMLDiffusionTensorDisplayPropertiesNode* node)
{
  Q_D(qSlicerTractographyDisplayGlyphWidget);

  d->DiffusionTensorDisplayPropertiesNode = node;

  d->GlyphTypeSelector->blockSignals(true);
  d->GlyphTypeSelector->clear();
  std::vector<int> supportedDisplayTypes;
  int i = d->DiffusionTensorDisplayPropertiesNode->GetFirstGlyphGeometry();
  for (; i <= d->DiffusionTensorDisplayPropertiesNode->GetLastGlyphGeometry(); i++)
    {
    d->GlyphTypeSelector->addItem(
        d->DiffusionTensorDisplayPropertiesNode->GetGlyphGeometryAsString(i), i);
    }
  d->GlyphTypeSelector->blockSignals(false);

  d->GlyphEigenvectorSelector->blockSignals(true);
  d->GlyphEigenvectorSelector->clear();
  std::vector<int> supportedEigenVectorTypes;
  i = d->DiffusionTensorDisplayPropertiesNode->GetFirstGlyphEigenvector();
  for (; i <= d->DiffusionTensorDisplayPropertiesNode->GetLastGlyphEigenvector(); i++)
    {
    d->GlyphEigenvectorSelector->addItem(
        d->DiffusionTensorDisplayPropertiesNode->GetGlyphEigenvectorAsString(i), i);
    }
  d->GlyphEigenvectorSelector->blockSignals(false);
}
//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::setGlyphType(int type)
{
  Q_D(qSlicerTractographyDisplayGlyphWidget);

  if (!d->DiffusionTensorDisplayPropertiesNode)
    {
    return;
    }
  d->DiffusionTensorDisplayPropertiesNode->SetGlyphGeometry(type);
  QWidget* widget = d->GlyphSubPropertiesWidget->
    findChild<QWidget *>(
    d->DiffusionTensorDisplayPropertiesNode->GetGlyphGeometryAsString()
    );
  if (widget)
    {
    d->GlyphSubPropertiesWidget->setCurrentWidget(widget);
    d->GlyphSubPropertiesWidget->setEnabled(true);
    }
  else
    d->GlyphSubPropertiesWidget->setEnabled(false);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::setGlyphEigenvector(int ev)
{
  Q_D(qSlicerTractographyDisplayGlyphWidget);

  if (!d->DiffusionTensorDisplayPropertiesNode)
    {
    return;
    }
  d->DiffusionTensorDisplayPropertiesNode->SetGlyphEigenvector(ev);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::setGlyphScaleFactor(double scale)
{
  Q_D(qSlicerTractographyDisplayGlyphWidget);

  if (!d->DiffusionTensorDisplayPropertiesNode)
    {
    return;
    }
  d->DiffusionTensorDisplayPropertiesNode->SetGlyphScaleFactor((int)scale);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::setGlyphSpacing(double spacing)
{
  Q_D(qSlicerTractographyDisplayGlyphWidget);

  if (!d->DiffusionTensorDisplayPropertiesNode)
    {
    return;
    }
  d->DiffusionTensorDisplayPropertiesNode->SetLineGlyphResolution((int)spacing);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::setTubeGlyphNumberOfSides(double sides)
{
  Q_D(qSlicerTractographyDisplayGlyphWidget);

  if (!d->DiffusionTensorDisplayPropertiesNode)
    {
    return;
    }
  d->DiffusionTensorDisplayPropertiesNode->SetTubeGlyphNumberOfSides((int)sides);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::setTubeGlyphRadius(double radius)
{
  Q_D(qSlicerTractographyDisplayGlyphWidget);

  if (!d->DiffusionTensorDisplayPropertiesNode)
    {
    return;
    }
  d->DiffusionTensorDisplayPropertiesNode->SetTubeGlyphRadius(radius);
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::
  updateWidgetFromMRMLDisplayNode()
{
  Q_D(qSlicerTractographyDisplayGlyphWidget);
  if ( !d->FiberBundleDisplayNode )
    {
    return;
    }
  if ( d->DiffusionTensorDisplayPropertiesNode !=
       d->FiberBundleDisplayNode->GetDiffusionTensorDisplayPropertiesNode() )
    {
    this->setDiffusionTensorDisplayPropertiesNode(
      d->FiberBundleDisplayNode->GetDiffusionTensorDisplayPropertiesNode() );
    }
  this->updateWidgetFromMRMLDisplayPropertiesNode();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayGlyphWidget::
  updateWidgetFromMRMLDisplayPropertiesNode()
{
  Q_D(qSlicerTractographyDisplayGlyphWidget);
  if ( !d->FiberBundleDisplayNode || !d->DiffusionTensorDisplayPropertiesNode )
    {
    return;
    }

  d->GlyphTypeSelector->setCurrentIndex(
    d->DiffusionTensorDisplayPropertiesNode->GetGlyphGeometry() );

  int geomId =
    d->DiffusionTensorDisplayPropertiesNode->GetGlyphGeometry();
  if (geomId == vtkMRMLDiffusionTensorDisplayPropertiesNode::Ellipsoids)
    d->GlyphEigenvectorSelector->setEnabled(false);
  else
    d->GlyphEigenvectorSelector->setEnabled(true);

  d->GlyphEigenvectorSelector->setCurrentIndex(
    d->DiffusionTensorDisplayPropertiesNode->GetGlyphEigenvector() );

  d->ScaleFactorSlider->setValue(
    d->DiffusionTensorDisplayPropertiesNode->GetGlyphScaleFactor() );
  d->SpacingSlider->setValue(
    d->DiffusionTensorDisplayPropertiesNode->GetLineGlyphResolution() );
  d->GlyphSidesSlider->setValue(
    d->DiffusionTensorDisplayPropertiesNode->GetTubeGlyphNumberOfSides() );
  d->GlyphRadiusSlider->setValue(
    d->DiffusionTensorDisplayPropertiesNode->GetTubeGlyphRadius() );
}
