/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageFillROI.cxx,v $
  Date:      $Date: 2006/04/13 19:28:29 $
  Version:   $Revision: 1.20 $

=========================================================================auto=*/
#include "vtkImageFillROI.h"

#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkPoints.h"

vtkCxxSetObjectMacro(vtkImageFillROI,Points,vtkPoints);

//------------------------------------------------------------------------------
vtkImageFillROI* vtkImageFillROI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageFillROI");
  if(ret)
    {
    return (vtkImageFillROI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageFillROI;
}


//----------------------------------------------------------------------------
vtkImageFillROI::vtkImageFillROI()
{
  this->Points = NULL;
  this->Value = 255;
  this->Shape = SHAPE_POLYGON;
  this->Radius = 0;
}

//----------------------------------------------------------------------------
vtkImageFillROI::~vtkImageFillROI()
{
  // We must UnRegister any object that has a vtkSetObjectMacro
  if (this->Points != NULL)
    {
    this->Points->UnRegister(this);
    }
}

//----------------------------------------------------------------------------
void vtkImageFillROI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Value: "  << this->Value;
  os << indent << "Radius: " << this->Radius;
  os << indent << "Shape: "  << this->Shape;

  // vtkSetObjectMacro
  os << indent << "Points: ";
  if (this->Points)
    {
    this->Points->PrintSelf(os << "\n" ,indent.GetNextIndent());
    }
  else
    {
    os << "(none)\n";
    }
}

//----------------------------------------------------------------------------
// small helper class to store edges for the edge table
class Edge
{
public:
  Edge() {this->next = NULL;};
  int yUpper;
  int dx, dy, dy2, dx2, dydx2, r, xInc, x;
  Edge *next;
};

// Inserts edge into list in order of increasing x field
void InsertEdge(Edge *list, Edge *edge)
{
  Edge *p, *q = list;

  p = q->next;
  while (p != NULL) 
    {
    if (edge->x < p->x)
      {
      p = NULL;
      }
    else 
      {
      q = p;
      p = p->next;
      }
    }
  edge->next = q->next;
  q->next = edge;
}

// Store lower-y coordinate and inverse slope for each slope.
// Adjust and store upper-y coordinate for edges that are the
// lower member of a monotonically increasing or decreasing
// pair of edges.
void MakeEdgeRec(int x1, int y1, int x2, int y2, Edge *edge, Edge *edges[])
{
  // p1 is lower than p2
  edge->dx = abs(x2 - x1);
  edge->dy = abs(y2 - y1);
  edge->dx2 = edge->dx << 1;
  edge->dy2 = edge->dy << 1;
  if (x1 < x2)
    {
    edge->xInc = 1;
    }
  else
    {
    edge->xInc = -1;
    }
  edge->x = x1;

  // < 45 degree slope
  if (edge->dy <= edge->dx)
    {
    edge->dydx2 = (edge->dy - edge->dx) << 1;
    edge->r = edge->dy2 - edge->dx;
    }
  // > 45 degree slope
  else
    {
    edge->dydx2 = (edge->dx - edge->dy) << 1;
    edge->r = edge->dx2 - edge->dy;
    }

  edge->yUpper = y2;

  InsertEdge(edges[y1], edge);
}

void BuildEdgeList(int nPts, int *xPts, int *yPts, Edge *edges[])
{
  Edge *edge;
  int i, x1, x2, y1, y2;

  x1 = xPts[nPts-1];
  y1 = yPts[nPts-1];

  for (i=0; i<nPts; i++) 
    {
    x2 = xPts[i];
    y2 = yPts[i];

    if (y1 != y2) 
      {
      // non-horizontal line
      edge = new Edge;
      if (y1 < y2)
        {
        // up-going edge
        MakeEdgeRec(x1, y1, x2, y2, edge, edges);
        }
      else
        {
        // down-going edge
        MakeEdgeRec(x2, y2, x1, y1, edge, edges);
        }
      }
    x1 = x2;
    y1 = y2;
    }
}

