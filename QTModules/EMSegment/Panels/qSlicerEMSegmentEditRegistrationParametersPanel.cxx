// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerEMSegmentEditRegistrationParametersPanel.h" 
#include "ui_qSlicerEMSegmentEditRegistrationParametersPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentEditRegistrationParametersPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentEditRegistrationParametersPanel>,
    public Ui_qSlicerEMSegmentEditRegistrationParametersPanel
{
public:
  qSlicerEMSegmentEditRegistrationParametersPanelPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditRegistrationParametersPanel::qSlicerEMSegmentEditRegistrationParametersPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentEditRegistrationParametersPanel);
  CTK_D(qSlicerEMSegmentEditRegistrationParametersPanel);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersPanel::printAdditionalInfo()
{
}
