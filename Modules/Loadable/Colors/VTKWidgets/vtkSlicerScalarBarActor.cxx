/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kevin Wang, Princess Margaret Cancer Centre
  and was supported by Cancer Care Ontario (CCO)'s ACRU program
  with funds provided by the Ontario Ministry of Health and Long-Term Care
  and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)

==============================================================================*/

// SlicerRt includes
#include "vtkSlicerScalarBarActor.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkScalarsToColors.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkViewport.h>
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>
#include <vtkScalarBarActorInternal.h>
#include <vtksys/RegularExpression.hxx>
#include <vtkTextActor.h>

#include <stdio.h> // for snprintf

#if defined(_WIN32) && !defined(__CYGWIN__)
#  define SNPRINTF _snprintf
#else
#  define SNPRINTF snprintf
#endif

vtkStandardNewMacro(vtkSlicerScalarBarActor);

//---------------------------------------------------------------------------
vtkSlicerScalarBarActor::vtkSlicerScalarBarActor()
{
  this->Superclass::DrawAnnotationsOff();
  this->UseAnnotationAsLabel = 0;
  this->CenterLabel = false;
}

//----------------------------------------------------------------------------
vtkSlicerScalarBarActor::~vtkSlicerScalarBarActor() = default;

//----------------------------------------------------------------------------
void vtkSlicerScalarBarActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "UseAnnotationAsLabel:   " << this->UseAnnotationAsLabel << "\n";
  os << indent << "CenterLabel:            " << this->CenterLabel << "\n";
}

