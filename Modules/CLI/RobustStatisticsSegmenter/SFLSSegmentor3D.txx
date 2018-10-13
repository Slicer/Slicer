#ifndef SFLSSegmentor3D_hpp_
#define SFLSSegmentor3D_hpp_

#include "SFLSSegmentor3D.h"

#include <algorithm>
#include <cmath>

#include <csignal>

#include <fstream>

#include "itkImageRegionIteratorWithIndex.h"

template <typename TPixel>
CSFLSSegmentor3D<TPixel>
::CSFLSSegmentor3D() : CSFLS()
{
  basicInit();
}

/* ============================================================
   basicInit    */
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::basicInit()
{
  m_numIter = 100;
  m_timeStep = 1.0;

  m_nx = 0;
  m_ny = 0;
  m_nz = 0;

  m_dx = 1.0;
  m_dy = 1.0;
  m_dz = 1.0;

  m_curvatureWeight = 0.0;

  m_insideVoxelCount = 0;
  m_insideVolume = 0;

  m_maxVolume = 1e10;      // in mm^3
  m_maxRunningTime = 3600; // in sec

  m_keepZeroLayerHistory = false;

  m_done = false;
}

/* ============================================================
   setNumIter    */
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::setNumIter(unsigned long n)
{
  m_numIter = n;
}

/* ============================================================
   setImage    */
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::setImage(typename ImageType::Pointer img)
{
  mp_img = img;

  TIndex start = mp_img->GetLargestPossibleRegion().GetIndex();
  TIndex origin = {{0, 0, 0}};
  if( start != origin )
    {
    std::cout << "Warrning: Force image start to be (0, 0, 0)\n";

    TRegion region = mp_img->GetLargestPossibleRegion();
    region.SetIndex(origin);

    mp_img->SetRegions(region);
    }

  TSize size = img->GetLargestPossibleRegion().GetSize();

  if( m_nx + m_ny + m_nz == 0 )
    {
    m_nx = size[0];
    m_ny = size[1];
    m_nz = size[2];

    typename ImageType::SpacingType spc = img->GetSpacing();
    m_dx = spc[0];
    m_dy = spc[1];
    m_dz = spc[2];
    }
  else if( m_nx != (long)size[0] || m_ny != (long)size[1] || m_nz != (long)size[2] )
    {
    std::cerr << "image sizes do not match. abort\n";
    raise(SIGABRT);
    }

  return;
}

/* ============================================================
   setMaxVolume    */
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::setMaxVolume(double v)
{
  if( v <= 0 )
    {
    std::cerr << "Error: max volume >= 0\n";
    raise(SIGABRT);
    }

  m_maxVolume = v * 1000; // v is in mL, m_maxVolume is in mm^3

  return;
}

/* ============================================================ */
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::setMaxRunningTime(double t)
{
  if( t <= 0 )
    {
    std::cerr << "Error: t <= 0\n";
    raise(SIGABRT);
    }

  m_maxRunningTime = t * 60; // t is in min, m_maxRunningTime is in second

  return;
}

/* ============================================================
   setCurvatureWeight    */
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::setCurvatureWeight(double a)
{
  if( a < 0 )
    {
    std::cerr << "Error: curvature weight < 0\n";
    raise(SIGABRT);
    }

  m_curvatureWeight = a;

  return;
}

/* ============================================================
   setMask    */
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::setMask(typename MaskImageType::Pointer mask)
{
  mp_mask = mask;

  TSize size = mask->GetLargestPossibleRegion().GetSize();

  TIndex start = mp_mask->GetLargestPossibleRegion().GetIndex();
  TIndex origin = {{0, 0, 0}};
  if( start != origin )
    {
    std::cout << "Warrning: Force mask start to be (0, 0, 0)\n";

    TRegion region = mp_mask->GetLargestPossibleRegion();
    region.SetIndex(origin);

    mp_mask->SetRegions(region);
    }

  if( m_nx + m_ny + m_nz == 0 )
    {
    m_nx = size[0];
    m_ny = size[1];
    m_nz = size[2];
    }
  else if( m_nx != (long)size[0] || m_ny != (long)size[1] || m_nz != (long)size[2] )
    {
    std::cerr << "image sizes do not match. abort\n";
    raise(SIGABRT);
    }

  return;
}

