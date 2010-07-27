// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerEMSegmentDefineAnatomicalTreePanel.h" 
#include "ui_qSlicerEMSegmentDefineAnatomicalTreePanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefineAnatomicalTreePanelPrivate : public ctkPrivate<qSlicerEMSegmentDefineAnatomicalTreePanel>,
                                              public Ui_qSlicerEMSegmentDefineAnatomicalTreePanel
{
public:
  qSlicerEMSegmentDefineAnatomicalTreePanelPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAnatomicalTreePanel::qSlicerEMSegmentDefineAnatomicalTreePanel(QWidget *_parent):
Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefineAnatomicalTreePanel);
  CTK_D(qSlicerEMSegmentDefineAnatomicalTreePanel);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreePanel::printAdditionalInfo()
{
}