//-----------------------------------------------------------------------------
void vtkSlicerScalarBarActor::LayoutTicks()
{
  if (this->LookupTable->GetIndexedLookup())
    { // no tick marks in indexed lookup mode.
    this->NumberOfLabelsBuilt = 0;
    return;
    }

  // find the best size for the ticks
  double* range = this->LookupTable->GetRange();

  // TODO: this should be optimized, maybe by keeping a list of
  // allocated mappers, in order to avoid creation/destruction of
  // their underlying text properties (i.e. each time a mapper is
  // created, text properties are created and shallow-assigned a font size
  // which value might be "far" from the target font size).
  this->Superclass::P->TextActors.resize(this->NumberOfLabels);

  // Does this map have its scale set to log?
  int isLogTable = this->LookupTable->UsingLogScale();

  // only print warning once in loop
  bool formatWarningPrinted = false;

  for (int i = 0; i < this->NumberOfLabels; i++)
    {
    this->P->TextActors[i].TakeReference(vtkTextActor::New());

    double val = 0.0;
    char labelString[512];
    // default in case of error with the annotations
    SNPRINTF(labelString, 511, "(none)");

    if ( isLogTable )
      {
      double lval;
      if (this->NumberOfLabels > 1)
        {
        lval = log10(range[0]) +
          static_cast<double>(i)/(this->NumberOfLabels-1) *
          (log10(range[1])-log10(range[0]));
        }
      else
        {
        lval = log10(range[0]) + 0.5*(log10(range[1])-log10(range[0]));
        }
      val = pow(10.0,lval);
      }
    else
      {
      if (this->NumberOfLabels > 1)
        {
        if (this->CenterLabel)
          {
          // labels are centered on the middle of a color swatch
          val = range[0] +
            (static_cast<double>(i) + 0.5) / this->NumberOfLabels
            * (range[1] - range[0]);
          }
        else
          {
          val = range[0] +
            static_cast<double>(i) / (this->NumberOfLabels - 1)
            * (range[1] - range[0]);
          }
        }
      else
        {
        val = range[0] + 0.5*(range[1]-range[0]);
        }
      }

    // if the lookuptable uses the new annotation functionality in VTK6.0
    // then use it as labels
    int numberOfAnnotatedValues = this->LookupTable->GetNumberOfAnnotatedValues();
    if (this->UseAnnotationAsLabel == 1)
      {
      if (numberOfAnnotatedValues > 1)
        {
        vtkIdType index = 0;
        if (this->NumberOfLabels > 1)
          {
          if (this->CenterLabel)
            {
            index = (static_cast<double>(i) + 0.5) / this->NumberOfLabels*numberOfAnnotatedValues;
            }
          else
            {
            index = static_cast<double>(i)/(this->NumberOfLabels-1)*(numberOfAnnotatedValues-1)+0.5;
            }
          if (index >= numberOfAnnotatedValues)
            {
            // make sure we do not attempt to use label index out of range
            // (this should not happen, but if it did then it would cause crash)
            index = numberOfAnnotatedValues;
            }
          }
        // try to make sure the label format supports a string
        // TODO issue 3802: replace with a more strict regular expression
        //
        vtksys::RegularExpression regExForString("%.*s");
        if (regExForString.find(this->LabelFormat))
          {
          SNPRINTF(labelString, 511, this->LabelFormat, this->LookupTable->GetAnnotation(index).c_str());
          }
        else
          {
          if (!formatWarningPrinted)
            {
            vtkWarningMacro("LabelFormat doesn't contain a string specifier!" << this->LabelFormat);
            formatWarningPrinted = true;
            }
          }
        }
      }
    else
      {
      // try to make sure the label format supports a floating point number
      // TODO issue 3802: replace with more strict regular expression
      vtksys::RegularExpression regExForDouble("%.*[fFgGeE]");
      if (regExForDouble.find(this->LabelFormat))
          {
          SNPRINTF(labelString, 511, this->LabelFormat, val);
          }
        else
          {
          if (!formatWarningPrinted)
            {
            vtkWarningMacro("LabelFormat doesn't contain a floating point specifier!" << this->LabelFormat);
            formatWarningPrinted = true;
            }
          }
      }
    this->P->TextActors[i]->SetInput(labelString);

    // Shallow copy here so that the size of the label prop is not affected
    // by the automatic adjustment of its text mapper's size (i.e. its
    // mapper's text property is identical except for the font size
    // which will be modified later). This allows text actors to
    // share the same text property, and in that case specifically allows
    // the title and label text prop to be the same.
    this->P->TextActors[i]->GetTextProperty()->ShallowCopy(
      this->LabelTextProperty);

    this->P->TextActors[i]->SetProperty(this->GetProperty());
    this->P->TextActors[i]->GetPositionCoordinate()->
      SetReferenceCoordinate(this->PositionCoordinate);
    }

  if (this->NumberOfLabels)
    {
    int labelSize[2];
    labelSize[0] = labelSize[1] = 0;
    int targetWidth, targetHeight;

    this->P->TickBox.Posn = this->P->ScalarBarBox.Posn;
    if ( this->Orientation == VTK_ORIENT_VERTICAL )
      { // NB. Size[0] = width, Size[1] = height
      // Ticks share the width with the scalar bar
      this->P->TickBox.Size[0] =
        this->P->Frame.Size[0] - this->P->ScalarBarBox.Size[0] -
        this->TextPad * 3;
      // Tick height could be adjusted if title text is
      // lowered by box constraints, but we won't bother:
      this->P->TickBox.Size[1] = this->P->Frame.Size[1] -
        this->P->TitleBox.Size[1] - 3 * this->TextPad -
        this->VerticalTitleSeparation;
      // Tick box height also reduced by NaN swatch size, if present:
      if (this->DrawNanAnnotation)
        {
        this->P->TickBox.Size[1] -=
          this->P->NanBox.Size[1] + this->P->SwatchPad;
        }

      if (this->TextPosition == vtkScalarBarActor::PrecedeScalarBar)
        {
        this->P->TickBox.Posn[0] = this->TextPad;
        }
      else
        {
        this->P->TickBox.Posn[0] += this->P->ScalarBarBox.Size[0] + 2 * this->TextPad;
        }

      targetWidth = this->P->TickBox.Size[0];
      targetHeight = static_cast<int>((this->P->TickBox.Size[1] -
          this->TextPad * (this->NumberOfLabels - 1)) /
        this->NumberOfLabels);
      }
    else
      { // NB. Size[1] = width, Size[0] = height
      // Ticks span the entire width of the frame
      this->P->TickBox.Size[1] = this->P->ScalarBarBox.Size[1];
      // Ticks share vertical space with title and scalar bar.
      this->P->TickBox.Size[0] =
        this->P->Frame.Size[0] - this->P->ScalarBarBox.Size[0] -
        4 * this->TextPad - this->P->TitleBox.Size[0];

      if (this->TextPosition == vtkScalarBarActor::PrecedeScalarBar)
        {
        this->P->TickBox.Posn[1] =
          this->P->TitleBox.Size[0] + 2 * this->TextPad;
        /* or equivalently: Posn[1] -=
          this->P->Frame.Size[0] -
          this->P->TitleBox.Size[0] - this->TextPad -
          this->P->ScalarBarBox.Size[0];
          */
        }
      else
        {
        this->P->TickBox.Posn[1] += this->P->ScalarBarBox.Size[0];
        }

      targetWidth = static_cast<int>((this->P->TickBox.Size[1] -
          this->TextPad * (this->NumberOfLabels - 1)) /
        this->NumberOfLabels);
      targetHeight = this->P->TickBox.Size[0];
      }

    vtkTextActor::SetMultipleConstrainedFontSize(
      this->P->Viewport, targetWidth, targetHeight,
      this->P->TextActors.PointerArray(), this->NumberOfLabels,
      labelSize);

    // Now adjust scalar bar size by the half-size of the first and last ticks
    this->P->ScalarBarBox.Posn[this->P->TL[1]]
      += labelSize[this->P->TL[1]] / 2.;
    this->P->ScalarBarBox.Size[1] -= labelSize[this->P->TL[1]];
    this->P->TickBox.Posn[this->P->TL[1]] += labelSize[this->P->TL[1]] / 2.;
    this->P->TickBox.Size[1] -= labelSize[this->P->TL[1]];

    if (this->Orientation == VTK_ORIENT_HORIZONTAL)
      {
      this->P->TickBox.Posn[1] += this->TextPad *
        (this->TextPosition == PrecedeScalarBar ? -1 : +1);
      this->P->TickBox.Size[1] -= this->TextPad;
      }
    }
  this->NumberOfLabelsBuilt = this->NumberOfLabels;
}

