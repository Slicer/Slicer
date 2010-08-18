// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerEMSegmentEditNodeBasedParametersPanel.h" 
#include "ui_qSlicerEMSegmentEditNodeBasedParametersPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentEditNodeBasedParametersPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentEditNodeBasedParametersPanel>,
    public Ui_qSlicerEMSegmentEditNodeBasedParametersPanel
{
public:
  qSlicerEMSegmentEditNodeBasedParametersPanelPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditNodeBasedParametersPanel::qSlicerEMSegmentEditNodeBasedParametersPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentEditNodeBasedParametersPanel);
  CTK_D(qSlicerEMSegmentEditNodeBasedParametersPanel);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersPanel::printAdditionalInfo()
{
}
