/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageGCR.cxx,v $
  Date:      $Date: 2006/01/06 17:57:10 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
#include "vtkImageGCR.h"
#include "vtkImageExtractComponents.h"
#include "vtkImageHistogramNormalization.h"
#include "vtkImageShrink3D.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkImageChangeInformation.h" 

#include <cmath>
#include <iomanip>

using namespace std;

// #include <vtkStructuredPointsWriter.h>
// static void Write(vtkImageData* image,const char* filename){
//   vtkStructuredPointsWriter* writer = vtkStructuredPointsWriter::New();
//   writer->SetFileTypeToBinary();
//   writer->SetInput(image);
//   writer->SetFileName(filename);
//   writer->Write();
//   writer->Delete();
// }

#include "vtkNRRDWriter.h" 
static void VolumeWriter (vtkImageData *DATA, char* fileName) {
  char blub[1024];
  sprintf(blub,"/home/pohl/temp/3_%s.nhdr", fileName);
  cout << "Write " << blub << endl;  
  vtkNRRDWriter *iwriter =  vtkNRRDWriter::New();
  iwriter->SetInput(DATA);
  iwriter->SetFileName(blub);
  iwriter->Write();
  iwriter->Delete();
}


void  _Print(vtkImageData *DATA,::ostream& os, vtkIndent indent)  {
  // DATA->PrintSelf(os,indent.GetNextIndent());
  os << indent <<  "Origin " << DATA->GetOrigin()[0] << " "  <<  DATA->GetOrigin()[1] << " "  <<  DATA->GetOrigin()[2] << endl;
  os << indent <<  "Extent " << DATA->GetExtent()[0] << " "  <<  DATA->GetExtent()[1] << " "  <<  DATA->GetExtent()[2] << " " 
     << DATA->GetExtent()[3] << " "  <<  DATA->GetExtent()[4] << " "  <<  DATA->GetExtent()[5] << endl; 
  os << indent <<  "Spacing " << DATA->GetSpacing()[0] << " "  <<  DATA->GetSpacing()[1] << " "  <<  DATA->GetSpacing()[2] << endl;
}


float* vtkImageGCR::vector(int nl,int nh)
{
  float *v;

  v=(float *)malloc((unsigned) (nh-nl+1)*sizeof(float));
  if (!v)
    {
    vtkErrorMacro("allocation failure in vector()");
    return 0;
    }
  return v-nl;
}

float** vtkImageGCR::matrix(int nrl,int nrh,int ncl,int nch)
{
  int i;
  float **m;

  m=(float **) malloc((unsigned) (nrh-nrl+1)*sizeof(float*));
  if (!m)
    {
    vtkErrorMacro("allocation failure 1 in matrix()");
    return 0;
    }
  m -= nrl;

  for(i=nrl;i<=nrh;i++)
    {
    m[i]=(float *) malloc((unsigned) (nch-ncl+1)*sizeof(float));
    if (!m[i])
      {
      vtkErrorMacro("allocation failure 2 in matrix()");
      return 0;
      }
    m[i] -= ncl;
    }
  return m;
}

void vtkImageGCR::free_vector(float* v,int nl,int nh)
{
  free((char*) (v+nl));
}

