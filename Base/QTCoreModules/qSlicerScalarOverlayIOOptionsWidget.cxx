/// Qt includes
#include <QDebug>

/// ScalarOverlay includes
#include "qSlicerScalarOverlayIOOptionsWidget.h"
#include "ui_qSlicerScalarOverlayIOOptionsWidget.h"

// MRML includes
#include <vtkMRMLNode.h>

//-----------------------------------------------------------------------------
class qSlicerScalarOverlayIOOptionsWidgetPrivate:
  public ctkPrivate<qSlicerScalarOverlayIOOptionsWidget>,
  public Ui_qSlicerScalarOverlayIOOptionsWidget
{
  CTK_DECLARE_PUBLIC(qSlicerScalarOverlayIOOptionsWidget);
public:
};


//-----------------------------------------------------------------------------
qSlicerScalarOverlayIOOptionsWidget::qSlicerScalarOverlayIOOptionsWidget(QWidget* parentWidget)
{
  CTK_INIT_PRIVATE(qSlicerScalarOverlayIOOptionsWidget);
  CTK_D(qSlicerScalarOverlayIOOptionsWidget);
  ctk_d()->setupUi(this);

  connect(d->ModelSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(updateProperties()));
}

//-----------------------------------------------------------------------------
bool qSlicerScalarOverlayIOOptionsWidget::isValid()const
{
  return this->qSlicerIOOptionsWidget::isValid() &&
    this->Properties.contains("modelNodeId");
}

//-----------------------------------------------------------------------------
void qSlicerScalarOverlayIOOptionsWidget::updateProperties()
{
  CTK_D(const qSlicerScalarOverlayIOOptionsWidget);
  bool oldValid = this->isValid();
  vtkMRMLNode* modelNode = d->ModelSelector->currentNode();
  qDebug() << "::updateProperties" << modelNode;
  if (modelNode)
    {
    this->Properties["modelNodeId"] = QString(modelNode->GetID());
    }
  else
    {
    this->Properties.remove("modelNodeId");
    }
  bool newValid = this->isValid();
  if (oldValid != newValid)
    {
    emit this->validChanged(newValid);
    }
}
