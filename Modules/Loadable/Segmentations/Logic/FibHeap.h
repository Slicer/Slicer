//***************************************************************************
// The Fibonacci heap implementation contained in FIBHEAP.H and FIBHEAP.CPP
// is Copyright (c) 1996 by John Boyer
//
// Once this Fibonacci heap implementation (the software) has been published
// by Dr. Dobb's Journal, permission to use and distribute the software is
// granted provided that this copyright notice remains in the source and
// and the author (John Boyer) is acknowledged in works that use this program.
//
// Every effort has been made to ensure that this implementation is free of
// errors.  Nonetheless, the author (John Boyer) assumes no liability regarding
// your use of this software.
//
// The author would also be very glad to hear from anyone who uses the
// software or has any feedback about it.
// Email: jboyer@gulf.csc.uvic.ca
//***************************************************************************

// exclude from VTK wrapping
#ifndef __VTK_WRAP__

#ifndef FIBHEAP_H
#define FIBHEAP_H

#include <cstdio>
#include <iostream>

// type for cost function - single precision is enough
typedef float NodeKeyValueType;

// type for storing a pixel index
// We could use 32-bit indices for images smaller than 4GB and 64-bit indices for larger images.
// However, >4GB images would require about 300GB RAM to store all the indices and distances,
// so for now we only support images smaller than 4GB.
typedef unsigned int NodeIndexType;

// .NAME FibHeapNode - Fibonacci Heap Node Class
// .SECTION Description
//
// This class stores all data of a node in the Fibonacci heap and implements
// basic operations on nodes (comparison, assignment, etc.).
//
// In the original implementation of John Boyer FibHeapNode was a base class
// and the implementation was completed in a derived class
// However, virtual methods reduced performance by a few percent and the
// virtual method table added extra 8 bytes for the size of each node,
// which was significant when one node represented a voxel (and we had
// hundreds of millions of voxels in an image).

class FibHeapNode
{
public:
  static const NodeIndexType NullNodeIndex;
  static const NodeKeyValueType NegativeInfinity;

  inline FibHeapNode()
    : m_Left(NullNodeIndex)
    , m_Right(NullNodeIndex)
    , m_Parent(NullNodeIndex)
    , m_Child(NullNodeIndex)
    , m_Index(NullNodeIndex)
  {
  }

  ~FibHeapNode() = default;

  /// Index stores this node's location in the node array
  inline NodeIndexType GetIndexValue() { return m_Index; }
  inline void SetIndexValue(NodeIndexType indexValue) { m_Index = indexValue; }

  /// Get key value.
  /// Key value can be set using operator=.
  inline NodeKeyValueType GetKeyValue() { return m_Key; }

  /// Set key value (that the nodes are sorted based on)
  inline void operator =(NodeKeyValueType newKeyVal)
  {
    m_Key = newKeyVal;
  }

  /// Set key value from another node
  inline void operator =(FibHeapNode& RHS)
  {
    m_Key = RHS.m_Key;
  }

  /// Compare key value
  inline bool operator ==(FibHeapNode& RHS)
  {
    return m_Key == RHS.m_Key;
  }

  /// Compare key value
  inline bool operator <(FibHeapNode& RHS)
  {
    return m_Key < RHS.m_Key;
  }

  void Print()
  {
    std::cout << m_Key;
  }

  NodeIndexType m_Left;
  NodeIndexType m_Right;
  NodeIndexType m_Parent;
  NodeIndexType m_Child;
  short m_Degree{0};
  bool m_Mark{false};

  NodeKeyValueType m_Key{0};
  NodeIndexType m_Index;
};

// .NAME FibHeap - Fibonacci Heap Class
// .SECTION Description
//
// This class implements a performance-optimized heap data structure.
// Using amortized analysis, the asymptotically fastest heap
// data structure is the Fibonacci heap.  The constants are a little
// high so the real speed gain will not be seen until larger data sets
// are required, but in most cases, if the data set is small, then the
// run-time will be negligible anyway.
//
// FibHeap class can then be used to Insert(), ExtractMin(), etc.,
// instances of FibHeapNode class.

/// \ingroup Slicer_QtModules_Segmentations

class FibHeap
{
public:
  FibHeap();
  virtual ~FibHeap();

  /// Set array that stores all nodes.
  /// The array is not owned by this object and must remain valid while this object is used.
  /// Nodes are stored in an array to allow referencing them using a 32-bit index
  /// instead of a 64-bit pointer. Since each node stores 4 pointer/index, this saves
  /// 16 bytes per node, which is very significant when e.g., one node corresponds to
  /// an image voxel and the image contains hundreds of millions of voxels.
  void SetHeapNodes(FibHeapNode* heapNodes);

