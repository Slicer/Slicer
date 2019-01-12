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

// CTK includes
#include "ctkMessageBox.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerPythonManager.h"
#include "qSlicerWebPythonProxy.h"

// --------------------------------------------------------------------------
qSlicerWebPythonProxy::qSlicerWebPythonProxy(QWidget *parent)
  : QWidget(parent)
{
}

// --------------------------------------------------------------------------
QString qSlicerWebPythonProxy::evalPython(const QString &python)
{
  ctkMessageBox* confirmationBox = new ctkMessageBox(this);
  confirmationBox->setAttribute(Qt::WA_DeleteOnClose);
  confirmationBox->setWindowTitle(tr("Allow Python execution?"));
  confirmationBox->setText("Allow the web page has asked to execute code using Slicer's python?");

  confirmationBox->addButton(tr("Allow"), QMessageBox::AcceptRole);
  confirmationBox->addButton(tr("Reject"), QMessageBox::RejectRole);

  confirmationBox->setDontShowAgainVisible(true);
  confirmationBox->setDontShowAgainSettingsKey("WebEngine/AllowPythonExecution");
  confirmationBox->setIcon(QMessageBox::Question);
  int resultCode = confirmationBox->exec();

  QString result;
  if (resultCode == QMessageBox::AcceptRole)
    {
    qSlicerPythonManager *pythonManager = qSlicerApplication::application()->pythonManager();
    result = pythonManager->executeString(python).toString();
    }
  qDebug() << "Running " << python << " result is " << result;
  return result;
}
