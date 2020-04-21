/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLExpandingWebView_p.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLExpandingWebView");
//--------------------------------------------------------------------------


const char *htmlPreamble =
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<title>Scene</title>"
  "</head>"
  "<body>";

const char *htmlPostscript =
  "</body>"
  "</html>";



//--------------------------------------------------------------------------
// qMRMLExpandingWebViewPrivate methods

//---------------------------------------------------------------------------
qMRMLExpandingWebViewPrivate::qMRMLExpandingWebViewPrivate(qMRMLExpandingWebView& object)
  : q_ptr(&object)
{
  this->MRMLScene = nullptr;
}

//---------------------------------------------------------------------------
qMRMLExpandingWebViewPrivate::~qMRMLExpandingWebViewPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLExpandingWebViewPrivate::init()
{
  Q_Q(qMRMLExpandingWebView);

  // Let the QWebView expand in both directions
  q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  q->setHtml("");
  q->show();
}

//---------------------------------------------------------------------------
void qMRMLExpandingWebViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  //Q_Q(qMRMLExpandingWebView);
  if (newScene == this->MRMLScene)
    {
    return;
    }

  this->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::StartBatchProcessEvent, this, SLOT(startProcessing()));

  this->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::EndBatchProcessEvent, this, SLOT(endProcessing()));

  this->MRMLScene = newScene;
}


// --------------------------------------------------------------------------
void qMRMLExpandingWebViewPrivate::startProcessing()
{
//  logger.trace("startProcessing");
//  Q_Q(qMRMLExpandingWebView);
}

//
// --------------------------------------------------------------------------
void qMRMLExpandingWebViewPrivate::endProcessing()
{
//  logger.trace("endProcessing");
  Q_Q(qMRMLExpandingWebView);
  q->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLExpandingWebViewPrivate::mrmlScene()
{
  return this->MRMLScene;
}


// --------------------------------------------------------------------------
// qMRMLExpandingWebView methods

// --------------------------------------------------------------------------
qMRMLExpandingWebView::qMRMLExpandingWebView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLExpandingWebViewPrivate(*this))
{
  Q_D(qMRMLExpandingWebView);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLExpandingWebView::~qMRMLExpandingWebView()
{
  this->setMRMLScene(nullptr);
}


//------------------------------------------------------------------------------
void qMRMLExpandingWebView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLExpandingWebView);
  if (newScene == d->MRMLScene)
    {
    return;
    }

  d->setMRMLScene(newScene);

  emit mrmlSceneChanged(newScene);
}

//---------------------------------------------------------------------------
vtkMRMLScene* qMRMLExpandingWebView::mrmlScene()const
{
  Q_D(const qMRMLExpandingWebView);
  return d->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLExpandingWebView::updateWidgetFromMRML()
{
  //qDebug() << "qMRMLExpandingWebView::updateWidgetFromMRML()";

  if (!this->mrmlScene())
    {
    return;
    }

  if (!this->isEnabled())
    {
    return;
    }

  /*
  // get a node
  char *nodeid = 0;

  if (!nodeid)
    {
    q->setHtml("");
    q->show();
    return;
    }
  */

  // Assemble the page
  //
  // 1. HTML page preamble
  // 2 to n-1: customise
  // n. HTML page poscript
  //
  QStringList html;
  html << htmlPreamble;       // 1. page header, css, javascript

  html << htmlPostscript;   // 5. page postscript, additional javascript

  //qDebug() << html.join("");

  // show the html
  this->setHtml(html.join(""));
  this->show();

}

//---------------------------------------------------------------------------
QSize qMRMLExpandingWebView::sizeHint()const
{
  // return a default size hint (invalid size)
  return QSize();
}
