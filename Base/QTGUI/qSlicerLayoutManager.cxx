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

// Qt includes
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QVariant>

// qMRMLWidgets includes
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>

// SlicerQt includes
#include "qSlicerLayoutManager.h"
#include "qSlicerApplication.h"
#ifdef Slicer_USE_PYTHONQT_WITH_TCL
#include "qSlicerPythonManager.h"
#endif

// qMRMLSlicer
#include <qMRMLLayoutManager_p.h>

// MRMLDisplayableManager includes
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceLogic.h>

// VTK includes
#include <vtkInteractorObserver.h>
#include <vtkCornerAnnotation.h>

//-----------------------------------------------------------------------------
class qSlicerLayoutManagerPrivate: public qMRMLLayoutManagerPrivate
{
public:
  qSlicerLayoutManagerPrivate(qSlicerLayoutManager& object);
  /// Instantiate a slice viewer corresponding to \a sliceViewName
  virtual QWidget* createSliceWidget(vtkMRMLSliceNode* sliceNode);

public:
  QString            ScriptedDisplayableManagerDirectory;
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
#ifdef Slicer_USE_PYTHONQT_WITH_TCL
    bool disablePython = qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython);
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

      //qDebug() << QString("qSlicerLayoutManagerPrivate::createSliceWidget - "
      //                    "%1 registered with python").arg(sliceLayoutName);
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
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::setScriptedDisplayableManagerDirectory(
  const QString& scriptedDisplayableManagerDirectory)
{
#ifdef Slicer_USE_PYTHONQT
  if (qSlicerCoreApplication::testAttribute(
        qSlicerCoreApplication::AA_DisablePython))
    {
    return;
    }
  Q_D(qSlicerLayoutManager);

  Q_ASSERT(QFileInfo(scriptedDisplayableManagerDirectory).isDir());
  d->ScriptedDisplayableManagerDirectory = scriptedDisplayableManagerDirectory;
  // Disable for now as we don't have any displayable managers and
  // loading the python file on Windows 64b in Debug crashes.
  //vtkMRMLSliceViewDisplayableManagerFactory* sliceFactory
  //  = vtkMRMLSliceViewDisplayableManagerFactory::GetInstance();
  //sliceFactory->RegisterDisplayableManager(
  //  QFileInfo(QDir(scriptedDisplayableManagerDirectory),
  //            "vtkScriptedExampleDisplayableManager.py")
  //    .absoluteFilePath().toLatin1());
  //vtkMRMLThreeDViewDisplayableManagerFactory* threeDFactory
  //  = vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance();
  //threeDFactory->RegisterDisplayableManager(
  //  QFileInfo(QDir(scriptedDisplayableManagerDirectory),
  //            "vtkScriptedExampleDisplayableManager.py")
  //    .absoluteFilePath().toLatin1());
#else
  Q_UNUSED(scriptedDisplayableManagerDirectory);
#endif
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::setCurrentModule(const QString& moduleName)
{
  emit this->selectModule(moduleName);
}
