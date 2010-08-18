// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerEMSegmentDefineAtlasPanel.h" 
#include "ui_qSlicerEMSegmentDefineAtlasPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefineAtlasPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentDefineAtlasPanel>,
    public Ui_qSlicerEMSegmentDefineAtlasPanel
{
public:
  qSlicerEMSegmentDefineAtlasPanelPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAtlasPanel::qSlicerEMSegmentDefineAtlasPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefineAtlasPanel);
  CTK_D(qSlicerEMSegmentDefineAtlasPanel);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAtlasPanel::printAdditionalInfo()
{
}
