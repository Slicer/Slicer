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
#include <QDesktopServices>
#include <QMainWindow>
#include <QMessageBox>

// SlicerQt includes
#include "qSlicerWelcomeModuleWidget.h"
#include "ui_qSlicerWelcomeModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerModulePanel.h"
#include "qSlicerCorePythonManager.h"

// CTK includes
#include "ctkButtonGroup.h"

class qSlicerMainWindow;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SlicerWelcome
class qSlicerWelcomeModuleWidgetPrivate: public Ui_qSlicerWelcomeModule
{
public:
  void setupUi(qSlicerWidget* widget);
};

//-----------------------------------------------------------------------------
qSlicerWelcomeModuleWidget::qSlicerWelcomeModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWelcomeModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerWelcomeModuleWidget::~qSlicerWelcomeModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::setup()
{
  Q_D(qSlicerWelcomeModuleWidget);
  d->setupUi(this);

  //--- connections.
  connect (d->LoadDicomDataButton, SIGNAL (clicked() ),
          this, SLOT (loadDicomData()) );
  connect (d->LoadNonDicomDataButton, SIGNAL(clicked()),
           this, SLOT (loadNonDicomData()) );
  connect (d->LoadSampleDataButton, SIGNAL(clicked()),
           this, SLOT (loadRemoteSampleData()) );
  connect (d->ViewTutorialsButton, SIGNAL(clicked()),
           this, SLOT (presentTutorials()) );

  this->Superclass::setup();
           
}

//-----------------------------------------------------------------------------
int qSlicerWelcomeModuleWidget::loadDicomData()
{
  Q_D(qSlicerWelcomeModuleWidget);
  // open add volume ui
  qSlicerIOManager *ioManager = qSlicerApplication::application()->ioManager();
  if (ioManager)
    {
    ioManager->openAddVolumesDialog();
    return (1);
    }
  return (0);

}


//-----------------------------------------------------------------------------
int qSlicerWelcomeModuleWidget::loadNonDicomData()
{
  Q_D(qSlicerWelcomeModuleWidget);
  // open the add data ui
  qSlicerIOManager *ioManager = qSlicerApplication::application()->ioManager();
  if (ioManager)
    {  
      ioManager->openAddDataDialog();
      return (1);
    }
  return (0);
}


//-----------------------------------------------------------------------------
int qSlicerWelcomeModuleWidget::loadRemoteSampleData()
{
  Q_D(qSlicerWelcomeModuleWidget);



  // is the module present?
  qSlicerModuleManager * moduleManager = qSlicerCoreApplication::application()->moduleManager();
  qSlicerAbstractCoreModule * sampleDataModule = moduleManager->module("SampleData");
  if(!sampleDataModule){
  QMessageBox::warning (this, tr("Raising SampleData Module:"), tr("Unfortunately, this module is not available in this Slicer session."), QMessageBox::Ok);
    return (0);
    }

  // open the sample data module 

/*
   //C++ test
  qSlicerModulePanel * modulePanel =   qSlicerApplication::application()->mainWindow()->modulePanel();
  if ( modulePanel)
    {
    modulePanel->setModule ("SampleData");
    }

*/
/*
    // Steve's code
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('SampleData')
*/

      qSlicerCorePythonManager * pythonManager = qSlicerCoreApplication::application()->corePythonManager();
      if (!pythonManager)
        {
  QMessageBox::warning (this, tr("Raising SampleData Module:"), tr("Unfortunately, the python script for displaying the sample data module is not present in this Slicer session."), QMessageBox::Ok);
        }

      pythonManager->executeString(QString("slicer.util.mainWindow().moduleSelector().selectModule('SampleData');"));
  
  return (1);
}



//-----------------------------------------------------------------------------
int qSlicerWelcomeModuleWidget::presentTutorials()
{
  Q_D(qSlicerWelcomeModuleWidget);
  // open browser with slicer wiki tutorials.
  QDesktopServices::openUrl(QUrl("http://www.slicer.org/slicerWiki/index.php/Slicer3.6:Training"));
  return (1);
}



//-----------------------------------------------------------------------------
// qSlicerWelcomeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  this->Ui_qSlicerWelcomeModule::setupUi(widget);

  // Create the button group ensuring that only one collabsibleWidgetButton will be open at a time
  ctkButtonGroup * group = new ctkButtonGroup(widget);
  
  // Add all collabsibleWidgetButton to a button group
  QList<ctkCollapsibleButton*> collapsibles = widget->findChildren<ctkCollapsibleButton*>();
  foreach(ctkCollapsibleButton* collapsible, collapsibles)
    {
    group->addButton(collapsible);
    }
}


