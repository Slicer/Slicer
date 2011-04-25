/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QFileInfo>
#include <QWidget>
#include <QVariant>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets includes
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>
//#include <qMRMLUtils.h>
//#include <qMRMLNodeFactory.h>

// SlicerQt includes
#include "vtkSlicerConfigure.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerApplication.h"
#ifdef Slicer_USE_PYTHONQT_WITH_TCL
#include "qSlicerPythonManager.h"
#endif

// qMRMLSlicer
#include <qMRMLLayoutManager_p.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceLogic.h>

// VTK includes
#include <vtkInteractorObserver.h>
#include <vtkCornerAnnotation.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtgui.qSlicerLayoutManager");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qSlicerLayoutManagerPrivate: public qMRMLLayoutManagerPrivate
{
public:
  qSlicerLayoutManagerPrivate(qSlicerLayoutManager& object);
  /// Instantiate a slice viewer corresponding to \a sliceViewName
  virtual QWidget* createSliceWidget(vtkMRMLSliceNode* sliceNode);

public:
  //QString            ScriptedDisplayableManagerDirectory;
};

// --------------------------------------------------------------------------
qSlicerLayoutManagerPrivate::qSlicerLayoutManagerPrivate(qSlicerLayoutManager& object)
  : qMRMLLayoutManagerPrivate(object)
{
}

// --------------------------------------------------------------------------
QWidget* qSlicerLayoutManagerPrivate::createSliceWidget(vtkMRMLSliceNode* sliceNode)
{
  qMRMLSliceWidget* sliceWidget = dynamic_cast<qMRMLSliceWidget*>(
    this->qMRMLLayoutManagerPrivate::createSliceWidget(sliceNode));

  if (sliceWidget)
    {
    bool disablePython = qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython);
    sliceWidget->setIgnoreScriptedDisplayableManagers(disablePython);
    sliceWidget->registerDisplayableManagers(this->ScriptedDisplayableManagerDirectory);
#ifdef Slicer_USE_PYTHONQT_WITH_TCL
    if (!disablePython)
      {
      QString sliceLayoutName(sliceNode->GetLayoutName());
      // Note: Python code shouldn't be added to the layout manager itself !
      // TODO: move this functionality to the scripted displayable manager...

      // Register this slice view with the python layer
      qSlicerPythonManager *py = qSlicerApplication::application()->pythonManager();
      py->executeString(QString("slicer.sliceWidget%1 = _sliceWidget()").arg(sliceLayoutName));

      QString pythonInstanceName = QString("slicer.sliceWidget%1_%2");

      py->addVTKObjectToPythonMain(
        pythonInstanceName.arg(sliceLayoutName, "sliceLogic"),
        sliceWidget->sliceController()->sliceLogic());

      py->addVTKObjectToPythonMain(
        pythonInstanceName.arg(sliceLayoutName, "interactorStyle"),
        sliceWidget->interactorStyle());

      py->addVTKObjectToPythonMain(
        pythonInstanceName.arg(sliceLayoutName, "cornerAnnotation"),
        sliceWidget->overlayCornerAnnotation());

      py->executeString(QString("registerScriptedDisplayableManagers('%1')").arg(sliceLayoutName));

      logger.trace(
          QString("createSliceWidget - %1 registered with python").arg(sliceLayoutName));
      }
#endif
    }
  return sliceWidget;
}

//------------------------------------------------------------------------------
// qSlicerLayoutManager methods

// -----------------------------------------------------------------------------
qSlicerLayoutManager::qSlicerLayoutManager(QWidget* widget)
  : qMRMLLayoutManager(new qSlicerLayoutManagerPrivate(*this), widget, widget)
{
  bool disablePython = qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython);
  this->setIgnoreScriptedDisplayableManagers(disablePython);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::setScriptedDisplayableManagerDirectory(
    const QString& scriptedDisplayableManagerDirectory)
{
#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    Q_D(qSlicerLayoutManager);

    Q_ASSERT(QFileInfo(scriptedDisplayableManagerDirectory).isDir());
    d->ScriptedDisplayableManagerDirectory = scriptedDisplayableManagerDirectory;
    }
#else
  Q_UNUSED(scriptedDisplayableManagerDirectory);
#endif
}
