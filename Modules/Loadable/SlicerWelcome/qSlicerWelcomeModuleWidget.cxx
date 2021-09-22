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
#include <QDesktopServices>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QSignalMapper>
#include <QTextStream>

// Slicer includes
#include "vtkSlicerConfigure.h" // For Slicer_BUILD_DICOM_SUPPORT
#include "vtkSlicerVersionConfigure.h"

// Slicer includes
#include "qSlicerWelcomeModuleWidget.h"
#include "ui_qSlicerWelcomeModuleWidget.h"
#include "qSlicerApplication.h"
#include "qSlicerIO.h"
#include "qSlicerIOManager.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerModulePanel.h"
#include "qSlicerUtils.h"

// CTK includes
#include "ctkButtonGroup.h"

// qMRML includes
#include "qMRMLWidget.h"

class qSlicerAppMainWindow;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SlicerWelcome
class qSlicerWelcomeModuleWidgetPrivate: public Ui_qSlicerWelcomeModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerWelcomeModuleWidget);
protected:
  qSlicerWelcomeModuleWidget* const q_ptr;
public:
  qSlicerWelcomeModuleWidgetPrivate(qSlicerWelcomeModuleWidget& object);
  void setupUi(qSlicerWidget* widget);

  bool selectModule(const QString& moduleName);

  QSignalMapper CollapsibleButtonMapper;
};

//-----------------------------------------------------------------------------
// qSlicerWelcomeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerWelcomeModuleWidgetPrivate::qSlicerWelcomeModuleWidgetPrivate(qSlicerWelcomeModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerWelcomeModuleWidget);

  this->Ui_qSlicerWelcomeModuleWidget::setupUi(widget);

  // QLabel's pixmap property loads the base image (ignores high-resolution @2x versions),
  // therefore we need to retrieve and set the best icon version manually.
  this->label->setPixmap(qMRMLWidget::pixmapFromIcon(QIcon(":/Images/WelcomeLogo.png")));

  // Create the button group ensuring that only one collabsibleWidgetButton will be open at a time
  ctkButtonGroup * group = new ctkButtonGroup(widget);

  // Add all collabsibleWidgetButton to a button group
  QList<ctkCollapsibleButton*> collapsibles = widget->findChildren<ctkCollapsibleButton*>();
  foreach(ctkCollapsibleButton* collapsible, collapsibles)
    {
    group->addButton(collapsible);
    }

  QObject::connect(this->OpenExtensionsManagerButton, SIGNAL(clicked()),
                   qSlicerApplication::application(), SLOT(openExtensionsManagerDialog()));


  // Lazily set the fitted browser source to avoid overhead when the module
  // is loaded.
  this->FeedbackCollapsibleWidget->setProperty("source", ":HTML/Feedback.html");
  this->WelcomeAndAboutCollapsibleWidget->setProperty("source", ":HTML/About.html");
  this->OtherUsefulHintsCollapsibleWidget->setProperty("source", ":HTML/OtherUsefulHints.html");
  this->AcknowledgmentCollapsibleWidget->setProperty("source", ":HTML/Acknowledgment.html");

  foreach(QWidget* widget, QWidgetList()
          << this->FeedbackCollapsibleWidget
          << this->WelcomeAndAboutCollapsibleWidget
          << this->OtherUsefulHintsCollapsibleWidget
          << this->AcknowledgmentCollapsibleWidget
          )
    {
    this->CollapsibleButtonMapper.setMapping(widget, widget);
    QObject::connect(widget, SIGNAL(contentsCollapsed(bool)),
                     &this->CollapsibleButtonMapper, SLOT(map()));
    }

  QObject::connect(&this->CollapsibleButtonMapper, SIGNAL(mapped(QWidget*)),
                   q, SLOT(loadSource(QWidget*)));
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::loadSource(QWidget* widget)
{
  // Lookup fitted browser
  ctkFittedTextBrowser* fittedTextBrowser =
      widget->findChild<ctkFittedTextBrowser*>();
  Q_ASSERT(fittedTextBrowser);
  if (fittedTextBrowser->source().isEmpty())
    {
    // Read content
    QString url = widget->property("source").toString();
    QFile source(url);
    if(!source.open(QIODevice::ReadOnly))
      {
      qWarning() << Q_FUNC_INFO << ": Failed to read" << url;
      return;
      }
    QTextStream in(&source);
    QString html = in.readAll();
    source.close();

    qSlicerCoreApplication* app = qSlicerCoreApplication::application();

    // Update occurrences of documentation URLs
    html = qSlicerUtils::replaceDocumentationUrlVersion(html,
      QUrl(app->documentationBaseUrl()).host(), app->documentationVersion());

    fittedTextBrowser->setHtml(html);
    }
}