template <typename TPixel>
bool
CSFLSSegmentor3D<TPixel>
::getPhiOfTheNbhdWhoIsClosestToZeroLevelInLayerCloserToZeroLevel(long ix, long iy, long iz, double& thePhi)
{
  /*--------------------------------------------------
   *
   * Look in all the neighbors, to find the phi value of the nbhd:
   * this nbhd should satisfy: 1. its layer is strictly closer to
   * the zero layer hence its value is thought to be updated. 2. If
   * there are several nbhd's belonging to the same layer, choose
   * the one whose phi value has the smallest abs value.  If (ix,
   * iy) is outside, go through all nbhd who is in the layer of
   * label = mylevel-1 pick the SMALLEST phi. If (ix, iy) is inside,
   * go through all nbhd who is in the layer of label = mylevel+1
   * pick the LARGEST phi.
   */
  TIndex idx = {{ix, iy, iz}};
  char   mylevel = mp_label->GetPixel(idx);

  //  char mylevel = mp_label->get(ix, iy, iz);
  bool foundNbhd = false;

  if( mylevel > 0 )
    {
    // find the SMALLEST phi
    thePhi = 10000;

    TIndex idx1 = {{ix + 1, iy, iz}};
    if( (ix + 1 < m_nx) && (mp_label->GetPixel(idx1) == mylevel - 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx1);
      thePhi = thePhi < itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }

    TIndex idx2 = {{ix - 1, iy, iz}};
    if( (ix - 1 >= 0 ) && (mp_label->GetPixel(idx2) == mylevel - 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx2);
      thePhi = thePhi < itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }

    TIndex idx3 = {{ix, iy + 1, iz}};
    if( (iy + 1 < m_ny) && (mp_label->GetPixel(idx3) == mylevel - 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx3);
      thePhi = thePhi < itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }

    TIndex idx4 = {{ix, iy - 1, iz}};
    if( ( (iy - 1) >= 0 ) && (mp_label->GetPixel(idx4) == mylevel - 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx4);
      thePhi = thePhi < itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }

    TIndex idx5 = {{ix, iy, iz + 1}};
    if( (iz + 1 < m_nz) && (mp_label->GetPixel(idx5) == mylevel - 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx5);
      thePhi = thePhi < itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }

    TIndex idx6 = {{ix, iy, iz - 1}};
    if( ( (iz - 1) >= 0 ) && (mp_label->GetPixel(idx6) == mylevel - 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx6);
      thePhi = thePhi < itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }

    }
  else
    {
    // find the LARGEST phi
    thePhi = -10000;

    TIndex idx1 = {{ix + 1, iy, iz}};
    if( (ix + 1 < m_nx) && (mp_label->GetPixel(idx1) == mylevel + 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx1);
      thePhi = thePhi > itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }

    TIndex idx2 = {{ix - 1, iy, iz}};
    if( (ix - 1 >= 0  ) && (mp_label->GetPixel(idx2) == mylevel + 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx2);
      thePhi = thePhi > itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }

    TIndex idx3 = {{ix, iy + 1, iz}};
    if( (iy + 1 < m_ny) && (mp_label->GetPixel(idx3) == mylevel + 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx3);
      thePhi = thePhi > itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }

    TIndex idx4 = {{ix, iy - 1, iz}};
    if( ( (iy - 1) >= 0 ) && (mp_label->GetPixel(idx4) == mylevel + 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx4);
      thePhi = thePhi > itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }

    TIndex idx5 = {{ix, iy, iz + 1}};
    if( (iz + 1 < m_nz) && (mp_label->GetPixel(idx5) == mylevel + 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx5);
      thePhi = thePhi > itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }

    TIndex idx6 = {{ix, iy, iz - 1}};
    if( ( (iz - 1) >= 0 ) && (mp_label->GetPixel(idx6) == mylevel + 1) )
      {
      double itsPhi = mp_phi->GetPixel(idx6);
      thePhi = thePhi > itsPhi ? thePhi : itsPhi;

      foundNbhd = true;
      }
    }

  return foundNbhd;
}

/* ============================================================
   normalizeForce
   Normalize m_force s.t. max(abs(m_force)) < 0.5 */
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::normalizeForce()
{
  unsigned long nLz = m_lz.size();

  if( m_force.size() != nLz )
    {
    std::cerr << "m_force.size() = " << m_force.size() << std::endl;
    std::cerr << "nLz = " << nLz << std::endl;

    std::cerr << "m_force.size() != nLz, abort.\n";
    raise(SIGABRT);
    }

  double fMax = fabs( m_force.front() );

  // for (std::list<double>::const_iterator itf = m_force.begin(); itf != m_force.end(); ++itf)
    {
    long nf = m_force.size();
    // for (std::list<double>::const_iterator itf = m_force.begin(); itf != m_force.end(); ++itf)
    for( long itf = 0; itf < nf; ++itf )
      {
      double v = fabs(m_force[itf]);
      fMax = fMax > v ? fMax : v;
      }
    }
  fMax /= 0.49;

    {
    long nf = m_force.size();
    // for (std::list<double>::iterator itf = m_force.begin(); itf != m_force.end(); ++itf)
    for( long itf = 0; itf < nf; ++itf )
      {
      // (*itf) /= (fMax + 1e-10);
      m_force[itf] /= (fMax + 1e-10);
      }
    }
}

/* ============================================================
   updateInsideVoxelCount    */
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::updateInsideVoxelCount()
{
  m_insideVoxelCount -= m_lIn2out.size();
  m_insideVoxelCount += m_lOut2in.size();

  m_insideVolume = m_insideVoxelCount * m_dx * m_dy * m_dz;

  return;
}

