#ifndef SFLSRobustStatSegmentor3DLabelMap_single_h_
#define SFLSRobustStatSegmentor3DLabelMap_single_h_

#include "SFLSSegmentor3D.h"

#include <list>
#include <vector>

// #include "boost/shared_ptr.hpp"

template <typename TPixel>
class CSFLSRobustStatSegmentor3DLabelMap : public CSFLSSegmentor3D<TPixel>
{
  /*----------------------------------------------------------------------
    just copy, not logic change */
public:
  typedef CSFLSSegmentor3D<TPixel> SuperClassType;

  typedef CSFLSRobustStatSegmentor3DLabelMap<TPixel> Self;
  // typedef boost::shared_ptr< Self > Pointer;

  typedef typename SuperClassType::NodeType   NodeType;
  typedef typename SuperClassType::CSFLSLayer CSFLSLayer;

  /*================================================================================
    ctor */
  CSFLSRobustStatSegmentor3DLabelMap() : CSFLSSegmentor3D<TPixel>()
  {
    basicInit();
  }

  /* New */
  // static Pointer New() { return Pointer(new Self); }

  void basicInit();

  /* just copy, not logic change
     ----------------------------------------------------------------------
     ----------------------------------------------------------------------
     ----------------------------------------------------------------------
     ---------------------------------------------------------------------- */

  typedef typename SuperClassType::TShortImage TLabelImage;
  typedef typename TLabelImage::Pointer       TLabelImagePointer;

  typedef typename SuperClassType::TFloatImage TFloatImage;
  typedef typename TFloatImage::Pointer        TFloatImagePointer;

  // typedef typename SuperClassType::TDoubleImage TDoubleImage;
  // typedef typename TDoubleImage::Pointer TDoubleImagePointer;

  typedef typename SuperClassType::MaskImageType TMaskImage;

  typedef typename SuperClassType::TIndex  TIndex;
  typedef typename SuperClassType::TSize   TSize;
  typedef typename SuperClassType::TRegion TRegion;

  /* ============================================================
   * functions
   * ============================================================*/

  void setInputLabelImage(TLabelImagePointer l);

  void doSegmenation() override;

  void computeForce() override;

  void setKernelWidthFactor(double f);

  void setIntensityHomogeneity(double h);

protected:
  /* data */
  TLabelImagePointer              m_inputLabelImage;
  std::vector<std::vector<long> > m_seeds; // in IJK

  std::vector<std::vector<double> > m_featureAtTheSeeds;

  long m_statNeighborX;
  long m_statNeighborY;
  long m_statNeighborZ;

  const static long m_numberOfFeature = 3;
  /* Store the robust stat as the feature at each point
     0: Meadian
     1: interquartile range (IRQ)
     2. median absolute deviation (MAD)
  */
  TLabelImagePointer              m_featureComputed; // if feature at this point is computed, then is 1
  std::vector<TFloatImagePointer> m_featureImageList;

  double m_kernelWidthFactor; // kernel_width = empirical_std/m_kernelWidthFactor, Eric has it at 10.0

  /* fn */
  void initFeatureComputedImage();

  void initFeatureImage();

  // void computeFeature();
  void computeFeatureAt(TIndex idx, std::vector<double>& f);

  void getRobustStatistics(std::vector<double>& samples, std::vector<double>& robustStat);

  void inputLableImageToSeeds();

  void seedToMask();

  void dialteSeeds();

  void getFeatureAroundSeeds();

  void estimateFeatureStdDevs();

  TPixel m_inputImageIntensityMin;
  TPixel m_inputImageIntensityMax;
  void computeMinMax();

  std::vector<std::vector<double> > m_PDFlearnedFromSeeds;  // each feature corresponds to a inner std::vector<double>
  void estimatePDFs();

  // void getFeatureAt(TDoubleImage::IndexType idx, std::vector<double>& f);

  void getThingsReady();

  // kernel
  std::vector<double> m_kernelStddev;
  double kernelEvaluation(const std::vector<double>& newFeature);

  double kernelEvaluationUsingPDF(const std::vector<double>& newFeature);

};

#include "SFLSRobustStatSegmentor3DLabelMap_single.txx"

#endif
