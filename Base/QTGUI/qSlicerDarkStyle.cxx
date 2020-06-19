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
#include <QLinearGradient>
#include <QPalette>

// qMRML includes
#include "qSlicerDarkStyle.h"

// --------------------------------------------------------------------------
// qSlicerStyle methods

// --------------------------------------------------------------------------
qSlicerDarkStyle::qSlicerDarkStyle() = default;

// --------------------------------------------------------------------------
qSlicerDarkStyle::~qSlicerDarkStyle() = default;

//------------------------------------------------------------------------------
QPalette qSlicerDarkStyle::standardPalette()const
{
  QPalette palette = this->Superclass::standardPalette();

  // See https://doc.qt.io/qt-5/qpalette.html#ColorRole-enum
  // Central color roles
  palette.setColor(QPalette::Window, "#323232");
  palette.setColor(QPalette::WindowText, Qt::white);
  palette.setColor(QPalette::Disabled, QPalette::WindowText, "#6d6d6d");
  palette.setColor(QPalette::Base, "#1e1e1e");
  palette.setColor(QPalette::Disabled, QPalette::Base, "#1e1e1e");
  palette.setColor(QPalette::AlternateBase, "#323232");
  palette.setColor(QPalette::ToolTipBase, Qt::white);
  palette.setColor(QPalette::ToolTipText, "#323232");
  palette.setColor(QPalette::Text, Qt::white);
  palette.setColor(QPalette::Disabled, QPalette::Text, "#6d6d6d");
  palette.setColor(QPalette::Button, "#323232");
  palette.setColor(QPalette::ButtonText, Qt::white);
  palette.setColor(QPalette::Disabled, QPalette::ButtonText, "#b4b4b4");
  palette.setColor(QPalette::BrightText, Qt::red);
  // Color roles used mostly for 3D bevel and shadow effects.
  palette.setColor(QPalette::Light, "#828284");  // Lighter than Button color.
  palette.setColor(QPalette::Midlight, "#5a5a5b");  // Between Button and Light.
  palette.setColor(QPalette::Dark, "#232323");  // Darker than Button.
  palette.setColor(QPalette::Mid, "#2b2b2b");  // Between Button and Dark.
  palette.setColor(QPalette::Shadow, "#141414");  // A very dark color.
  // Color roles relate to selected (marked) items
  palette.setColor(QPalette::Highlight, "#3ca4ff");
  palette.setColor(QPalette::Disabled, QPalette::Highlight, "#505050");
  palette.setColor(QPalette::HighlightedText, Qt::white);
  palette.setColor(QPalette::Disabled, QPalette::HighlightedText, "#6d6d6d");
  // Color roles related to hyperlinks
  palette.setColor(QPalette::Link, "#3ca4ff");
  return palette;
}