//-----------------------------------------------------------------------------
void vtkSlicerScalarBarActor::ConfigureTicks()
{
  double val;
  double sizeTextData[2];
  for (int i = 0; i < this->NumberOfLabelsBuilt; ++i)
  {

    if (this->CenterLabel)
    {
      // labels are centered on the middle of a color swatch
      val = (this->NumberOfLabelsBuilt > 1 ?
        (static_cast<double>(i) + 0.5) / this->NumberOfLabelsBuilt : 0.5) *
        this->P->TickBox.Size[1] + this->P->TickBox.Posn[this->P->TL[1]];
    }
    else
    {
      val = (this->NumberOfLabelsBuilt > 1 ?
        static_cast<double>(i) / (this->NumberOfLabelsBuilt - 1) : 0.5) *
        this->P->TickBox.Size[1] + this->P->TickBox.Posn[this->P->TL[1]];
    }
    this->P->TextActors[i]->GetSize(this->P->Viewport, sizeTextData);
    if (this->Orientation == VTK_ORIENT_VERTICAL)
    { // VERTICAL
      this->P->TextActors[i]->GetTextProperty()->SetJustification(
        this->TextPosition == PrecedeScalarBar ? VTK_TEXT_RIGHT : VTK_TEXT_LEFT);
      this->P->TextActors[i]->GetTextProperty()
        ->SetVerticalJustificationToBottom();
      this->P->TextActors[i]->SetPosition(
        this->TextPosition == vtkScalarBarActor::PrecedeScalarBar ?
        this->P->TickBox.Posn[0] + this->P->TickBox.Size[0] :
        this->P->TickBox.Posn[0],
        //this->P->TickBox.Posn[0],
        val - 0.5 * sizeTextData[1]);
    }
    else
    { // HORIZONTAL
      this->P->TextActors[i]->GetTextProperty()->SetJustificationToCentered();
      this->P->TextActors[i]->GetTextProperty()->SetVerticalJustification(
        this->TextPosition == PrecedeScalarBar ? VTK_TEXT_TOP : VTK_TEXT_BOTTOM);
      this->P->TextActors[i]->SetPosition(val,
        this->TextPosition == PrecedeScalarBar ?
        this->P->TickBox.Posn[1] + this->P->TickBox.Size[0] :
        this->P->TickBox.Posn[1]);
    }
  }
}
