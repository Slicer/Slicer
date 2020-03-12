/*=========================================================================

  Program:   Extract Skeleton
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
//  ===============================================
//  SkelGraph.cc
//  Graphs a Thinning-skeleton
//  + Processing utilities of the graph
//  ===============================================

// STD includes
#include <algorithm>
#include <iostream>
#include <vector>

#include "SkelGraph.h"

/*
===============================================
Constructors, Destructor
===============================================
*/

SkelGraph::SkelGraph()
{
  m_MaximalPathLength = -1;
}

SkelGraph::~SkelGraph()
{
  ResetGraph();
}

void SkelGraph::ResetGraph()
{
  m_Graph.clear();
  m_MaximalPath.clear();
  m_MaximalPathLength = -1;
}

// -------------------------------------------------------------------------
// Public Methods
// -------------------------------------------------------------------------

void SkelGraph::ExtractSkeletalGraph(const unsigned char *image, const int dim[3])
// Graph compilation of a thinning skeleton
{
  ResetGraph();

  int size_image = dim[0] * dim[1] * dim[2];

  int* label_image = new int[size_image];
  for( int i = 0; i < size_image; i++ )
    {
    label_image[i] = 0;
    }

  // determine m_EndPointsTemp = points that have exactly 1 neighbor
  std::deque<Coord3i> endPoints;
  FindEndpoints(endPoints, image, dim);
  // if (DEBUG_VSKEL)
  //  std::cout << m_EndPointsTemp.size() << " Endpoints found" << std::endl;

  // if  (while) untreated endpoint -> put it into branchesToDo list
  // if  (while) branchesToDo list non-empty -> follow branches
  //    following of branch has 1 neighbor -> label, cont
  //    branch ends                        -> stop, next
  //    multiple branches emerge           -> stop, put in branchesToDo list

  // if  (while) untreated endpoint -> put it into branchesToDo list
  for (std::deque<Coord3i>::iterator act_endpoint = endPoints.begin(); act_endpoint != endPoints.end(); ++act_endpoint)
    {
    if (label_image[(*act_endpoint)[0] + dim[0] * ((*act_endpoint)[1] + dim[1] * (*act_endpoint)[2])])
      {
      // this endpoint has been already labeled (processed)
      continue;
      }

    // add new branch
    std::list<skel_branch> branchesToDo;
    skel_branch *branch_elem = AddNewBranchToDo(branchesToDo);
    branch_elem->end_1_point = *act_endpoint;
    branch_elem->end_2_point = *act_endpoint;
    branch_elem->points.push_back(*act_endpoint);

    while (!branchesToDo.empty())
      {
      std::list<skel_branch>::iterator act_branch = branchesToDo.begin();

      // act_branch != branchesToDo->end() &&
      // if  (while) branchesToDo list non-empty -> follow branches
      bool branch_done = false;
      Coord3i act_point = act_branch->end_2_point;
      int branchID = act_branch->branchID;
      // label endpoint
      label_image[act_point[0] + dim[0] * (act_point[1] + dim[1] * act_point[2])] = branchID;
      while (!branch_done)
        {
        std::deque<Coord3i> neighbors;
        GetValidNeighbors(label_image, act_point, neighbors, image, dim);
        const size_t num_nb = neighbors.size();
        if( num_nb == 0 )
          {
          //    branch ends                        -> stop, next
          branch_done = true;
          }
        else if( num_nb == 1 )
          {
          //    following of branch has 1 neighbor -> label, cont
          Coord3i pt = *(neighbors.begin());
          // update length
          // since act_point[0] - pt[0] is either [-1,0,1] -> abs == ^2
          act_branch->length += pointdistance(act_point, pt);
          act_branch->points.push_back(pt);
          act_point = pt;
          label_image[act_point[0]
                      + dim[0] * (act_point[1] + dim[1] * act_point[2])] = branchID;
          }
        else
          {
          //    multiple branches emerge -> stop, put in branchesToDo list
          branch_done = true;
          std::vector<skel_branch*> neighborBranches;
          for (std::deque<Coord3i>::iterator act_neighbor = neighbors.begin(); act_neighbor != neighbors.end(); ++act_neighbor)
            {
            Coord3i pt = *act_neighbor;
            skel_branch* newElem = AddNewBranchToDo(branchesToDo);
            neighborBranches.push_back(newElem);
            // label start Coord3i
            newElem->end_1_point = pt;
            newElem->end_2_point = pt;
            newElem->points.push_back(pt);
            label_image[pt[0] + dim[0] * (pt[1] + dim[1] * pt[2])] = newElem->branchID;
            // update ends with act_branch
            newElem->end_1_neighbors.push_back(act_branch->branchID);
            act_branch->end_2_neighbors.push_back(newElem->branchID);
            }
          // update ends of new branches with each other
          for( size_t ii = 0; ii < num_nb; ii++ )
            {
            for( size_t jj = 0; jj < num_nb; jj++ )
              {
              if( ii != jj )
                {
                neighborBranches[ii]->end_1_neighbors.push_back(neighborBranches[jj]->branchID);
                }
              }
            }
          }   // else
        } // while (! branch_done)

      // copy branch from branchesToDo to m_Graph
      m_Graph.push_back(*act_branch);
      // remove it from branchesToDo
      branchesToDo.pop_front();
      }
    }

  // done

  delete [] label_image; label_image = nullptr;
  image = nullptr; // no delete, since image points to original

  // if (DEBUG_VSKEL)
  //   std::cout << m_Graph.size() << " Branches found" << std::endl;
  // m_Graph print
  // PrintGraph();
}

