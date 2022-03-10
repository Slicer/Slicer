import slicer
import sitkUtils as su

import unittest


class SitkUtilsTests(unittest.TestCase):

    def setUp(self):
        pass

    def test_SimpleITK_SlicerPushPull(self):
        """ Download the MRHead node
        """
        import SampleData
        SampleData.downloadSample("MRHead")
        volumeNode1 = slicer.util.getNode('MRHead')
        self.assertEqual(volumeNode1.GetName(), "MRHead")

        """ Verify that pulling SimpleITK image from Slicer and then pushing it
        back creates an identical volume.
        """

        sitkimage = su.PullVolumeFromSlicer(volumeNode1)
        self.assertIsNotNone(sitkimage)

        volumeNode1Copy = su.PushVolumeToSlicer(sitkimage, name="MRHead", className="vtkMRMLScalarVolumeNode")
        self.assertIsNotNone(volumeNode1Copy)

        """ Verify that image is not overwritten but a new one is created """
        self.assertEqual(volumeNode1, slicer.util.getNode('MRHead'),
                         'Original volume is changed')
        self.assertNotEqual(volumeNode1, volumeNode1Copy,
                         'Copy of original volume is not created')

        """ Few modification of the image : Direction, Origin """
        sitkimage.SetDirection((-1.0, 1.0, 0.0, 0.0, -1.0, 1.0, 1.0, 0.0, 1.0))
        sitkimage.SetOrigin((100.0, 100.0, 100.0))

        """ Few pixel changed """
        size = sitkimage.GetSize()
        for x in range(0,size[0],int(size[0]/10)):
            for y in range(0,size[1],int(size[1]/10)):
                for z in range(0,size[2],int(size[2]/10)):
                    sitkimage.SetPixel(x,y,z,0)

        volumeNode1Modified = su.PushVolumeToSlicer(sitkimage, name="ImageChanged", className="vtkMRMLScalarVolumeNode")
        self.assertEqual(volumeNode1Modified.GetName(), "ImageChanged",
                         'Volume name is not set correctly')
        self.assertNotEqual(volumeNode1.GetMTime(), volumeNode1Modified.GetMTime(),
                            'Error Push Pull: Modify Time are the same')

        """ Test the consistency between sitkimage and volumeNode1Modified
        """
        tmp = volumeNode1Modified.GetOrigin()
        valToCompare = (-tmp[0], -tmp[1], tmp[2])
        self.assertEqual(valToCompare,sitkimage.GetOrigin(),
                         'Modified origin mismatch')

        """ Test push with all parameter combinations """
        for volumeClassName in ['vtkMRMLScalarVolumeNode', 'vtkMRMLLabelMapVolumeNode']:
            volumeNodeTested = None
            volumeNodeNew = None
            for pushToNewNode in [True, False]:
                print("volumeClassName : %s" % volumeClassName )
                print("pushToNewNode : %s " % pushToNewNode )

                if pushToNewNode:
                    volumeNodeTested = su.PushVolumeToSlicer(sitkimage,
                        name='volumeNode-'+volumeClassName+"-"+str(pushToNewNode),
                        className=volumeClassName)
                    existingVolumeNode = volumeNodeTested
                else:
                    volumeNodeTested = su.PushVolumeToSlicer(sitkimage, existingVolumeNode)

                self.assertEqual(volumeNodeTested.GetClassName(), volumeClassName, 'Created volume node class is incorrect')

        slicer.mrmlScene.Clear(0)

    def tearDown(self):
        pass