template <class T>
static void vtkImageFillROIDrawPolygon(int nx, int ny, int nPts, int *xPts, int *yPts, 
    T value, T *outPtr)
{
  int    i, scan, done;
  Edge *active, *p, *q, *del;
  int x, x1, x2;
  T *ptr;

  // Build a list of edges for each pixel in y direction.
  // The max possible number of edges is then the 
  // height of the image, so allocate this many edge pointers.
  Edge **edges = new Edge*[ny];

  for (i=0; i<ny; i++)
    {
    edges[i] = new Edge;
    }

  BuildEdgeList(nPts, xPts, yPts, edges);
  active = new Edge;

  for (scan=0; scan < ny; scan++) 
    {
    // BuildActiveList(int scan, Edge *active, Edge *edges[])
    p = edges[scan]->next;
    while (p) 
      {
      q = p->next;
      InsertEdge(active, p);
      p = q;
      }

    if (active->next) 
      {

      // DeleteFromActiveList(int scan, Edge *active) 
      q = active;
      p = active->next;
      while (p) 
        {
        if (scan >= p->yUpper) 
          {
          p = p->next;

          // Delete After q
          del = q->next;
          q->next = del->next;
          delete del;
          } 
        else 
          {
          q = p;
          p = p->next;
          }
        }

      // FillScan(int nx, int scan, Edge *active, T value, T *outPtr)
      p = active->next;
      ptr = &outPtr[scan*nx];
      while (p) 
        {
        q = p->next;
        if (!q) 
          {
          fprintf(stderr, "ODD FILL: y=%d\n", scan);
          break;
          }
        x1 = p->x;
        x2 = q->x;

        // Fill from left edge up to AND INCLUDING (x <= x2) right edge
        // (this differs from conventional polygon fill)
        for (x = x1; x < x2; x++)
          {
          ptr[x] = value;
          }
        p = q->next;
        }

      // UpdateActiveList(int scan, Edge *active)
      // Update 'x' field 
      q = active;
      p = active->next;
      while (p) 
        {
        // < 45 degree slope
        if (p->dy <= p->dx) 
          {
          done = 0;
          while (done == 0)
            {
            p->x += p->xInc;
            if (p->r <= 0)
              {
              p->r += p->dy2;
              }
            else 
              {
              done = 1;
              p->r += p->dydx2;
              }
            }
          }
        // > 45
        else 
          {
          if (p->r <= 0)
            {
            p->r += p->dx2;
            }
          else 
            {
            p->x += p->xInc;
            p->r += p->dydx2;
            }
          }
        q = p;
        p = p->next;
        }

      // ResortActiveList(Edge *active)
      p = active->next;
      active->next = NULL;
      while (p) 
        {
        q = p->next;
        InsertEdge(active, p);
        p = q;
        }
      }//if(active->next)
    }//for

  // Free edge records that have been allocated
  for (i=0; i<ny; i++)
    {
    delete edges[i];
    }
  delete active;

  // Free array of edge pointers.
  delete [] edges;
}

// This corresponds to "DrawLine" in vtkImageDrawROI.cxx. Both are
// used to draw the "Polygons" shape (before and after the shape is Applied).
template <class T>
static void DrawLinesFast(int nx, int ny, int nPts, int *xPts, int *yPts,
                          T value, T *outPtr)
{
  int i, x, y, x1, y1, x2, y2, xx1, xx2, yy1, yy2;
  int n = nPts;
  int dx, dy, dy2, dx2, r, dydx2, xInc;
  (void)ny; // unused ??

  for (i=0; i < n-1; i++)
    {
    xx1 = xPts[i];
    yy1 = yPts[i];
    xx2 = xPts[i+1];
    yy2 = yPts[i+1];

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

    // Draw first point with radius r
    outPtr[y*nx+x] = value;

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
            // Don't draw: don't want thick lines here
            y++;
            r += dydx2;
            }
          // Draw point with radius r
          outPtr[y*nx+x] = value;
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
            // Don't draw: don't want thick lines here
            y++;
            r += dydx2;
            }
          // Draw point with radius r
          outPtr[y*nx+x] = value;
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
          // Draw now only because it's thick

          x += xInc;
          r += dydx2;
          }
        // Draw point with radius r
        outPtr[y*nx+x] = value;
        }
      }
    }//for
}