void SkelGraph::PrintGraph()
// print actual m_Graph
{
  std::deque<skel_branch>::iterator act_graph;
  std::deque<int>::iterator         act_nb;

  act_graph = m_Graph.begin();
  std::cout << "Graph : " << std::endl;
  int cnt = 0;
  std::cout << "Number: Branch Br.ID | N 1 | N 2 | Length | End1 | End2 " << std::endl;
  while( act_graph != m_Graph.end() )
    {
    cnt++;
    std::cout << cnt << ": Br. " << act_graph->branchID;

    if( !act_graph->end_1_neighbors.empty() )
      {
      act_nb = act_graph->end_1_neighbors.begin();
      std::cout << "| ";
      while( act_nb != act_graph->end_1_neighbors.end() )
        {
        std::cout << *act_nb << ", ";
        act_nb++;
        }
      }
    else
      {
      std::cout << "| None";
      }

    if(!act_graph->end_2_neighbors.empty())
      {
      act_nb = act_graph->end_2_neighbors.begin();
      std::cout << "| ";
      while( act_nb != act_graph->end_2_neighbors.end() )
        {
        std::cout << *act_nb << ", ";
        act_nb++;
        }
      }
    else
      {
      std::cout << "|  None";
      }
    std::cout << "| " << act_graph->length << "| "
         << act_graph->end_1_point[0] << "," << act_graph->end_1_point[1] << ","
         << act_graph->end_1_point[2] << " | "
         << act_graph->end_2_point[0] << "," << act_graph->end_2_point[1] << ","
         << act_graph->end_2_point[2] << " | " << std::endl;
    act_graph++;
    }
}

