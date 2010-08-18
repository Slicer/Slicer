// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerEMSegmentDefineInputChannelsPanel.h" 
#include "ui_qSlicerEMSegmentDefineInputChannelsPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefineInputChannelsPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentDefineInputChannelsPanel>,
    public Ui_qSlicerEMSegmentDefineInputChannelsPanel
{
public:
  qSlicerEMSegmentDefineInputChannelsPanelPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineInputChannelsPanel::qSlicerEMSegmentDefineInputChannelsPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefineInputChannelsPanel);
  CTK_D(qSlicerEMSegmentDefineInputChannelsPanel);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineInputChannelsPanel::printAdditionalInfo()
{
}