  /// Insert() - O(1) actual; O(2) amortized
  ///
  /// I am using O(2) here to indicate that although Insert() is
  /// constant time, its amortized rating is more costly because some
  /// of the work of inserting is done by other operations such as
  /// ExtractMin(), which is where tree-balancing occurs.
  ///
  /// The child pointer is deliberately not set to nullptr because Insert()
  /// is also used internally to help put whole trees onto the root list.
  void Insert(FibHeapNode *NewNode);

  /// Union() - O(1) actual; O(1) amortized
  void Union(FibHeap *OtherHeap);

  /// Minimum - O(1) actual; O(1) amortized
  inline FibHeapNode *Minimum()
  {
    return m_MinRoot;
  }

  /// ExtractMin() - O(n) worst-case actual; O(lg n) amortized
  FibHeapNode *ExtractMin();

  /// DecreaseKey() - O(lg n) actual; O(1) amortized
  ///
  /// The O(lg n) actual cost stems from the fact that the depth, and
  /// therefore the number of ancestor parents, is bounded by O(lg n).
  int DecreaseKey(FibHeapNode* theNode, NodeKeyValueType keyValue);

  /// Delete() - O(lg n) amortized; ExtractMin() dominates
  ///
  /// Notice that if we don't own the heap nodes, then we clear the
  /// m_NegInfinityFlag on the deleted node.  Presumably, the programmer
  /// will be reusing the node.
  int Delete(FibHeapNode *theNode);

  inline bool IsEmpty()
  {
    return (m_MinRoot == nullptr);
  }

  // Extra utility functions
  long GetNumNodes()
  {
    return m_NumNodes;
  };
  long GetNumTrees()
  {
    return m_NumTrees;
  };
  long GetNumMarkedNodes()
  {
    return m_NumMarkedNodes;
  };

  /// Print()
  ///
  /// Used internally for debugging purposes.  The function prints the key
  /// value for each node along the root list, then it calls itself on each
  /// child list.
  void Print(FibHeapNode *Tree = nullptr, FibHeapNode *theParent=nullptr);

private:
  // Internal functions that help to implement the Standard Operations
  inline void Exchange(FibHeapNode* &N1, FibHeapNode* &N2)
  {
    FibHeapNode *Temp;
    Temp = N1;
    N1 = N2;
    N2 = Temp;
  }

  // Internal function that reorganizes heap as part of an ExtractMin().
  // We must find new minimum in heap, which could be anywhere along the
  // root list.  The search could be O(n) since all nodes could be on
  // the root list.  So, we reorganize the tree into more of a binomial forest
  // structure, and then find the new minimum on the consolidated O(lg n) sized
  // root list, and in the process set each m_Parent pointer to nullptr, and count
  // the number of resulting subtrees.
  //
  // Note that after a list of n inserts, there will be n nodes on the root
  // list, so the first ExtractMin() will be O(n) regardless of whether or
  // not we consolidate.  However, the consolidation causes subsequent
  // ExtractMin() operations to be O(lg n).  Furthermore, the extra cost of
  // the first ExtractMin() is covered by the higher amortized cost of
  // Insert(), which is the real governing factor in how costly the first
  // ExtractMin() will be.
  void Consolidate();

  // The node y is removed from the root list and becomes a subtree of node x.
  void Link(FibHeapNode *, FibHeapNode *);
  void AddToRootList(FibHeapNode *);

  // Remove node x from the child list of its parent node y
  void Cut(FibHeapNode *, FibHeapNode *);

  // Cuts each node in parent list, putting successive ancestor nodes on the
  // root list until we either arrive at the root list or until we find an
  // ancestor that is unmarked.  When a mark is set (which only happens during
  // a cascading cut), it means that one child subtree has been lost; if a
  // second subtree is lost later during another cascading cut, then we move
  // the node to the root list so that it can be re-balanced on the next
  // consolidate.
  void CascadingCut(FibHeapNode *);

  inline FibHeapNode* HeapNodeFromIndex(NodeIndexType index)
  {
    return index == FibHeapNode::NullNodeIndex ? nullptr : m_HeapNodes + index;
  }

  FibHeapNode* m_MinRoot;
  long m_NumNodes;
  long m_NumTrees;
  long m_NumMarkedNodes;
  FibHeapNode* m_HeapNodes; // array containing all the heap nodes

};

#endif /* FIBHEAP_H */

#endif //__VTK_WRAP__