/* ============================================================
   oneStepLevelSetEvolution    */
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::oneStepLevelSetEvolution()
{
  // create 'changing status' lists
  CSFLSLayer Sz;
  CSFLSLayer Sn1;
  CSFLSLayer Sp1;
  CSFLSLayer Sn2;
  CSFLSLayer Sp2;

  m_lIn2out.clear();
  m_lOut2in.clear();

  /*--------------------------------------------------
    1. add F to phi(Lz), create Sn1 & Sp1
    scan Lz values [-2.5 -1.5)[-1.5 -.5)[-.5 .5](.5 1.5](1.5 2.5]
    ========                */
    {
    // std::list<double>::const_iterator itf = m_force.begin();

    long                              nz = m_lz.size();
    std::vector<CSFLSLayer::iterator> m_lzIterVct( nz );
      {
      long iiizzz = 0;
      for( CSFLSLayer::iterator itz = m_lz.begin(); itz != m_lz.end(); ++itz )
        {
        m_lzIterVct[iiizzz++] = itz;
        }
      }
    //    for (CSFLSLayer::iterator itz = m_lz.begin(); itz != m_lz.end(); ++itf)
    // #pragma omp parallel for
    for( long iiizzz = 0; iiizzz < nz; ++iiizzz )
      {
      long itf = iiizzz;

      CSFLSLayer::iterator itz = m_lzIterVct[iiizzz];

      long ix = (*itz)[0];
      long iy = (*itz)[1];
      long iz = (*itz)[2];

      TIndex idx = {{ix, iy, iz}};

      double phi_old = mp_phi->GetPixel(idx);
      double phi_new = phi_old + m_force[itf];

      /*----------------------------------------------------------------------
        Update the lists of pt who change the state, for faster
        energy fnal computation. */
      if( phi_old <= 0 && phi_new > 0 )
        {
        m_lIn2out.push_back(NodeType(ix, iy, iz) );
        }

      if( phi_old > 0  && phi_new <= 0 )
        {
        m_lOut2in.push_back(NodeType(ix, iy, iz) );
        }

      //           // DEBUG
      //           if (phi_new > 3.1 || phi_new < -3.1)
      //             {
      //               std::cout<<"phi_old = "<<phi_old<<std::endl;
      //               std::cout<<"its lbl = "<<(int)mp_label->get(ix, iy)<<std::endl;

      //               std::cerr<<"phi_new > 3.1 || phi_new < -3.1\n";
      //               raise(SIGABRT);
      //             }

      mp_phi->SetPixel(idx, phi_new);

      if( phi_new > 0.5 )
        {
        Sp1.push_back(*itz);
        itz = m_lz.erase(itz);
        }
      else if( phi_new < -0.5 )
        {
        Sn1.push_back(*itz);
        itz = m_lz.erase(itz);
        }
      else
        {
        ++itz;
        }
      /*--------------------------------------------------
        NOTE, mp_label are (should) NOT update here. They should
        be updated with Sz, Sn/p's
        --------------------------------------------------*/
      }
    }

  //     // debug
  //     labelsCoherentCheck1();
  /*--------------------------------------------------
    2. update Ln1,Lp1,Lp2,Lp2, ****in that order****

    2.1 scan Ln1 values [-2.5 -1.5)[-1.5 -.5)[-.5 .5](.5 1.5](1.5 2.5]
    ==========                     */
  for( CSFLSLayer::iterator itn1 = m_ln1.begin(); itn1 != m_ln1.end(); )
    {
    long ix = (*itn1)[0];
    long iy = (*itn1)[1];
    long iz = (*itn1)[2];

    TIndex idx = {{ix, iy, iz}};

    double thePhi;
    bool   found = getPhiOfTheNbhdWhoIsClosestToZeroLevelInLayerCloserToZeroLevel(ix, iy, iz, thePhi);

    if( found )
      {
      double phi_new = thePhi - 1;
      mp_phi->SetPixel(idx, phi_new);

      if( phi_new >= -0.5 )
        {
        Sz.push_back(*itn1);
        itn1 = m_ln1.erase(itn1);
        }
      else if( phi_new < -1.5 )
        {
        Sn2.push_back(*itn1);
        itn1 = m_ln1.erase(itn1);
        }
      else
        {
        ++itn1;
        }
      }
    else
      {
      /*--------------------------------------------------
        No nbhd in inner (closer to zero contour) layer, so
        should go to Sn2. And the phi shold be further -1
      */
      Sn2.push_back(*itn1);
      itn1 = m_ln1.erase(itn1);

      mp_phi->SetPixel(idx, mp_phi->GetPixel(idx) - 1);
      }
    }

  //     // debug
  //     labelsCoherentCheck1();
  /*--------------------------------------------------
    2.2 scan Lp1 values [-2.5 -1.5)[-1.5 -.5)[-.5 .5](.5 1.5](1.5 2.5]
    ========          */
  for( CSFLSLayer::iterator itp1 = m_lp1.begin(); itp1 != m_lp1.end(); )
    {
    long ix = (*itp1)[0];
    long iy = (*itp1)[1];
    long iz = (*itp1)[2];

    TIndex idx = {{ix, iy, iz}};

    double thePhi;
    bool   found = getPhiOfTheNbhdWhoIsClosestToZeroLevelInLayerCloserToZeroLevel(ix, iy, iz, thePhi);

    if( found )
      {
      double phi_new = thePhi + 1;
      mp_phi->SetPixel(idx, phi_new);

      if( phi_new <= 0.5 )
        {
        Sz.push_back(*itp1);
        itp1 = m_lp1.erase(itp1);
        }
      else if( phi_new > 1.5 )
        {
        Sp2.push_back(*itp1);
        itp1 = m_lp1.erase(itp1);
        }
      else
        {
        ++itp1;
        }
      }
    else
      {
      /*--------------------------------------------------
        No nbhd in inner (closer to zero contour) layer, so
        should go to Sp2. And the phi shold be further +1
      */

      Sp2.push_back(*itp1);
      itp1 = m_lp1.erase(itp1);

      mp_phi->SetPixel(idx, mp_phi->GetPixel(idx) + 1);
      }
    }

  //     // debug
  //     labelsCoherentCheck1();
  /*--------------------------------------------------
    2.3 scan Ln2 values [-2.5 -1.5)[-1.5 -.5)[-.5 .5](.5 1.5](1.5 2.5]
    ==========                                      */
  for( CSFLSLayer::iterator itn2 = m_ln2.begin(); itn2 != m_ln2.end(); )
    {
    long ix = (*itn2)[0];
    long iy = (*itn2)[1];
    long iz = (*itn2)[2];

    TIndex idx = {{ix, iy, iz}};

    double thePhi;
    bool   found = getPhiOfTheNbhdWhoIsClosestToZeroLevelInLayerCloserToZeroLevel(ix, iy, iz, thePhi);

    if( found )
      {
      double phi_new = thePhi - 1;
      mp_phi->SetPixel(idx, phi_new);

      if( phi_new >= -1.5 )
        {
        Sn1.push_back(*itn2);
        itn2 = m_ln2.erase(itn2);
        }
      else if( phi_new < -2.5 )
        {
        itn2 = m_ln2.erase(itn2);
        mp_phi->SetPixel(idx, -3);
        mp_label->SetPixel(idx, -3);
        }
      else
        {
        ++itn2;
        }
      }
    else
      {
      itn2 = m_ln2.erase(itn2);
      mp_phi->SetPixel(idx, -3);
      mp_label->SetPixel(idx, -3);
      }
    }

  //     // debug
  //     labelsCoherentCheck1();
  /*--------------------------------------------------
    2.4 scan Lp2 values [-2.5 -1.5)[-1.5 -.5)[-.5 .5](.5 1.5](1.5 2.5]
    ========= */
  for( CSFLSLayer::iterator itp2 = m_lp2.begin(); itp2 != m_lp2.end(); )
    {
    long   ix = (*itp2)[0];
    long   iy = (*itp2)[1];
    long   iz = (*itp2)[2];
    TIndex idx = {{ix, iy, iz}};

    double thePhi;
    bool   found = getPhiOfTheNbhdWhoIsClosestToZeroLevelInLayerCloserToZeroLevel(ix, iy, iz, thePhi);

    if( found )
      {
      double phi_new = thePhi + 1;
      mp_phi->SetPixel(idx, phi_new);

      if( phi_new <= 1.5 )
        {
        Sp1.push_back(*itp2);
        itp2 = m_lp2.erase(itp2);
        }
      else if( phi_new > 2.5 )
        {
        itp2 = m_lp2.erase(itp2);
        mp_phi->SetPixel(idx, 3);
        mp_label->SetPixel(idx, 3);
        }
      else
        {
        ++itp2;
        }
      }
    else
      {
      itp2 = m_lp2.erase(itp2);
      mp_phi->SetPixel(idx, 3);
      mp_label->SetPixel(idx, 3);
      }
    }

  //     // debug
  //     labelsCoherentCheck1();
  /*--------------------------------------------------
    3. Deal with S-lists Sz,Sn1,Sp1,Sn2,Sp2
    3.1 Scan Sz */
  for( CSFLSLayer::iterator itSz = Sz.begin(); itSz != Sz.end(); ++itSz )
    {
    long   ix = (*itSz)[0];
    long   iy = (*itSz)[1];
    long   iz = (*itSz)[2];
    TIndex idx = {{ix, iy, iz}};

    m_lz.push_back(*itSz);
    mp_label->SetPixel(idx, 0);
    }

  //     // debug
  //     labelsCoherentCheck1();
  /*--------------------------------------------------
    3.2 Scan Sn1     */
  for( CSFLSLayer::iterator itSn1 = Sn1.begin(); itSn1 != Sn1.end(); ++itSn1 )
    {
    long ix = (*itSn1)[0];
    long iy = (*itSn1)[1];
    long iz = (*itSn1)[2];

    TIndex idx = {{ix, iy, iz}};

    m_ln1.push_back(*itSn1);
    // itSn1 = Sn1.erase(itSn1);

    mp_label->SetPixel(idx, -1);

    TIndex idx1 = {{ix + 1, iy, iz}};
    if( (ix + 1 < m_nx) && doubleEqual(mp_phi->GetPixel(idx1), -3.0) )
      {
      Sn2.push_back(NodeType(ix + 1, iy, iz) );
      mp_phi->SetPixel(idx1, mp_phi->GetPixel(idx) - 1 );
      }

    TIndex idx2 = {{ix - 1, iy, iz}};
    if( (ix - 1 >= 0) && doubleEqual(mp_phi->GetPixel(idx2), -3.0) )
      {
      Sn2.push_back(NodeType(ix - 1, iy, iz) );
      mp_phi->SetPixel(idx2, mp_phi->GetPixel(idx) - 1);
      }

    TIndex idx3 = {{ix, iy + 1, iz}};
    if( (iy + 1 < m_ny) && doubleEqual(mp_phi->GetPixel(idx3), -3.0) )
      {
      Sn2.push_back(NodeType(ix, iy + 1, iz) );
      mp_phi->SetPixel(idx3, mp_phi->GetPixel(idx) - 1 );
      }

    TIndex idx4 = {{ix, iy - 1, iz}};
    if( (iy - 1 >= 0) && doubleEqual(mp_phi->GetPixel(idx4), -3.0) )
      {
      Sn2.push_back(NodeType(ix, iy - 1, iz) );
      mp_phi->SetPixel(idx4, mp_phi->GetPixel(idx) - 1 );
      }

    TIndex idx5 = {{ix, iy, iz + 1}};
    if( (iz + 1 < m_nz) && doubleEqual(mp_phi->GetPixel(idx5), -3.0) )
      {
      Sn2.push_back(NodeType(ix, iy, iz + 1) );
      mp_phi->SetPixel(idx5, mp_phi->GetPixel(idx) - 1 );
      }

    TIndex idx6 = {{ix, iy, iz - 1}};
    if( (iz - 1 >= 0) && doubleEqual(mp_phi->GetPixel(idx6), -3.0) )
      {
      Sn2.push_back(NodeType(ix, iy, iz - 1) );
      mp_phi->SetPixel(idx6, mp_phi->GetPixel(idx) - 1 );
      }
    }

  //     // debug
  //     labelsCoherentCheck1();
  /*--------------------------------------------------
    3.3 Scan Sp1     */
  for( CSFLSLayer::iterator itSp1 = Sp1.begin(); itSp1 != Sp1.end(); ++itSp1 )
    {
    long ix = (*itSp1)[0];
    long iy = (*itSp1)[1];
    long iz = (*itSp1)[2];

    TIndex idx = {{ix, iy, iz}};

    m_lp1.push_back(*itSp1);
    mp_label->SetPixel(idx, 1);

    TIndex idx1 = {{ix + 1, iy, iz}};
    if( (ix + 1 < m_nx) && doubleEqual(mp_phi->GetPixel(idx1), 3.0) )
      {
      Sp2.push_back(NodeType(ix + 1, iy, iz) );
      mp_phi->SetPixel(idx1, mp_phi->GetPixel(idx) + 1 );
      }

    TIndex idx2 = {{ix - 1, iy, iz}};
    if( (ix - 1 >= 0) && doubleEqual(mp_phi->GetPixel(idx2), 3.0) )
      {
      Sp2.push_back(NodeType(ix - 1, iy, iz) );
      mp_phi->SetPixel(idx2, mp_phi->GetPixel(idx) + 1);
      }

    TIndex idx3 = {{ix, iy + 1, iz}};
    if( (iy + 1 < m_ny) && doubleEqual(mp_phi->GetPixel(idx3), 3.0) )
      {
      Sp2.push_back(NodeType(ix, iy + 1, iz) );
      mp_phi->SetPixel(idx3, mp_phi->GetPixel(idx) + 1 );
      }

    TIndex idx4 = {{ix, iy - 1, iz}};
    if( (iy - 1 >= 0) && doubleEqual(mp_phi->GetPixel(idx4), 3.0) )
      {
      Sp2.push_back(NodeType(ix, iy - 1, iz) );
      mp_phi->SetPixel(idx4, mp_phi->GetPixel(idx) + 1 );
      }

    TIndex idx5 = {{ix, iy, iz + 1}};
    if( (iz + 1 < m_nz) && doubleEqual(mp_phi->GetPixel(idx5), 3.0) )
      {
      Sp2.push_back(NodeType(ix, iy, iz + 1) );
      mp_phi->SetPixel(idx5, mp_phi->GetPixel(idx) + 1 );
      }

    TIndex idx6 = {{ix, iy, iz - 1}};
    if( (iz - 1 >= 0) && doubleEqual(mp_phi->GetPixel(idx6), 3.0) )
      {
      Sp2.push_back(NodeType(ix, iy, iz - 1) );
      mp_phi->SetPixel(idx6, mp_phi->GetPixel(idx) + 1 );
      }
    }

  //     // debug
  //     labelsCoherentCheck1();

  /*--------------------------------------------------
    3.4 Scan Sn2     */
    {
    // debug
    int aaa = 0;
    for( CSFLSLayer::iterator itSn2 = Sn2.begin(); itSn2 != Sn2.end(); ++itSn2, ++aaa )
      {
      long ix = (*itSn2)[0];
      long iy = (*itSn2)[1];
      long iz = (*itSn2)[2];

      TIndex idx = {{ix, iy, iz}};

      m_ln2.push_back(*itSn2);

      mp_label->SetPixel(idx, -2);

      //           // debug
      //           labelsCoherentCheck1();
      }
    }
  /*--------------------------------------------------
    3.5 Scan Sp2     */
  for( CSFLSLayer::iterator itSp2 = Sp2.begin(); itSp2 != Sp2.end(); ++itSp2 )
    {
    long ix = (*itSp2)[0];
    long iy = (*itSp2)[1];
    long iz = (*itSp2)[2];

    TIndex idx = {{ix, iy, iz}};

    m_lp2.push_back(*itSp2);

    mp_label->SetPixel(idx, 2);
    }

  //     // debug
  //     labelsCoherentCheck1();

}

