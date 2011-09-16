// .NAME vtkImageGraph - Abstract Filter used in slicer to plot graphs
#ifndef __vtkImageGraph_h
#define __vtkImageGraph_h

#include "vtkImageSource.h"
#include "vtkEMSegmentStep.h"


// From vtkImagePlot
#define SET_PIXEL(x,y,color){ ptr  =&outPtr[y*NumXScalar+x*3]; memcpy(ptr,color,3);}


//BTX
class   VTK_EMSEGMENT_EXPORT GraphList {
public:
   vtkFloatingPointType* GetColor() {return this->Color;}
   void SetColor (vtkFloatingPointType value[3]) {memcpy(this->Color,value,sizeof(vtkFloatingPointType)*3);}

   void SetType(int val) {this->Type = val; }
   int GetType() {return this->Type; }

   int GetID() {return this->ID;}

  ~GraphList() {this->ID = -1;} 
   GraphList();

   vtkFloatingPointType Color[3];
   int ID;
   int Type;
}; 

// Description:
// Type of curve:
//  0 = curve representing contious data (e.g. /) 
//  1 = curve representing discrete data (e.g. _|) 
class  VTK_EMSEGMENT_EXPORT GraphEntryList : public GraphList {
public:
   vtkImageData* GetGraphEntry() {return this->GraphEntry;} 
   void SetGraphEntry(vtkImageData* value) {this->GraphEntry = value;} 

   GraphEntryList* GetNext() {return this->Next;} 

   void SetIgnoreGraphMinGraphMax(bool flag) {this->IgnoreGraphMinGraphMax = flag;} 
   bool GetIgnoreGraphMinGraphMax() {return this->IgnoreGraphMinGraphMax;}

   int AddEntry(vtkImageData* plot, vtkFloatingPointType col[3],int type, bool ignore);  
   int DeleteEntry(int delID);

   GraphEntryList* MatchGraphEntry(vtkImageData* value);

   int GetNumFollowingEntries();

   GraphEntryList();
   ~GraphEntryList();

protected:

   vtkImageData* GraphEntry;
   bool IgnoreGraphMinGraphMax;
   GraphEntryList* Next; 
};
//ETX
class vtkScalarsToColors;
class vtkIndirectLookupTable;
class vtkLookupTable;

