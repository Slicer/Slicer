/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageGraph.cxx,v $
  Date:      $Date: 2006/02/23 01:43:33 $
  Version:   $Revision: 1.12 $

=========================================================================auto=*/
#include "vtkImageGraph.h"
#include "vtkSlicerApplication.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkScalarsToColors.h"
#include "vtkLookupTable.h"
#include "vtkImageData.h"
#include "vtkIndirectLookupTable.h"

#include <stdlib.h>

// vtkStandardNewMacro(vtkImageGraph);
vtkCxxSetObjectMacro(vtkImageGraph,LookupTable,vtkScalarsToColors);

//------------------------------------------------------------------------------
GraphList::GraphList() {
  memset(this->Color,0,sizeof(vtkFloatingPointType)*3); 
  this->ID             = -1;
  this->Type           = 0;
}

//------------------------------------------------------------------------------
GraphEntryList::GraphEntryList() {
  this->GraphEntry     = NULL; 
  this->Next           = NULL; 
  this->IgnoreGraphMinGraphMax = false;
}

//------------------------------------------------------------------------------
GraphEntryList::~GraphEntryList() {
   this->GraphEntry = NULL; 
   if (this->Next != NULL) delete this->Next;
} 

//------------------------------------------------------------------------------
int GraphEntryList::AddEntry(vtkImageData* plot, vtkFloatingPointType col[3],int type, bool ignore) {
  // First entry
  if (this->ID == -1) {
    this->GraphEntry = plot;
    memcpy(this->Color,col,sizeof(vtkFloatingPointType)*3);
    this->ID             = 0;
    this->Type           = type;
    this->IgnoreGraphMinGraphMax = ignore;
    return this->ID;
  } 
  
  GraphEntryList* ListPtr = this;   
  // Make sure you do not enter the same graph twice
  while ((ListPtr->Next != NULL) && (ListPtr->GraphEntry != plot) )  ListPtr = ListPtr->Next;
  if (ListPtr->Next == NULL) {
    ListPtr->Next = new GraphEntryList;
    ListPtr->Next->ID = ListPtr->ID + 1;
    ListPtr = ListPtr->Next;
    ListPtr->GraphEntry = plot;
    memcpy(ListPtr->Color,col,sizeof(vtkFloatingPointType)*3);
    ListPtr->Type = type;  
    ListPtr->IgnoreGraphMinGraphMax = ignore;
  } else {
    memcpy(ListPtr->Color,col,3*sizeof(vtkFloatingPointType));
    ListPtr->Type = type;
    ListPtr->IgnoreGraphMinGraphMax = ignore;
  }
  return ListPtr->ID;
}

//------------------------------------------------------------------------------
int GraphEntryList::DeleteEntry(int delID) {
  GraphEntryList* ListPtr = this;
  GraphEntryList* PrevListPtr = NULL;
  
  while ((ListPtr->ID < delID) && (ListPtr->Next != NULL))  {
    PrevListPtr = ListPtr; 
    ListPtr = ListPtr->Next;
  }
  if ((ListPtr->ID != delID) || (ListPtr->ID < 0)) return 0;
  if (PrevListPtr ==  NULL) {
    // Very first Graph Entry 
    if (ListPtr->Next == NULL) {
      // No more entries
      ListPtr->ID = -1;     
      ListPtr->GraphEntry = NULL;
      memset(ListPtr->Color,0,sizeof(vtkFloatingPointType)*3);
      ListPtr->Type = 0;  
    } else {
      // Copy the second entry on the first position
      ListPtr->ID                     =  ListPtr->Next->ID;
      ListPtr->GraphEntry             =  ListPtr->Next->GraphEntry;
      memcpy(ListPtr->Color,ListPtr->Next->Color,sizeof(vtkFloatingPointType)*3);
      ListPtr->Type                   =  ListPtr->Next->Type;  
      ListPtr->IgnoreGraphMinGraphMax =  ListPtr->Next->IgnoreGraphMinGraphMax;
      PrevListPtr   =  ListPtr->Next;
      ListPtr->Next                   =  ListPtr->Next->Next;

      // Delete Second entry
      PrevListPtr->Next = NULL;
      delete PrevListPtr;
    }
    return 1;
  } 
  PrevListPtr->Next = ListPtr->Next;
  ListPtr->Next = NULL;
  delete ListPtr;
  return 1; 
}

//------------------------------------------------------------------------------
GraphEntryList* GraphEntryList::MatchGraphEntry(vtkImageData* value) {
  if (this->GraphEntry == value) return this;
  if (this->Next != NULL) return this->Next->MatchGraphEntry(value);
  return NULL;
}