/*================================================================================
  initializeLabel*/
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::initializeLabel()
{
  if( m_nx + m_ny + m_nz == 0 )
    {
    std::cerr << "set mp_img first.\n";
    raise(SIGABRT);
    }

  // find interface and mark as 0, create Lz
  char defaultLabel = 0;
  //  mp_label.reset(new cArray3D< char >(m_nx, m_ny, m_nz, defaultLabel) );

  mp_label = LabelImageType::New();
  TRegion region = mp_img->GetLargestPossibleRegion();

  mp_label->SetRegions( region );
  mp_label->Allocate();

  mp_label->CopyInformation(mp_img);

  mp_label->FillBuffer(defaultLabel);

  return;
}

/*================================================================================
  initializePhi*/
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::initializePhi()
{
  if( m_nx + m_ny + m_nz == 0 )
    {
    std::cerr << "set mp_img first.\n";
    raise(SIGABRT);
    }

  double arbitraryInitPhi = 1000;
  //  mp_phi.reset(new cArray3D< double >(m_nx, m_ny, m_nz, arbitraryInitPhi) );

  mp_phi = LSImageType::New();
  TRegion region = mp_img->GetLargestPossibleRegion();

  mp_phi->SetRegions( region );
  mp_phi->Allocate();

  mp_phi->CopyInformation(mp_img);

  mp_phi->FillBuffer(arbitraryInitPhi);

  return;
}