void vtkImageGCR::free_matrix(float** m,int nrl,int nrh,int ncl,int nch)
{
  int i;

  for(i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
  free((char*) (m+nrl));
}

#define ITMAX 100
#define CGOLD 0.3819660
#define ZEPS 1.0e-10
#define SIGN(a,b) ((b) > 0.0 ? fabs(a) : -fabs(a))
#define SHFT(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);

float vtkImageGCR::brent(float ax,float bx,float cx,
             float (vtkImageGCR::*f)(float),
             float tol,float* xmin)
{
  int iter;
  float a,b,d=0,etemp,fu,fv,fw,fx,p,q,r,tol1,tol2,u,v,w,x,xm;
  float e=0.0;

  a=((ax < cx) ? ax : cx);
  b=((ax > cx) ? ax : cx);
  x=w=v=bx;
  fw=fv=fx=(this->*f)(x);
  for (iter=1;iter<=ITMAX;iter++)
    {
    xm=0.5*(a+b);
    tol2=2.0*(tol1=tol*fabs(x)+ZEPS);
    if (fabs(x-xm) <= (tol2-0.5*(b-a)))
      {
      *xmin=x;
      return fx;
      }
    if (fabs(e) > tol1)
      {
      r=(x-w)*(fx-fv);
      q=(x-v)*(fx-fw);
      p=(x-v)*q-(x-w)*r;
      q=2.0*(q-r);
      if (q > 0.0) p = -p;
      q=fabs(q);
      etemp=e;
      e=d;
      if (fabs(p) >= fabs(0.5*q*etemp) || p <= q*(a-x) || p >= q*(b-x))
    d=CGOLD*(e=(x >= xm ? a-x : b-x));
      else
    {
    d=p/q;
    u=x+d;
    if (u-a < tol2 || b-u < tol2)
      d=SIGN(tol1,xm-x);
    }
      }
    else
      {
      d=CGOLD*(e=(x >= xm ? a-x : b-x));
      }
    u=(fabs(d) >= tol1 ? x+d : x+SIGN(tol1,d));
    fu=(this->*f)(u);
    if (fu <= fx)
      {
      if (u >= x) a=x; else b=x;
      SHFT(v,w,x,u);
      SHFT(fv,fw,fx,fu);
      }
    else
      {
      if (u < x) a=u; else b=u;
      if (fu <= fw || w == x)
    {
    v=w;
    w=u;
    fv=fw;
    fw=fu;
    }
      else if (fu <= fv || v == x || v == w)
    {
    v=u;
    fv=fu;
    }
      }
    }
  vtkErrorMacro("Too many iterations in BRENT");
  return 0;
}

#undef ITMAX
#undef CGOLD
#undef ZEPS
#undef SIGN

float vtkImageGCR::f1dim(float x)
{
  int j;
  float f,*xt;

  xt=vector(1,ncom);
  for (j=1;j<=ncom;j++) xt[j]=pcom[j]+x*xicom[j];
  f=(this->*nrfunc)(xt);
  free_vector(xt,1,ncom);
  return f;
}

#define GOLD 1.618034
#define GLIMIT 100.0
#define TINY 1.0e-20
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define SIGN(a,b) ((b) > 0.0 ? fabs(a) : -fabs(a))
#define SHFT(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);

void vtkImageGCR::mnbrak(float* ax,float* bx,float* cx,float* fa,float* fb,float* fc, float (vtkImageGCR::*func)(float))
{
  float ulim,u,r,q,fu,dum;

  *fa=(this->*func)(*ax);
  *fb=(this->*func)(*bx);
  if (*fb > *fa)
    {
    SHFT(dum,*ax,*bx,dum);
    SHFT(dum,*fb,*fa,dum);
    }
  *cx=(*bx)+GOLD*(*bx-*ax);
  *fc=(this->*func)(*cx);
  while (*fb > *fc)
    {
    r=(*bx-*ax)*(*fb-*fc);
    q=(*bx-*cx)*(*fb-*fa);
    u=(*bx)-((*bx-*cx)*q-(*bx-*ax)*r)/
      (2.0*SIGN(MAX(fabs(q-r),TINY),q-r));
    ulim=(*bx)+GLIMIT*(*cx-*bx);
    if ((*bx-u)*(u-*cx) > 0.0)
      {
      fu=(this->*func)(u);
      if (fu < *fc)
    {
    *ax=(*bx);
    *bx=u;
    *fa=(*fb);
    *fb=fu;
    return;
    }
      else if (fu > *fb)
    {
    *cx=u;
    *fc=fu;
    return;
    }
      u=(*cx)+GOLD*(*cx-*bx);
      fu=(this->*func)(u);
      }
    else if ((*cx-u)*(u-ulim) > 0.0)
      {
      fu=(this->*func)(u);
      if (fu < *fc)
    {
    SHFT(*bx,*cx,u,*cx+GOLD*(*cx-*bx))
      SHFT(*fb,*fc,fu,(this->*func)(u))
      }
      }
    else if ((u-ulim)*(ulim-*cx) >= 0.0)
      {
      u=ulim;
      fu=(this->*func)(u);
      }
    else
      {
      u=(*cx)+GOLD*(*cx-*bx);
      fu=(this->*func)(u);
      }
    SHFT(*ax,*bx,*cx,u);
    SHFT(*fa,*fb,*fc,fu);
    }
}

#undef GOLD
#undef GLIMIT
#undef TINY
#undef MAX
#undef SIGN
#undef SHFT

#define TOL 10e-2

void vtkImageGCR::linmin(float* p,float* xi,int n,float* fret,
             float (vtkImageGCR::*func)(float*))
{
  int j;
  float xx,xmin,fx,fb,fa,bx,ax;

  ncom=n;
  pcom=vector(1,n);
  xicom=vector(1,n);
  nrfunc=func;
  for (j=1;j<=n;j++)
    {
    pcom[j]=p[j];
    xicom[j]=xi[j];
    }
  ax=0.0;
  xx=1.0;
  bx=2.0;
  mnbrak(&ax,&xx,&bx,&fa,&fx,&fb,&vtkImageGCR::f1dim);
  *fret=brent(ax,xx,bx,&vtkImageGCR::f1dim,TOL,&xmin);
  for (j=1;j<=n;j++)
    {
    xi[j] *= xmin;
    p[j] += xi[j];
    }
  free_vector(xicom,1,n);
  free_vector(pcom,1,n);
}

#undef TOL

// this should be in nr but it seg faults unless I copy it here
#define ITMAX 200
static float sqrarg;
#define SQR(a) (sqrarg=(a),sqrarg*sqrarg)
void vtkImageGCR::powell(float* p,float** xi,int n,
             float ftol,int* iter,
             float* fret,float (vtkImageGCR::*func)(float*))
{
  int i,ibig,j;
  float t,fptt,fp,del;
  float *pt,*ptt,*xit;

  pt=vector(1,n);
  ptt=vector(1,n);
  xit=vector(1,n);
  *fret=(this->*func)(p);
  for (j=1;j<=n;j++) pt[j]=p[j];
  for (*iter=1;;(*iter)++)
    {
    fp=(*fret);
    ibig=0;
    del=0.0;
    for (i=1;i<=n;i++)
      {
      for (j=1;j<=n;j++) xit[j]=xi[j][i];
      fptt=(*fret);
      linmin(p,xit,n,fret,func);
      if (fabs(fptt-(*fret)) > del)
    {
    del=fabs(fptt-(*fret));
    ibig=i;
    }
      }
    if (2.0*fabs(fp-(*fret)) <= ftol*(fabs(fp)+fabs(*fret)))
      {
      free_vector(xit,1,n);
      free_vector(ptt,1,n);
      free_vector(pt,1,n);
      return;
      }
    if (*iter == ITMAX)
      {
      vtkErrorMacro("To many iterations in powell");
      return;
      }
    for (j=1;j<=n;j++)
      {
      ptt[j]=2.0*p[j]-pt[j];
      xit[j]=p[j]-pt[j];
      pt[j]=p[j];
      }
    fptt=(this->*func)(ptt);
    if (fptt < fp)
      {
      t=2.0*(fp-2.0*(*fret)+fptt)*SQR(fp-(*fret)-del)-del*SQR(fp-fptt);
      if (t < 0.0)
    {
    linmin(p,xit,n,fret,func);
    for (j=1;j<=n;j++) xi[j][ibig]=xit[j];
    }
      }
    }
}
#undef ITMAX
#undef SQR

static inline int vtkImageGCRFloor(float x, float &f)
{
  int ix = int(x);
  f = x-ix;
  if (f < 0) { f = x - (--ix); }

  return ix;
}

vtkImageGCR* vtkImageGCR::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageGCR");
  if(ret)
    {
    return (vtkImageGCR*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageGCR;
}

vtkImageGCR::vtkImageGCR()
{
  this->Target=0;
  this->Source=0;
  this->Mask=0;
  this->WorkTarget=vtkImageData::New();
  this->WorkSource=vtkImageData::New();
  this->WorkMask=vtkImageData::New();
  this->WorkTransform=vtkTransform::New();
  this->GeneralTransform=vtkGeneralTransform::New();
  this->TransformDomain=2;
  this->Interpolation=1;
  this->InterpolationFunction=&vtkImageGCR::ComputeWithPVInterpolation;
  this->Criterion=1;
  this->Verbose=0;
  
  this->ncom=0;
  this->pcom=0;
  this->xicom=0;
}

vtkImageGCR::~vtkImageGCR()
{
  this->SetTarget(0);
  this->SetSource(0);
  this->SetMask(0);
  this->WorkTarget->Delete();
  this->WorkTarget=0;
  this->WorkSource->Delete();
  this->WorkSource=0;
  this->WorkMask->Delete();
  this->WorkMask=0;
  if(this->WorkTransform)
    {
    this->WorkTransform->Delete();
    }
  if(this->GeneralTransform)
    {
    this->GeneralTransform->Delete();
    }
}

void vtkImageGCR::ComputeWithTrilinearInterpolation(float* point,
                            unsigned char* tptr,
                            unsigned char* sptr,
                            int* ext,
                            int* inc,
                            float jh[256][256])
{
  unsigned char q;
  if(this->TrilinearInterpolation(point,sptr,ext,inc,q))
    {
    ++jh[q][*tptr];
    }
} 

void vtkImageGCR::ComputeWithPVInterpolation(float* point,
                         unsigned char* tptr,
                         unsigned char* sptr,
                         int* ext,
                         int* inc,
                         float jh[256][256])
{
  unsigned char p[8];
  float w[8];
  if(this->TrilinearWeights(point,sptr,ext,inc,
                (unsigned char (*)[2][2])p,
                (float (*)[2][2])w))
    {
    unsigned char v = *tptr;
    unsigned char pp;
    for(int k=0;k<8;++k)
      {
      pp = (unsigned char)(p[k]+0.5);
      jh[pp][v] += w[k];
      }
    }
}

float vtkImageGCR::ComputeL1(float jh[256][256])
{
  float med[256];
  float ni=0;
  //Modified for Liu.
  int i,j;

  for(//int 
      i=0;i<256;++i)
    {
    med[i]=0;
    float n=0;
    for(//int 
        j=0;j<256;++j)
      {
      n+=jh[i][j];
      }
    ni+=n;
    float n2=0;
    float mid=n/2;
//     bool avg=false;
//     if(fmod(n,2)!=0)
//       {
//       avg=true;
//       }
// Modified By Liu for(int j=0;j<256;++j)
    for(j=0;j<256;++j)
      {
      n2+=jh[i][j];
      if(n2>=mid)
    {
//     // annoying case when median is between two classes.
//     // compute average value of the two classes.
//     if(avg && n2=mid)
//       {
//       for(int k=j;k<256;++k)
//         {
//         if(jh[i][k]!=0)
//           {
//           med[i]=(j+k)/2.0;
//           break;
//           }
//         }
//       }
//     else
//       {
    med[i]=j;
    //      }
    break;
    }
      }
    }
  float medi=0;
  float n2=0;
  float mid=ni/2;
//   bool avg=false;
//   if(fmod(ni,2)!=0)
//     {
//     avg=true;
//     }
  for(//int 
      j=0;j<256;++j)
    {
    for(//int 
        i=0;i<256;++i)
      {
      n2+=jh[i][j];
      }
    if(n2>=mid)
      {
      medi=j;
      break;
      }
    }

  float S=0;
  float S0=0;
//  for(int i=0;i<256;++i) 
// Modofied by Liu

  for( i=0;i<256;++i)
      {
    for(//int 
        j=0;j<256;++j)
      {
      S += jh[i][j] * abs(j-med[i]);
      S0 += jh[i][j] * abs(j-medi);
      }
    }

  double gcr;
  if(S0!=0)
    {
    gcr=1-S/S0;
    }
  else
    {
    gcr=0;
    }

  if(this->Verbose)
    {
    cout << "  CR=" << gcr << "\r";
    cout.flush();
    }

  return 1-gcr;
} 

float vtkImageGCR::ComputeL2(float jh[256][256])
{
  double stats[256][3];
  double sum=0;
  double sum2=0;
  double n=0;

  //Modified by Liu
  int i,j;
  
  for(//int 
      i=0;i<256;++i)
    {
    stats[i][0]=0;
    stats[i][1]=0;
    stats[i][2]=0;
    for(//int 
        j=0;j<256;++j)
      {
      double v=jh[i][j];
      double vv=v*j;
      stats[i][0]+=vv;
      stats[i][1]+=vv*j;
      stats[i][2]+=v;
      }
    sum+=stats[i][0];
    sum2+=stats[i][1];
    n+=stats[i][2];
    }

  float S=0;
  //for(int i=0;i<256;++i)
//Modified by Liu
  for(i=0;i<256;++i)  
  {
    if(stats[i][2]!=0)
      {
      S += stats[i][1] - stats[i][0] * stats[i][0] / stats[i][2];
      }
    }

  float S0=0;
  if(n!=0)
    {
    S0=sum2 - sum*sum/n;
    }

  double gcr;
  if(S0!=0)
    {
    gcr=1-S/S0;
    }
  else
    {
    gcr=0;
    }

  if(this->Verbose)
    {
    cout << "  CR=" << gcr << "\r";
    cout.flush();
    }

  return 1-gcr;
} 

float vtkImageGCR::ComputeCO(float jh[256][256])
{
  double mi, mj, si, sj;
  double n;

  mi=0;
  mj=0;
  si=0;
  sj=0;
  n=0;
  for(int i=0;i<256;++i)
    {
    double sumi=0;
    double sumj=0;
    for(int j=0;j<256;++j)
      {
      sumi+=jh[i][j];
      sumj+=jh[j][i];
      }
    n+=sumi;
    mi+=sumi*i;
    mj+=sumj*i;
    si+=sumi*i*i;
    sj+=sumj*i*i;
    }

  float S=0;
  if(n!=0)
    {
    mi/=n;
    mj/=n;
    si/=n;
    sj/=n;
    si-=mi*mi;
    sj-=mj*mj;
    si=sqrt(si);
    sj=sqrt(sj);
    
    for(int i=0;i<256;++i)
      {
      for(int j=0;j<256;++j)
        {
        S+=(jh[i][j]/n)*(i-mi)*(j-mj)/(si*sj);
        }
      }
    }
  else
    {
    S=-1;
    }

  if(this->Verbose)
    {
    cout << "  CO=" << S << "\r";
    cout.flush();
    }

  return -S;
} 

float vtkImageGCR::ComputeMI(float jh[256][256])
{
  double pi[256];
  double pj[256];
  double n=0;
  
  for(int i=0;i<256;++i)
    {
    double sumi=0;
    double sumj=0;
    for(int j=0;j<256;++j)
      {
      sumi+=jh[i][j];
      sumj+=jh[j][i];
      }
    pi[i]=sumi;
    pj[i]=sumj;
    n+=sumi;
    }

  double S=0;
  if(n!=0)
    {
    double l2=log(2.);
    double ln=log(n);
    for(int i=0;i<256;++i)
      {
      for(int j=0;j<256;++j)
        {
        if(jh[i][j]!=0)
          {
          S+=jh[i][j]*(log(jh[i][j]/(pi[i]*pj[j]))+ln)/l2;
          }
        }
      }
    S/=n;
    }
  
  if(this->Verbose)
    {
    cout << "  MI=" << S << "\r";
    cout.flush();
    }

  return -S;
} 

float vtkImageGCR::Compute()
{
  vtkDebugMacro(<< "Target: " << this->WorkTarget << " Source: "
        << this->WorkSource << " Mask: " << this->WorkMask);

  int* tExt = this->WorkTarget->GetExtent();
  int* sExt = this->WorkSource->GetExtent();
  vtkDebugMacro(<< "Extent: " << tExt[0] << " " << tExt[1] << " "
        << tExt[2] << " " << tExt[3] << " "
        << tExt[4] << " " << tExt[5]);
  
  vtkFloatingPointType* tOrigin = this->WorkTarget->GetOrigin();
  vtkFloatingPointType* tSpacing = this->WorkTarget->GetSpacing();
  vtkFloatingPointType* sOrigin = this->WorkSource->GetOrigin();
  vtkFloatingPointType* sSpacing = this->WorkSource->GetSpacing();
  float sInvSpacing[3];
  sInvSpacing[0] = 1.0f/sSpacing[0];
  sInvSpacing[1] = 1.0f/sSpacing[1];
  sInvSpacing[2] = 1.0f/sSpacing[2];

  // Get increments to march through data
  unsigned char* tptr =
    static_cast<unsigned char*>(this->WorkTarget->GetScalarPointerForExtent(tExt));
  unsigned char* sptr =
    static_cast<unsigned char*>(this->WorkSource->GetScalarPointerForExtent(sExt));

  // Kilian int vtkIdType tincX,tincY,tincZ;
  vtkIdType tincX,tincY,tincZ;
  //cout << "Different From before - int before" << endl;
  this->WorkTarget->GetContinuousIncrements(tExt,tincX,tincY,tincZ);
  //cout << "**tincX "<< tincX << " " << tincY << " " << tincZ << endl;
  //cout << "tExt "<< tExt[0] << " " << tExt[1] << " " << tExt[2] << " " << tExt[3] << " " << tExt[4] << " " << tExt[5] << endl;

  // Kilian different
  // int* sInc = this->WorkSource->GetIncrements();
  vtkIdType* _sInc = this->WorkSource->GetIncrements();
  int sInc[3] = { int(_sInc[0]),  int(_sInc[1]),  int(_sInc[2])} ;
  // cout << "sInc "<< sInc[0] << " " << sInc[1] << " " << sInc[2] << endl;

  unsigned char* mptr = 0;
  // int mincX,mincY,mincZ;
  vtkIdType  mincX,mincY,mincZ;

  if(this->GetMask() != 0)
    {
    mptr = static_cast<unsigned char*>(this->WorkMask->GetScalarPointerForExtent(tExt));
    this->WorkTarget->GetContinuousIncrements(this->WorkMask->GetExtent(),mincX,mincY,mincZ);
    }

  float jh[256][256];
  for(int i=0;i<256;++i)
    {
    for(int j=0;j<256;++j)
      {
      jh[i][j]=0;
      }
    }
  
  float point[3];
  vtkAbstractTransform* trans=this->GeneralTransform->GetInverse();
  trans->Update();
  for(int z=tExt[4];z<=tExt[5];++z)
    {
    for(int y=tExt[2];y<=tExt[3];++y)
      {
      for(int x=tExt[0];x<=tExt[1];++x)
    {
    if(!mptr || *mptr)
      {
      point[0] = x*tSpacing[0] + tOrigin[0];
      point[1] = y*tSpacing[1] + tOrigin[1];
      point[2] = z*tSpacing[2] + tOrigin[2];
      trans->InternalTransformPoint(point,point);
      point[0] = (point[0] - sOrigin[0])*sInvSpacing[0];
      point[1] = (point[1] - sOrigin[1])*sInvSpacing[1];
      point[2] = (point[2] - sOrigin[2])*sInvSpacing[2];

      (this->*InterpolationFunction)(point,tptr,sptr,sExt,sInc,jh);
      }
    ++tptr;
    if(mptr)
      {
      ++mptr;
      }
    }
      tptr += tincY;
      if(mptr)
    {
    mptr += mincY;
    }
      }
    tptr += tincZ;
    if(mptr)
      {
      mptr += mincZ;
      }
    }

  double gcr=0;
  switch(this->Criterion)
    {
    case VTK_IMAGE_GCR_L1:
      gcr=this->ComputeL1(jh);
      break;
    case VTK_IMAGE_GCR_L2:
      gcr=this->ComputeL2(jh);
      break;
    case VTK_IMAGE_GCR_CO:
      gcr=this->ComputeCO(jh);
      break;
    case VTK_IMAGE_GCR_MI:
      gcr=this->ComputeMI(jh);
      break;
    default:
      vtkErrorMacro("Unknown criterion" << this->Criterion);
      gcr=1;
      break;
    }
  
  return gcr;
} 

void vtkImageGCR::SetInterpolation(int i)
{ 
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Interpolation to " << i); 
  if (this->Interpolation != i) 
    { 
    this->Interpolation = i;
    switch(this->Interpolation)
      {
      case 0:
    this->InterpolationFunction=&vtkImageGCR::ComputeWithTrilinearInterpolation;
    break;
      case 1:
    this->InterpolationFunction=&vtkImageGCR::ComputeWithPVInterpolation;
    break;
      default:
    vtkErrorMacro("SetInterpolation: unknown type of interprolation: " << i);
    return;
      }
    this->Modified();
    }
} 

static void vtkImageGCRP2Translation(float* p, vtkTransform* trans)
{
  trans->Identity();
  trans->Translate(p[1],p[2],p[3]);
}

static void vtkImageGCRP2Rigid(float* p, vtkTransform* trans)
{
  trans->Identity();
  trans->Translate(p[1],p[2],p[3]);
  trans->RotateWXYZ(vtkMath::Norm(&p[4]),p[4],p[5],p[6]);
}

static void vtkImageGCRP2Similarity(float* p, vtkTransform* trans)
{
  trans->Identity();
  trans->Translate(p[2],p[3],p[4]);
  trans->RotateWXYZ(vtkMath::Norm(&p[5]),p[5],p[6],p[7]);
  trans->Scale(p[1],p[1],p[1]);
}

static void vtkImageGCRP2Affine(float* p, vtkTransform* trans)
{
  trans->Identity();
  trans->Translate(p[7],p[8],p[9]);
  trans->RotateWXYZ(vtkMath::Norm(&p[10]),p[10],p[11],p[12]);
  trans->Scale(p[1],p[2],p[3]);
  trans->RotateWXYZ(vtkMath::Norm(&p[4]),p[4],p[5],p[6]);
}

void vtkImageGCR::InternalUpdate()
{
  vtkDebugMacro("Executing main code");

  vtkImageData* t=this->GetTarget();
  vtkImageData* s=this->GetSource();

  if(t==0)
    {
    vtkErrorMacro("No Target");
    return;
    }

  if(s==0)
    {
    vtkErrorMacro("No Source");
    return;
    }

  this->GeneralTransform->Identity();
  this->GeneralTransform->PostMultiply();
  this->WorkTransform->PostMultiply();
  this->GeneralTransform->Concatenate(this->WorkTransform);
  
  this->NormalizeImages();

  int n=12;
  float* p=this->vector(1,n);
  float** xi=this->matrix(1,n,1,n);
  float ftol=10e-3;
  int iter;
  float fret;
  for(int i=1;i<=n;++i)
    {
    p[i]=0;
    xi[i][i]=1;
    for(int j=i+1;j<=n;++j)
      {
      xi[i][j]=xi[j][i]=0;
      }
    }

#ifdef _WIN32
  ios_base::fmtflags old_options=cout.flags(ios::showpos|ios::fixed|
                       ios::showpoint|ios::internal);
#else                        
  //Modified by Liu.  For PC, maybe it should  make old_options wiht type long 
  ios::fmtflags old_options=cout.flags(ios::showpos|ios::fixed|
                       ios::showpoint|ios::internal);
#endif


  // first, let deal with the special cases
  if(this->GetTransformDomain()==-1)
    {
    // MI needs a little push out of the regular grid...
    if(this->Criterion==VTK_IMAGE_GCR_MI)
      {
      p[4] = p[5] = p[6] = 1;
      }
    
    if(this->Verbose)
      {
      cout << "Translation registration" << endl;
      }
    this->powell(p,xi,3,ftol,&iter,&fret,
         &vtkImageGCR::MinimizeWithTranslationTransform);
    this->Matrix->DeepCopy(this->WorkTransform->GetMatrix());
    if(this->Verbose)
      {
      cout << endl << "  " << iter << " iterations" << endl;
      }
    }

  // and now the transformation groups
  if(this->GetTransformDomain()>=0)
    {
    // MI needs a little push out of the regular grid...
    if(this->Criterion==VTK_IMAGE_GCR_MI)
      {
      p[4] = p[5] = p[6] = 1;
      }
    
    if(this->Verbose)
      {
      cout << "Rigid registration" << endl;
      }
    this->powell(p,xi,6,ftol,&iter,&fret,
         &vtkImageGCR::MinimizeWithRigidTransform);
    if(this->GetTransformDomain()==0)
      {
      this->Matrix->DeepCopy(this->WorkTransform->GetMatrix());
      }
    if(this->Verbose)
      {
      cout << endl << "  " << iter << " iterations" << endl;
      }
    }

  if(this->GetTransformDomain()>=1)
    {
    for(int i=7;i>=2;--i)
      {
      p[i]=p[i-1];
      }
    p[1] = 1;
    if(this->Verbose)
      {
      cout << "Similarity registration" << endl;
      }
    this->powell(p,xi,7,ftol,&iter,&fret,
         &vtkImageGCR::MinimizeWithSimilarityTransform);
    if(this->GetTransformDomain()==1)
      {
      this->Matrix->DeepCopy(this->WorkTransform->GetMatrix());
      }
    if(this->Verbose)
      {
      cout << endl << "  " << iter << " iterations" << endl;
      }
    }

  if(this->GetTransformDomain()>=2)
    {
    for(int i=12;i>=7;--i)
      {
      p[i]=p[i-5];
      }
    p[2] = p[3] = p[1];
    p[4] = p[5] = p[6] = 0;
    
    // than go for full affine
    if(this->Verbose)
      {
      cout << "Affine registration" << endl;
      }
    this->powell(p,xi,12,ftol,&iter,&fret,
         &vtkImageGCR::MinimizeWithAffineTransform);
    if(this->GetTransformDomain()==2)
      {
      this->Matrix->DeepCopy(this->WorkTransform->GetMatrix());
      }
    if(this->Verbose)
      {
      cout << endl << "  " << iter << " iterations" << endl;
      }
    }

  cout.flags(old_options);
  this->free_vector(p,1,n);
  this->free_matrix(xi,1,n,1,n);

  this->vtkTransform::InternalUpdate();
}

bool vtkImageGCR::TrilinearWeights(float *point,unsigned char* inPtr,
                   int inExt[6], int inInc[3],
                   unsigned char p[2][2][2],
                   float w[2][2][2])
{
  float fx,fy,fz;
  int floorX = int(vtkImageGCRFloor(point[0],fx));
  int floorY = int(vtkImageGCRFloor(point[1],fy));
  int floorZ = int(vtkImageGCRFloor(point[2],fz));

  int inIdX0 = floorX-inExt[0];
  int inIdY0 = floorY-inExt[2];
  int inIdZ0 = floorZ-inExt[4];

  int inIdX1 = inIdX0 + (fx != 0);
  int inIdY1 = inIdY0 + (fy != 0);
  int inIdZ1 = inIdZ0 + (fz != 0);
  
  if (inIdX0 < 0 || inIdX1 > inExt[1]-inExt[0]
      || inIdY0 < 0 || inIdY1 > inExt[3]-inExt[2]
      || inIdZ0 < 0 || inIdZ1 > inExt[5]-inExt[4] )
    {// out of bounds: don't do anything.
    return false;
    }

  // compute trilinear weights
  int factX = inIdX0*inInc[0];
  int factY = inIdY0*inInc[1];
  int factZ = inIdZ0*inInc[2];
   
  int factX1 = inIdX1*inInc[0];
  int factY1 = inIdY1*inInc[1];
  int factZ1 = inIdZ1*inInc[2];
    
  p[0][0][0] = inPtr[factX+factY+factZ];
  p[0][0][1] = inPtr[factX+factY+factZ1];
  p[0][1][0] = inPtr[factX+factY1+factZ];
  p[0][1][1] = inPtr[factX+factY1+factZ1];
  p[1][0][0] = inPtr[factX1+factY+factZ];
  p[1][0][1] = inPtr[factX1+factY+factZ1];
  p[1][1][0] = inPtr[factX1+factY1+factZ];
  p[1][1][1] = inPtr[factX1+factY1+factZ1];
      
  float rx = 1.0f - fx;
  float ry = 1.0f - fy;
  float rz = 1.0f - fz;
      
  float ryrz = ry*rz;
  float ryfz = ry*fz;
  float fyrz = fy*rz;
  float fyfz = fy*fz;

  w[0][0][0] = rx*ryrz;
  w[0][0][1] = rx*ryfz;
  w[0][1][0] = rx*fyrz;
  w[0][1][1] = rx*fyfz;
  w[1][0][0] = fx*ryrz;
  w[1][0][1] = fx*ryfz;
  w[1][1][0] = fx*fyrz;
  w[1][1][1] = fx*fyfz;

  return true;
}

bool vtkImageGCR::TrilinearInterpolation(float *point,
                     unsigned char *inPtr,
                     int inExt[6], int inInc[3],
                     unsigned char& q)
{
  unsigned char p[2][2][2];
  float w[2][2][2];
  if(!this->TrilinearWeights(point,inPtr,inExt,inInc,p,w))
    {
    return false;
    }
   
  q = static_cast<unsigned char>(w[0][0][0]*p[0][0][0] +
                 w[0][0][1]*p[0][0][1] +
                 w[0][1][0]*p[0][1][0] +
                 w[0][1][1]*p[0][1][1] +
                 w[1][0][0]*p[1][0][0] +
                 w[1][0][1]*p[1][0][1] +
                 w[1][1][0]*p[1][1][0] +
                 w[1][1][1]*p[1][1][1] +
                 0.5);
  return true;
}

float vtkImageGCR::MinimizeWithTranslationTransform(float* p){
  vtkDebugMacro("");

  if(this->TwoD)
    {
    p[3]=0;
    }
  vtkImageGCRP2Translation(p,this->WorkTransform);
  if(this->Verbose >= 2)
    {
      // Modified by Liu

    //cout <<"  1:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[1]
    // <<" 2:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[2]
    // <<" 3:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[3];
        cout.width(8);
        cout.precision(4);
        cout.fill('0');
        cout << "  1:" <<p[1] << " 2:"<<p[2]<<" 3:"<<p[3];




    }
  return this->Compute();
}

float vtkImageGCR::MinimizeWithRigidTransform(float* p){
  vtkDebugMacro("");
  
  if(this->TwoD)
    {
    p[3]=p[4]=p[5]=0;
    }
  vtkImageGCRP2Rigid(p,this->WorkTransform);
  if(this->Verbose >= 2)
    {
      //Modified by Liu.
        cout.width(8);
        cout.precision(4);
        cout.fill('0');
//    cout <<"  1:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[1]
//     <<" 2:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[2]
//     <<" 3:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[3]
//     <<" 4:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[4]
//     <<" 5:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[5]
//     <<" 6:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[6];
cout <<"  1:"<<p[1]
     <<" 2:"<<p[2]
     <<" 3:"<<p[3]
     <<" 4:"<<p[4]
     <<" 5:"<<p[5]
     <<" 6:"<<p[6];


    }
  return this->Compute();
}

float vtkImageGCR::MinimizeWithSimilarityTransform(float* p){
  if(this->TwoD)
    {
    p[4]=p[5]=p[6]=0;
    }

  vtkImageGCRP2Similarity(p,this->WorkTransform);
  if(this->Verbose >= 2)
    {
     //Modified by Liu.
      
        cout.width(8);
        cout.precision(4);
        cout.fill('0');

    //cout <<"  1:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[1]
    // <<" 2:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[2]
    // <<" 3:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[3]
    // <<" 4:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[4]
    // <<" 5:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[5]
    // <<" 6:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[6]
     //<<" 7:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[7];
cout <<"  1:"<<p[1]
     <<" 2:"<<p[2]
     <<" 3:"<<p[3]
     <<" 4:"<<p[4]
     <<" 5:"<<p[5]
     <<" 6:"<<p[6]
     <<" 7:"<<p[7];

    }
  return this->Compute();
}

float vtkImageGCR::MinimizeWithAffineTransform(float* p){
  if(this->TwoD)
    {
    p[3]=1;
    p[9]=p[4]=p[5]=p[10]=p[11]=0;
    }

  vtkImageGCRP2Affine(p,this->WorkTransform);
  if(this->Verbose >= 2)
    {

      // Modified by Liu
   
         cout.width(8);
        cout.precision(4);
        cout.fill('0');
//cout <<"  1:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[1]
//     <<" 2:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[2]
//     <<" 3:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[3]
//     <<" 4:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[4]
//     <<" 5:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[5]
//     <<" 6:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[6]
//     <<" 7:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[7]
//     <<" 8:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[8]
//     <<" 9:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[9]
//     <<" 10:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[10]
//     <<" 11:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[11]
//     <<" 12:"<<setw(8)<<setprecision(4)<<setfill('0')<<p[12];
    
     

    cout <<"  1:"<<p[1]
     <<" 2:"<<p[2]
     <<" 3:"<<p[3]
     <<" 4:"<<p[4]
     <<" 5:"<<p[5]
     <<" 6:"<<p[6]
     <<" 7:"<<p[7]
     <<" 8:"<<p[8]
     <<" 9:"<<p[9]
     <<" 10:"<<p[10]
     <<" 11:"<<p[11]
     <<" 12:"<<p[12];
    }
  return this->Compute();
}

void vtkImageGCR::NormalizeImages()
{
  vtkImageData* t=this->GetTarget();
  vtkImageData* s=this->GetSource();
  vtkImageData* m=this->GetMask();

  t->Update();
  s->Update();
  
  int* dims = t->GetDimensions();
  int sx=dims[0]/50;
  int sy=dims[1]/50;
  int sz=dims[2]/50;
  if(sx==0) sx=1;
  if(sy==0) sy=1;
  if(sz==0) sz=1;

  // extract first component
  vtkImageExtractComponents* Extract_t = vtkImageExtractComponents::New();
  Extract_t->SetComponents(0);
  Extract_t->SetInput(t);
  Extract_t->Update();
  
  // shrink target
  vtkImageShrink3D* Shrink = vtkImageShrink3D::New();
  Shrink->AveragingOff();
  Shrink->SetShrinkFactors(sx,sy,sz);
  Shrink->SetInput(Extract_t->GetOutput());


  vtkImageHistogramNormalization* Normalized = vtkImageHistogramNormalization::New();
  Normalized->SetOutputScalarTypeToUnsignedChar();
  Normalized->SetInput(Shrink->GetOutput());
  Normalized->SetOutput(this->WorkTarget);

  this->WorkTarget->Update();
  this->WorkTarget->SetSource(0);

  // shrink mask
  if(m)
    {
    Shrink->SetInput(m);
    Shrink->SetOutput(this->WorkMask);
    this->WorkMask->Update();
    this->WorkMask->SetSource(0);
    }
  
  // normalize source
  vtkImageExtractComponents* Extract_s = vtkImageExtractComponents::New();
  Extract_s->SetComponents(0);
  Extract_s->SetInput(s);
  Extract_s->Update();

  // Kilian - Feb 08 : I do not know why but the output format is changed if you do not 
  // => unreliable results - Test this is also true for slicer2 
  vtkImageChangeInformation *Extract_si =  vtkImageChangeInformation::New();
  Extract_si->SetInput(Extract_s->GetOutput());
  Extract_si->SetOutputOrigin(s->GetOrigin());
  Extract_si->SetOutputSpacing(s->GetSpacing());
  Extract_si->Update();

  Normalized->SetInput(Extract_si->GetOutput());
  Normalized->SetOutput(this->WorkSource);

  this->WorkSource->Update();
  this->WorkSource->SetSource(0);

  
  // VolumeWriter (Extract->GetOutput(),"EXTRACT_S"); 
  //  _Print(Extract_s->GetOutput(),cout,ind);
 
  //   Write(this->WorkTarget,"/tmp/t.vtk"); 
  //   Write(this->WorkSource,"/tmp/s.vtk");

  Extract_si->Delete();
  Extract_s->Delete();
  Shrink->Delete();
  Normalized->Delete();
  Extract_t->Delete();
}


void vtkImageGCR::Inverse()
{
  vtkImageData* source=this->GetTarget();
  this->SetTarget(this->GetSource());
  this->SetSource(source);
}

//Modified by Liu
//void vtkImageGCR::PrintSelf(ostream& os, vtkIndent indent)
void vtkImageGCR::PrintSelf(::ostream& os, vtkIndent indent)
{
  // this->vtkTransform::PrintSelf(os,indent);

  os << indent << "TransformDomain: " << this->TransformDomain << "\n";
  os << indent << "Interpolation: " << this->Interpolation << "\n";
  os << indent << "Criterion: " << this->Criterion << "\n";
  os << indent << "Verbose: " << this->Verbose << "\n";
  
  os << indent << "Target: " << this->Target << "\n";
  if(this->Target)
    {
      // this->Target->PrintSelf(os,indent.GetNextIndent());
      _Print(this->Target,os,indent.GetNextIndent());
    }
  os << indent << "Source: " << this->Source << "\n";
  if(this->Source)
    {
      // this->Source->PrintSelf(os,indent.GetNextIndent());
    _Print(this->Source,os,indent.GetNextIndent());

    }
  os << indent << "Mask: " << this->Mask << "\n";
  if(this->Mask)
    {
      // this->Mask->PrintSelf(os,indent.GetNextIndent());
      _Print(this->Mask,os,indent.GetNextIndent());
    }
  os << indent << "WorkTarget: " << this->WorkTarget << "\n";
  if(this->WorkTarget)
    {
      // this->WorkTarget->PrintSelf(os,indent.GetNextIndent());
      _Print(this->WorkTarget,os,indent.GetNextIndent());
    }
  os << indent << "WorkSource: " << this->WorkSource << "\n";
  if(this->WorkSource)
    {
      // this->WorkSource->PrintSelf(os,indent.GetNextIndent());
      _Print(this->WorkSource,os,indent.GetNextIndent());
    }
  os << indent << "WorkMask: " << this->WorkMask << "\n";
  if(this->WorkMask)
    {
      // this->WorkMask->PrintSelf(os,indent.GetNextIndent());
      _Print(this->WorkMask,os,indent.GetNextIndent());
    }
  os << indent << "GeneralTransform: " << this->GeneralTransform << "\n";
  if(this->GeneralTransform && 0)
    {
    this->GeneralTransform->PrintSelf(os,indent.GetNextIndent());
    }
  os << indent << "WorkTransform: " << this->WorkTransform << "\n";
  if(this->WorkTransform && 0 )
    {
    this->WorkTransform->PrintSelf(os,indent.GetNextIndent());
    }
}
