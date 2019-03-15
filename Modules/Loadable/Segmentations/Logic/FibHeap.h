// exclude from VTK wrapping
#ifndef __VTK_WRAP__

#ifndef FIBHEAP_H
#define FIBHEAP_H

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


#define OK      0
#define NOTOK   -1

#include <cstdio>

//======================================================
// Fibonacci Heap Node Class
//======================================================

class FibHeap;

class FibHeapNode
{
  friend class FibHeap;
  FibHeapNode *Left, *Right, *Parent, *Child;
  short Degree, Mark, NegInfinityFlag;

protected:
  inline int  FHN_Cmp(FibHeapNode& RHS) {
    if (NegInfinityFlag)
      return RHS.NegInfinityFlag ? 0 : -1;
    return RHS.NegInfinityFlag ? 1 : 0;
  }
  inline void FHN_Assign(FibHeapNode& RHS) {
    NegInfinityFlag = RHS.NegInfinityFlag;
  }

public:
  inline FibHeapNode() {
    Left = Right = Parent = Child = nullptr;
    Degree = Mark = NegInfinityFlag = 0;
  }
  virtual ~FibHeapNode();
  virtual void operator =(FibHeapNode& RHS);
  virtual int  operator ==(FibHeapNode& RHS);
  virtual int  operator <(FibHeapNode& RHS);

  virtual void Print();
};

//========================================================================
// Fibonacci Heap Class
//========================================================================

class FibHeap
{
  FibHeapNode *MinRoot;
  long NumNodes, NumTrees, NumMarkedNodes;
  int HeapOwnershipFlag;

public:
  FibHeap();
  virtual ~FibHeap();

  // The Standard Heap Operations
  void Insert(FibHeapNode *NewNode);
  void Union(FibHeap *OtherHeap);
  inline FibHeapNode *Minimum() { return MinRoot; }
  FibHeapNode *ExtractMin();
  int DecreaseKey(FibHeapNode *theNode, FibHeapNode& NewKey);
  int Delete(FibHeapNode *theNode);
  bool IsEmpty() {return (MinRoot == nullptr);}

  // Extra utility functions
  int  GetHeapOwnership() { return HeapOwnershipFlag; };
  void SetHeapOwnership() { HeapOwnershipFlag = 1; };
  void ClearHeapOwnership() { HeapOwnershipFlag = 0; };
  long GetNumNodes() { return NumNodes; };
  long GetNumTrees() { return NumTrees; };
  long GetNumMarkedNodes() { return NumMarkedNodes; };
  void Print(FibHeapNode *Tree = nullptr, FibHeapNode *theParent=nullptr);

private:
  // Internal functions that help to implement the Standard Operations
  inline void _Exchange(FibHeapNode* &N1, FibHeapNode* &N2) {
    FibHeapNode *Temp; Temp = N1; N1 = N2; N2 = Temp;
  }
  void _Consolidate();
  void _Link(FibHeapNode *, FibHeapNode *);
  void _AddToRootList(FibHeapNode *);
  void _Cut(FibHeapNode *, FibHeapNode *);
  void _CascadingCut(FibHeapNode *);
};

#endif /* FIBHEAP_H */

#endif //__VTK_WRAP__