void SkelGraph::FindMaximalPath()
// extract maximal path between 2 points in the m_Graph
{
  for (std::deque<skel_branch>::iterator branch = m_Graph.begin(); branch != m_Graph.end(); ++branch)
    {
    branch->max_path_length = 0.0;
    branch->max_path.clear();
    }

  for (std::deque<skel_branch>::iterator act_endbranch = m_Graph.begin(); act_endbranch != m_Graph.end(); ++act_endbranch)
    {
    //  search for next entry that has neighbors but
    // end_1_neighbors == nullptr OR act_endbranch->end_2_neighbors != nullptr
    if (act_endbranch->end_1_neighbors.empty() && act_endbranch->end_2_neighbors.empty())
      {
      // no neighbors
      continue;
      }
    if (!act_endbranch->end_1_neighbors.empty() && !act_endbranch->end_2_neighbors.empty())
      {
      // neighbors on both sides
      continue;
      }

    // reset temporary acc path and its length
    for (std::deque<skel_branch>::iterator branch = m_Graph.begin(); branch != m_Graph.end(); ++branch)
      {
      branch->acc_length = 0.0;
      branch->acc_path.clear();
      }

    // do cost traversal
    std::deque< skel_branch* > wait_list;
    wait_list.push_back(&(*act_endbranch));
    while( !wait_list.empty() )
      {
      // get next entry in wait_list
      skel_branch * act_node = *(wait_list.begin());
      wait_list.pop_front();

      // add to path
      act_node->acc_length += act_node->length;
      act_node->acc_path.push_back(act_node->branchID);
      int act_pos_id = act_node->branchID;
      std::deque<skel_branch>::iterator act_pos_list = m_Graph.begin();
      // since the graph_id's are the location of its member in the list,
      // we can use advance for random access
      advance(act_pos_list, act_pos_id - 1);
      // std::cout << "A " << act_pos_id  << std::endl;
      for( int i = 0; i < 2; i++ )
        {
        std::deque<int> * cont_end = nullptr;
        Coord3i cont_end_point;
        if( i == 0 )
          {
          cont_end = &(act_node->end_2_neighbors);
          cont_end_point = act_node->end_2_point;
          }
        else if( i == 1 )
          {
          cont_end = &(act_node->end_1_neighbors);
          cont_end_point = act_node->end_1_point;
          }
        if (cont_end->empty())
          {
          continue;
          }

        // add all neighbors to wait_list that are not yet treated
        for(std::deque<int>::iterator neighbors = cont_end->begin(); neighbors != cont_end->end(); ++neighbors)
          {
          // get neighbours entry
          int distance = *neighbors - act_pos_id;
          std::deque<skel_branch>::iterator act_pos_neigh = act_pos_list;
          advance(act_pos_neigh, distance);
          skel_branch * act_neighbor = &(*(act_pos_neigh) );
          if (!act_neighbor->acc_path.empty())
            {
            // neighbour already treated
            continue;
            }
          wait_list.push_back(act_neighbor);
          // update entries of neighbour
          // since act_point[0] - pt[0] is either [-1,0,1] -> abs == ^2
          // add distance between branches to length at preceding branch
          Coord3i cont_neigh_point;
          act_neighbor->acc_length = act_node->acc_length;
          // determine connection costs -> since we do not know which one is the
          // corresponding endpoint of the neighbour, we have to try out and take
          // the one combination that yields the smallest costs
          double conn_costs1 = pointdistance(act_neighbor->end_1_point, cont_end_point);
          double conn_costs2 = pointdistance(act_neighbor->end_2_point, cont_end_point);
          act_neighbor->acc_length += (conn_costs1 < conn_costs2 ? conn_costs1 : conn_costs2);
          // copy path
          // initiate with copy of path of preceding branch
          act_neighbor->acc_path = act_node->acc_path;
          }
        }
      }

    // look for maximum
    skel_branch * act_max_node = nullptr;
    double act_max_val = -1;
    for (std::deque<skel_branch>::iterator branch = m_Graph.begin(); branch != m_Graph.end(); ++branch)
      {
      if (branch->acc_length > act_max_val)
        {
        act_max_val = branch->acc_length;
        act_max_node = &(*(branch) );
        }
      }
    // copy maximal path
    act_endbranch->max_path_length = act_max_val;
    act_endbranch->max_path = act_max_node->acc_path;

    //     if (DEBUG_VSKEL) {
    //       std::cout << act_endbranch->branchID
    //      << " : max length = " << act_endbranch->max_path_length
    //      << " | path = ";
    //       //double len = 0.0;
    //       std::deque<int>::iterator act_path = act_endbranch->max_path->begin();
    //       //std::deque<skel_branch>::iterator old_pos = m_Graph.end();
    //       while (act_path != act_endbranch->max_path->end()) {
    //   // std::deque<skel_branch>::iterator act_pos = m_Graph.begin();
    //   // advance(act_pos, *act_path - 1);
    //   //len += act_pos->length;
    //   //if (old_pos != m_Graph.end()) {
    //   //double len1 = sqrt(abs(act_pos->end_1_point[0] - old_pos->end_2_point[0]) +
    //   //         abs(act_pos->end_1_point[1] - old_pos->end_2_point[1]) +
    //   //         abs(act_pos->end_1_point[2] - old_pos->end_2_point[2]));
    //   //  double len2 = sqrt(abs(act_pos->end_2_point[0] - old_pos->end_1_point[0]) +
    //   //         abs(act_pos->end_2_point[1] - old_pos->end_1_point[1]) +
    //   //         abs(act_pos->end_2_point[2] - old_pos->end_1_point[2]));
    //   //  if (len1 < len2) len += len1; else len += len2;
    //   //}
    //   //old_pos = act_pos;
    //   std::cout << *act_path << ",";
    //   // std::cout << len << "-";
    //   ++act_path;
    //       }
    //       std::cout << std::endl;
    //     }
    }

  // Get Maximum of all maximal paths (which is double contained, otherweise it would
  // not be maximal )

  skel_branch* maximalPathStartBranch = nullptr;
  m_MaximalPathLength = -1.0;
  for (std::deque<skel_branch>::iterator branch = m_Graph.begin(); branch != m_Graph.end(); ++branch)
    {
    if (branch->max_path_length > m_MaximalPathLength)
      {
      m_MaximalPathLength = branch->max_path_length;
      maximalPathStartBranch = &(*(branch) );
      }
    }
  if (maximalPathStartBranch)
    {
    m_MaximalPath = maximalPathStartBranch->max_path;
    }
  else
    {
    m_MaximalPath.clear();
    }
}

