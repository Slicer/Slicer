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
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// Texts widgets includes
#include "qMRMLTextWidget.h"

// MRML includes
#include <vtkMRMLTextNode.h>

// CTK includes
#include <ctkMessageBox.h>

// Qt includes
#include <QStyle>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qMRMLTextWidgetPrivate
  : public Ui_qMRMLTextWidget
{
  Q_DECLARE_PUBLIC(qMRMLTextWidget);
protected:
  qMRMLTextWidget* const q_ptr;

public:
  qMRMLTextWidgetPrivate( qMRMLTextWidget& object);
  ~qMRMLTextWidgetPrivate();
  virtual void setupUi(qMRMLTextWidget*);

  vtkSmartPointer<vtkMRMLTextNode> CurrentTextNode;
  bool TextEditModified;
  bool TextNodeContentsModified;

protected:
  bool Editing;

public:
  bool isEditing();
  void setEditing(bool editing);
};

// --------------------------------------------------------------------------
qMRMLTextWidgetPrivate::qMRMLTextWidgetPrivate(qMRMLTextWidget& object)
  : q_ptr(&object)
  , TextEditModified(false)
  , TextNodeContentsModified(false)
  , Editing(false)
{
}

//-----------------------------------------------------------------------------
qMRMLTextWidgetPrivate::~qMRMLTextWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qMRMLTextWidgetPrivate::setupUi(qMRMLTextWidget* widget)
{
  this->Ui_qMRMLTextWidget::setupUi(widget);
}

// --------------------------------------------------------------------------
bool qMRMLTextWidgetPrivate::isEditing()
{
  return this->Editing;
}

// --------------------------------------------------------------------------
void qMRMLTextWidgetPrivate::setEditing(bool editing)
{
  Q_Q(qMRMLTextWidget);
  if (this->Editing == editing)
    {
    return;
    }

  this->Editing = editing;
  q->editingChanged(this->Editing);
}

//-----------------------------------------------------------------------------
// qMRMLTextWidget methods

//-----------------------------------------------------------------------------
qMRMLTextWidget::qMRMLTextWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr(new qMRMLTextWidgetPrivate(*this))
{
  this->setup();
}

//-----------------------------------------------------------------------------
qMRMLTextWidget::~qMRMLTextWidget()
{
  this->setMRMLTextNode(nullptr);
}

//-----------------------------------------------------------------------------
void qMRMLTextWidget::setup()
{
  Q_D(qMRMLTextWidget);

  d->setupUi(this);

  connect(d->TextEdit, SIGNAL(textChanged()), this, SLOT(onTextEditChanged()));
  connect(d->EditButton, SIGNAL(clicked()), this, SLOT(startEdits()));
  connect(d->CancelButton, SIGNAL(clicked()), this, SLOT(cancelEdits()));
  connect(d->SaveButton, SIGNAL(clicked()), this, SLOT(saveEdits()));
  connect(this, SIGNAL(updateWidgetFromMRMLRequested()), this, SLOT(updateWidgetFromMRML()));
  connect(this, SIGNAL(updateMRMLFromWidgetRequested()), this, SLOT(updateMRMLFromWidget()));

  this->updateWidgetFromMRMLRequested();
}

