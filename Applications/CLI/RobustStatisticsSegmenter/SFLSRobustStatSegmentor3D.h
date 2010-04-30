#ifndef SFLSRobustStatSegmentor3D_h_
#define SFLSRobustStatSegmentor3D_h_

#include "SFLSSegmentor3D.h"

#include <list>
#include <vector>

//#include "boost/shared_ptr.hpp"

template< typename TPixel >
class CSFLSRobustStatSegmentor3D : public CSFLSSegmentor3D< TPixel >
{
  /*---------------------------------------------------------------------- 
    just copy, not logic change */

public:
  typedef CSFLSSegmentor3D< TPixel > SuperClassType;

  typedef CSFLSRobustStatSegmentor3D< TPixel > Self;


  typedef typename SuperClassType::NodeType NodeType;
  typedef typename SuperClassType::CSFLSLayer CSFLSLayer;
  //typedef boost::shared_ptr< Self > Pointer;


  /*================================================================================
    ctor */
  CSFLSRobustStatSegmentor3D() : CSFLSSegmentor3D< TPixel >()
  {
    basicInit();
  }

//   /* New */
//   static Pointer New() { return Pointer(new Self); }

  void basicInit();

  /* just copy, not logic change
     ----------------------------------------------------------------------
     ----------------------------------------------------------------------
     ----------------------------------------------------------------------
     ---------------------------------------------------------------------- */

  typedef typename SuperClassType::TCharImage TLabelImage;
  typedef typename TLabelImage::Pointer TLabelImagePointer;


//   typedef typename SuperClassType::TDoubleImage TDoubleImage;
//   typedef typename TDoubleImage::Pointer TDoubleImagePointer;

  typedef typename SuperClassType::TFloatImage TFloatImage;
  typedef typename TFloatImage::Pointer TFloatImagePointer;


  typedef typename SuperClassType::MaskImageType TMaskImage;

  typedef typename SuperClassType::TIndex TIndex;

  /* ============================================================
   * functions
   * ============================================================*/

  void setSeeds(const std::vector<std::vector<long> >& seeds) {m_seeds = seeds;}

  void doSegmenation();

  void computeForce();

  void setKernelWidthFactor(double f);
  void setIntensityHomogeneity(double h);

protected:
  /* data */

  std::vector<std::vector<long> > m_seeds; // in IJK

  std::vector< std::vector<double> > m_featureAtTheSeeds;

  long m_statNeighborX;
  long m_statNeighborY;
  long m_statNeighborZ;

  const static long m_numberOfFeature = 3;
  /* Store the robust stat as the feature at each point 
     0: Meadian
     1: interquartile range (IRQ)
     2. median absolute deviation (MAD)
  */
  TLabelImagePointer m_featureComputed; // if feature at this point is computed, then is 1
  //std::vector<TDoubleImagePointer> m_featureImageList;
  std::vector<TFloatImagePointer> m_featureImageList;


  double m_kernelWidthFactor; // kernel_width = empirical_std/m_kernelWidthFactor, Eric has it at 10.0


  /* fn */
  void initFeatureComputedImage();
  void initFeatureImage();

  //void computeFeature();  
  void computeFeatureAt(TIndex idx, std::vector<double>& f);

  void getRobustStatistics(std::vector<double>& samples, std::vector<double>& robustStat);
  void seedToMask();
  void dialteSeeds();
  void getFeatureAroundSeeds();
  void estimateFeatureStdDevs();

  //void getFeatureAt(TDoubleImage::IndexType idx, std::vector<double>& f);

  void getThingsReady();


  // kernel 
  std::vector<double> m_kernelStddev;
  double kernelEvaluation(const std::vector<double>& newFeature);

};


#include "SFLSRobustStatSegmentor3D.txx"

#endif