//-----------------------------------------------------------------------------
bool qSlicerWelcomeModuleWidgetPrivate::selectModule(const QString& moduleName)
{
  Q_Q(qSlicerWelcomeModuleWidget);
  qSlicerModuleManager * moduleManager = qSlicerCoreApplication::application()->moduleManager();
  if (!moduleManager)
    {
    return false;
    }
  qSlicerAbstractCoreModule * module = moduleManager->module(moduleName);
  if(!module)
    {
    QMessageBox::warning(
          q, qSlicerWelcomeModuleWidget::tr("Raising %1 Module:").arg(moduleName),
          qSlicerWelcomeModuleWidget::tr("Unfortunately, this requested module is not available in this Slicer session."),
          QMessageBox::Ok);
    return false;
    }
  qSlicerLayoutManager * layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    return false;
    }
  layoutManager->setCurrentModule(moduleName);
  return true;
}

//-----------------------------------------------------------------------------
// qSlicerWelcomeModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerWelcomeModuleWidget::qSlicerWelcomeModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWelcomeModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerWelcomeModuleWidget::~qSlicerWelcomeModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::setup()
{
  Q_D(qSlicerWelcomeModuleWidget);
  d->setupUi(this);

  connect(d->LoadDicomDataButton, SIGNAL(clicked()),
          this, SLOT (loadDicomData()));
  connect(d->LoadNonDicomDataButton, SIGNAL(clicked()),
          this, SLOT (loadNonDicomData()));
  connect(d->LoadSampleDataButton, SIGNAL(clicked()),
          this, SLOT (loadRemoteSampleData()));
  connect(d->EditApplicationSettingsButton, SIGNAL(clicked()),
          this, SLOT (editApplicationSettings()));
  connect(d->ExploreLoadedDataPushButton, SIGNAL(clicked()),
          this, SLOT (exploreLoadedData()));

#ifndef Slicer_BUILD_DICOM_SUPPORT
  d->LoadDicomDataButton->setDisabled(true);
#endif

  this->Superclass::setup();

  d->FeedbackCollapsibleWidget->setCollapsed(false);
}


//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::editApplicationSettings()
{
  qSlicerApplication::application()->settingsDialog()->exec();
}



//-----------------------------------------------------------------------------
bool qSlicerWelcomeModuleWidget::loadDicomData()
{
  Q_D(qSlicerWelcomeModuleWidget);
  return d->selectModule("DICOM");
}


//-----------------------------------------------------------------------------
bool qSlicerWelcomeModuleWidget::loadNonDicomData()
{
  qSlicerIOManager *ioManager = qSlicerApplication::application()->ioManager();
  if (!ioManager)
    {
    return false;
    }
  return ioManager->openAddDataDialog();
}


//-----------------------------------------------------------------------------
bool qSlicerWelcomeModuleWidget::loadRemoteSampleData()
{
  Q_D(qSlicerWelcomeModuleWidget);
  return d->selectModule("SampleData");
}


//-----------------------------------------------------------------------------
bool qSlicerWelcomeModuleWidget::exploreLoadedData()
{
  Q_D(qSlicerWelcomeModuleWidget);
  return d->selectModule("Data");
}