//------------------------------------------------------------------------------
int GraphEntryList::GetNumFollowingEntries() {
  if (this->Next != NULL) return this->Next->GetNumFollowingEntries()+1;
  return 0;
}

//------------------------------------------------------------------------------
vtkImageGraph* vtkImageGraph::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageGraph");
  if(ret)
    {
    return (vtkImageGraph*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageGraph;
}


//----------------------------------------------------------------------------
vtkImageGraph::vtkImageGraph()
{
  this->GraphMin = this->GraphMax = 0.0;
  this->Dimension = this->Xlength = this->Ylength =  this->CurveThickness = 0;

  // Background range
  this->DataBackRange[0] = this->DataBackRange[1] = 0;

  this->LookupTable   = NULL;
}

//----------------------------------------------------------------------------
void vtkImageGraph::DeleteVariables() {
  if (this->LookupTable != NULL)  {
    this->LookupTable->UnRegister(this);
  }
}

//----------------------------------------------------------------------------
unsigned long vtkImageGraph::GetMTime() {
  unsigned long t1, t2;
  GraphEntryList* ListPtr = &(this->GraphList);

  t1 = this->vtkImageSource::GetMTime();

  if (this->LookupTable) {
    t2 = this->LookupTable->GetMTime();
    if (t2 > t1) t1 = t2;
  }

  if (ListPtr->GetGraphEntry() != NULL) {
    while (ListPtr != NULL) {
      t2 = ListPtr->GetGraphEntry()->GetMTime();
      if (t2 > t1) t1 = t2;
      ListPtr = ListPtr->GetNext();
    }
  }
  return t1;
}

//-----------------------------------------------------------------------------
int vtkImageGraph::AddCurveRegion(vtkImageData *plot,vtkFloatingPointType color0,vtkFloatingPointType color1,vtkFloatingPointType color2, int type, int ignore) {
  vtkFloatingPointType color[3];
  GraphEntryList* result;
  bool ignoreFlag = true;

  if (ignore == 0)
  {
      ignoreFlag = false;
  }
  result = this->GraphList.MatchGraphEntry(plot);
  // Plot does not exist in graph => Add it to it 
  if (result == NULL) {
    this->Modified();  
    color[0] = color0; color[1] = color1; color[2] = color2;
    return GraphList.AddEntry(plot,color,type, ignoreFlag);  
  }
  // Plot exists in graph => Check if we have to update values 
  memcpy(color,result->GetColor(),3*sizeof(vtkFloatingPointType)); 
  if ((color[0] != color0) || (color[1] != color1) || (color[2] != color2)) {
    color[0] = color0; color[1] = color1; color[2] = color2;
    result->SetColor(color);
    this->Modified();  
  } 

  if (type != result->GetType()) {
    result->SetType(type);
    this->Modified();  
  }
  if (ignoreFlag != result->GetIgnoreGraphMinGraphMax()) {
    result->SetIgnoreGraphMinGraphMax(ignoreFlag);
    this->Modified();  
  }
  return result->GetID();
}

//-----------------------------------------------------------------------------
int vtkImageGraph::DeleteCurveRegion(int id) {
  int result = GraphList.DeleteEntry(id); 
  if (result) this->Modified();  
  return result;
}

//-----------------------------------------------------------------------------
void vtkImageGraph::SetIgnoreGraphMinGraphMax(vtkImageData *plot, int flag) {
  GraphEntryList* result = this->GraphList.MatchGraphEntry(plot);
  bool FlagBool = (flag != 0 ? true : false);
  if (result == NULL) {
    vtkErrorMacro("SetIgnoreGraphMinGraphMax: Curve/region could not be updated, bc it is not part of graph!");
    return;
  }
  if (result->GetIgnoreGraphMinGraphMax() != FlagBool) {
    result->SetIgnoreGraphMinGraphMax(FlagBool);
    this->Modified(); 
  }
}

//-----------------------------------------------------------------------------
int vtkImageGraph::GetIgnoreGraphMinGraphMax(vtkImageData *plot) {
  GraphEntryList* result = this->GraphList.MatchGraphEntry(plot);
  if (result == NULL) {
    vtkErrorMacro("GetIgnoreGraphMinGraphMax: Curve/region could not be updated, bc it is not part of graph!");
    return -1;
  }
  return (int) result->GetIgnoreGraphMinGraphMax();
}

//-----------------------------------------------------------------------------
void vtkImageGraph::SetColor(vtkImageData *plot, vtkFloatingPointType color0, vtkFloatingPointType color1, vtkFloatingPointType color2) {
  vtkFloatingPointType color[3];
  GraphEntryList* result = this->GraphList.MatchGraphEntry(plot);
  // Plot does not exist in graph => Add it to it 
  if (result == NULL) {
    vtkErrorMacro("SetColor: Curve/region could not be updated, bc it is not part of graph!");
    return;
  }
  // Plot exists in graph => Check if we have to update values 
  memcpy(color,result->GetColor(),3*sizeof(vtkFloatingPointType)); 
  if ((color[0] != color0) || (color[1] != color1) || (color[2] != color2)) {
    color[0] = color0; color[1] = color1; color[2] = color2;
    result->SetColor(color);
    this->Modified();  
  } 
}

//-----------------------------------------------------------------------------
vtkFloatingPointType* vtkImageGraph::GetColor(vtkImageData *plot) {
  GraphEntryList* result = this->GraphList.MatchGraphEntry(plot);
  if (result == NULL) {
    vtkErrorMacro("GetColor: Curve/region could not be updated, bc it is not part of graph!");
    return NULL;
  }
  return result->GetColor();
}

//----------------------------------------------------------------------------
vtkIndirectLookupTable* vtkImageGraph::CreateUniformIndirectLookupTable() {
  vtkIndirectLookupTable* Table = vtkIndirectLookupTable::New();

  Table->SetWindow(1);
  Table->SetLevel(0);
  Table->SetLowerThreshold(0); 
  Table->SetUpperThreshold(0);
  Table->SetApplyThreshold(0); 
        
  Table->SetLookupTable(this->CreateLookupTable(0, 0, 0.9, 0.9, 0.0, 0.0)); 
  Table->Build();
  return Table;
}
//----------------------------------------------------------------------------
// Everytime you want to change the color of the graph you have tp destroy the old lookup table in the indirectlookuptable and assign a new 
// one . Update does not work properly here 
vtkLookupTable* vtkImageGraph::CreateLookupTable(vtkFloatingPointType SatMin, vtkFloatingPointType SatMax, vtkFloatingPointType ValMin, vtkFloatingPointType ValMax, vtkFloatingPointType HueMin, vtkFloatingPointType HueMax) {
  vtkFloatingPointType blub[2];
  vtkLookupTable* Look = vtkLookupTable::New();
  blub[0] = SatMin;
  blub[1] = SatMax;
  Look->SetSaturationRange(blub);
  // If you want to have it grey or so change it here  Look->SetValueRange(blub);
  blub[0] = ValMin;
  blub[1] = ValMax; 
  Look->SetValueRange(blub);
  blub[0] = HueMin;
  blub[1] = HueMax;
  Look->SetHueRange(blub);
  Look->Build();    
  return Look;
}

//----------------------------------------------------------------------------
void vtkImageGraph::ChangeColorOfIndirectLookupTable(vtkIndirectLookupTable* Table, vtkFloatingPointType SatMin, vtkFloatingPointType SatMax, vtkFloatingPointType ValMin, vtkFloatingPointType ValMax, vtkFloatingPointType HueMin, vtkFloatingPointType HueMax) {
  Table->GetLookupTable()->Delete();
  Table->SetLookupTable(this->CreateLookupTable(SatMin, SatMax, ValMin, ValMax, HueMin, HueMax));
  Table->Build();
}
//----------------------------------------------------------------------------
void vtkImageGraph::ExecuteInformation()
{
  vtkImageData *output = this->GetOutput();
  int Extent[6];
  Extent[0] = Extent[2] = Extent[4] = Extent[5] = 0;
  Extent[1] = this->Xlength - 1; 
  Extent[3] = this->Ylength - 1;

  output->SetWholeExtent(Extent);
  output->SetScalarType(VTK_UNSIGNED_CHAR);
  output->SetNumberOfScalarComponents(3);
}
//----------------------------------------------------------------------------
template <class Tin> 
static void vtkImageGraphGetMinMax(Tin *Ptr, int ext[6], int incY, int incZ, vtkFloatingPointType &Min, vtkFloatingPointType &Max) {
  int x,y,z;
  int maxZ = ext[5] - ext[4] + 1;
  int maxY = ext[3] - ext[2] + 1;
  int maxX = ext[1] - ext[0] + 1;

  Min = Max = (vtkFloatingPointType) *Ptr;
  for (z = 0; z < maxZ; z++) {
    for (y = 0; y < maxY; y++) {
      for (x = 0; x < maxX; x++) {
    if (Max < vtkFloatingPointType(*Ptr)) Max = (vtkFloatingPointType) *Ptr;
    else if (Min > vtkFloatingPointType(*Ptr)) Min = (vtkFloatingPointType) *Ptr;
    Ptr++;
      }
      Ptr += incY;
    }
    Ptr += incZ;
  }
}

//----------------------------------------------------------------------------
// This is stolen from vtkImagePlot
// Kilian- I generated basically a new version of this filter allowing multiple curves to be represented 
// Draw line including first, but not second end point
// Discrete , bc this line only makes sense if you have discrete data => '_|' instead of '/'
static void DrawDiscreteLine(int xx1, int yy1, int xx2, int yy2, unsigned char color[3],
                             unsigned char *outPtr, int NumXScalar, int Xlength, int radius)
{
    unsigned char *ptr;
    int r, dx, dy, dy2, dx2, dydx2;
    int x, y, xInc;
    int x1, y1, x2, y2;
    int rad=radius, rx1, rx2, ry1, ry2, rx, ry;

    // Sort points so x1,y1 is below x2,y2
    if (yy1 <= yy2) 
  {
        x1 = xx1;
        y1 = yy1;
        x2 = xx2;
        y2 = yy2;
    } 
  else 
  {
        x1 = xx2;
        y1 = yy2;
        x2 = xx1;
        y2 = yy1;
    }
    dx = abs(x2 - x1);
    dy = abs(y2 - y1);
    dx2 = dx << 1;
    dy2 = dy << 1;
    if (x1 < x2)
  {
        xInc = 1;
  }
    else
  {
        xInc = -1;
  }
    x = x1;
    y = y1;

    // Draw first point
    rx1 = x - rad; ry1 = y - rad;
    rx2 = x + rad; ry2 = y + rad;
  for (ry=ry1; ry <= ry2; ry++)
  {
        for (rx=rx1; rx <= rx2; rx++)
    {
            SET_PIXEL(rx, ry, color);
    }
  }

    // < 45 degree slope
    if (dy <= dx)
    {
        dydx2 = (dy-dx) << 1;
        r = dy2 - dx;

        // Draw up to (not including) end point
        if (x1 < x2)
        {
            while (x < x2)
            {
                x += xInc;
                if (r <= 0)
        {
                    r += dy2;
        }
                else 
        {
                    // Draw here for a thick line
                    rx1 = x - rad; ry1 = y - rad;
                    rx2 = x + rad; ry2 = y + rad;
                    for (ry=ry1; ry <= ry2; ry++)
          {
                        for (rx=rx1; rx <= rx2; rx++)
            {
                            SET_PIXEL(rx, ry, color);
            }
          }
                    y++;
                    r += dydx2;
                }
                rx1 = x - rad; ry1 = y - rad;
                rx2 = x + rad; ry2 = y + rad;
                for (ry=ry1; ry <= ry2; ry++)
        {
                    for (rx=rx1; rx <= rx2; rx++)
          {
                        SET_PIXEL(rx, ry, color);
          }
        }
            }
        }
        else
        {
            while (x > x2)
            {
                x += xInc;
                if (r <= 0)
        {
                    r += dy2;
        }
                else 
        {
                    // Draw here for a thick line
                    rx1 = x - rad; ry1 = y - rad;
                    rx2 = x + rad; ry2 = y + rad;
                    for (ry=ry1; ry <= ry2; ry++)
          {
                        for (rx=rx1; rx <= rx2; rx++)
            {
                            SET_PIXEL(rx, ry, color);
            }
          }
                    y++;
                    r += dydx2;
                }
                rx1 = x - rad; ry1 = y - rad;
                rx2 = x + rad; ry2 = y + rad;
                for (ry=ry1; ry <= ry2; ry++)
        {
                    for (rx=rx1; rx <= rx2; rx++)
          {
                        SET_PIXEL(rx, ry, color);
          }
        }
            }
        }
    }

    // > 45 degree slope
    else
    {
        dydx2 = (dx-dy) << 1;
        r = dx2 - dy;

        // Draw up to (not including) end point
        while (y < y2)
        {
            y++;
            if (r <= 0)
      {
                r += dx2;
      }
            else 
      {
                // Draw here for a thick line
                rx1 = x - rad; ry1 = y - rad;
                rx2 = x + rad; ry2 = y + rad;
                for (ry=ry1; ry <= ry2; ry++)
        {
                    for (rx=rx1; rx <= rx2; rx++)
          {
                        SET_PIXEL(rx, ry, color);
          }
        }
                x += xInc;
                r += dydx2;
            }
            rx1 = x - rad; ry1 = y - rad;
            rx2 = x + rad; ry2 = y + rad;
            for (ry=ry1; ry <= ry2; ry++)
      {
                for (rx=rx1; rx <= rx2; rx++)
        {
                    SET_PIXEL(rx, ry, color);
        }
      }
        }
    }
}

//----------------------------------------------------------------------------
// Draw Curve 
template <class Tin> 
static void vtkImageGraphDrawCurve(vtkImageGraph *self,Tin *CurvePtr,int outIncY,unsigned char color[3],int type, vtkFloatingPointType GraphMin, vtkFloatingPointType GraphMax, unsigned char *outPtr) {
  vtkFloatingPointType         ForUnits;
  int           Xlength         = self->GetXlength();
  int           Ylength         = self->GetYlength();
  int           CurveThickness  = self->GetCurveThickness();
  int           y1, y2, idxX, Xborder, Yborder;
  int           NumXScalar      = Xlength*3 +outIncY;

  if (GraphMin == GraphMax) ForUnits = 1.0;
  else ForUnits = (vtkFloatingPointType)(Ylength - 1) / (GraphMax - GraphMin);
  Yborder = Ylength - CurveThickness - 1;
  Xborder = Xlength - CurveThickness - 1;
  for (idxX = 0; idxX < Xlength; idxX++) {
    y1 = (int)(ForUnits * (vtkFloatingPointType)(CurvePtr[0] - GraphMin));
    y2 = (int)(ForUnits * (vtkFloatingPointType)(CurvePtr[1] - GraphMin));
    // Clip at boundary
    if (y1 < CurveThickness) { y1 = CurveThickness;
    } else if (y1 > Yborder) y1 = Yborder; 
    
    if (y2 < CurveThickness) {y2 = CurveThickness;
    } else if (y2 > Yborder)  y2 = Yborder ;
    if (idxX >= CurveThickness && idxX < Xborder) {
      if (type) DrawDiscreteLine(idxX, y1, idxX+1, y2, color, outPtr, NumXScalar, Xlength, CurveThickness);
      else DrawContinousLine(idxX, y1, idxX+1, y2, color, outPtr, NumXScalar, CurveThickness);
    }
    CurvePtr++;
  }
}

//----------------------------------------------------------------------------
// Draw Region 
template <class Tin> 
static void vtkImageGraphDrawRegion(vtkImageGraph *self,Tin **RegionPtr,int *RegionIncY, unsigned char **RegionColor, vtkFloatingPointType* CurveRegionMin, vtkFloatingPointType* CurveRegionMax, int NumRegion, unsigned char *outPtr, int outIncY) {
  
  int            idxX, idxY, idxR, Index,idxC;
  int            Xlength        = self->GetXlength();
  int            Ylength        = self->GetYlength();
  vtkFloatingPointType          Max;
  vtkFloatingPointType          ColorRatio;
  unsigned char  FinalColor[3];
  vtkFloatingPointType          *RegionDif= new vtkFloatingPointType[NumRegion]; 
  // GraphEntryList *ListPtr        = self->GetGraphList();

  for (idxR = 0; idxR < NumRegion; idxR++) {
    if (CurveRegionMax[idxR] > CurveRegionMin[idxR] ) RegionDif[idxR] =  2*(CurveRegionMax[idxR] - CurveRegionMin[idxR]);
    else RegionDif[idxR] =1.0;
  }  

  for (idxY = 0; idxY < Ylength; idxY++) {
    for (idxX = 0; idxX < Xlength; idxX++) {
      Max   = *RegionPtr[0];
      Index = 0;
      RegionPtr[0] ++;
      for (idxR = 1; idxR < NumRegion; idxR++) {
    if (*RegionPtr[idxR] > Max) {
      Max   = *RegionPtr[idxR]; 
      Index = idxR;
    }
    RegionPtr[idxR] ++;
      }

      // Makes output color dependent on value
      ColorRatio =  (Max - CurveRegionMin[Index]) / RegionDif[Index] + 0.5;
      for (idxC = 0; idxC < 3 ; idxC ++) FinalColor[idxC] = (unsigned char)(RegionColor[Index][idxC] * ColorRatio); 
      memcpy(outPtr,FinalColor,3);       
      outPtr += 3;
    }
    for (idxR = 0; idxR < NumRegion; idxR++) RegionPtr[idxR] += RegionIncY[idxR];
    outPtr += outIncY;
  }
  delete[] RegionDif; 
}

//----------------------------------------------------------------------------
void vtkImageGraph::Draw1DGraph(vtkImageData *data) {
  unsigned char       *outPtr;
  int                 *TempExt;
  vtkIdType TempIncX, TempIncY, TempIncZ;

  GraphEntryList      *ListPtr;
  vtkImageData        *TempCurve;
  unsigned char       color[3];
  int                 *outExt;
  vtkIdType                  outIncX, outIncY, outIncZ;
  vtkFloatingPointType               minY, maxY;

  // Get increments to march through data 
  outExt = data->GetExtent();
  data->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  outPtr = (unsigned char *) data->GetScalarPointer(outExt[0],outExt[2],outExt[4]);

  //-------------------------------------------------
  // 1. Step: Draw Background 
  //-------------------------------------------------
  this->DrawBackground(outPtr,outIncY);

  //-------------------------------------------------
  // 2. Step: Draw Curves / Region
  //-------------------------------------------------
  ListPtr = &(this->GraphList);
  TempCurve = ListPtr->GetGraphEntry();
  if (TempCurve == NULL) return;
  while (ListPtr != NULL) {
    TempCurve = ListPtr->GetGraphEntry(); 
    ConvertColor(ListPtr->GetColor(),color);
    TempExt = TempCurve->GetExtent();
    if (ListPtr->GetIgnoreGraphMinGraphMax() == false) {
      switch (TempCurve->GetScalarType()) {
    vtkTemplateMacro8(vtkImageGraphDrawCurve, this, (VTK_TT*) TempCurve->GetScalarPointerForExtent(TempExt), outIncY,
              color, ListPtr->GetType(), this->GraphMin,this->GraphMax, outPtr);
      default:
    vtkErrorMacro(<< "Execute: Unknown ScalarType");
    return;
      } 
    } else {
      TempCurve->GetContinuousIncrements(TempExt, TempIncX, TempIncY, TempIncZ);
      switch (TempCurve->GetScalarType()) {
    vtkTemplateMacro6(vtkImageGraphGetMinMax, (VTK_TT*) TempCurve->GetScalarPointerForExtent(TempExt), TempExt, TempIncY, TempIncZ, minY, maxY);
      default:
    vtkErrorMacro(<< "Execute: Unknown ScalarType");
    return;
      }
      switch (TempCurve->GetScalarType()) {
    vtkTemplateMacro8(vtkImageGraphDrawCurve, this, (VTK_TT*) TempCurve->GetScalarPointerForExtent(TempExt), outIncY,
              color, ListPtr->GetType(), minY,maxY, outPtr); 
      default:
    vtkErrorMacro(<< "Execute: Unknown ScalarType");
    return;
      }
    }
    ListPtr = ListPtr->GetNext();
  }
}

//---------------------------------------------------------------------------- 
void vtkImageGraph::Draw2DGraph(vtkImageData *data, int NumRegion, vtkFloatingPointType* CurveRegionMin, vtkFloatingPointType* CurveRegionMax) {
  int                 *outExt;
  vtkIdType                 outIncX, outIncY, outIncZ;
  unsigned char       *outPtr;

  GraphEntryList *ListPtr    = &(this->GraphList); 

  outExt = data->GetExtent();
  data->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  outPtr = (unsigned char *) data->GetScalarPointer(outExt[0],outExt[2],outExt[4]);

  if (NumRegion == 0) {  
    this->DrawBackground(outPtr,outIncY);
    return;
  }

  int            idxR;
  vtkIdType      TempIncX, TempIncY, TempIncZ;
  int *TempExt;
  vtkImageData   *TempCurve = 0;
  void           **RegionPtr = new void*[NumRegion];
  unsigned char  **RegionColor = new unsigned char*[NumRegion];
  int            *RegionIncY = new int[NumRegion];  

  for (idxR =0; idxR < NumRegion; idxR++) {
    RegionColor[idxR] = new unsigned char[3];
    ConvertColor(ListPtr->GetColor(),RegionColor[idxR]);

    TempCurve = ListPtr->GetGraphEntry(); 
    TempExt   = TempCurve->GetExtent();
    TempCurve->GetContinuousIncrements(TempExt, TempIncX, TempIncY, TempIncZ);
    RegionIncY[idxR] = (int) TempIncY;
    RegionPtr[idxR] = (void *)   TempCurve->GetScalarPointerForExtent(TempExt);

    ListPtr = ListPtr->GetNext();
  }
  switch (TempCurve->GetScalarType()) {
    vtkTemplateMacro9(vtkImageGraphDrawRegion,this,(VTK_TT**) RegionPtr,RegionIncY, RegionColor, CurveRegionMin, CurveRegionMax, NumRegion, outPtr, outIncY);
      default:
    vtkErrorMacro(<< "Execute: Unknown ScalarType");
    return;
  }
  for (idxR =0; idxR < NumRegion; idxR++) delete[] RegionColor[idxR];
  delete[] RegionColor;
  delete[] RegionPtr;
  delete[] RegionIncY;  
}

//----------------------------------------------------------------------------
// CurveRegionMin , CurveRegionMax are arrays defining the minimum maximum for every Curve or Region
void vtkImageGraph::CalculateGraphMinGraphMax (vtkFloatingPointType* CurveRegionMin, vtkFloatingPointType* CurveRegionMax) {
  GraphEntryList      *ListPtr = &(this->GraphList);
  vtkImageData        *TempCurve = ListPtr->GetGraphEntry();
  vtkIdType                  TempIncX, TempIncY, TempIncZ;
  int *TempExt;
  int                 index = 0 ;
  // If there is no curve defined we can skip the next section
  if (TempCurve != NULL) {
    // Initialize global min and max;
    if (this->Dimension < 2) { 
      while((ListPtr->GetIgnoreGraphMinGraphMax() == true) && (ListPtr->GetNext() != NULL)) {
        ListPtr = ListPtr->GetNext();
        index ++;
      }
    }
    if ((ListPtr->GetIgnoreGraphMinGraphMax() == false) || (this->Dimension > 1)){  
      TempCurve = ListPtr->GetGraphEntry();
      TempExt = TempCurve->GetExtent();
      TempCurve->GetContinuousIncrements(TempExt, TempIncX, TempIncY, TempIncZ);
      switch (TempCurve->GetScalarType()) {
        vtkTemplateMacro6(vtkImageGraphGetMinMax, (VTK_TT*) TempCurve->GetScalarPointerForExtent(TempExt), TempExt, TempIncY, TempIncZ,CurveRegionMin[index], CurveRegionMax[index]);
        default:
          vtkErrorMacro(<< "Execute: Unknown ScalarType");
          return;
      }
      this->GraphMin = CurveRegionMin[index];
      this->GraphMax = CurveRegionMax[index];
      while (ListPtr->GetNext() != NULL) {
        ListPtr = ListPtr->GetNext();
        index ++;
        if ((ListPtr->GetIgnoreGraphMinGraphMax() == false) || (this->Dimension > 1)) { 
          TempCurve = ListPtr->GetGraphEntry();
          TempExt = TempCurve->GetExtent();
          TempCurve->GetContinuousIncrements(TempExt, TempIncX, TempIncY, TempIncZ);
          switch (TempCurve->GetScalarType()) {
            vtkTemplateMacro6(vtkImageGraphGetMinMax,(VTK_TT*) TempCurve->GetScalarPointerForExtent(TempExt), TempExt, TempIncY, TempIncZ,CurveRegionMin[index], CurveRegionMax[index]);
          default:
            vtkErrorMacro(<< "Execute: Unknown ScalarType");
            return;
          }
          if (this->GraphMax < CurveRegionMax[index]) this->GraphMax = CurveRegionMax[index];
          if (this->GraphMin > CurveRegionMin[index]) this->GraphMin = CurveRegionMin[index];
        }
      }
    }
  }
}
//----------------------------------------------------------------------------
void vtkImageGraph::DrawBackground(unsigned char *outPtr, int outIncY) {
  vtkFloatingPointType          val,BackUnits; 
  int            idxX, idxY;
  int            SIZE_OF_CHAR    = 3*sizeof(unsigned char);
  int            SIZE_OF_XLENGTH = this->Xlength*3 *sizeof(unsigned char);
  int            NumXScalar = this->Xlength*3 + outIncY;
  unsigned char  *BackXAxis = new unsigned char[this->Xlength*3];
  unsigned char  *BackXAxisPtr = BackXAxis;
  
  if (this->Xlength > 1) {
    BackUnits = (vtkFloatingPointType)(this->DataBackRange[1] - this->DataBackRange[0]) / (vtkFloatingPointType)(this->Xlength - 1.0);
  } else {
    BackUnits = 0.0;
  }
  for (idxX = 0; idxX < this->Xlength; idxX++)  {
    val = (vtkFloatingPointType) this->DataBackRange[0] + BackUnits * idxX;
    memcpy(BackXAxisPtr, this->LookupTable->MapValue(val),SIZE_OF_CHAR);    
    BackXAxisPtr += 3;
  }
  for (idxY = 0; idxY < this->Ylength; idxY++) {
    memcpy(outPtr, BackXAxis,SIZE_OF_XLENGTH);
    outPtr += NumXScalar;
  }
  delete[] BackXAxis; 
}

//----------------------------------------------------------------------------
// modified version from vtkImagePlot
void vtkImageGraph::ExecuteData(vtkDataObject *output) {
  //-------------------------------------------------
  // 1. Step: Initialize and check
  //-------------------------------------------------
  vtkImageData        *data = this->AllocateOutputData(output);
  int                 *TempExt;
  GraphEntryList      *ListPtr;
  vtkImageData        *TempCurve;
  int                 ScalarType;
  vtkFloatingPointType               *CurveRegionMin,*CurveRegionMax;
  int                 NumRegion;

  if (data->GetScalarType() != VTK_UNSIGNED_CHAR) {
    vtkErrorMacro("Execute: This source only outputs unsigned char");
    return;
  }

  // Check if Data so it is up data 
  if (this->LookupTable == NULL) {
    vtkErrorMacro("Please define a lookuptable for the background !");
    return;
  }
 
  if (this->Xlength < 1) {
    vtkErrorMacro("Execute: Xlength has to be positive !");
    return;
  }
  if (this->Ylength < 1) {
    vtkErrorMacro("Execute: Ylength has to be positive !");
    return;
  }

  // Lets check dimension of input 
  ListPtr = &(this->GraphList);
  TempCurve = ListPtr->GetGraphEntry();
  NumRegion  =  ListPtr->GetNumFollowingEntries();

  if (TempCurve != NULL) {
    NumRegion++;
    CurveRegionMin = new vtkFloatingPointType[NumRegion];
    CurveRegionMax = new vtkFloatingPointType[NumRegion];
    
    ScalarType = TempCurve->GetScalarType();
    while (ListPtr != NULL) {
      TempCurve = ListPtr->GetGraphEntry();
      // Check Dimension of curve
      TempExt = TempCurve->GetExtent();
      if ((TempExt[1] - TempExt[0] + 1) != this->Xlength) {
    vtkErrorMacro("Execute: Length of all curves / regions is not compliant with Xlength setting (accepted: "<< this->Xlength << " given: " 
              << TempExt[1] - TempExt[0] + 1 << "!");
    return;
      }
      if (this->Dimension > 1) {
    if ((TempExt[3] - TempExt[2] + 1) != this->Ylength) {
      vtkErrorMacro("Execute: Width of a region (" << TempExt[3] - TempExt[2] + 1 << ") is not compliant with Ylength setting ("<<this->Ylength<<")!");
      return;
    }
    if (ScalarType != TempCurve->GetScalarType()) {
      vtkErrorMacro("Execute: Scalar Type of all the input regions have to be the same !");
      return;
    }
      } else {
    if ((TempExt[3] - TempExt[2]) != 0) {
      vtkErrorMacro("Execute: Not all Curves are 1D  !");
      return;
    } 
    // Check if they are of the same type
    
      }
      ListPtr = ListPtr->GetNext();
    }
  } else {
    CurveRegionMin = CurveRegionMax = NULL;
  }

  CalculateGraphMinGraphMax (CurveRegionMin,CurveRegionMax); 
  if (this->Dimension == 1) {
    this->Draw1DGraph(data);
  } else this->Draw2DGraph(data,NumRegion,CurveRegionMin, CurveRegionMax); 

  if (NumRegion) {
    delete[] CurveRegionMin;
    delete[] CurveRegionMax;
  }
}


void vtkImageGraph::PrintSelf(ostream& os, vtkIndent indent)
{
  GraphEntryList      *ListPtr;

  vtkImageSource::PrintSelf(os,indent);
  os << indent << "Dimension:      " << this->Dimension      << endl;
  os << indent << "CurveThickness: " << this->CurveThickness << endl;
  os << indent << "Xlength:        " << this->Xlength        << endl;
  os << indent << "GraphMin:       " << this->GraphMin       << endl;
  os << indent << "GraphMax:       " << this->GraphMax       << endl;
  os << indent << "Ylength:        " << this->Ylength        << endl;
  os << indent << "DataBackRange:  " << this->DataBackRange[0] << " " << this->DataBackRange[1] << endl;
  os << indent << "LookupTable:    ";
  if (this->LookupTable) {
    os << endl;
    this->LookupTable->PrintSelf(os,indent.GetNextIndent());
  }
  else  os << "(NULL)" << endl;
  ListPtr = &(this->GraphList);
  if (ListPtr->GetGraphEntry() != NULL) {
     os << indent << "Curves:  " << endl;
     indent = indent.GetNextIndent();
     while (ListPtr != NULL) {
       os << indent << "ID:                     " << ListPtr->GetID() << endl;
       os << indent << "Color:                  " << ListPtr->GetColor()[0] << " " <<  ListPtr->GetColor()[1] << " " <<  ListPtr->GetColor()[2] << endl;
       os << indent << "Type:                   " << ListPtr->GetType() << endl;
       os << indent << "IgnoreGraphMinGraphMax: " << ListPtr->GetIgnoreGraphMinGraphMax() << endl;
       os << indent << "GraphEntry:             " << ListPtr->GetGraphEntry() << endl;
       ListPtr = ListPtr->GetNext();
     }
  } else os << indent << "Curves:         (NULL)" << endl;
}