class VTK_EMSEGMENT_EXPORT vtkImageGraph : public vtkImageSource
{
public:
  static vtkImageGraph *New();
  vtkTypeMacro(vtkImageGraph,vtkImageSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetMacro(Dimension, int);
  vtkGetMacro(Dimension, int);

  // Description:
  // Length of canvas/graph in X direction
  vtkSetMacro(Xlength, int);
  vtkGetMacro(Xlength, int);

  // Description:
  // Length of canvas/graph in Y direction
  vtkSetMacro(Ylength, int);
  vtkGetMacro(Ylength, int);

  // Description:
  // Global extrema over all the curves/regions 
  vtkGetMacro(GraphMax, vtkFloatingPointType);
  vtkGetMacro(GraphMin, vtkFloatingPointType);

  // Description:
  // Define the value range of the background (i.e. LookupTable)
  vtkSetVector2Macro(DataBackRange, int);
  vtkGetVector2Macro(DataBackRange, int);

  // Defines Background of Graph
  virtual void SetLookupTable(vtkScalarsToColors*);
  vtkGetObjectMacro(LookupTable,vtkScalarsToColors);
   
  // Description:
  // Thickness for the curve 
  vtkSetMacro(CurveThickness, int);
  vtkGetMacro(CurveThickness, int);

  // Description: 
  // Ignore the GraphMin - GraphMax setting of the graph
  // This only makes sense in 1D when we plot two curves with independent output range
  // e.g. histogram and gaussian curve.
  void SetIgnoreGraphMinGraphMax(vtkImageData *plot, int flag);
  int GetIgnoreGraphMinGraphMax(vtkImageData *plot);

  // Description: 
  // Set the color of a curve 
  void SetColor(vtkImageData *plot, vtkFloatingPointType color0, vtkFloatingPointType color1,vtkFloatingPointType color2);
  vtkFloatingPointType* GetColor(vtkImageData *plot);

  GraphEntryList* GetGraphList() { return &this->GraphList;}

  // Description:
  // Add a curve or region to the graph. It returns the ID of the data entry back
  int AddCurveRegion(vtkImageData *plot,vtkFloatingPointType color0,vtkFloatingPointType color1,vtkFloatingPointType color2, int type, int ignore);

  // Description:
  // Delete a curve/region from the list -> if successfull returns 1 - otherwise 0; 
  int DeleteCurveRegion(int id);

  // Description:
  // Creates a Indirect Lookup Table just with white color => needed to genereate a simple background  
  vtkIndirectLookupTable* CreateUniformIndirectLookupTable();

  // Description:
  // Creates a Lookup Table - if you want to change the color of the IndirectLookupTable
  // you first have to delete the old LookupTable and than recreate it with this function
  // and assign it again to the IndirectLookupTable
  vtkLookupTable* CreateLookupTable(vtkFloatingPointType SatMin, vtkFloatingPointType SatMax, vtkFloatingPointType ValMin, vtkFloatingPointType ValMax, vtkFloatingPointType HueMin, vtkFloatingPointType HueMax); 

  // Description:
  // Change the color of the Indirect Table 
  void ChangeColorOfIndirectLookupTable(vtkIndirectLookupTable* Table, vtkFloatingPointType SatMin, vtkFloatingPointType SatMax, vtkFloatingPointType ValMin, vtkFloatingPointType ValMax, vtkFloatingPointType HueMin, vtkFloatingPointType HueMax);

  unsigned long GetMTime();
protected:
  vtkImageGraph();
  void DeleteVariables();
  ~vtkImageGraph() {this->DeleteVariables();};
  virtual void ExecuteInformation();
  virtual void ExecuteData(vtkDataObject *data);

  void Draw1DGraph(vtkImageData *data);
  void Draw2DGraph(vtkImageData *data,int NumRegion, vtkFloatingPointType* CurveRegionMin, vtkFloatingPointType* CurveRegionMax);
  void CalculateGraphMinGraphMax (vtkFloatingPointType* CurveRegionMin, vtkFloatingPointType* CurveRegionMax);
  void DrawBackground(unsigned char *outPtr, int outIncY);

  int Dimension;
  int CurveThickness;
 
  int Xlength;
  int Ylength;

  vtkFloatingPointType GraphMin;
  vtkFloatingPointType GraphMax;

  GraphEntryList GraphList;
 
  vtkScalarsToColors *LookupTable;
  int DataBackRange[2];

private:
  vtkImageGraph(const vtkImageGraph&);  // Not implemented.
  void operator=(const vtkImageGraph&);  // Not implemented.
};

//----------------------------------------------------------------------------
// from vtkImagePlot
inline void ConvertColor(vtkFloatingPointType *f, unsigned char *c)
{
    c[0] = (int)(f[0] * 255.0);
    c[1] = (int)(f[1] * 255.0);
    c[2] = (int)(f[2] * 255.0);
}

//----------------------------------------------------------------------------
inline void DrawThickPoint (int Xpos, int Ypos, unsigned char color[3], unsigned char *outPtr, 
                int NumXScalar, int radius) {
  unsigned char *ptr;
  int x ,y;
  Xpos -= radius;
  for (x = -radius; x <= radius; x++) {
    for (y = -radius; y <= radius; y++) SET_PIXEL(Xpos, y+Ypos, color);
    Xpos ++;
  }
}

//----------------------------------------------------------------------------
inline void DrawContinousLine(int xx1, int yy1, int xx2, int yy2, unsigned char color[3],
                              unsigned char *outPtr, int NumXScalar, int radius) {
  vtkFloatingPointType slope; 
  int Yold = yy1, Ynew, x,y;

  if (xx2 != xx1)  {
    slope = vtkFloatingPointType(yy2 - yy1)/vtkFloatingPointType(xx2 - xx1);
    DrawThickPoint(xx1, yy1, color, outPtr, NumXScalar, radius);
    for(x=xx1+1; x <=  xx2; x++) { 
      Ynew = (int) slope*(x - xx1) + yy1; 
      if (slope < 0) for (y = Yold; y >= Ynew; y --) DrawThickPoint(x, y, color, outPtr, NumXScalar, radius); 
      else  for (y = Yold; y <= Ynew; y ++)  DrawThickPoint(x, y, color, outPtr, NumXScalar, radius);
      Yold = Ynew; 
    } 
  } else {
    if (yy1 > yy2) { 
      for (y = yy2; y <= yy1; y++) DrawThickPoint(xx1, y, color, outPtr, NumXScalar, radius); 
    } else for (y = yy1; y <= yy2; y++) DrawThickPoint(xx1, y, color, outPtr, NumXScalar, radius); 
  }
}
#endif

  
