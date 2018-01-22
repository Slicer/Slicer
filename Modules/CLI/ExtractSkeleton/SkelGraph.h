/*=========================================================================

  Program:   Extract Skeleton
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef _SKEL_GRAPH_H_
#define _SKEL_GRAPH_H_

#include <deque>
#include <list>
#include "coordTypes.h"

struct skel_branch
  {
  skel_branch()
  {
    branchID = -1;
    length = 0;
    acc_length = 0;
    max_path_length = 0;
  }
  int branchID;     // == position in m_Graph
  double length; // length between end points
  std::deque<Coord3i> points;

  double acc_length;  // for temporary use when searching maximal path
  std::deque<int> acc_path;

  double max_path_length;
  std::deque<int> max_path;         // maximal path

  Coord3i end_1_point;
  Coord3i end_2_point;
  std::deque<int> end_1_neighbors; // id's == one can use advance for random access
  std::deque<int> end_2_neighbors;
  };

class SkelGraph
{
public:
  SkelGraph();
  virtual ~SkelGraph();

  // Print info on all m_Graph nodes to standard output
  void PrintGraph();

  // Extract skeletal m_Graph
  // Limitation: currently image spacing is not taken into account.
  // If image spacing is highly anisotropic then longest path computation
  // may not give optimal results, and sampling distance (when calling
  // SampleAlongMaximalPath) may be uneven.
  void ExtractSkeletalGraph(const unsigned char *image, const int dim[3]);

  // Extract maximal path between 2 points in the m_Graph
  void FindMaximalPath();

  // Sample points along the maximal path.
  // requestedNumberOfPoints is an approximate number of points to be returned.
  void SampleAlongMaximalPath(int requestedNumberOfPoints, std::deque<Coord3i> &axis_points);

private:

  // private routines

  // adds a new element with default values to To_do list
  skel_branch* AddNewBranchToDo(std::list<skel_branch> &branchesToDo);

  // find all endpoints in image
  void FindEndpoints(std::deque<Coord3i> &endPoints, const unsigned char *image, const int dim[3]);

  // tests whether (x,y,z) is an endpoint
  int IsEndpoint(int x, int y, int z, const unsigned char *image, const int dim[3]);

  // returns a list of valid neighbors at act_point
  // points that exist in skeleton, but are yet unlabeled
  void GetValidNeighbors(int* label_image, Coord3i &act_point, std::deque<Coord3i> &neighbors, const unsigned char *image, const int dim[3]);

  void ResetGraph();

  // Extracted Graph
  std::deque<skel_branch> m_Graph;

  // To_Do list, only of temporary use for extract m_Graph
  //std::deque<skel_branch> * branchesToDo;

  // List of branch IDs that make up the longest path
  std::deque<int> m_MaximalPath;
  double m_MaximalPathLength;


};

#endif
