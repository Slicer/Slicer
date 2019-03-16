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

#ifndef __qSlicerPresetComboBox_p_h
#define __qSlicerPresetComboBox_p_h

// CTK includes
#include <ctkComboBox.h>

// VolumeRendering Widgets includes
#include "qSlicerPresetComboBox.h"

//-----------------------------------------------------------------------------
class qSlicerPresetComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qSlicerPresetComboBox);
protected:
  qSlicerPresetComboBox* const q_ptr;

public:
  qSlicerPresetComboBoxPrivate(qSlicerPresetComboBox& object);
  void init();

public:
  bool ShowIcons;
};

//-----------------------------------------------------------------------------
class qSlicerIconComboBox : public ctkComboBox
{
  Q_OBJECT

public:
  /// Constructors
  typedef ctkComboBox Superclass;
  explicit qSlicerIconComboBox(QWidget* parent=nullptr);

public:
  void showPopup() override;

private:
  Q_DISABLE_COPY(qSlicerIconComboBox);
};

#endif
