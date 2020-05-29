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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QTimer>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerDataDialog.h"
#include "qSlicerIO.h"
#include "qSlicerIOOptionsWidget.h"

// STD includes


//-----------------------------------------------------------------------------
class qSlicerDummyIOOptionsWidget
  : public qSlicerIOOptionsWidget
{
public:
  qSlicerDummyIOOptionsWidget(QWidget *parent=nullptr): qSlicerIOOptionsWidget(parent){}
  ~qSlicerDummyIOOptionsWidget() override = default;
  QSize minimumSizeHint()const override {return QSize(300, 30);}
  QSize sizeHint()const override{return QSize(500,30);}

private:
  Q_DISABLE_COPY(qSlicerDummyIOOptionsWidget);
};

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Dummy
class qSlicerDummyIO: public qSlicerIO
{
public:
  qSlicerDummyIO(QObject* parent = nullptr):qSlicerIO(parent){}
  ~qSlicerDummyIO() override = default;
  QString description()const override{return "Dummy";}
  IOFileType fileType()const override{return QString("UserFile");}
  virtual QStringList extensions()const{return QStringList(QString("All Files(*)"));}
  qSlicerIOOptions* options()const override{return new qSlicerDummyIOOptionsWidget;}

  virtual bool load(const IOProperties& properties);
};

//-----------------------------------------------------------------------------
bool qSlicerDummyIO::load(const IOProperties& properties)
{
  Q_UNUSED(properties);
  return true;
}

//-----------------------------------------------------------------------------
int qSlicerDataDialogTest1(int argc, char * argv[] )
{
  qSlicerApplication app(argc, argv);
  app.coreIOManager()->registerIO(new qSlicerDummyIO(nullptr));

  qSlicerDataDialog dataDialog;

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    // Quit the dialog
    QTimer::singleShot(100, &app, SLOT(quit()));
    // Quit the app
    QTimer::singleShot(120, &app, SLOT(quit()));
    }

  return dataDialog.exec();
}

