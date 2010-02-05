/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
#include "vtkPichonFastMarchingPDF.h"
#include <math.h>
#include "vtkObjectFactory.h"

vtkPichonFastMarchingPDF::vtkPichonFastMarchingPDF( int _realizationMax )
{
  sigma2SmoothPDF=0.25;

  this->realizationMax=_realizationMax;

  bins = new int [realizationMax+1];
  //  assert( bins!=NULL );
  if(!(bins!=NULL))
    {
      vtkErrorMacro("Error in vtkFastMarching, vtkPichonFastMarchingPDF::vtkPichonFastMarchingPDF(...), not enough memory for allocation of 'bins'");
      return;
    }

  smoothedBins = new double [realizationMax+1];
  //  assert( smoothedBins!=NULL );
  if(!(smoothedBins!=NULL))
    {
      vtkErrorMacro("Error in vtkFastMarching, vtkPichonFastMarchingPDF::vtkPichonFastMarchingPDF(...), not enough memory for allocation of 'smoothedBins'");
      return;
    }

  coefGauss = new double[realizationMax+1];
  //  assert( coefGauss!=NULL );
  if(!(bins!=NULL))
    {
      vtkErrorMacro("Error in vtkFastMarching, vtkPichonFastMarchingPDF::vtkPichonFastMarchingPDF(...), not enough memory for allocation of 'bins'");
      return;
    }
  
  reset();

  // default values
  memorySize=10000;
  updateRate=1000;
}

vtkPichonFastMarchingPDF* vtkPichonFastMarchingPDF::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPichonFastMarchingPDF");
  if(ret)
    {
      return (vtkPichonFastMarchingPDF*)ret;
    }

  // If the factory was unable to create the object, then create it here.
  return new vtkPichonFastMarchingPDF;
}

vtkPichonFastMarchingPDF::~vtkPichonFastMarchingPDF()
{
  reset(); // to empty the containers

  delete [] bins;
  delete [] smoothedBins;
  delete [] coefGauss;
}

void vtkPichonFastMarchingPDF::reset( void )
{
  counter=0;

  while( inBins.size()>0 )
    inBins.pop_back();

  while( toBeAdded.size()>0 )
    toBeAdded.pop_back();

  m1=m2=0.0;
  sigma2=mean=0.0;

  for(int k=0;k<=realizationMax;k++)
    bins[k]=0;  

  nRealInBins=0;
}

bool vtkPichonFastMarchingPDF::willUseGaussian( void )
{
  return nRealInBins<50*sqrt(sigma2);
}

double vtkPichonFastMarchingPDF::value( int k )
{
  if( !( (k>=0) && (k<=realizationMax) ) )
    {
      vtkErrorMacro( "Error in vtkPichonFastMarchingPDF::value(k)!" << endl
             << "k=" << k << " realizationMax=" 
             << realizationMax << endl );

      return valueGauss( k );
    }

  // if we have enough points then use the histogram
  if( !willUseGaussian() )
    return valueHisto( k );

  // otherwise we make a gaussian assumption
  return valueGauss( k );
}

double vtkPichonFastMarchingPDF::valueGauss( int k )
{
  return 1.0/sqrt(2*M_PI*sigma2)*exp( -0.5*(double(k)-mean)*(double(k)-mean)/sigma2 );
}

double vtkPichonFastMarchingPDF::valueHisto( int k )
{
  return smoothedBins[k];
}

void vtkPichonFastMarchingPDF::update( void )
{
  int r;

  // move all points from tobeadded to inbins
  while(toBeAdded.size()>0)
    {
      r=toBeAdded[toBeAdded.size()-1];
      toBeAdded.pop_back();

      inBins.push_front( r );
      bins[r]++;

      m1+=r;
      m2+=r*r;
    }

  if(memorySize!=-1)
    {
      // if inbins contains too many points, remove them
      while(inBins.size()>(unsigned)memorySize)
    {
      r=inBins[inBins.size()-1];
      inBins.pop_back();
      
      bins[r]--;
      m1-=r;
      m2-=r*r;
    }
    }

  nRealInBins=(signed)inBins.size();

  //assert( nRealInBins>0 );
  if(!( nRealInBins>0 ))
    {
      vtkErrorMacro("Error in vtkFastMarching, vtkPichonFastMarchingPDF::vtkPichonFastMarchingPDF(...), !nRealInBins>0");
      return;
    }


  // update moments
  mean=m1/double(nRealInBins);
  sigma2=m2/double(nRealInBins)-mean*mean;

  // create smoothed histogram
  double sigma2Smooth=sigma2SmoothPDF*sigma2;
  
  // create lookup table for smoothing
  for(int k=0;k<=realizationMax;k++)
    coefGauss[k]=exp(-0.5*double(k*k)/sigma2Smooth);

  {

    double val;
    double nval;
    double coef;

    for(int k=0;k<=realizationMax;k++)
      {
    val=0.0;
    nval=0.0;

    for(int j=0;j<=realizationMax;j++)
      {
        coef=coefGauss[abs(k-j)];

        val+=coef*double(bins[j]);
        nval+=coef;
      }

    smoothedBins[k]=val/nval/double(nRealInBins);
      }
  }
}

void vtkPichonFastMarchingPDF::addRealization( int k )
{
  //assert(finite(k)!=0);
  if(!(finite(k)!=0))
    {
      vtkErrorMacro("Error in vtkFastMarching, vtkPichonFastMarchingPDF::vtkPichonFastMarchingPDF(...), !(finite(k)!=0)");
      return;
    }

  toBeAdded.push_front(k);

  counter++;

  // update if (either or) :
  // - we have not been updated for updateRate rounds
  // - the number of points waiting to be taken into
  //   consideration  is more than half of our memory
  if( (updateRate!=-1) && 
      ( (counter%updateRate)==0 
    || ((memorySize!=-1) && (toBeAdded.size()>((unsigned int)(memorySize/2)))) ) )
    update();
}

/*
bool vtkPichonFastMarchingPDF::isUnlikelyGauss( double k )
{
  return fabs( k-getMean() )>3.0*sqrt( getSigma2() );
}

bool vtkPichonFastMarchingPDF::isUnlikelyBigGauss( double k )
{
  return ( k-getMean() ) > ( 2.0*sqrt( getSigma2() ) );
}
*/

void vtkPichonFastMarchingPDF::show( void )
{
  cout << "realizationMax=" << realizationMax << endl;
  cout << "nRealInBins=" <<  nRealInBins << endl;
  cout << "mean=" << mean << endl;
  cout << "sqrt( sigma2 )=" << sqrt( sigma2 ) << endl;

  for(int k=0;k<=realizationMax;k++)
    cout << value(k) << endl;

  cout << "---" << endl;
}

void vtkPichonFastMarchingPDF::setMemory( int mem )
{
  memorySize=mem;
}

void vtkPichonFastMarchingPDF::setUpdateRate( int rate )
{
  updateRate=rate;

  if( (updateRate!=-1) && (updateRate<10) )
    updateRate=10;
}

