/* ============================================================
   initializeSFLSFromMask    */
template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::initializeSFLSFromMask()
{
  if( !mp_mask )
    {
    std::cerr << "set mp_mask first.\n";
    raise(SIGABRT);
    }

  initializePhi();
  initializeLabel();

//   //debug//
//   //  std::cout<<"[m_nx, m_ny, m_nz] = "<<"[ "<<m_nx<<", "<<m_ny<<", "<<m_nz<<"]\n";
//   typedef itk::ImageRegionIteratorWithIndex<MaskImageType> TImageRegionIteratorWithIndex;
//   TImageRegionIteratorWithIndex it(mp_mask, mp_mask->GetLargestPossibleRegion() );
//   it.GoToBegin();
//   std::cout<<it.GetIndex()<<std::endl;

//   exit(0);
//   //DEBUG//

//   typename MaskImageType::IndexType start = mp_mask->GetLargestPossibleRegion().GetIndex();
  for( long iz = 0; iz < m_nz; ++iz )
    {
    for( long iy = 0; iy < m_ny; ++iy )
      {
      for( long ix = 0; ix < m_nx; ++ix )
        {
        TIndex idx = {{ix, iy, iz}};
        TIndex idx1 = {{ix - 1, iy, iz}};
        TIndex idx2 = {{ix + 1, iy, iz}};
        TIndex idx3 = {{ix, iy - 1, iz}};
        TIndex idx4 = {{ix, iy + 1, iz}};
        TIndex idx5 = {{ix, iy, iz - 1}};
        TIndex idx6 = {{ix, iy, iz + 1}};

        // mark the inside and outside of label and phi
        if( mp_mask->GetPixel(idx) == 0 )
          {
          mp_label->SetPixel(idx, 3);
          mp_phi->SetPixel(idx, 3);
          }
        else
          {
          mp_label->SetPixel(idx, -3);
          mp_phi->SetPixel(idx, -3);

          ++m_insideVoxelCount;

          if( (iy + 1 < m_ny && mp_mask->GetPixel(idx4) == 0)    \
              || (iy - 1 >= 0 && mp_mask->GetPixel(idx3) == 0)   \
              || (ix + 1 < m_nx && mp_mask->GetPixel(idx2) == 0) \
              || (ix - 1 >= 0 && mp_mask->GetPixel(idx1) == 0)   \
              || (iz + 1 < m_nz && mp_mask->GetPixel(idx6) == 0) \
              || (iz - 1 >= 0 && mp_mask->GetPixel(idx5) == 0) )
            {
            m_lz.push_back(NodeType(ix, iy, iz) );

            mp_label->SetPixel(idx, 0);
            mp_phi->SetPixel(idx, 0.0);
            }
          }
        }
      }
    }

  m_insideVolume = m_insideVoxelCount * m_dx * m_dy * m_dz;

//   //debug//
//   douher::writeImage3<char>(mp_label, "label.mha");
//   douher::writeImage3<unsigned char>(mp_mask, "mask.mha");
//   douher::writeImage3<double>(mp_phi, "phi.mha");
//   exit(0);
//   //DEBUG//
// scan Lz to create Ln1 and Lp1
  for( CSFLSLayer::const_iterator it = m_lz.begin(); it != m_lz.end(); ++it )
    {
    long ix = (*it)[0];
    long iy = (*it)[1];
    long iz = (*it)[2];

    if( ix + 1 < m_nx )
      {
      TIndex idx = {{ix + 1, iy, iz}};

      if( mp_label->GetPixel(idx) == 3 )
        {
        mp_label->SetPixel(idx, 1);
        mp_phi->SetPixel(idx, 1);

        m_lp1.push_back( NodeType(ix + 1, iy, iz) );
        }
      else if( mp_label->GetPixel(idx) == -3 )
        {
        mp_label->SetPixel(idx, -1);
        mp_phi->SetPixel(idx, -1);

        m_ln1.push_back( NodeType(ix + 1, iy, iz) );
        }
      }

    if( ix - 1 >= 0 )
      {
      TIndex idx = {{ix - 1, iy, iz}};

      if( mp_label->GetPixel(idx) == 3 )
        {
        mp_label->SetPixel(idx, 1);
        mp_phi->SetPixel(idx, 1);

        m_lp1.push_back( NodeType(ix - 1, iy, iz) );
        }
      else if( mp_label->GetPixel(idx) == -3 )
        {
        mp_label->SetPixel(idx, -1);
        mp_phi->SetPixel(idx, -1);

        m_ln1.push_back( NodeType(ix - 1, iy, iz) );
        }
      }

    if( iy + 1 < m_ny )
      {
      TIndex idx = {{ix, iy + 1, iz}};

      if( mp_label->GetPixel(idx) == 3 )
        {
        mp_label->SetPixel(idx, 1);
        mp_phi->SetPixel(idx, 1);

        m_lp1.push_back(NodeType(ix, iy + 1, iz) );
        }
      else if( mp_label->GetPixel(idx) == -3 )
        {
        mp_label->SetPixel(idx, -1);
        mp_phi->SetPixel(idx, -1);

        m_ln1.push_back( NodeType(ix, iy + 1, iz) );
        }
      }

    if( iy - 1 >= 0 )
      {
      TIndex idx = {{ix, iy - 1, iz}};

      if( mp_label->GetPixel(idx) == 3 )
        {
        mp_label->SetPixel(idx, 1);
        mp_phi->SetPixel(idx, 1);

        m_lp1.push_back( NodeType(ix, iy - 1, iz) );
        }
      else if( mp_label->GetPixel(idx) == -3 )
        {
        mp_label->SetPixel(idx, -1);
        mp_phi->SetPixel(idx, -1);

        m_ln1.push_back( NodeType(ix, iy - 1, iz) );
        }
      }

    if( iz + 1 < m_nz )
      {
      TIndex idx = {{ix, iy, iz + 1}};

      if( mp_label->GetPixel(idx) == 3 )
        {
        mp_label->SetPixel(idx, 1);
        mp_phi->SetPixel(idx, 1);

        m_lp1.push_back(NodeType(ix, iy, iz + 1) );
        }
      else if( mp_label->GetPixel(idx) == -3 )
        {
        mp_label->SetPixel(idx, -1);
        mp_phi->SetPixel(idx, -1);

        m_ln1.push_back( NodeType(ix, iy, iz + 1) );
        }
      }

    if( iz - 1 >= 0 )
      {
      TIndex idx = {{ix, iy, iz - 1}};

      if( mp_label->GetPixel(idx) == 3 )
        {
        mp_label->SetPixel(idx, 1);
        mp_phi->SetPixel(idx, 1);

        m_lp1.push_back( NodeType(ix, iy, iz - 1) );
        }
      else if( mp_label->GetPixel(idx) == -3 )
        {
        mp_label->SetPixel(idx, -1);
        mp_phi->SetPixel(idx, -1);

        m_ln1.push_back( NodeType(ix, iy, iz - 1) );
        }
      }
    }
  // scan Ln1 to create Ln2
  for( CSFLSLayer::const_iterator it = m_ln1.begin(); it != m_ln1.end(); ++it )
    {
    long ix = (*it)[0];
    long iy = (*it)[1];
    long iz = (*it)[2];

    TIndex idx1 = {{ix + 1, iy, iz}};
    if( ix + 1 < m_nx && mp_label->GetPixel(idx1) == -3 )
      {
      mp_label->SetPixel(idx1, -2);
      mp_phi->SetPixel(idx1, -2);

      m_ln2.push_back( NodeType(ix + 1, iy, iz) );
      }

    TIndex idx2 = {{ix - 1, iy, iz}};
    if( ix - 1 >= 0 && mp_label->GetPixel(idx2) == -3 )
      {
      mp_label->SetPixel(idx2, -2);
      mp_phi->SetPixel(idx2, -2);

      m_ln2.push_back( NodeType(ix - 1, iy, iz) );
      }

    TIndex idx3 = {{ix, iy + 1, iz}};
    if( iy + 1 < m_ny && mp_label->GetPixel(idx3) == -3 )
      {
      mp_label->SetPixel(idx3, -2);
      mp_phi->SetPixel(idx3, -2);

      m_ln2.push_back( NodeType(ix, iy + 1, iz) );
      }

    TIndex idx4 = {{ix, iy - 1, iz}};
    if( iy - 1 >= 0 && mp_label->GetPixel(idx4) == -3 )
      {
      mp_label->SetPixel(idx4, -2);
      mp_phi->SetPixel(idx4, -2);

      m_ln2.push_back( NodeType(ix, iy - 1, iz) );
      }

    TIndex idx5 = {{ix, iy, iz + 1}};
    if( iz + 1 < m_nz && mp_label->GetPixel(idx5) == -3 )
      {
      mp_label->SetPixel(idx5, -2);
      mp_phi->SetPixel(idx5, -2);

      m_ln2.push_back( NodeType(ix, iy, iz + 1) );
      }

    TIndex idx6 = {{ix, iy, iz - 1}};
    if( iz - 1 >= 0 && mp_label->GetPixel(idx6) == -3 )
      {
      mp_label->SetPixel(idx6, -2);
      mp_phi->SetPixel(idx6, -2);

      m_ln2.push_back( NodeType(ix, iy, iz - 1) );
      }
    }
  // scan Lp1 to create Lp2
  for( CSFLSLayer::const_iterator it = m_lp1.begin(); it != m_lp1.end(); ++it )
    {
    long ix = (*it)[0];
    long iy = (*it)[1];
    long iz = (*it)[2];

    TIndex idx1 = {{ix + 1, iy, iz}};
    if( ix + 1 < m_nx && mp_label->GetPixel(idx1) == 3 )
      {
      mp_label->SetPixel(idx1, 2);
      mp_phi->SetPixel(idx1, 2);

      m_lp2.push_back( NodeType(ix + 1, iy, iz) );
      }

    TIndex idx2 = {{ix - 1, iy, iz}};
    if( ix - 1 >= 0 && mp_label->GetPixel(idx2) == 3 )
      {
      mp_label->SetPixel(idx2, 2);
      mp_phi->SetPixel(idx2, 2);

      m_lp2.push_back( NodeType(ix - 1, iy, iz) );
      }

    TIndex idx3 = {{ix, iy + 1, iz}};
    if( iy + 1 < m_ny && mp_label->GetPixel(idx3) == 3 )
      {
      mp_label->SetPixel(idx3, 2);
      mp_phi->SetPixel(idx3, 2);

      m_lp2.push_back( NodeType(ix, iy + 1, iz) );
      }

    TIndex idx4 = {{ix, iy - 1, iz}};
    if( iy - 1 >= 0 && mp_label->GetPixel(idx4) == 3 )
      {
      mp_label->SetPixel(idx4, 2);
      mp_phi->SetPixel(idx4, 2);

      m_lp2.push_back( NodeType(ix, iy - 1, iz) );
      }

    TIndex idx5 = {{ix, iy, iz + 1}};
    if( iz + 1 < m_nz && mp_label->GetPixel(idx5) == 3 )
      {
      mp_label->SetPixel(idx5, 2);
      mp_phi->SetPixel(idx5, 2);

      m_lp2.push_back( NodeType(ix, iy, iz + 1) );
      }

    TIndex idx6 = {{ix, iy, iz - 1}};
    if( iz - 1 >= 0 && mp_label->GetPixel(idx6) == 3 )
      {
      mp_label->SetPixel(idx6, 2);
      mp_phi->SetPixel(idx6, 2);

      m_lp2.push_back( NodeType(ix, iy, iz - 1) );
      }
    }
}

