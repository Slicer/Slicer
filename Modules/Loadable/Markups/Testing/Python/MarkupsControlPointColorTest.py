"""Tests for the per-control-point color API on vtkMRMLMarkupsNode.

Covers the data-layer round-trip across the per-point setters, getters,
clearers, and structural mutations (Add / Insert / Remove / Swap /
RemoveAll), plus the JSON storage round-trip. Render-side coverage is
in the manual test scripts under the same directory; this test is
purely a no-GUI sanity check.
"""
import json
import os
import tempfile

import slicer


def fail(msg):
    raise Exception("MarkupsControlPointColorTest: " + msg)


def near(a, b, tol=2.0 / 255.0):
    return abs(a - b) < tol


slicer.mrmlScene.Clear()

fids = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
for i in range(3):
    fids.AddControlPoint([i * 10.0, 0.0, 0.0])

# Initially no overrides
for i in range(3):
    if fids.IsNthControlPointColorOverridden(i):
        fail(f"point {i} should not be overridden initially")

fids.SetNthControlPointColor(0, 1.0, 0.0, 0.0, 1.0)
fids.SetNthControlPointColor(2, 0.0, 0.5, 1.0, 0.5)

# Override flags
if not fids.IsNthControlPointColorOverridden(0):
    fail("point 0 should be overridden")
if fids.IsNthControlPointColorOverridden(1):
    fail("point 1 should NOT be overridden")
if not fids.IsNthControlPointColorOverridden(2):
    fail("point 2 should be overridden")

# Read back via GetNthControlPointColor
rgba = [0.0, 0.0, 0.0, 0.0]
if not fids.GetNthControlPointColor(0, rgba):
    fail("GetNthControlPointColor(0) should return True")
if not (near(rgba[0], 1.0) and near(rgba[3], 1.0)):
    fail(f"point 0 color mismatch: {rgba}")

# Insert in the middle: existing overrides shift right by one
ok = fids.InsertControlPoint(1, [5.0, 5.0, 0.0], "L")
if not ok:
    fail("InsertControlPoint should succeed")
if fids.GetNumberOfControlPoints() != 4:
    fail(f"expected 4 cps after insert, got {fids.GetNumberOfControlPoints()}")
if not fids.IsNthControlPointColorOverridden(0):
    fail("after insert: point 0 should still be overridden")
if fids.IsNthControlPointColorOverridden(1):
    fail("after insert: new point 1 should NOT be overridden")
if fids.IsNthControlPointColorOverridden(2):
    fail("after insert: old point 1 (now 2) should NOT be overridden")
if not fids.IsNthControlPointColorOverridden(3):
    fail("after insert: old point 2 (now 3) should still be overridden")

# Remove leading override: subsequent indices shift down
fids.RemoveNthControlPoint(0)
if fids.GetNumberOfControlPoints() != 3:
    fail(f"expected 3 cps after remove, got {fids.GetNumberOfControlPoints()}")
if fids.IsNthControlPointColorOverridden(0):
    fail("after remove: point 0 should not be overridden")
if not fids.IsNthControlPointColorOverridden(2):
    fail("after remove: point 2 should still be overridden")

# Swap: override flag and color follow the swapped index
fids.SwapControlPoints(0, 2)
if not fids.IsNthControlPointColorOverridden(0):
    fail("after swap: point 0 should be overridden")
if fids.IsNthControlPointColorOverridden(2):
    fail("after swap: point 2 should NOT be overridden")

# Clear single
fids.ClearNthControlPointColor(0)
if fids.IsNthControlPointColorOverridden(0):
    fail("after ClearNthControlPointColor: point 0 should be cleared")

# Clear all
fids.SetNthControlPointColor(0, 0.2, 0.3, 0.4)
fids.SetNthControlPointColor(1, 0.5, 0.6, 0.7)
fids.ClearAllControlPointColors()
for i in range(fids.GetNumberOfControlPoints()):
    if fids.IsNthControlPointColorOverridden(i):
        fail(f"after ClearAllControlPointColors: point {i} should be cleared")

# JSON storage round-trip: write then read into a fresh node, verify per-point
# colors round-trip and that "color" is omitted for non-overridden points.
fidsRT = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
fidsRT.AddControlPoint([0.0, 0.0, 0.0])
fidsRT.AddControlPoint([1.0, 0.0, 0.0])
fidsRT.AddControlPoint([2.0, 0.0, 0.0])
fidsRT.SetNthControlPointColor(0, 0.9, 0.1, 0.2, 1.0)
fidsRT.SetNthControlPointColor(2, 0.3, 0.4, 0.5, 0.6)

tmpPath = tempfile.NamedTemporaryFile(suffix=".mrk.json", delete=False).name
try:
    snWrite = slicer.app.coreIOManager().createAndAddDefaultStorageNode(fidsRT)
    snWrite.SetFileName(tmpPath)
    if not snWrite.WriteData(fidsRT):
        fail("WriteData failed")

    # Sanity check on the written file: only the two overridden points should
    # have a "color" field. (The display node section also emits its own
    # "color" property, so parse the JSON properly rather than text-counting.)
    with open(tmpPath) as f:
        parsed = json.load(f)
    cpsWithColor = sum(1 for cp in parsed["markups"][0]["controlPoints"] if "color" in cp)
    if cpsWithColor != 2:
        fail(f'expected 2 controlPoints with "color" field, got {cpsWithColor}')

    loaded = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
    snRead = slicer.app.coreIOManager().createAndAddDefaultStorageNode(loaded)
    snRead.SetFileName(tmpPath)
    if not snRead.ReadData(loaded):
        fail("ReadData failed")

    if loaded.GetNumberOfControlPoints() != 3:
        fail(f"after JSON round-trip: expected 3 cps, got {loaded.GetNumberOfControlPoints()}")
    if not loaded.IsNthControlPointColorOverridden(0):
        fail("after JSON round-trip: point 0 should be overridden")
    if loaded.IsNthControlPointColorOverridden(1):
        fail("after JSON round-trip: point 1 should NOT be overridden")
    if not loaded.IsNthControlPointColorOverridden(2):
        fail("after JSON round-trip: point 2 should be overridden")

    rgba = [0.0, 0.0, 0.0, 0.0]
    loaded.GetNthControlPointColor(0, rgba)
    if not (near(rgba[0], 0.9) and near(rgba[1], 0.1) and near(rgba[2], 0.2) and near(rgba[3], 1.0)):
        fail(f"after JSON round-trip: point 0 RGBA mismatch: {rgba}")
    loaded.GetNthControlPointColor(2, rgba)
    if not (near(rgba[0], 0.3) and near(rgba[1], 0.4) and near(rgba[2], 0.5) and near(rgba[3], 0.6)):
        fail(f"after JSON round-trip: point 2 RGBA mismatch: {rgba}")
finally:
    if os.path.exists(tmpPath):
        os.unlink(tmpPath)

# Display node flag setter / getter
d = fids.GetDisplayNode()
if d is None:
    fids.CreateDefaultDisplayNodes()
    d = fids.GetDisplayNode()
d.SetUseControlPointColors(True)
if not d.GetUseControlPointColors():
    fail("UseControlPointColors flag setter/getter mismatch")
d.SetUseControlPointColors(False)
if d.GetUseControlPointColors():
    fail("UseControlPointColors clear failed")

print("MarkupsControlPointColorTest passed")
