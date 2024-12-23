/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include <vtkMRMLSliceViewDisplayableManagerFactory.h>

// Slicer includes
#include <vtkSlicerApplicationLogic.h>
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

// CornerText includes
#include "qSlicerCornerTextModule.h"
#include "qSlicerCornerTextModuleWidget.h"
#include "vtkMRMLAbstractDisplayableManager.h"
#include "vtkMRMLCornerTextDisplayableManager.h"
#include "vtkMRMLDefaultAnnotationPropertyValueProvider.h"

// DisplayableManager initialization
#include <vtkAutoInit.h>

// Qt includes
#include <QSettings>
#include <QObject>
#include <QDebug>

// QtGUI includes
#include <qSlicerApplication.h>
#ifdef Slicer_USE_PYTHONQT
#include <qSlicerPythonManager.h>
#endif

// MRML includes
#include <vtkMRMLCornerTextLogic.h>
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceView.h>

// VTK includes
#include <vtkCornerAnnotation.h>

VTK_MODULE_INIT(vtkSlicerCornerTextModuleMRMLDisplayableManager)

//-----------------------------------------------------------------------------
class qSlicerCornerTextModulePrivate
{
public:
  qSlicerCornerTextModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerCornerTextModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCornerTextModulePrivate::qSlicerCornerTextModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCornerTextModule methods

//-----------------------------------------------------------------------------
qSlicerCornerTextModule::qSlicerCornerTextModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCornerTextModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCornerTextModule::~qSlicerCornerTextModule()
{
  writeSettings();
}

//-----------------------------------------------------------------------------
QString qSlicerCornerTextModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerCornerTextModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCornerTextModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerCornerTextModule::icon() const
{
  return QIcon(":/Icons/CornerText.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerCornerTextModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCornerTextModule::dependencies() const
{
  return QStringList();
}

void qSlicerCornerTextModule::readSettings() const
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  vtkMRMLCornerTextLogic *cornerTextLogic =
      this->appLogic()->GetCornerTextLogic();

  cornerTextLogic->SetSliceViewAnnotationsEnabled(settings->value("DataProbe/sliceViewAnnotations.enabled", 1).toBool());
  cornerTextLogic->SetDisplayStrictness(settings->value("DataProbe/sliceViewAnnotations.displayLevel", 1).toInt());
  cornerTextLogic->SetFontSize(settings->value("DataProbe/sliceViewAnnotations.fontSize", 14).toInt());
  cornerTextLogic->SetFontFamily(settings->value("DataProbe/sliceViewAnnotations.fontFamily", "Times").toString().toStdString());

  cornerTextLogic->SetBottomLeftEnabled(settings->value("DataProbe/sliceViewAnnotations.bottomLeft", "1").toBool());
  cornerTextLogic->SetTopLeftEnabled(settings->value("DataProbe/sliceViewAnnotations.topLeft", "1").toBool());
  cornerTextLogic->SetTopRightEnabled(settings->value("DataProbe/sliceViewAnnotations.topRight", "1").toBool());
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextModule::writeSettings() const
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  vtkMRMLCornerTextLogic *cornerTextLogic =
      this->appLogic()->GetCornerTextLogic();

  if (!cornerTextLogic)
  {
    qWarning() << Q_FUNC_INFO << " failed: cornerTextLogic is invalid";
    return;
  }

  settings->setValue("DataProbe/sliceViewAnnotations.enabled", cornerTextLogic->GetSliceViewAnnotationsEnabled());
  settings->setValue("DataProbe/sliceViewAnnotations.displayLevel", cornerTextLogic->GetDisplayStrictness());
  settings->setValue("DataProbe/sliceViewAnnotations.fontSize", cornerTextLogic->GetFontSize());
  settings->setValue("DataProbe/sliceViewAnnotations.fontFamily", QString::fromStdString(cornerTextLogic->GetFontFamily()));

  settings->setValue("DataProbe/sliceViewAnnotations.bottomLeft", cornerTextLogic->GetBottomLeftEnabled());
  settings->setValue("DataProbe/sliceViewAnnotations.topLeft", cornerTextLogic->GetTopLeftEnabled());
  settings->setValue("DataProbe/sliceViewAnnotations.topRight", cornerTextLogic->GetTopRightEnabled());
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextModule::onLayoutChanged(int layout)
{
  foreach(const QString& sliceViewName, qSlicerApplication::application()->layoutManager()->sliceViewNames())
    {
    vtkCornerAnnotation *cornerAnnotation =
        qSlicerApplication::application()
            ->layoutManager()
            ->sliceWidget(sliceViewName)
            ->overlayCornerAnnotation();

    vtkMRMLAbstractDisplayableManager* displayableManager =
        qSlicerApplication::application()
            ->layoutManager()
            ->sliceWidget(sliceViewName)
            ->sliceView()
            ->displayableManagerByClassName("vtkMRMLCornerTextDisplayableManager");

    vtkMRMLCornerTextDisplayableManager* cornerTextDisplayableManager;
    if ((cornerTextDisplayableManager =
            vtkMRMLCornerTextDisplayableManager::SafeDownCast(
                displayableManager)))
    {
      cornerTextDisplayableManager->SetCornerAnnotation(cornerAnnotation);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextModule::setup()
{
  this->Superclass::setup();

  // Register displayable managers
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkMRMLCornerTextDisplayableManager");

  // Register default annotation provider
  vtkNew<vtkMRMLDefaultAnnotationPropertyValueProvider> defultProvider;
  defultProvider->SetAppLogic(this->appLogic());
  this->appLogic()->GetCornerTextLogic()->RegisterPropertyValueProvider(
      "Default", defultProvider);

  // Read DataProbe settings
  this->readSettings();

  // Observe layout manager (may not be available if running without main window)
  if (qSlicerApplication::application()->layoutManager())
  {
    QObject::connect(qSlicerApplication::application()->layoutManager(), SIGNAL(layoutChanged(int)),
      this, SLOT(onLayoutChanged(int)));
  }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerCornerTextModule
::createWidgetRepresentation()
{
  return new qSlicerCornerTextModuleWidget;
}