// This corresponds to "DrawThickLine" in vtkImageDrawROI.cxx. Both are
// used to draw the "Lines" shape (before and after the lines are Applied).
template <class T>
static void DrawLines(int nx, int ny, int z, int radius,
                       int nPts, int *xPts, int *yPts,
                       T value, vtkImageData *outData)
{
  int i, x, y, x1, y1, x2, y2, xx1, xx2, yy1, yy2;
  int min0, max0, min1, max1, min2, max2;
  outData->GetExtent(min0, max0, min1, max1, min2, max2);
  T *outPtr;
  int n = nPts;
  int rad = radius;
  //int d = 2*rad+1;
  int dx, dy, dy2, dx2, r, dydx2;
  int xx, yy, xInc;
  (void)nx; (void)ny; // unused ??

  for (i=0; i < n-1; i++)
    {
    xx1 = xPts[i];
    yy1 = yPts[i];
    xx2 = xPts[i+1];
    yy2 = yPts[i+1];

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

    if (x1-rad >= min0 && x1+rad <= max0 &&
      y1-rad >= min1 && y2+rad <= max1 &&
      x2-rad >= min0 && x2+rad <= max0)
      {
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

      // Draw first point with radius r
      for (yy = y-rad; yy <= y+rad; yy++) 
        {
        outPtr = (T*)(outData->GetScalarPointer(x-rad, yy, z));
        for (xx = x-rad; xx <= x+rad; xx++) 
          {
          *outPtr = value;
          outPtr++;
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
              // Draw now only because it's thick
              for (yy = y-rad; yy <= y+rad; yy++) 
                {
                outPtr = (T*)(outData->GetScalarPointer(x-rad, yy, z));
                for (xx = x-rad; xx <= x+rad; xx++) 
                  {
                  *outPtr = value;
                  outPtr++;
                  }
                }
              y++;
              r += dydx2;
              }
            // Draw point with radius r
            for (yy = y-rad; yy <= y+rad; yy++) 
              {
              outPtr = (T*)(outData->GetScalarPointer(x-rad, yy, z));
              for (xx = x-rad; xx <= x+rad; xx++) 
                {
                *outPtr = value;
                outPtr++;
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
              // Draw now only because it's thick
              for (yy = y-rad; yy <= y+rad; yy++) 
                {
                outPtr = (T*)(outData->GetScalarPointer(x-rad, yy, z));
                for (xx = x-rad; xx <= x+rad; xx++) 
                  {
                  *outPtr = value;
                  outPtr++;
                  }
                }
              y++;
              r += dydx2;
              }
            // Draw point with radius r
            for (yy = y-rad; yy <= y+rad; yy++) 
              {
              outPtr = (T*)(outData->GetScalarPointer(x-rad, yy, z));
              for (xx = x-rad; xx <= x+rad; xx++) 
                {
                *outPtr = value;
                outPtr++;
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
            // Draw now only because it's thick

            x += xInc;
            r += dydx2;
            }
          // Draw point with radius r
          for (yy = y-rad; yy <= y+rad; yy++) 
            {
            outPtr = (T*)(outData->GetScalarPointer(x-rad, yy, z));
            for (xx = x-rad; xx <= x+rad; xx++) 
              {
              *outPtr = value;
              outPtr++;
              }
            }
          }
        }
      }
    }
}

template <class T>
static void DrawPoints(int nx, int ny, int z, int radius,
        int nPts, int *xPts, int *yPts,
        T value, vtkImageData *outData)
{
  int i, x, y, x1, y1;
  T *outPtr;
  int min0, max0, min1, max1, min2, max2;
  outData->GetExtent(min0, max0, min1, max1, min2, max2);
  int n = nPts;
  int r = radius;
  //int d = 2*r+1;
  (void)nx; (void)ny; // unused ??

  for (i=0; i < n; i++)
    {
    x1 = xPts[i];
    y1 = yPts[i];

    if (x1-r >= min0 && x1+r <= max0 &&
      y1-r >= min1 && y1+r <= max1)
      {
      for (y = y1-r; y <= y1+r; y++)
        {
        outPtr = (T*)(outData->GetScalarPointer(x1-r, y, z));
        for (x = x1-r; x <= x1+r; x++)
          {
          *outPtr = value;
          outPtr++;
          }
        }
      }
    }
}

template <class T>
static void vtkImageFillROIExecute(vtkImageFillROI* self,
                                   vtkImageData *outData, T* outPtr)
{
  T value = (T)(self->GetValue());
  int r = self->GetRadius();
  int i, j, x, y, z, nPts, nx, ny, outExt[6];
  int *xPts, *yPts;
  vtkFloatingPointType *pt;

  vtkPoints *points = self->GetPoints();
  if (points == NULL)
    return;

  outData->GetExtent(outExt);
  nx = outExt[1]-outExt[0]+1;
  ny = outExt[3]-outExt[2]+1;
  z = outExt[4];

  // Convert to int
  nPts = points->GetNumberOfPoints();
  if (nPts == 0) return;
  xPts = new int[nPts];
  yPts = new int[nPts];
  for (i=0,j=0; i<nPts; i++)
    {
    pt = points->GetPoint(i);
    x = (int)(pt[0]);
    y = (int)(pt[1]);
    if (x >= outExt[0] && x <= outExt[1] &&
      y >= outExt[2] && y <= outExt[3])
      {
      xPts[j] = x;
      yPts[j] = y;
      j++;
      }
    }
  nPts = j;

  outPtr = (T*)outData->GetScalarPointerForExtent(outExt);

  // zero out the background (added when filter switched from
  // in place to being image to image).
  for (int xx = 0; xx < nx; xx++)
    {
    for (int yy = 0; yy < ny; yy++)
      {
      outPtr[xx + nx*yy] = 0;
      }
    }

  switch (self->GetShape())
    {
    case SHAPE_POLYGON:
    if (nPts >= 3)
      {
      vtkImageFillROIDrawPolygon(nx, ny, nPts, xPts, yPts,
        (T)value, (T*)outPtr);
      // Draw lines too because polygons don't include top, right edges
      DrawLinesFast(nx, ny, nPts, xPts, yPts, value, (T*)outPtr);
      }
    break;

    case SHAPE_LINES:
    if (nPts >= 2)
      {
      DrawLines(nx, ny, z, r, nPts, xPts, yPts, value, outData);
      }
    break;

    case SHAPE_POINTS:
    if (nPts >= 1)
      {
      DrawPoints(nx, ny, z, r, nPts, xPts, yPts, value, outData);
      }
    break;
    }

  delete [] xPts;
  delete [] yPts;
}

//----------------------------------------------------------------------------
void vtkImageFillROI::ExecuteData(vtkDataObject *out)
{
  // Make sure the Input has been set.
  if ( this->GetInput() == NULL )
    {
    vtkErrorMacro(<< "ExecuteData: Input is not set.");
    return;
    }
    
  this->AllocateOutputData(out);

  if ( this->GetInput()->GetDataObjectType() != VTK_IMAGE_DATA )
    {
    vtkWarningMacro ("was sent non-image data data object");
    return;
    }

  vtkImageData *inData = (vtkImageData *) this->GetInput();

  void *ptr = NULL;
  int x1, *inExt;

  // ensure 1 component data
  x1 = inData->GetNumberOfScalarComponents();
  if (x1 != 1)
    {
    vtkErrorMacro("Input has "<<x1<<" components instead of 1.");
    return;
    }

  // Ensure intput is 2D
  inExt = inData->GetWholeExtent();
  if (inExt[5] != inExt[4])
    {
    vtkErrorMacro("Input must be 2D.");
    return;
    }

  switch (this->GetOutput()->GetScalarType())
    {
    vtkTemplateMacro( vtkImageFillROIExecute ( this, this->GetOutput(), static_cast<VTK_TT*>(ptr) ) );
    default: 
      {
      vtkErrorMacro(<< "Execute: Unknown ScalarType\n");
      return;
      }
    }
}