//------------------------------------------------------------------------------
void qMRMLTextWidget::setMRMLNode(vtkMRMLNode* node)
{
  this->setMRMLTextNode(vtkMRMLTextNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLTextWidget::setMRMLTextNode(vtkMRMLTextNode* node)
{
  Q_D(qMRMLTextWidget);
  if (node == d->CurrentTextNode)
    {
    // not changed
    return;
    }

  // Reconnect the appropriate nodes
  this->qvtkReconnect(d->CurrentTextNode, node, vtkMRMLTextNode::TextModifiedEvent, this, SLOT(onTextNodeContentsModified()));
  d->CurrentTextNode = node;
  d->Editing = false;
  this->updateWidgetFromMRMLRequested();
  this->mrmlNodeChanged(node);
}

//------------------------------------------------------------------------------
vtkMRMLTextNode* qMRMLTextWidget::mrmlTextNode()const
{
  Q_D(const qMRMLTextWidget);
  return d->CurrentTextNode;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLTextWidget::mrmlNode()const
{
  Q_D(const qMRMLTextWidget);
  return d->CurrentTextNode;
}

//-----------------------------------------------------------------------------
void qMRMLTextWidget::onTextNodeContentsModified()
{
  Q_D(qMRMLTextWidget);
  d->TextNodeContentsModified = true;
  this->updateWidgetFromMRMLRequested();
}

//-----------------------------------------------------------------------------
void qMRMLTextWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLTextWidget);

  bool updateText = true;
  bool editing = d->isEditing();
  if (this->ReadOnly)
    {
    updateText = true;
    d->TextEdit->setReadOnly(true);
    d->EditButton->setVisible(false);
    d->CancelButton->setVisible(false);
    d->SaveButton->setVisible(false);
    }
  else if (this->AutoSave)
    {
    updateText = true;
    d->TextEdit->setReadOnly(false);
    d->EditButton->setVisible(false);
    d->CancelButton->setVisible(false);
    d->SaveButton->setVisible(false);
    }
  else if (editing)
    {
    updateText = false;
    d->TextEdit->setReadOnly(false);
    d->EditButton->setVisible(false);
    d->CancelButton->setVisible(true);
    d->SaveButton->setVisible(true);
    }
  else
    {
    updateText = true;
    d->TextEdit->setReadOnly(true);
    d->EditButton->setVisible(true);
    d->CancelButton->setVisible(false);
    d->SaveButton->setVisible(false);
    }

  d->EditButton->setEnabled(!editing);
  d->CancelButton->setEnabled(editing);
  d->SaveButton->setEnabled(editing);

  bool wasBlocking = d->TextEdit->blockSignals(true);
  if (!d->CurrentTextNode)
    {
    d->TextEdit->setReadOnly(true);
    d->TextEdit->setText("");
    d->TextNodeContentsModified = false;
    d->EditButton->setEnabled(false);
    d->CancelButton->setEnabled(false);
    d->SaveButton->setEnabled(false);
    }
  else if (updateText)
    {
    int position = d->TextEdit->textCursor().position();
    std::string text;
    if (d->CurrentTextNode)
      {
      text = d->CurrentTextNode->GetText();
      }
    d->TextEdit->setText(text.c_str());
    d->TextNodeContentsModified = false;
    QTextCursor cursor = d->TextEdit->textCursor();
    position = std::min(position, d->TextEdit->toPlainText().length());
    cursor.setPosition(position);
    d->TextEdit->setTextCursor(cursor);
    }
  d->TextEdit->blockSignals(wasBlocking);

  if (d->TextNodeContentsModified)
    {
    d->SaveButton->setToolTip("The original text has been modified since editing was started");
    QIcon warningIcon = this->style()->standardIcon(QStyle::SP_MessageBoxWarning);
    d->SaveButton->setIcon(warningIcon);
    d->SaveButton->setIconSize(QSize(12, 12));
    }
  else
    {
    d->SaveButton->setToolTip("The original text has been modified since editing was started");
    d->SaveButton->setIcon(QIcon());
    }

  this->updateWidgetFromMRMLFinished();
}

//-----------------------------------------------------------------------------
void qMRMLTextWidget::updateMRMLFromWidget()
{
  Q_D(qMRMLTextWidget);
  if (!d->CurrentTextNode)
    {
    return;
    }
  std::string text = d->TextEdit->toPlainText().toStdString();
  d->CurrentTextNode->SetText(text.c_str(), VTK_ENCODING_UTF_8);
  this->updateMRMLFromWidgetFinished();
}

//------------------------------------------------------------------------------
void qMRMLTextWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  this->updateWidgetFromMRMLRequested();
}

//------------------------------------------------------------------------------
bool qMRMLTextWidget::isReadOnly()
{
  return this->ReadOnly;
}

//------------------------------------------------------------------------------
bool qMRMLTextWidget::isEditing()
{
  Q_D(qMRMLTextWidget);
  return d->isEditing();
}

//------------------------------------------------------------------------------
void qMRMLTextWidget::setReadOnly(bool readOnly)
{
  Q_D(qMRMLTextWidget);
  if (this->ReadOnly == readOnly)
    {
    return;
    }
  this->ReadOnly = readOnly;

  this->updateMRMLFromWidgetRequested();
  this->readOnlyChanged(this->ReadOnly);
  this->updateWidgetFromMRMLRequested();
}

//------------------------------------------------------------------------------
bool qMRMLTextWidget::isAutoSave()
{
  Q_D(qMRMLTextWidget);
  return this->AutoSave;
}

//------------------------------------------------------------------------------
void qMRMLTextWidget::setAutoSave(bool autoSave)
{
  Q_D(qMRMLTextWidget);
  if (this->AutoSave == autoSave)
    {
    return;
    }

  this->AutoSave = autoSave;
  if (this->AutoSave)
    {
    d->setEditing(false);
    }

  this->updateMRMLFromWidgetRequested();
  this->autoSaveChanged(this->AutoSave);
  this->updateWidgetFromMRMLRequested();
}

//------------------------------------------------------------------------------
bool qMRMLTextWidget::wordWrap()
{
  Q_D(qMRMLTextWidget);
  if (d->TextEdit->wordWrapMode() == QTextOption::NoWrap)
    {
    return false;
    }
  return true;
}

//------------------------------------------------------------------------------
void qMRMLTextWidget::setWordWrap(bool wordWrap)
{
  Q_D(qMRMLTextWidget);
  d->TextEdit->setWordWrapMode(wordWrap ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
}

//------------------------------------------------------------------------------
void qMRMLTextWidget::onTextEditChanged()
{
  Q_D(qMRMLTextWidget);

  d->TextEditModified = true;
  if (!d->CurrentTextNode || !this->AutoSave)
    {
    return;
    }

  this->updateMRMLFromWidgetRequested();
}

//------------------------------------------------------------------------------
void qMRMLTextWidget::startEdits()
{
  Q_D(qMRMLTextWidget);
  d->TextEditModified = false;
  d->TextEdit->setFocus();
  d->setEditing(true);
  this->updateWidgetFromMRMLRequested();
}

//------------------------------------------------------------------------------
void qMRMLTextWidget::cancelEdits()
{
  Q_D(qMRMLTextWidget);
  d->setEditing(false);
  this->updateWidgetFromMRMLRequested();
}

//------------------------------------------------------------------------------
void qMRMLTextWidget::saveEdits()
{
  Q_D(qMRMLTextWidget);
  this->updateMRMLFromWidgetRequested();
  d->setEditing(false);
  this->updateWidgetFromMRMLRequested();
}

//------------------------------------------------------------------------------
QTextEdit* qMRMLTextWidget::textEditWidget()
{
  Q_D(qMRMLTextWidget);
  return d->TextEdit;
}
