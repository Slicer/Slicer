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

// MRMLWidgets includes
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>

// Slicer includes
#include "qSlicerLayoutManager.h"
#include "qSlicerApplication.h"

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

public:
  QString            ScriptedDisplayableManagerDirectory;
};

// --------------------------------------------------------------------------
qSlicerLayoutManagerPrivate::qSlicerLayoutManagerPrivate(qSlicerLayoutManager& object)
  : qMRMLLayoutManagerPrivate(object)
{
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
  //    .absoluteFilePath().toUtf8());
  //vtkMRMLThreeDViewDisplayableManagerFactory* threeDFactory
  //  = vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance();
  //threeDFactory->RegisterDisplayableManager(
  //  QFileInfo(QDir(scriptedDisplayableManagerDirectory),
  //            "vtkScriptedExampleDisplayableManager.py")
  //    .absoluteFilePath().toUtf8());
#else
  Q_UNUSED(scriptedDisplayableManagerDirectory);
#endif
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::setCurrentModule(const QString& moduleName)
{
  emit this->selectModule(moduleName);
}
