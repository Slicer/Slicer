import sys, os, re, nose
from nose.tools import assert_equal
from collections import namedtuple

import numpy as np
from numpy.testing import assert_allclose
import slicer

Context = namedtuple('Ctx', ['data_dir', 'temp_dir'])

#===============================================================================

mrmlcore_testdata_path = "Libs/MRML/Core/Testing/TestData/"


multishell_dwi_451 = os.path.join(mrmlcore_testdata_path, "multishell-DWI-451dir.nhdr")

#================================================================================
NRRD = namedtuple('NRRD', ['header', 'bvalue', 'gradients'])

def parse_nhdr(path):
    # TODO: NRRD b-matrix form?
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
            assert(not kvdict.has_key(key))
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
def test_vtkMRMLNRRDStorageNode(ctx):
    testnrrd_path = os.path.join(ctx.data_dir, multishell_dwi_451)

    # load NRRD into Slicer
    storagenode = slicer.vtkMRMLNRRDStorageNode()
    storagenode.SetFileName(testnrrd_path)
    dw_node = slicer.vtkMRMLDiffusionWeightedVolumeNode()
    storagenode.ReadData(dw_node)

    slicer_grads = dw_node.GetDiffusionGradients()
    slicer_numgrads = slicer_grads.GetNumberOfTuples()

    # load NRRD with direct parser
    ext_nrrd = parse_nhdr(testnrrd_path)

    # basic assertions
    assert( len(ext_nrrd.gradients) == slicer_numgrads )


    _ext_bval = ext_nrrd.bvalue
    # Note: vtkDataArray.GetMaxNorm gives max for scalar array.
    _node_bval = dw_node.GetBValues().GetMaxNorm()
    _attr_bval = float(dw_node.GetAttribute("DWMRI_b-value"))
    nose.tools.assert_equal(_ext_bval, _node_bval)
    nose.tools.assert_equal(_ext_bval, _attr_bval)

    # Gradients in the node attribute dictionary must exactly
    #   match those on-disk.
    for i in range(0, slicer_numgrads):
        g_from_nhdr = ext_nrrd.gradients[i]
        g_from_attr = np.fromstring(
                        dw_node.GetAttribute("DWMRI_gradient_{:04d}".format(i)),
                                             sep=" ", dtype=np.float64)

    assert_allclose(np.linalg.norm(g_from_nhdr - g_from_attr), 0.0, atol=1e-12)

    # Gradients in the node DiffusionGradients API must be
    #   match *normalized* gradient.
    for i in range(0, slicer_numgrads):
        _g_tmp = ext_nrrd.gradients[i]
        _g_tmp_norm = np.linalg.norm(_g_tmp)
        # normalize
        g_from_nhdr = _g_tmp if _g_tmp_norm == 0.0       \
                             else (_g_tmp * 1/_g_tmp_norm)

        g_from_node = np.array(slicer_grads.GetTuple3(i))
        print _g_tmp, g_from_nhdr, g_from_node

        assert_allclose(np.linalg.norm(g_from_nhdr - g_from_node), 0.0, atol=1e-12)


if __name__ == '__main__':
    # TODO make sure data paths exist
    ctx = Context(data_dir = sys.argv[1],
                  temp_dir = sys.argv[2])

    success = test_vtkMRMLNRRDStorageNode(ctx)

    sys.exit(success)
