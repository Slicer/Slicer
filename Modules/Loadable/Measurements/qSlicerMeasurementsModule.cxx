/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerMeasurementsModule.h"
#include "qSlicerMeasurementsModuleWidget.h"

//-----------------------------------------------------------------------------
#include <QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerMeasurementsModule, qSlicerMeasurementsModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Measurements
class qSlicerMeasurementsModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CPP(qSlicerMeasurementsModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerMeasurementsModule::~qSlicerMeasurementsModule()
{
}

//-----------------------------------------------------------------------------
QIcon qSlicerMeasurementsModule::icon()const
{
  return QIcon(":/Icons/Measurements.png");
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerMeasurementsModule::createWidgetRepresentation()
{
  return new qSlicerMeasurementsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerMeasurementsModule::createLogic()
{
  //return new qSlicerMeasurementsModuleLogic;
  return 0;
}
