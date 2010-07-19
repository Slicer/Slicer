// SlicerQt includes
#include "qSlicerROIModule.h"
#include "qSlicerROIModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerROIModulePrivate: public ctkPrivate<qSlicerROIModule>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerROIModule);
};

//-----------------------------------------------------------------------------
qSlicerROIModule::qSlicerROIModule(QObject* parentObject)
  :qSlicerAbstractCoreModule(parentObject)
{
  CTK_INIT_PRIVATE(qSlicerROIModule);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerROIModule::createWidgetRepresentation()
{
  return new qSlicerROIModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerROIModule::createLogic()
{
  return 0;
}

//-----------------------------------------------------------------------------
QString qSlicerROIModule::helpText()const
{
  QString help =
    "The ROI module creates and edits Region Of Interest (ROI). "
    "An ROI is a rectangle region in the RAS space. It is defined by the "
    "location of it's center and three extents of it's sides. The ROI can be "
    "rotated and/or translated relative to the RAS space by placing it inside "
    "the Transformation node in the Data module. The ROI module allows user to "
    "create new ROI's, change their visibility, and also change the size and "
    "location of ROI's.<br>Region Of Interest (ROI) is used in other modules such "
    "as ClipModels, VolumeRendering, etc. <br>For more information see "
    "<a href=\"%1/Modules:ROI-Module-Documentation-3.6\">%1/Modules:ROIModule-"
    "Documentation-3.6</a>";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerROIModule::acknowledgementText()const
{
  QString about =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer "
    "Community.<br>"
    "See <a href=\"http://www.slicer.org\">www.slicer.org</a> for details.<br>"
    "The ROI module was contributed by Alex Yarmarkovich, Isomics "
    "Inc. with help from others at SPL, BWH (Ron Kikinis)";
  return about;
}
