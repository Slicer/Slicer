// Qt includes
#include <QDebug>

// EMSegment includes
#include "qSlicerEMSegmentDefinePreprocessingPanel.h" 
#include "ui_qSlicerEMSegmentDefinePreprocessingPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefinePreprocessingPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentDefinePreprocessingPanel>,
    public Ui_qSlicerEMSegmentDefinePreprocessingPanel
{
public:
  qSlicerEMSegmentDefinePreprocessingPanelPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefinePreprocessingPanel::qSlicerEMSegmentDefinePreprocessingPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefinePreprocessingPanel);
  CTK_D(qSlicerEMSegmentDefinePreprocessingPanel);
  d->setupUi(this);
}

