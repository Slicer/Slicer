// Qt includes
#include <QDebug>

// EMSegment includes
#include "qSlicerEMSegmentRunSegmentationPanel.h" 
#include "ui_qSlicerEMSegmentRunSegmentationPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentRunSegmentationPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentRunSegmentationPanel>,
    public Ui_qSlicerEMSegmentRunSegmentationPanel
{
public:
  qSlicerEMSegmentRunSegmentationPanelPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentRunSegmentationPanel::qSlicerEMSegmentRunSegmentationPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentRunSegmentationPanel);
  CTK_D(qSlicerEMSegmentRunSegmentationPanel);
  d->setupUi(this);
}
