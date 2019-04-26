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

  This file was originally developed by Steve Pieper, Isomics Inc.
  and was partially funded by NSF grant DBI 1759883

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QMainWindow>

// CTK includes
#include "ctkMessageBox.h"

// Slicer includes
#include "qSlicerApplication.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerPythonManager.h"
#endif
#include "qSlicerWebPythonProxy.h"

// --------------------------------------------------------------------------
qSlicerWebPythonProxy::qSlicerWebPythonProxy(QObject *parent)
  : QObject(parent)
{
  this->pythonEvaluationAllowed = false;
}

// --------------------------------------------------------------------------
bool qSlicerWebPythonProxy::isPythonEvaluationAllowed()
{
#ifdef Slicer_USE_PYTHONQT
  if (this->pythonEvaluationAllowed)
    {
    return true;
    }

  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  if (!app || qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    return false;
    }

  ctkMessageBox* confirmationBox = new ctkMessageBox(qSlicerApplication::application()->mainWindow());
  confirmationBox->setAttribute(Qt::WA_DeleteOnClose);
  confirmationBox->setWindowTitle(tr("Allow Python execution?"));
  confirmationBox->setText("Allow the web page has asked to execute code using Slicer's python?");

  confirmationBox->addButton(tr("Allow"), QMessageBox::AcceptRole);
  confirmationBox->addButton(tr("Reject"), QMessageBox::RejectRole);

  confirmationBox->setDontShowAgainVisible(true);
  confirmationBox->setDontShowAgainSettingsKey("WebEngine/AllowPythonExecution");
  confirmationBox->setIcon(QMessageBox::Question);
  int resultCode = confirmationBox->exec();

  if (resultCode == QMessageBox::AcceptRole)
    {
    this->pythonEvaluationAllowed = true;
    }
#endif
  return this->pythonEvaluationAllowed;
}

// --------------------------------------------------------------------------
QString qSlicerWebPythonProxy::evalPython(const QString &python)
{

  QString result;
#ifdef Slicer_USE_PYTHONQT
  if (this->isPythonEvaluationAllowed())
    {
    qSlicerPythonManager *pythonManager = qSlicerApplication::application()->pythonManager();
    result = pythonManager->executeString(python).toString();
    qDebug() << "Running " << python << " result is " << result;
    }
#else
  Q_UNUSED(python);
#endif
  return result;
}