// /* ============================================================
//    doSegmenation    */
// template< typename TPixel >
// void
// CSFLSSegmentor3D< TPixel >
// ::doSegmenation()
// {
// //   double arbitraryInitPhi = 1000;
// //   mp_phi.reset(new cArray3D< double >(m_nx, m_ny, m_nz, arbitraryInitPhi) );

//   // douher::saveAsImage3< double >(mp_phi, "init0.nrrd");

//   /*============================================================
//    * From the initial mask, generate: 1. SFLS, 2. mp_label and
//    * 3. mp_phi.
//    */
//   initializeSFLS();

//   //douher::saveAsImage3< double >(mp_phi, "initPhi.nrrd");

//   for (unsigned int it = 0; it < m_numIter; ++it)
//     {
//       std::cout<<"iteration "<<it<<"\n"<<std::flush;

//       /*--------------------------------------------------
//         Compute the force on the zero level set, NOT on the whole domain.
//         This is NOT implemented in this base class.

//         This function will compute the m_force. m_force has the same
//         size as the m_ln, indicating the change at each pixel on the
//         zero level set.
//       */
//       computeForce();

//       normalizeForce();

//       //         // debug
//       //         for (std::list< double >::const_iterator itf = this->m_force.begin(); itf != this->m_force.end();
// ++itf)
//       //           {
//       //             std::cout<<(*itf)<<", ";
//       //           }
//       //         std::cout<<std::endl<<it<<std::endl<<std::endl;

