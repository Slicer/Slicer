#!/usr/bin/env python

#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

import os
import re
import sys
import tempfile

from SlicerAppTesting import *

"""
This test verifies the --report-startup-timing command-line option: that the startup
timing report is written once startup is complete, that it contains every startup phase
and the per-module breakdown, that the phases add up to the reported total, that the
total fits within the process runtime measured from the outside, and that without the
option no report is written.

The report is read back from the application's error log, retrieved by a --python-code
snippet that writes it to a file, rather than from the standard streams: whether anything
reaches those depends on whether the launcher and the application were built as console
or GUI executables, and the log is the one place the report goes in every combination.

Usage:
    SlicerReportStartupTimingTest.py /path/to/Slicer
"""

# Runs inside Slicer after startup (and therefore after the report has been logged):
# writes the log entry holding the report, or nothing, to the file named by the
# environment variable.
EXTRACT_REPORT_PYTHON_CODE = (
    "import os;"
    " m = slicer.app.errorLogModel();"
    " entries = [m.logEntryDescription(i) for i in range(m.logEntryCount())];"
    " report = next((e for e in entries if 'Startup timing report' in e), '');"
    " open(os.environ['SLICER_REPORT_STARTUP_TIMING_TEST_OUTPUT'], 'w').write(report)"
)

# One line per startup phase, in the order the phases finish. The wording is part of the
# report's contract with the user, so a wording change is expected to fail this test.
PHASE_NAMES = [
    "Before the process entry point, loading the application's own libraries",
    "Initializing the application",
    "Registering modules",
    "Instantiating modules",
    "Initializing the user interface",
    "Loading modules",
    "Showing the main window",
]

TOTAL_NAME = "Total, from the creation of the process"


def parse_report(output):
    """Return (phase durations in ms by name, total ms, module rows) parsed from the report."""
    phaseDurationsMs = {}
    totalMs = None
    # (name, total, instantiated, loaded), all in fractional milliseconds
    moduleRows = []
    inReport = False
    for line in output.splitlines():
        if line.strip() == "Startup timing report":
            inReport = True
            continue
        if not inReport:
            continue
        phaseMatch = re.match(r"^  (.+): (\d+) ms$", line)
        if phaseMatch:
            if phaseMatch.group(1) == TOTAL_NAME:
                totalMs = int(phaseMatch.group(2))
                # The total closes the timing report; anything after it (such as the
                # startup library prefetching report) is not parsed here.
                break
            phaseDurationsMs[phaseMatch.group(1)] = int(phaseMatch.group(2))
            continue
        moduleMatch = re.match(r"^    (.+): (\d+\.\d) ms \((\d+\.\d) ms \+ (\d+\.\d) ms\)$", line)
        if moduleMatch:
            moduleRows.append((moduleMatch.group(1),
                               float(moduleMatch.group(2)),
                               float(moduleMatch.group(3)),
                               float(moduleMatch.group(4))))
    return (phaseDurationsMs, totalMs, moduleRows)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(os.path.basename(sys.argv[0]) + " /path/to/Slicer")
        exit(EXIT_FAILURE)
    slicer_executable = os.path.expanduser(sys.argv[1])

    reportFile = tempfile.NamedTemporaryFile(suffix=".txt", delete=False)
    reportFile.close()
    os.environ["SLICER_REPORT_STARTUP_TIMING_TEST_OUTPUT"] = reportFile.name

    def read_extracted_report():
        with open(reportFile.name) as f:
            return f.read()

    # CLI modules are disabled only to shorten the test; loadable and scripted modules
    # still register, instantiate and load, which is what the per-module rows report.
    common_args = [
        "--testing",
        "--disable-builtin-cli-modules",
        "--python-code", EXTRACT_REPORT_PYTHON_CODE,
    ]

    # Test that the report is written and adds up. The run is timed from the outside so
    # that the reported total can be checked against reality, not only against itself.
    args = list(common_args)
    args.append("--report-startup-timing")
    runSlicerAndExitWithTime = timecall(runSlicerAndExit)
    (wallClockSeconds, (returnCode, stdout, stderr)) = runSlicerAndExitWithTime(slicer_executable, args)
    assert returnCode == EXIT_SUCCESS
    output = read_extracted_report()
    assert "Startup timing report" in output, "report not found in the application log"

    (phaseDurationsMs, totalMs, moduleRows) = parse_report(output)
    for phaseName in PHASE_NAMES:
        assert phaseName in phaseDurationsMs, "missing phase: " + phaseName
        assert phaseDurationsMs[phaseName] >= 0, "negative duration for phase: " + phaseName
    assert totalMs is not None, "missing total"

    # The reported total covers the creation of the process to the end of the startup,
    # which lies strictly inside what was measured from the outside: the wall clock also
    # covers the launcher, the event loop and the teardown. A reported total that is zero
    # or exceeds the wall clock is not a rounding problem but a wrong measurement, such as
    # the process creation time having been read from the wrong source.
    wallClockMs = wallClockSeconds * 1000.0
    assert 0 < totalMs <= wallClockMs, \
        f"reported total of {totalMs} ms is not within the measured process runtime of {wallClockMs:.0f} ms"

    # Each phase is rounded to a whole millisecond independently of the total, so the sum
    # may be off by half a millisecond per line.
    roundingToleranceMs = len(PHASE_NAMES) + 1
    sumMs = sum(phaseDurationsMs.values())
    assert abs(sumMs - totalMs) <= roundingToleranceMs, \
        f"phases add up to {sumMs} ms but the total says {totalMs} ms"

    assert len(moduleRows) > 0, "no per-module rows found"
    headerMatch = re.search(r"Modules, slowest first, as instantiate \+ load \((\d+) of (\d+)\):", output)
    assert headerMatch, "module list header not found"
    assert len(moduleRows) == int(headerMatch.group(1)), "module row count does not match the header"
    for (name, moduleTotal, instantiated, loaded) in moduleRows:
        assert abs(moduleTotal - (instantiated + loaded)) <= 0.25, \
            f"module {name}: {instantiated} + {loaded} does not add up to {moduleTotal}"
    # Slowest first
    moduleTotals = [row[1] for row in moduleRows]
    assert moduleTotals == sorted(moduleTotals, reverse=True), "module rows are not sorted slowest first"
    print("Test report content - passed\n")

    # Test that without the option there is no report
    args = list(common_args)
    args.extend(["--no-main-window", "--disable-modules"])
    (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, args)
    assert returnCode == EXIT_SUCCESS
    assert read_extracted_report() == "", "report written without --report-startup-timing"
    print("Test no report without option - passed\n")

    os.remove(reportFile.name)
