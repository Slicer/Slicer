// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerEMSegmentDefineTaskPanel.h" 
#include "ui_qSlicerEMSegmentDefineTaskPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefineTaskPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentDefineTaskPanel>,
    public Ui_qSlicerEMSegmentDefineTaskPanel
{
public:
  qSlicerEMSegmentDefineTaskPanelPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineTaskPanel::qSlicerEMSegmentDefineTaskPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefineTaskPanel);
  CTK_D(qSlicerEMSegmentDefineTaskPanel);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineTaskPanel::printAdditionalInfo()
{
}