void SkelGraph::SampleAlongMaximalPath(int requestedNumberOfPoints, std::deque<Coord3i> &axis_points)
{
  axis_points.clear();
  double minimumDistance = m_MaximalPathLength / (requestedNumberOfPoints - 1);
  skel_branch* previousBranch = nullptr;
  Coord3i previousPointPosition;
  for (std::deque<int>::iterator branchId = m_MaximalPath.begin(); branchId != m_MaximalPath.end(); ++branchId)
    {
    skel_branch& branch = m_Graph[(*branchId) - 1];
    // Check if we need forward or reverse point order
    bool reversePointOrder = false;
    if (previousBranch)
      {
      if (std::find(branch.end_2_neighbors.begin(), branch.end_2_neighbors.end(), previousBranch->branchID) != branch.end_2_neighbors.end())
        {
        reversePointOrder = true;
        }
      }
    else
      {
      // first branch
      if (branch.end_2_neighbors.empty())
        {
        reversePointOrder = true;
        previousPointPosition = branch.points.back();
        }
      else
        {
        previousPointPosition = branch.points.front();
        }
      axis_points.push_back(previousPointPosition);
      }
    // Append point positions
    if (reversePointOrder)
      {
      for (std::deque<Coord3i>::reverse_iterator point = branch.points.rbegin(); point != branch.points.rend(); ++point)
        {
        if (pointdistance(previousPointPosition, *point) < minimumDistance)
          {
          continue;
          }
        axis_points.push_back(*point);
        previousPointPosition = *point;
        }
      }
    else
      {
      for (std::deque<Coord3i>::iterator point = branch.points.begin(); point != branch.points.end(); ++point)
        {
        if (pointdistance(previousPointPosition, *point) < minimumDistance)
          {
          continue;
          }
        axis_points.push_back(*point);
        previousPointPosition = *point;
        }
      }
    previousBranch = &branch;

    // Make sure the last point is included exactly
    if (branchId + 1 == m_MaximalPath.end())
      {
      Coord3i lastPoint = (reversePointOrder ? branch.points.front() : branch.points.back());
      // Remove one before last point if it is too close to the last point
      if (pointdistance(previousPointPosition, lastPoint) < minimumDistance)
        {
        axis_points.pop_back();
        }
      axis_points.push_back(lastPoint);
      }
    }
}


