
// Qt includes
#include <QtPlugin>

// SlicerQt includes
#include <qSlicerCoreApplication.h>

// EMSegment Logic includes
#include <vtkSlicerEMSegmentLogic.h>

// EMSegment QTModule includes
#include "qSlicerEMSegmentModule.h"
#include "qSlicerEMSegmentModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerEMSegmentModule, qSlicerEMSegmentModule);

//-----------------------------------------------------------------------------
class qSlicerEMSegmentModulePrivate: public ctkPrivate<qSlicerEMSegmentModule>
{
public:
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentModule::qSlicerEMSegmentModule(QObject* _parent)
  :Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentModule);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentModule::helpText()const
{
  QString help =
    "EMSegment Module: Use this module to segment a set of images (target images) using the "
    "tree-based EM segmentation algorithm of K. Pohl et al."
    "<ul>"
    "<li>Use the pull down menu to select a collection of parameters to edit"
    " (or create a new collection).</li>"
    "<li>Use the <b>Back</b> and <b>Next</b> to navigate through the "
    "stages of filling in the algorithm parameters.</li>"
    "<li>When the parameters are specified, use the button on the "
    "last step to start the segmentation process.</li>"
    "</ul>"
    "See <a href=%1/Modules:EMSegmentTemplateBuilder3.6>Modules:EMSegmentTemplateBuilder3.6</a>";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerEMSegmentModule::createWidgetRepresentation()
{
  return new qSlicerEMSegmentModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerEMSegmentModule::createLogic()
{
  return vtkSlicerEMSegmentLogic::New();
}
