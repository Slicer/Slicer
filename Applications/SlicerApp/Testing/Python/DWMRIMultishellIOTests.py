import sys, os, re, nose
from nose.tools import assert_equal
from collections import namedtuple

import numpy as np
import numpy.testing
from vtk.util import numpy_support
import slicer

#===============================================================================

mrmlcore_testdata_path = "Libs/MRML/Core/Testing/TestData/"


multishell_dwi_451 = os.path.join(mrmlcore_testdata_path, "multishell-DWI-451dir.nhdr")

#================================================================================
NRRD = namedtuple('NRRD', ['header', 'bvalue', 'gradients'])

def parse_nhdr(path):
    dwmri_bval_key      = "DWMRI_b-value"
    dwmri_grad_keybase  = "DWMRI_gradient_"
    dwmri_grad_key_n    = "DWMRI_gradient_{:04d}"

    kvdict = {}
    grad_count = 0

    with open(path, "rU") as f:
        magic = f.readline().strip()
        assert(magic == "NRRD0005")

        while True:
            line = f.readline()
            # NRRD data section is separated by a newline
            if (line == "\n") or (line == "") or (line is None):
                break

            # careful about precedence -- ":=" must match first
            key, val = [x.strip() for x in re.split(":=|=|:", line)]
            assert(key not in kvdict)
            kvdict[key] = val

            if key.startswith(dwmri_grad_keybase):
                _gn = int(key[ len(dwmri_grad_keybase):None ])
                # monotonic keys
                assert( _gn == grad_count ) # offset
                grad_count += 1

        bvalue = float(kvdict[dwmri_bval_key])
        grads = np.zeros((grad_count, 3))

        # parse gradients
        for i in range(0, grad_count):
            grad_str = kvdict[dwmri_grad_key_n.format(i)]
            grads[i] = np.fromstring(grad_str, count=3, dtype=np.float64, sep=" ")

    return NRRD(header=kvdict, bvalue=bvalue, gradients=grads)


#================================================================================
def normalize(vec):
    norm = np.linalg.norm(vec)
    if norm == 0.0:
        return vec
    else:
        return vec * 1/norm


def test_nrrd_dwi_load(first_file, second_file=None):
    """
    - load a DWI NRRD file into Slicer
    - validate b values and gradient vectors against original header
        - check the values in the vtkMRMLDiffusionWeightedVolumeNode
        - check the values in the vtkMRMLDWVNode attribute dictionary
    """
    if second_file is None:
        second_file = first_file

    # load NRRD into Slicer
    storagenode = slicer.vtkMRMLNRRDStorageNode()
    storagenode.SetFileName(first_file)
    dw_node = slicer.vtkMRMLDiffusionWeightedVolumeNode()
    storagenode.ReadData(dw_node)

    slicer_grads = numpy_support.vtk_to_numpy(dw_node.GetDiffusionGradients())
    slicer_numgrads = slicer_grads.shape[0]

    # load NRRD with pure-python parser
    parsed_nrrd = parse_nhdr(second_file)

    ##################################
    # 1) check the number of gradients

    assert( len(parsed_nrrd.gradients) == slicer_numgrads )

    ##################################
    # 2) check the node b values and gradients are correct

    # Note: vtkDataArray.GetMaxNorm gives max for scalar array.
    # max b value from the node
    nose.tools.assert_equal(parsed_nrrd.bvalue, dw_node.GetBValues().GetMaxNorm())

    max_parsed_grad_norm = np.max(np.apply_along_axis(np.linalg.norm, 1, parsed_nrrd.gradients))

    for i in range(0, slicer_numgrads):
        g_parsed_raw = parsed_nrrd.gradients[i]
        g_parsed_normed = normalize(g_parsed_raw)

        bval_parsed = parsed_nrrd.bvalue * pow(np.linalg.norm(g_parsed_raw) / max_parsed_grad_norm, 2)
        np.testing.assert_almost_equal(bval_parsed, dw_node.GetBValue(i), decimal=7,
                                       err_msg="MRMLNode b value does not match NRRD header")

        g_from_node = slicer_grads[i, :]

        # gradients stored in the vtkMRMLDiffusionWeightedVolumeNode must be *normalized*.
        np.testing.assert_allclose(np.linalg.norm(g_parsed_normed - g_from_node), 0.0, atol=1e-15)

    # b value from the node attribute dictionary
    np.testing.assert_equal(parsed_nrrd.bvalue, float(dw_node.GetAttribute("DWMRI_b-value")))

    # 3) check gradients in the node attribute dictionary
    #    gradients must match the value on-disk.
    for i in range(0, slicer_numgrads):
        grad_key = "DWMRI_gradient_{:04d}".format(i)
        parsed_gradient = np.fromstring(parsed_nrrd.header[grad_key], count=3, sep=' ', dtype=np.float64)
        attr_gradient =   np.fromstring(dw_node.GetAttribute(grad_key), count=3, sep=' ', dtype=np.float64)

        np.testing.assert_array_almost_equal(parsed_gradient, attr_gradient, decimal=12,
                                             err_msg="NHDR gradient does not match gradient in node attribute dictionary")

    return (parsed_nrrd, dw_node)

def test_nrrd_dwi_roundtrip(test_nrrd_path):
    """DWI NRRD round-trip test
    - loads and saves a NRRD file via Slicer's I/O, twice
    - checks the node values against the original file each time
    """

    import tempfile

    # load and re-save NRRD once
    storagenode1 = slicer.vtkMRMLNRRDStorageNode()
    storagenode1.SetFileName(test_nrrd_path)
    dw_node1 = slicer.vtkMRMLDiffusionWeightedVolumeNode()
    storagenode1.ReadData(dw_node1)
    __f_tmp_nrrd1 = tempfile.NamedTemporaryFile(suffix=".nhdr", dir=tmp_dir, delete=False)
    tmp_nrrd1 = __f_tmp_nrrd1.name
    storagenode1.SetFileName(tmp_nrrd1)
    storagenode1.WriteData(dw_node1)

    parsed_nrrd2, dw_node2 = test_nrrd_dwi_load(test_nrrd_path, tmp_nrrd1)

    # re-save NRRD again
    storagenode2 = slicer.vtkMRMLNRRDStorageNode()
    __f_tmp_nrrd2 = tempfile.NamedTemporaryFile(suffix=".nhdr", dir=tmp_dir, delete=False)
    tmp_nrrd2 = __f_tmp_nrrd2.name
    storagenode2.SetFileName(tmp_nrrd2)
    storagenode2.WriteData(dw_node2)

    # test twice-saved file against original NRRD
    parsed_nrrd3, dw_node3 = test_nrrd_dwi_load(test_nrrd_path, tmp_nrrd2)


def run_tests(data_dir, tmp_dir):
    # construct path to test data
    testnrrd_path = os.path.join(data_dir, multishell_dwi_451)

    test_nrrd_dwi_load(testnrrd_path)
    test_nrrd_dwi_roundtrip(testnrrd_path)


if __name__ == '__main__':
    # TODO make sure data paths exist
    data_dir = sys.argv[1]
    tmp_dir = sys.argv[2]

    try:
        run_tests(data_dir, tmp_dir)
        exit(slicer.util.EXIT_SUCCESS)
    except:
        raise

    exit(slicer.util.EXIT_SUCCESS)