//       //         //debug//
//       //         labelsCoherentCheck1();

//       oneStepLevelSetEvolution();

//       //         //debug//
//       //         std::cout<<"-----------------------"<<it<<"---------------------------"<<std::endl;
//       //         std::cout<<"lz \t ln1 \t ln2 \t lp1 \t lp2 \n";
//       //
//
//
//
//
//
//    std::cout<<m_lz.size()<<"\t"<<m_ln1.size()<<"\t"<<m_ln2.size()<<"\t"<<m_lp1.size()<<"\t"<<m_lp2.size()<<std::endl;
//       //         std::cout<<"--------------------------------------------------"<<std::endl;

//       //         // debug
//       //         labelsCoherentCheck1();

//       //        douher::saveAsImage3< double >(mp_phi, "temp.nrrd");

// updateInsideVoxelCount();
//     }
// }

/* getLevelSetFunction */
template <typename TPixel>
itk::Image<float, 3>::Pointer
CSFLSSegmentor3D<TPixel>
::getLevelSetFunction()
{
//   if (!m_done)
//     {
//       std::cerr<<"Error: not done.\n";
//       raise(SIGABRT);
//     }

  return mp_phi;
}

/*============================================================
  computeKappa

  Compute kappa at a point in the zero level set  */
template <typename TPixel>
double
CSFLSSegmentor3D<TPixel>
::computeKappa(long ix, long iy, long iz)
{
  // double kappa = 0;

  double dx = 0;
  double dy = 0;
  double dz = 0;

  double dxx = 0;
  double dyy = 0;
  double dzz = 0;

  double dx2 = 0;
  double dy2 = 0;
  double dz2 = 0;

  double dxy = 0;
  double dxz = 0;
  double dyz = 0;

  char xok = 0;
  char yok = 0;
  char zok = 0;

  TIndex idx = {{ix, iy, iz}};
  TIndex idx1 = {{ix - 1, iy, iz}};
  TIndex idx2 = {{ix + 1, iy, iz}};
  TIndex idx3 = {{ix, iy - 1, iz}};
  TIndex idx4 = {{ix, iy + 1, iz}};
  TIndex idx5 = {{ix, iy, iz - 1}};
  TIndex idx6 = {{ix, iy, iz + 1}};

  if( ix + 1 < m_nx && ix - 1 >= 0 )
    {
    xok = 1;
    }

  if( iy + 1 < m_ny && iy - 1 >= 0 )
    {
    yok = 1;
    }

  if( iz + 1 < m_nz && iz - 1 >= 0 )
    {
    zok = 1;
    }

  if( xok )
    {
    dx  = (mp_phi->GetPixel(idx2) - mp_phi->GetPixel(idx1) ) / (2.0 * m_dx);
    dxx = (mp_phi->GetPixel(idx2) - 2.0 * (mp_phi->GetPixel(idx) ) + mp_phi->GetPixel(idx1) ) / (m_dx * m_dx);
    dx2 = dx * dx;
    }

  if( yok )
    {
    dy  = ( (mp_phi->GetPixel(idx4) - mp_phi->GetPixel(idx3) ) ) / (2.0 * m_dy);
    dyy = (mp_phi->GetPixel(idx4) - 2 * (mp_phi->GetPixel(idx) ) + mp_phi->GetPixel(idx3) ) / (m_dy * m_dy);
    dy2 = dy * dy;
    }

  if( zok )
    {
    dz  = ( (mp_phi->GetPixel(idx6) - mp_phi->GetPixel(idx5) ) ) / (2.0 * m_dz);
    dzz = (mp_phi->GetPixel(idx6) - 2.0 * (mp_phi->GetPixel(idx) ) + mp_phi->GetPixel(idx5) ) / (m_dz * m_dz);
    dz2 = dz * dz;
    }

  if( xok && yok )
    {
    TIndex idx_1 = {{ix + 1, iy + 1, iz}};
    TIndex idx_2 = {{ix - 1, iy - 1, iz}};
    TIndex idx_3 = {{ix + 1, iy - 1, iz}};
    TIndex idx_4 = {{ix - 1, iy + 1, iz}};

    dxy = 0.25 * (mp_phi->GetPixel(idx_1) + mp_phi->GetPixel(idx_2) - mp_phi->GetPixel(idx_3) - mp_phi->GetPixel(idx_4) ) \
      / (m_dx * m_dy);
    }

  if( xok && zok )
    {
    TIndex idx_1 = {{ix + 1, iy, iz + 1}};
    TIndex idx_2 = {{ix - 1, iy, iz - 1}};
    TIndex idx_3 = {{ix + 1, iy, iz - 1}};
    TIndex idx_4 = {{ix - 1, iy, iz + 1}};

    dxz = 0.25 * (mp_phi->GetPixel(idx_1) + mp_phi->GetPixel(idx_2) - mp_phi->GetPixel(idx_3) - mp_phi->GetPixel(idx_4) ) \
      / (m_dx * m_dz);
    }

  if( yok && zok )
    {
    TIndex idx_1 = {{ix, iy + 1, iz + 1}};
    TIndex idx_2 = {{ix, iy - 1, iz - 1}};
    TIndex idx_3 = {{ix, iy + 1, iz - 1}};
    TIndex idx_4 = {{ix, iy - 1, iz + 1}};

    dyz = 0.25 * (mp_phi->GetPixel(idx_1) + mp_phi->GetPixel(idx_2) - mp_phi->GetPixel(idx_3) - mp_phi->GetPixel(idx_4) ) \
      / (m_dy * m_dz);
    }

  return (dxx
          * (dy2
             + dz2) + dyy * (dx2 + dz2) + dzz * (dx2 + dy2) - 2 * dx * dy * dxy - 2 * dx * dz * dxz - 2 * dy * dz * dyz) \
         / (dx2 + dy2 + dz2 + vnl_math::eps);
}

