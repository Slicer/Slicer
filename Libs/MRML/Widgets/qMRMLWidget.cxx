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
#include <QIcon>
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

  // Enable QIcon to provide higher-resolution pixmaps than the size in device independent units.
  // These pixmaps render sharply on a high-dpi display.
  // If Qt::AA_UseHighDpiPixmaps is enabled then when an icon is loaded by the filename "base.png" then the icon
  // class will also look for higher-resolution variants of this image by the names base@2x.png, base@3x.png, etc.
  // On a high-DPI monitor (with DevicePixelRatio > 1) QIcon will actually load a higher-resolution pixmap
  // that best matches the DevicePixelRatio of the display, resulting in crisp rendering of the icon.
  // See https://doc.qt.io/qt-5/qicon.html#pixmap for more details.
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

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

//-----------------------------------------------------------------------------
QPixmap qMRMLWidget::pixmapFromIcon(const QIcon& icon)
{
  // QIcon stores multiple versions of the image (in different sizes) and uses the
  // most suitable one (depending on DevicePixelRatio).
  // In cases where a QIcon cannot be used (such as in QLabel), we need to get the best
  // QPixmap from the QIcon (base.png, base@2x, ...) manually.

  // To achieve this, we first determine the pixmap size in device independent units,
  // which is the size of the base image (icon.availableSizes().first(), because for that
  // DevicePixelRatio=1.0), and then we retieve the pixmap for this size.

  QPixmap pixmap = icon.pixmap(icon.availableSizes().first());
  return pixmap;
}
