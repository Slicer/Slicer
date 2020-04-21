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

// MRMLWidgets includes
#include "qMRMLWidget.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QApplication>
#include <QSurfaceFormat>

#ifdef _WIN32
#include <Windows.h> //for SetProcessDPIAware
#endif

//-----------------------------------------------------------------------------
class qMRMLWidgetPrivate
{
public:
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
};

//-----------------------------------------------------------------------------
// qMRMLWidget methods

//-----------------------------------------------------------------------------
qMRMLWidget::qMRMLWidget(QWidget * _parent, Qt::WindowFlags f):Superclass(_parent, f)
  , d_ptr(new qMRMLWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qMRMLWidget::~qMRMLWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLWidget);
  if (newScene == d->MRMLScene)
    {
    return ;
    }
  d->MRMLScene = newScene;
  emit mrmlSceneChanged(newScene);
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qMRMLWidget::mrmlScene() const
{
  Q_D(const qMRMLWidget);
  return d->MRMLScene;
}

//-----------------------------------------------------------------------------
void qMRMLWidget::preInitializeApplication()
{
  #ifdef Q_OS_MACX
  if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
    {
    // Fix Mac OS X 10.9 (mavericks) font issue
    // https://bugreports.qt-project.org/browse/QTBUG-32789
    QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif

#ifdef _WIN32
  // Qt windows defaults to the PROCESS_PER_MONITOR_DPI_AWARE for DPI display
  // on windows. Unfortunately, this doesn't work well on multi-screens setups.
  // By calling SetProcessDPIAware(), we force the value to
  // PROCESS_SYSTEM_DPI_AWARE instead which fixes those issues.
  SetProcessDPIAware();
#endif

  QSurfaceFormat format = QVTKOpenGLNativeWidget::defaultFormat();

  QString openGLProfileStr = qgetenv(MRML_APPLICATION_OPENGL_PROFILE_ENV);
  openGLProfileStr = openGLProfileStr.toLower();
  if (openGLProfileStr.isEmpty() || openGLProfileStr=="default")
    {
    // Use default profile
#ifdef _WIN32
    // Enable OpenGL compatibility profile on Windows.
    // It fixes display update issues and should not have any
    // side effect.
    // Compatibility profile is only requested for Windows, as it is
    // not fully supported on Mac, and there is no known issue
    // on Linux that would require requesting compatibility profile.
    // More details: https://gitlab.kitware.com/vtk/vtk/issues/17572
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
#endif
    }
  else
    {
    // Force a specific profile
    if (openGLProfileStr == "no")
      {
      format.setProfile(QSurfaceFormat::NoProfile);
      }
    else if (openGLProfileStr == "core")
      {
      format.setProfile(QSurfaceFormat::CoreProfile);
      }
    else if (openGLProfileStr == "compatibility")
      {
      format.setProfile(QSurfaceFormat::CompatibilityProfile);
      }
    }

  // Set default surface format for QVTKOpenGLWidget. Disable multisampling to
  // support volume rendering and other VTK functionality that reads from the
  // framebuffer; see https://gitlab.kitware.com/vtk/vtk/issues/17095.
  format.setSamples(0);

  QSurfaceFormat::setDefaultFormat(format);

  // Enable automatic scaling based on the pixel density of the monitor
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  // Enables resource sharing between the OpenGL contexts used by classes like QOpenGLWidget and QQuickWidget
  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
}

//-----------------------------------------------------------------------------
void qMRMLWidget::postInitializeApplication()
{
  // Currently there is no initialization steps to be performed after
  // application creation right now, but we still keep this method
  // as a placeholder to make it easier to add steps later.
}
