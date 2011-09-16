// QT includes
#include <QDebug>
#include <QColor>

// qMRML includes
#include "qSlicerTractographyDisplayBasicWidget.h"
#include "ui_qSlicerTractographyDisplayBasicWidget.h"

// MRML includes
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLFiberBundleNode.h>
#include <vtkMRMLFiberBundleDisplayNode.h>

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qSlicerTractographyDisplayBasicWidgetPrivate: 
  public Ui_qSlicerTractographyDisplayBasicWidget
{
  Q_DECLARE_PUBLIC(qSlicerTractographyDisplayBasicWidget);

protected:
  qSlicerTractographyDisplayBasicWidget* const q_ptr;

public:
  qSlicerTractographyDisplayBasicWidgetPrivate(qSlicerTractographyDisplayBasicWidget& object);
  void init();

  vtkMRMLFiberBundleNode* FiberBundleNode;
  vtkMRMLFiberBundleDisplayNode *LineDN, *TubeDN, *GlyphDN;
};

//------------------------------------------------------------------------------
qSlicerTractographyDisplayBasicWidgetPrivate::qSlicerTractographyDisplayBasicWidgetPrivate
                                      (qSlicerTractographyDisplayBasicWidget& object)
  : q_ptr(&object)
{
  this->FiberBundleNode = 0;
  this->LineDN = 0;
  this->TubeDN = 0;
  this->GlyphDN = 0;
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayBasicWidgetPrivate::init()
{
  Q_Q(qSlicerTractographyDisplayBasicWidget);
  this->setupUi(q);

  QObject::connect( this->LineVisibility, SIGNAL(stateChanged(int)), q,
                    SLOT(setLineVisibility(int)) );
  QObject::connect(  this->TubeVisibility, SIGNAL(stateChanged(int)), q,
                    SLOT(setTubeVisibility(int)) );
  QObject::connect( this->GlyphVisibility, SIGNAL(stateChanged(int)), q,
                    SLOT(setGlyphVisibility(int)) );
  QObject::connect( this->TubeSliceIntersectVisibility, SIGNAL(stateChanged(int)), q,
                    SLOT(setTubeSliceIntersectVisibility(int)) );
}



//------------------------------------------------------------------------------
qSlicerTractographyDisplayBasicWidget::qSlicerTractographyDisplayBasicWidget(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTractographyDisplayBasicWidgetPrivate(*this))
{
  Q_D(qSlicerTractographyDisplayBasicWidget);
  d->init();
}

//------------------------------------------------------------------------------
qSlicerTractographyDisplayBasicWidget::~qSlicerTractographyDisplayBasicWidget()
{
}

//------------------------------------------------------------------------------
vtkMRMLFiberBundleNode* qSlicerTractographyDisplayBasicWidget::fiberBundleNode()const
{
  Q_D(const qSlicerTractographyDisplayBasicWidget);
  return d->FiberBundleNode;
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayBasicWidget::setFiberBundleNode(vtkMRMLNode* node)
{
  this->setFiberBundleNode(vtkMRMLFiberBundleNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayBasicWidget::
  setFiberBundleNode
  (vtkMRMLFiberBundleNode* fiberBundleNode)
{
  Q_D(qSlicerTractographyDisplayBasicWidget);
  vtkMRMLFiberBundleNode *oldNode = this->fiberBundleNode();
  vtkMRMLFiberBundleDisplayNode *oldLDN = NULL;
  vtkMRMLFiberBundleDisplayNode *oldTDN = NULL;
  vtkMRMLFiberBundleDisplayNode *oldGDN = NULL;

  if (d->FiberBundleNode )
    {
    oldLDN = d->FiberBundleNode->GetLineDisplayNode();
    oldTDN = d->FiberBundleNode->GetTubeDisplayNode();
    oldGDN = d->FiberBundleNode->GetGlyphDisplayNode();
    }
  if (fiberBundleNode)
    {
    d->LineDN = fiberBundleNode->GetLineDisplayNode();
    d->TubeDN = fiberBundleNode->GetTubeDisplayNode();
    d->GlyphDN = fiberBundleNode->GetGlyphDisplayNode();
    }

  d->FiberBundleNode = fiberBundleNode;

  qvtkReconnect( oldNode, d->FiberBundleNode,
                vtkCommand::ModifiedEvent, this, 
                SLOT(updateWidgetFromMRML()) );

  //if ( d->LineDN )
    qvtkReconnect( oldLDN, d->LineDN,
                  vtkCommand::ModifiedEvent, this,
                  SLOT(updateWidgetFromMRML()) );
  //if ( d->TubeDN )
    qvtkReconnect( oldTDN, d->TubeDN,
                  vtkCommand::ModifiedEvent, this,
                  SLOT(updateWidgetFromMRML()) );
  //if ( d->GlyphDN )
    qvtkReconnect( oldGDN, d->GlyphDN,
                  vtkCommand::ModifiedEvent, this,
                  SLOT(updateWidgetFromMRML()) );

  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qSlicerTractographyDisplayBasicWidget::
  updateWidgetFromMRML()
{
  Q_D(qSlicerTractographyDisplayBasicWidget);
  if ( !d->FiberBundleNode )
    {
    return;
    }

  if ( d->FiberBundleNode->GetLineDisplayNode() )
    {
    d->LineVisibility->setChecked( 
      d->FiberBundleNode->GetLineDisplayNode()->GetVisibility() );
    }

  if ( d->FiberBundleNode->GetTubeDisplayNode() )
    {
    int tubeVis = d->FiberBundleNode->GetTubeDisplayNode()->GetVisibility();
    d->TubeVisibility->setChecked(tubeVis);
    d->TubeSliceIntersectVisibility->setChecked(
      d->FiberBundleNode->GetTubeDisplayNode()->GetSliceIntersectionVisibility() );
    }

  if ( d->FiberBundleNode->GetGlyphDisplayNode() )
    {
    d->GlyphVisibility->setChecked( 
      d->FiberBundleNode->GetGlyphDisplayNode()->GetVisibility() );
    }
}

void qSlicerTractographyDisplayBasicWidget::
  setLineVisibility(int state)
{
  Q_D(qSlicerTractographyDisplayBasicWidget);
  if (!d->FiberBundleNode)
    return;
  vtkMRMLFiberBundleDisplayNode* dNode = d->FiberBundleNode->GetLineDisplayNode();
  if (dNode)
    dNode->SetVisibility( (state > 0) );
}

void qSlicerTractographyDisplayBasicWidget::
  setTubeVisibility(int state)
{
  Q_D(qSlicerTractographyDisplayBasicWidget);
  if (!d->FiberBundleNode)
    return;
  vtkMRMLFiberBundleDisplayNode* dNode = d->FiberBundleNode->GetTubeDisplayNode();
  if (dNode)
    dNode->SetVisibility( (state > 0) );

}

void qSlicerTractographyDisplayBasicWidget::
  setGlyphVisibility(int state)
{
  Q_D(qSlicerTractographyDisplayBasicWidget);
  if (!d->FiberBundleNode)
    return;
  vtkMRMLFiberBundleDisplayNode* dNode = d->FiberBundleNode->GetGlyphDisplayNode();
  if (dNode)
    dNode->SetVisibility( (state > 0) );

}

void qSlicerTractographyDisplayBasicWidget::
  setTubeSliceIntersectVisibility(int state)
{
  Q_D(qSlicerTractographyDisplayBasicWidget);
  if(!d->FiberBundleNode)
    return;
  vtkMRMLFiberBundleDisplayNode* dNode = d->FiberBundleNode->GetTubeDisplayNode();
  if(dNode)
    dNode->SetSliceIntersectionVisibility(state > 0);
}
