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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Segmentations includes
#include "qSlicerSegmentEditorEraseEffect.h"

//-----------------------------------------------------------------------------
class qSlicerSegmentEditorEraseEffectPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSegmentEditorEraseEffect);
protected:
  qSlicerSegmentEditorEraseEffect* const q_ptr;
public:
  qSlicerSegmentEditorEraseEffectPrivate(qSlicerSegmentEditorEraseEffect& object);
  ~qSlicerSegmentEditorEraseEffectPrivate() override;
public:
  QIcon EraseIcon;
};

//-----------------------------------------------------------------------------
qSlicerSegmentEditorEraseEffectPrivate::qSlicerSegmentEditorEraseEffectPrivate(qSlicerSegmentEditorEraseEffect& object)
  : q_ptr(&object)
{
  this->EraseIcon = QIcon(":Icons/Erase.png");
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorEraseEffectPrivate::~qSlicerSegmentEditorEraseEffectPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorEraseEffect methods

//----------------------------------------------------------------------------
qSlicerSegmentEditorEraseEffect::qSlicerSegmentEditorEraseEffect(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSegmentEditorEraseEffectPrivate(*this) )
{
  this->m_Name = QString("Erase");
  this->m_AlwaysErase = true;
  this->m_ShowEffectCursorInThreeDView = true;
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorEraseEffect::~qSlicerSegmentEditorEraseEffect() = default;

//---------------------------------------------------------------------------
QIcon qSlicerSegmentEditorEraseEffect::icon()
{
  Q_D(qSlicerSegmentEditorEraseEffect);

  return d->EraseIcon;
}

//---------------------------------------------------------------------------
QString const qSlicerSegmentEditorEraseEffect::helpText()const
{
  return "<html>Erase from current segment with a round brush<br>."
    "<p><ul style=\"margin: 0\">"
    "<li><b>Left-button drag-and-drop:</b> erase from segment around the mouse pointer.</li>"
    "<li><b>Shift + mouse wheel</b> or <b>+/- keys:</b> adjust brush size.</li>"
    "<li><b>Ctrl + mouse wheel:</b> slice view zoom in/out.</li>"
    "</ul><p>"
    "Editing is available both in slice and 3D views."
    "<p></html>";
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect* qSlicerSegmentEditorEraseEffect::clone()
{
  return new qSlicerSegmentEditorEraseEffect();
}
