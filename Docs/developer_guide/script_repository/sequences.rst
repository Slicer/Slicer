Sequences
~~~~~~~~~

Access voxels of a 4D volume as numpy array
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   # Get sequence node
   import SampleData
   sequenceNode = SampleData.SampleDataLogic().downloadSample("CTPCardioSeq")
   # Alternatively, get the first sequence node in the scene:
   # sequenceNode = slicer.util.getNodesByClass("vtkMRMLSequenceNode")[0]

   # Get voxels of itemIndex'th volume as numpy array
   itemIndex = 5
   voxelArray = slicer.util.arrayFromVolume(sequenceNode.GetNthDataNode(itemIndex))

Get index value
^^^^^^^^^^^^^^^

.. code-block:: python

   print("Index value of {0}th item: {1} = {2} {3}".format(
     itemIndex,
     sequenceNode.GetIndexName(),
     sequenceNode.GetNthIndexValue(itemIndex),
     sequenceNode.GetIndexUnit()))

Browse a sequence and access currently displayed nodes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   # Get a sequence node
   import SampleData
   sequenceNode = SampleData.SampleDataLogic().downloadSample("CTPCardioSeq")

   # Find corresponding sequence browser node
   browserNode = slicer.modules.sequences.logic().GetFirstBrowserNodeForSequenceNode(sequenceNode)

   # Print sequence information
   print("Number of items in the sequence: {0}".format(browserNode.GetNumberOfItems()))
   print("Index name: {0}".format(browserNode.GetMasterSequenceNode().GetIndexName()))

   # Jump to a selected sequence item
   browserNode.SetSelectedItemNumber(5)

   # Get currently displayed volume node voxels as numpy array
   volumeNode = browserNode.GetProxyNode(sequenceNode)
   voxelArray = slicer.util.arrayFromVolume(volumeNode)

Concatenate all sequences in the scene into a new sequence
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   # Get all sequence nodes in the scene
   sequenceNodes = slicer.util.getNodesByClass("vtkMRMLSequenceNode")
   mergedSequenceNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceNode", "Merged sequence")

   # Merge all sequence nodes into a new sequence node
   mergedIndexValue = 0
   for sequenceNode in sequenceNodes:
     for itemIndex in range(sequenceNode.GetNumberOfDataNodes()):
       dataNode = sequenceNode.GetNthDataNode(itemIndex)
       mergedSequenceNode.SetDataNodeAtValue(dataNode, str(mergedIndexValue))
       mergedIndexValue += 1
     # Delete the sequence node we copied the data from, to prevent sharing of the same
     # node by multiple sequences
     slicer.mrmlScene.RemoveNode(sequenceNode)

   # Create a sequence browser node for the new merged sequence
   mergedSequenceBrowserNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceBrowserNode", "Merged")
   mergedSequenceBrowserNode.AddSynchronizedSequenceNode(mergedSequenceNode)
   slicer.modules.sequencebrowser.setToolBarActiveBrowserNode(mergedSequenceBrowserNode)
   # Show proxy node in slice viewers
   mergedProxyNode = mergedSequenceBrowserNode.GetProxyNode(mergedSequenceNode)
   slicer.util.setSliceViewerLayers(background=mergedProxyNode)
