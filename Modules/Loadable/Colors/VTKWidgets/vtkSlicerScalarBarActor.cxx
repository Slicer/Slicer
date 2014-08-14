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
#if (VTK_MAJOR_VERSION <= 5)
#else
#include <vtkScalarBarActorInternal.h>
#include <vtksys/RegularExpression.hxx>
#include <vtkTextActor.h>

#include <stdio.h> // for snprintf

#if defined(_WIN32) && !defined(__CYGWIN__)
#  define SNPRINTF _snprintf
#else
#  define SNPRINTF snprintf
#endif
#endif

vtkStandardNewMacro(vtkSlicerScalarBarActor);

//---------------------------------------------------------------------------
vtkSlicerScalarBarActor::vtkSlicerScalarBarActor()
{
#if (VTK_MAJOR_VERSION <= 5)
  this->ColorNames = NULL;
  vtkSmartPointer<vtkStringArray> colorNames = vtkSmartPointer<vtkStringArray>::New();
  this->SetColorNames(colorNames);
  this->UseColorNameAsLabel = 0;
#else
  this->Superclass::DrawAnnotationsOff();
  this->UseAnnotationAsLabel = 0;
#endif
}

//----------------------------------------------------------------------------
vtkSlicerScalarBarActor::~vtkSlicerScalarBarActor()
{
#if (VTK_MAJOR_VERSION <= 5)
  this->SetColorNames(NULL);
#endif
}

//----------------------------------------------------------------------------
void vtkSlicerScalarBarActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

#if (VTK_MAJOR_VERSION <= 5)
  os << indent << "UseColorNameAsLabel:   " << this->UseColorNameAsLabel << "\n";
#else
  os << indent << "UseAnnotationAsLabel:   " << this->UseAnnotationAsLabel << "\n";
#endif
}

#if (VTK_MAJOR_VERSION <= 5)
//---------------------------------------------------------------------------
int vtkSlicerScalarBarActor::SetColorName(int ind, const char *name)
{
  if (!this->LookupTable)
    {
    vtkWarningMacro(<<"Need a lookup table to render a scalar bar");
    return 0;
    }
  vtkLookupTable* lookupTable = vtkLookupTable::SafeDownCast(this->LookupTable);
  if (lookupTable)
    {
    if (lookupTable->GetNumberOfColors() != this->ColorNames->GetNumberOfValues())
      {
      this->ColorNames->SetNumberOfValues(lookupTable->GetNumberOfColors());
      }

    vtkStdString newName(name);
    if (this->ColorNames->GetValue(ind) != newName)
      {
      this->ColorNames->SetValue(ind, newName);
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkSlicerScalarBarActor::AllocateAndSizeLabels(int *labelSize,
                                              int *size,
                                              vtkViewport *viewport,
                                              double *range)
{
  labelSize[0] = labelSize[1] = 0;

  if (this->GetUseColorNameAsLabel() == 1)
    {
    this->NumberOfLabels = this->ColorNames->GetNumberOfValues();
    }

  this->TextMappers = new vtkTextMapper * [this->NumberOfLabels];
  this->TextActors = new vtkActor2D * [this->NumberOfLabels];

  char string[512];

  double val = 0.0; //TODO: Better variable name
  int i = 0;

  // TODO: this should be optimized, maybe by keeping a list of
  // allocated mappers, in order to avoid creation/destruction of
  // their underlying text properties (i.e. each time a mapper is
  // created, text properties are created and shallow-assigned a font size
  // which value might be "far" from the target font size).

  // is this a vtkLookupTable or a subclass of vtkLookupTable
  // with its scale set to log
  int isLogTable = this->LookupTable->UsingLogScale();

  for (i=0; i < this->NumberOfLabels; i++)
    {
    this->TextMappers[i] = vtkTextMapper::New();

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
        val = range[0] +
          static_cast<double>(i)/(this->NumberOfLabels-1)
          * (range[1]-range[0]);
        }
      else
        {
        val = range[0] + 0.5*(range[1]-range[0]);
        }
      }
    //
    if (this->GetUseColorNameAsLabel() == 1)
      {
      strcpy(string, this->ColorNames->GetValue(i).c_str());
      }
    else
      {
      sprintf(string, this->LabelFormat, val);
      }
    this->TextMappers[i]->SetInput(string);

    // Shallow copy here so that the size of the label prop is not affected
    // by the automatic adjustment of its text mapper's size (i.e. its
    // mapper's text property is identical except for the font size
    // which will be modified later). This allows text actors to
    // share the same text property, and in that case specifically allows
    // the title and label text prop to be the same.
    this->TextMappers[i]->GetTextProperty()->ShallowCopy(
      this->LabelTextProperty);

    this->TextActors[i] = vtkActor2D::New();
    this->TextActors[i]->SetMapper(this->TextMappers[i]);
    this->TextActors[i]->SetProperty(this->GetProperty());
    this->TextActors[i]->GetPositionCoordinate()->
      SetReferenceCoordinate(this->PositionCoordinate);
    }

  if (this->NumberOfLabels)
    {
    int targetWidth, targetHeight;

    if ( this->Orientation == VTK_ORIENT_VERTICAL )
      {
      targetWidth = static_cast<int>(0.6*size[0]);
      targetHeight = static_cast<int>(0.86*size[1]/this->NumberOfLabels);
      }
    else
      {
      targetWidth = static_cast<int>(size[0]*0.8/this->NumberOfLabels);
      targetHeight = static_cast<int>(0.25*size[1]);
      }

    vtkTextMapper::SetMultipleConstrainedFontSize(viewport,
                                                  targetWidth,
                                                  targetHeight,
                                                  this->TextMappers,
                                                  this->NumberOfLabels,
                                                  labelSize);
    }
}
#else
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
        val = range[0] +
          static_cast<double>(i)/(this->NumberOfLabels-1)
          * (range[1]-range[0]);
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
        double indx = 0.0;
        int index  = 0;
        if (this->NumberOfLabels > 1)
          {
          indx = static_cast<double>(i)/(this->NumberOfLabels-1)*(numberOfAnnotatedValues-1);
          }
        else
          {
          indx = 0.5*numberOfAnnotatedValues;
          }
        index = static_cast<int>(indx+0.5);
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
#endif