// -------------------------------------------------------------------------
// Private Methods
// -------------------------------------------------------------------------

void SkelGraph::FindEndpoints(std::deque<Coord3i> &endPoints, const unsigned char *image, const int dim[3])
{
  endPoints.clear();
  for( int z = 1; z < dim[2] - 1; z++ )
    {
    for( int y = 1; y < dim[1] - 1; y++ )
      {
      for( int x = 1; x < dim[0] - 1; x++ )
        {
        if( image[x + dim[0] * ( y + dim[1] * z )] && IsEndpoint(x, y, z, image, dim) )
          {
          // x,y,z is an endpoint
          Coord3i elem;
          elem[0] = x;
          elem[1] = y;
          elem[2] = z;
          endPoints.push_back(elem);
          }
        }
      }
    }
}

int SkelGraph::IsEndpoint(int x, int y, int z, const unsigned char *image, const int dim[3])
{
  int nb = 0;
  int pz = z - 1;
  for (int i = 0; i < 3; i++)
    {
    int py = y - 1;
    for( int j = 0; j < 3; j++ )
      {
      int px = x - 1;
      for( int k = 0; k < 3; k++ )
        {
        if( image[px + dim[0] * ( py + dim[1] * pz )] )
          {
          if (nb == 2)
            {
            // it would be more than 2 neighbors, not an endpoint
            return false;
            }
          nb++;
          }
        px++;
        }
      py++;
      }
    pz++;
    }

  return (nb == 2);   // == 1 neighbor
}

skel_branch* SkelGraph::AddNewBranchToDo(std::list<skel_branch> &branchesToDo)
{
  skel_branch new_elem;
  new_elem.branchID = static_cast<int>(branchesToDo.size() + m_Graph.size() + 1);
  branchesToDo.push_back(new_elem);
  return &(branchesToDo.back());
}

void SkelGraph::GetValidNeighbors(int* label_image, Coord3i &act_point, std::deque<Coord3i> &neighbors, const unsigned char *image, const int dim[3])
{
  int pz = act_point[2] - 1;

  for( int i = 0; i < 3; i++ )
    {
    int py = act_point[1] - 1;
    for( int j = 0; j < 3; j++ )
      {
      int px = act_point[0] - 1;
      for( int k = 0; k < 3; k++ )
        {
        if( image[px + dim[0] * ( py + dim[1] * pz )] &&
            !label_image[px + dim[0] * ( py + dim[1] * pz )] )
          {
          // point exist in skeleton, but is yet unlabeled
          Coord3i pt_elem;
          pt_elem[0] = px;
          pt_elem[1] = py;
          pt_elem[2] = pz;
          neighbors.push_back(pt_elem);
          }
        px++;
        }
      py++;
      }
    pz++;
    }

  //   if (DEBUG_VSKEL) {
  //     std::deque<Coord3i>::iterator act_neighbor = neighbors->begin();
  //     int num_nb = neighbors->size();
  //     int i = 0;
  //     while (act_neighbor != neighbors->end()) {
  //       i++;
  //       Coord3i * pt = &(*act_neighbor);
  //       std::cout << i << ".Neighbour: "<< pt[0] << "," << pt[1] << "," << pt[2] << std::endl;
  //       ++act_neighbor;
  //     }
  //   }

}
