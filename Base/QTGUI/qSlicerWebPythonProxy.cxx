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
#include "qSlicerWebPythonProxy_p.h"

//------------------------------------------------------------------------------
qSlicerWebPythonProxyPrivate::~qSlicerWebPythonProxyPrivate() = default;

CTK_GET_CPP(qSlicerWebPythonProxy, bool, verbose, Verbose);
CTK_SET_CPP_EMIT(qSlicerWebPythonProxy, bool, setVerbose, Verbose, verboseChanged);

// --------------------------------------------------------------------------
bool qSlicerWebPythonProxyPrivate::isPythonEvaluationAllowed()
{
#ifdef Slicer_USE_PYTHONQT
  if (this->PythonEvaluationAllowed)
  {
    return true;
  }

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  if (!app || qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
  {
    return false;
  }

  ctkMessageBox* confirmationBox = new ctkMessageBox(qSlicerApplication::application()->mainWindow());
  confirmationBox->setAttribute(Qt::WA_DeleteOnClose);
  confirmationBox->setWindowTitle(qSlicerWebPythonProxy::tr("Allow Python execution?"));
  confirmationBox->setText(qSlicerWebPythonProxy::tr("Allow the web page to execute code using Slicer's python?"));

  confirmationBox->addButton(qSlicerWebPythonProxy::tr("Allow"), QMessageBox::AcceptRole);
  confirmationBox->addButton(qSlicerWebPythonProxy::tr("Reject"), QMessageBox::RejectRole);

  confirmationBox->setDontShowAgainVisible(true);
  confirmationBox->setDontShowAgainSettingsKey("WebEngine/AllowPythonExecution");
  confirmationBox->setIcon(QMessageBox::Question);
  int resultCode = confirmationBox->exec();

  if (resultCode == QMessageBox::AcceptRole)
  {
    this->PythonEvaluationAllowed = true;
  }
#endif
  return this->PythonEvaluationAllowed;
}

//------------------------------------------------------------------------------
qSlicerWebPythonProxy::qSlicerWebPythonProxy(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerWebPythonProxyPrivate)
{
}

//------------------------------------------------------------------------------
qSlicerWebPythonProxy::qSlicerWebPythonProxy(qSlicerWebPythonProxyPrivate* pimpl)
  : d_ptr(pimpl)
{
}

//------------------------------------------------------------------------------
qSlicerWebPythonProxy::~qSlicerWebPythonProxy() = default;

// --------------------------------------------------------------------------
QString qSlicerWebPythonProxy::evalPython(const QString& python, int mode)
{
  Q_D(qSlicerWebPythonProxy);

  ctkAbstractPythonManager::ExecuteStringMode executeStringMode{ ctkAbstractPythonManager::FileInput };
  switch (mode)
  {
    case qSlicerWebPythonProxy::EvalInput: executeStringMode = ctkAbstractPythonManager::EvalInput; break;
    case qSlicerWebPythonProxy::FileInput: executeStringMode = ctkAbstractPythonManager::FileInput; break;
    case qSlicerWebPythonProxy::SingleInput: executeStringMode = ctkAbstractPythonManager::SingleInput; break;
    default: qWarning() << Q_FUNC_INFO << " failed: Unknown mode" << mode; break;
  }

  QString result;
#ifdef Slicer_USE_PYTHONQT
  if (d->isPythonEvaluationAllowed())
  {
    qSlicerPythonManager* pythonManager = qSlicerApplication::application()->pythonManager();
    result = pythonManager->executeString(python, executeStringMode).toString();
    if (this->verbose())
    {
      qDebug() << "Running " << python << " result is " << result;
    }
  }
#else
  Q_UNUSED(python);
#endif
  return result;
}