template <typename TPixel>
typename CSFLSSegmentor3D<TPixel>::CSFLSLayer
CSFLSSegmentor3D<TPixel>
::getZeroLayerAtIteration(unsigned long i)
{
  if( !m_keepZeroLayerHistory )
    {
    std::cerr << "Error: no history stored.";
    std::cerr << "By default, they are not. Set keepZeroLayerHistory(true) *before* doSegmentation to keep history.\n";
    raise(SIGABRT);
    }
  else if( i >= m_numIter )
    {
    std::cerr << "Error: history requested, " << i << ", exceeds number of records, " << m_numIter << std::endl;
    raise(SIGABRT);
    }
  else
    {
    // return the i-th zero-layer
    return m_zeroLayerHistory[i];
    }
}

template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::writeZeroLayerAtIterationToFile(unsigned long i, const char* name)
{
  if( !m_keepZeroLayerHistory )
    {
    std::cerr << "Error: no history stored.";
    std::cerr << "By default, they are not. Set keepZeroLayerHistory(true) *before* doSegmentation to keep history.\n";
    raise(SIGABRT);
    }
  else if( i >= m_numIter )
    {
    std::cerr << "Error: history requested, " << i << ", exceeds number of records, " << m_numIter << std::endl;
    raise(SIGABRT);
    }
  else
    {
    // write it out
    const CSFLSLayer& requestedHistory = m_zeroLayerHistory[i];

    std::ofstream f(name);

    typename ImageType::PointType physicalPoint;
    typename ImageType::IndexType index;
    for( CSFLSLayer::const_iterator itz = requestedHistory.begin(); itz != requestedHistory.end(); ++itz )
      {
//           /* output ijk */
//           f<<*itz;
//           /* _output ijk */

      /* output physical points */
      index[0] = (*itz)[0];
      index[1] = (*itz)[1];
      index[2] = (*itz)[2];

      mp_img->TransformIndexToPhysicalPoint(index, physicalPoint);
      // the returned physical coord is in physicalPoint, but in
      // RAS, but Slicer displays LPS, so add - at the first two coords:
      f << -physicalPoint[0] << " " << -physicalPoint[1] << " " << physicalPoint[2] << "\n";
      /* _output physical points */

      }
    f.close();
    }

  return;
}

template <typename TPixel>
void
CSFLSSegmentor3D<TPixel>
::writeZeroLayerToFile(const char* namePrefix)
{
  for( unsigned long i = 0; i < m_numIter; ++i )
    {
    char thisName[1000];
    sprintf(thisName, "%s_%ld.layer", namePrefix, i);
    writeZeroLayerAtIterationToFile(i, thisName);
    }

  return;
}

#endif
