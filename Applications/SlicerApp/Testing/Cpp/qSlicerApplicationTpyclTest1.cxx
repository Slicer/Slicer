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
#include <QTemporaryFile>
#include <QTextStream>

// PythonQT includes
#include <PythonQt.h>

// SlicerApp includes
#include "qSlicerApplication.h"
#include "qSlicerPythonManager.h"

// VTK includes
#include <vtkNew.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

// STD includes
#include <cstdlib>

namespace
{
//---------------------------------------------------------------------------
QString evaluateTcl(qSlicerPythonManager * pythonManager, const QString& tclCode)
{
  pythonManager->executeString("from __main__ import tcl");
  // TODO How could we catch Tcl exception ?
  pythonManager->executeString(QString("__rv = tcl('%1')").arg(tclCode));
  QVariant returnValue = pythonManager->getVariable("__rv");
  return returnValue.toString();
}
} // end of anonymous namespace

//---------------------------------------------------------------------------
int qSlicerApplicationTpyclTest1(int argc, char * argv[])
{
  qSlicerApplication app(argc, argv);
  qSlicerPythonManager * pythonManager = app.pythonManager();

  QTemporaryFile tclFile("qSlicerApplicationTpyclTest1-XXXXXX.tcl");
  //tclFile.setAutoRemove(false);
  if (!tclFile.open())
    {
    std::cerr << "Line " << __LINE__
              << " - Failed to open temporary file" << qPrintable(tclFile.fileName());
    return EXIT_FAILURE;
    }
  QTextStream out(&tclFile);
  out << "namespace eval qSlicerApplicationTypclTest {" << endl;
  out << "  variable VTKTABLE" << endl;
  out << endl;
  out << "  proc SetTable { table } {" << endl;
  out << "    variable VTKTABLE" << endl;
  out << "    puts \"table=$table\"" << endl;
  out << "    set VTKTABLE $table" << endl;
  out << "  }" << endl;
  out << endl;
  out << "  proc DumpTable { } {" << endl;
  out << "    variable VTKTABLE" << endl;
  out << "    puts \"VTKTABLE=$VTKTABLE\"" << endl;
  out << "    $VTKTABLE Dump" << endl;
  out << "  }" << endl;
  out << "}" << endl;
  tclFile.close();

  qDebug() << "tclFile" << tclFile.fileName();

  QString tclCode = QString("from __main__ import tcl\ntcl('source %1')\n");
  pythonManager->executeString(tclCode.arg(tclFile.fileName()));

  vtkNew<vtkTable> table;
  vtkNew<vtkStringArray> stringArray;
  stringArray->SetNumberOfValues(4);
  stringArray->SetName("stringArray");
  stringArray->InsertValue(0, "zero");
  stringArray->InsertValue(1, "one");
  stringArray->InsertValue(2, "two");
  stringArray->InsertValue(3, "three");
  table->AddColumn(stringArray.GetPointer());

  QString varName("table4tpycl");
  QString varNameWithinModule = QString("slicer.") + varName;
  pythonManager->addVTKObjectToPythonMain(varNameWithinModule, table.GetPointer());
  //qDebug() << "varName" << varName;
  //qDebug() << "varNameWithinModule" << varNameWithinModule;

  tclCode = QString("proc ::%1  {args} {::tpycl::methodCaller %1 %2 $args}")
      .arg(varName).arg(varNameWithinModule);
  evaluateTcl(pythonManager, tclCode);

  tclCode = QString("::qSlicerApplicationTypclTest::SetTable %1");
  evaluateTcl(pythonManager, tclCode.arg(varName));

  tclCode = QString("::qSlicerApplicationTypclTest::DumpTable");
  evaluateTcl(pythonManager, tclCode);

  return EXIT_SUCCESS;
}

