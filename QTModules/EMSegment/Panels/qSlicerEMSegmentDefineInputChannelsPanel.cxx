// Qt includes
#include <QDebug>

// EMSegment includes
#include "qSlicerEMSegmentDefineInputChannelsPanel.h" 
#include "ui_qSlicerEMSegmentDefineInputChannelsPanel.h"
#include "qSlicerEMSegmentInputChannelListWidget.h"

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
void qSlicerEMSegmentDefineInputChannelsPanel::updateMRMLFromWidget()
{
  CTK_D(qSlicerEMSegmentDefineInputChannelsPanel);
  d->EMSegmentInputChannelListWidget->updateMRMLFromWidget();
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerEMSegmentDefineInputChannelsPanel, qSlicerEMSegmentInputChannelListWidget*,
            inputChannelListWidget, EMSegmentInputChannelListWidget);
