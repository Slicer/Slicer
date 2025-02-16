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
#include "qSlicerApplication.h"
#include "qSlicerCornerTextDICOMAnnotationPropertyValueProvider.h"
#include "qSlicerLayoutManager.h"
#include <vtkSlicerApplicationLogic.h>

// CornerText includes
#include "qSlicerCornerTextModule.h"
#include "qSlicerCornerTextModuleWidget.h"
#include "vtkMRMLAbstractDisplayableManager.h"
#include "vtkMRMLCornerTextDisplayableManager.h"
#include "vtkMRMLDefaultAnnotationPropertyValueProvider.h"

// DisplayableManager initialization
#include <vtkAutoInit.h>

// Qt includes
#include <QDebug>
#include <QObject>
#include <QSettings>

// QtGUI includes
#include <qSlicerApplication.h>
#ifdef Slicer_USE_PYTHONQT
#include <qSlicerPythonManager.h>
#endif

// MRML includes
#include <qMRMLSliceView.h>
#include <qMRMLSliceWidget.h>
#include <vtkMRMLCornerTextLogic.h>

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
  this->writeSettings();
}

//-----------------------------------------------------------------------------
QString qSlicerCornerTextModule::helpText() const
{
  return "The CornerText module enables annotations on the border of slice views.";
}

//-----------------------------------------------------------------------------
QString qSlicerCornerTextModule::acknowledgementText() const
{
  return "";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCornerTextModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Andrew Howe (Kitware)");
  moduleContributors << QString("Jean-Christophe Fillion-Robin (Kitware)");
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
  cornerTextLogic->SetIncludeDisplayLevelsLte(settings->value("DataProbe/sliceViewAnnotations.displayLevel", 1).toInt());
  cornerTextLogic->SetFontSize(settings->value("DataProbe/sliceViewAnnotations.fontSize", 14).toInt());
  cornerTextLogic->SetFontFamily(settings->value("DataProbe/sliceViewAnnotations.fontFamily", "Times").toString().toStdString());

  cornerTextLogic->SetBottomLeftEnabled(settings->value("DataProbe/sliceViewAnnotations.bottomLeft", "1").toBool());
  cornerTextLogic->SetTopLeftEnabled(settings->value("DataProbe/sliceViewAnnotations.topLeft", "1").toBool());
  cornerTextLogic->SetTopRightEnabled(settings->value("DataProbe/sliceViewAnnotations.topRight", "1").toBool());
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextModule::writeSettings() const
{
  vtkMRMLCornerTextLogic* cornerTextLogic = this->appLogic()->GetCornerTextLogic();
  if (!cornerTextLogic)
  {
    qWarning() << Q_FUNC_INFO << " failed: cornerTextLogic is invalid";
    return;
  }

  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();

  settings->setValue("DataProbe/sliceViewAnnotations.enabled", static_cast<int>(cornerTextLogic->GetSliceViewAnnotationsEnabled()));
  settings->setValue("DataProbe/sliceViewAnnotations.displayLevel", cornerTextLogic->GetIncludeDisplayLevelsLte());
  settings->setValue("DataProbe/sliceViewAnnotations.fontSize", cornerTextLogic->GetFontSize());
  settings->setValue("DataProbe/sliceViewAnnotations.fontFamily", QString::fromStdString(cornerTextLogic->GetFontFamily()));

  settings->setValue("DataProbe/sliceViewAnnotations.bottomLeft", static_cast<int>(cornerTextLogic->GetBottomLeftEnabled()));
  settings->setValue("DataProbe/sliceViewAnnotations.topLeft", static_cast<int>(cornerTextLogic->GetTopLeftEnabled()));
  settings->setValue("DataProbe/sliceViewAnnotations.topRight", static_cast<int>(cornerTextLogic->GetTopRightEnabled()));
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextModule::onLayoutChanged(int layout)
{
  Q_UNUSED(layout);
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  foreach(const QString& sliceViewName, layoutManager->sliceViewNames())
  {
    vtkCornerAnnotation* cornerAnnotation = layoutManager->sliceWidget(sliceViewName)->overlayCornerAnnotation();

    vtkMRMLAbstractDisplayableManager* displayableManager =
        layoutManager->sliceWidget(sliceViewName)
            ->sliceView()
            ->displayableManagerByClassName(
                "vtkMRMLCornerTextDisplayableManager");

    vtkMRMLCornerTextDisplayableManager* cornerTextDisplayableManager = vtkMRMLCornerTextDisplayableManager::SafeDownCast(displayableManager);
    if (cornerTextDisplayableManager)
    {
      cornerTextDisplayableManager->SetCornerAnnotation(cornerAnnotation);
    }
    else
    {
      vtkErrorWithObjectMacro(displayableManager, "qSlicerCornerTextModule::onLayoutChanged: corner text displayable manager not obtained from slice view.");
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
  vtkNew<vtkMRMLDefaultAnnotationPropertyValueProvider> defaultProvider;
  defaultProvider->SetAppLogic(this->appLogic());
  this->appLogic()->GetCornerTextLogic()->RegisterPropertyValueProvider("Default", defaultProvider);

  // Read DataProbe settings
  this->readSettings();

  // Observe layout manager (may not be available if running without main window)
  if (qSlicerApplication::application()->layoutManager())
  {
    QObject::connect(qSlicerApplication::application()->layoutManager(), SIGNAL(layoutChanged(int)), this, SLOT(onLayoutChanged(int)));
  }

  // Register the DICOM property value provider
#ifdef Slicer_BUILD_DICOM_SUPPORT
  vtkNew<qSlicerCornerTextDICOMAnnotationPropertyValueProvider> DICOMProvider;
  this->appLogic()->GetCornerTextLogic()->RegisterPropertyValueProvider("DICOM", DICOMProvider);
#endif
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerCornerTextModule
::createWidgetRepresentation()
{
  return new qSlicerCornerTextModuleWidget;
}
