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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QPalette>

// qMRML includes
#include "qSlicerDarkStyle.h"

// --------------------------------------------------------------------------
// qSlicerStyle methods

// --------------------------------------------------------------------------
qSlicerDarkStyle::qSlicerDarkStyle()
{
}

// --------------------------------------------------------------------------
qSlicerDarkStyle::~qSlicerDarkStyle()
{
}

//------------------------------------------------------------------------------
QPalette qSlicerDarkStyle::standardPalette()const
{
  QPalette palette = this->Superclass::standardPalette();

  // From darkest to lightest
  palette.setColor(QPalette::Shadow, Qt::black);

  palette.setColor(QPalette::Window, "#2c2c2e");
  palette.setColor(QPalette::AlternateBase, "#2c2c2e");
  palette.setColor(QPalette::ToolTipBase, "#2c2c2e");
  palette.setColor(QPalette::Dark, "#2c2c2e");

  palette.setColor(QPalette::Mid, "#363638");

  palette.setColor(QPalette::Button, "#454545");

  palette.setColor(QPalette::Midlight, "#626262");

  palette.setColor(QPalette::Base, "#767676");
  palette.setColor(QPalette::Light, "#767676");

  palette.setColor(QPalette::WindowText, "#e2e2e2");
  palette.setColor(QPalette::Text, "#e2e2e2");
  palette.setColor(QPalette::ToolTipText, "#e2e2e2");
  palette.setColor(QPalette::ButtonText, "#e2e2e2");

  palette.setColor(QPalette::BrightText, Qt::white);
  return palette;
}
