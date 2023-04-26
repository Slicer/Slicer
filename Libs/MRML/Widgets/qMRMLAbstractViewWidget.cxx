/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

// Qt includes
#include <QDebug>

// CTK includes
#include <ctkVTKAbstractView.h>

// qMRML includes
#include "qMRMLAbstractViewWidget.h"

// MRML includes
#include <vtkMRMLAbstractViewNode.h>

// --------------------------------------------------------------------------
// qMRMLAbstractViewWidget methods

// --------------------------------------------------------------------------
qMRMLAbstractViewWidget::qMRMLAbstractViewWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
{
}

// --------------------------------------------------------------------------
QColor qMRMLAbstractViewWidget::viewColor() const
{
  if (!this->mrmlAbstractViewNode())
    {
    qWarning() << Q_FUNC_INFO << " failed: view node is invalid";
    return QColor(127, 127, 127);
    }
  double* layoutColorVtk = this->mrmlAbstractViewNode()->GetLayoutColor();
  QColor layoutColor = QColor::fromRgbF(layoutColorVtk[0], layoutColorVtk[1], layoutColorVtk[2]);
  return layoutColor;
}

// --------------------------------------------------------------------------
void qMRMLAbstractViewWidget::setViewColor(const QColor& newViewColor)
{
  if (!this->mrmlAbstractViewNode())
    {
    qWarning() << Q_FUNC_INFO << " failed: view node is invalid";
    return;
    }
  double layoutColor[3] = { newViewColor.redF(), newViewColor.greenF(), newViewColor.blueF() };
  this->mrmlAbstractViewNode()->SetLayoutColor(layoutColor);
}

//---------------------------------------------------------------------------
void qMRMLAbstractViewWidget::setViewLabel(const QString& newViewLabel)
{
  if (!this->mrmlAbstractViewNode())
    {
    qWarning() << Q_FUNC_INFO << " failed: view node is invalid";
    return;
    }
  std::string newViewLabelString = newViewLabel.toStdString();
  this->mrmlAbstractViewNode()->SetLayoutLabel(newViewLabelString.c_str());
}

//---------------------------------------------------------------------------
QString qMRMLAbstractViewWidget::viewLabel() const
{
  if (!this->mrmlAbstractViewNode())
    {
    qWarning() << Q_FUNC_INFO << " failed: view node is invalid";
    return "";
    }
  return this->mrmlAbstractViewNode()->GetLayoutLabel();
}

//---------------------------------------------------------------------------
vtkMRMLAbstractLogic* qMRMLAbstractViewWidget::logic() const
{
  return nullptr;
}

//---------------------------------------------------------------------------
void qMRMLAbstractViewWidget::setRenderPaused(bool pause)
{
  if (pause)
    {
    this->pauseRender();
    }
  else
    {
    this->resumeRender();
    }
}

//---------------------------------------------------------------------------
void qMRMLAbstractViewWidget::pauseRender()
{
  ctkVTKAbstractView* view = qobject_cast<ctkVTKAbstractView*>(this->viewWidget());
  if (view)
    {
    view->pauseRender();
    }
}

//---------------------------------------------------------------------------
void qMRMLAbstractViewWidget::resumeRender()
{
  ctkVTKAbstractView* view = qobject_cast<ctkVTKAbstractView*>(this->viewWidget());
  if (view)
    {
    view->resumeRender();
    }
}
