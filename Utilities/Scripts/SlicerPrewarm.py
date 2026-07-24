"""Pre-warm the macOS first-load security scan for Slicer libraries.

On macOS every newly written Mach-O image is scanned by the system security
stack (syspolicyd, then an XProtect YARA scan) the first time it is mapped for
execution -- about 0.13 s per file, and the result is cached per file and
persists across reboots. Slicer ships ~900 libraries, so a fresh build or a
freshly downloaded copy can spend a couple of minutes on this before the first
window appears, while later launches take seconds.

This script performs that scan up front so it is already cached when Slicer
starts. It maps each library with PROT_EXEC, which makes the kernel run the
identical security evaluation and cache it -- without loading the library,
resolving its dependencies, or spawning a process. The evaluation is serialized
by the system daemon, so a single sequential pass is as fast as any parallel
scheme and much simpler.

Importantly, this does not make the first launch faster: the scan is
unavoidable and cannot be parallelized. What it buys is that the wait becomes
visible and explained (an optional progress splash) instead of a frozen window
-- which matters on macOS because application bundles have no launcher and so
no launcher splash to cover the gap. The per-file verdict survives a reboot, so
the stamp file limits the work to the libraries that are actually new: the
first launch after an install, a build, or an update. Unchanged libraries are
never re-processed.

This is macOS-specific. The comparable first-load costs on Windows (antivirus)
and Linux (cold page cache) are addressed differently and are out of scope
here; this script does nothing on other platforms.

It runs in PythonSlicer and is invoked by the macOS application bundle
bootstrap (CMake/SlicerAppBootstrap.c.in) before the application binary is
executed, showing the native progress splash. The bootstrap is the bundle's
CFBundleExecutable in both a build tree and an installed/packaged app, so a
local build behaves the same as a downloaded one.

Usage:
  SlicerPrewarm.py [--scan DIR]... [--stamp FILE] [--gui] [--quiet] [LIB]...

  --scan DIR   directory to scan recursively for shared libraries
  --stamp FILE only process libraries newer than FILE, and touch FILE on
               completion (created if missing: first run processes everything)
  --gui        show a native progress splash
  --quiet      do not print per-library progress lines
  --jobs N     accepted for backward compatibility; ignored (the security
               evaluation is serialized by the system, so the warm-up is
               sequential)
"""

import os
import sys
import time

LIBRARY_EXTENSIONS = (".dylib", ".so")


def parse_arguments(argv):
    options = {"scan": [], "stamp": None, "gui": False, "quiet": False,
               "libraries": []}
    index = 0
    while index < len(argv):
        argument = argv[index]
        if argument == "--scan":
            index += 1
            options["scan"].append(argv[index])
        elif argument == "--stamp":
            index += 1
            options["stamp"] = argv[index]
        elif argument == "--jobs":
            index += 1  # accepted but ignored (see module docstring)
        elif argument == "--gui":
            options["gui"] = True
        elif argument == "--quiet":
            options["quiet"] = True
        else:
            options["libraries"].append(argument)
        index += 1
    return options


def collect_libraries(options):
    libraries = list(options["libraries"])
    for directory in options["scan"]:
        for root, directories, files in os.walk(directory):
            # CLI module libraries are loaded lazily on first use (module
            # metadata comes from the .xml files next to them), not during
            # application startup, so pre-warming them is not worthwhile.
            directories[:] = [d for d in directories if d != "cli-modules"]
            for name in files:
                if name.endswith(LIBRARY_EXTENSIONS):
                    libraries.append(os.path.join(root, name))
    libraries = sorted(set(libraries))
    stamp = options["stamp"]
    if stamp and os.path.exists(stamp):
        # Only libraries built or installed since the last warm-up need
        # processing: the operating system caches the first-load verdict per
        # file and (verified) that verdict survives a reboot, so an unchanged
        # library never needs warming again.
        stamp_time = os.path.getmtime(stamp)
        libraries = [path for path in libraries
                     if os.path.getmtime(path) > stamp_time]
    return libraries


def warm_library(path):
    """Trigger (and cache) the macOS security evaluation of a Mach-O file by
    mapping it with PROT_EXEC. The mapping is expected to be rejected by Library
    Validation -- the file is signed for a different process -- but the
    syspolicyd/XProtect scan has already run and been cached by then, so the
    rejection is ignored. This warms the cache without loading the library or
    resolving its dependencies.
    """
    import mmap
    try:
        descriptor = os.open(path, os.O_RDONLY)
    except OSError:
        return
    try:
        region = mmap.mmap(descriptor, 0, mmap.MAP_PRIVATE,
                           mmap.PROT_READ | mmap.PROT_EXEC)
        region.close()
    except (OSError, ValueError):
        pass  # rejection is expected; the scan already ran and cached
    finally:
        os.close(descriptor)


def warm_libraries(libraries, report, idle=None, should_stop=None):
    """Warm the macOS first-load security cache for each library in turn.

    Calls report(done, total, name) after each library and idle() after each
    (e.g. to keep a splash responsive). If should_stop() returns True the
    warm-up is abandoned and False is returned; libraries already processed
    keep their cache benefit. The scan is serialized by the system daemon, so
    a single sequential pass is as fast as any parallel scheme.
    """
    total = len(libraries)
    for done, path in enumerate(libraries, 1):
        if should_stop and should_stop():
            return False
        warm_library(path)
        report(done, total, os.path.basename(path))
        if idle:
            idle()
    return True


class RemainingTimeEstimator:
    """Estimate the remaining time from the observed completion rate. The
    per-library security-scan cost is essentially uniform (a fixed cost per
    Mach-O), so a running average is accurate after only a few libraries.

    The displayed value never increases: if the honest estimate rises above
    what was last shown, the countdown holds until reality catches up, rather
    than counting down to a number it cannot keep.
    """

    def __init__(self, total):
        self.total = total
        self.start = time.monotonic()
        self.displayed = None

    def update(self, done):
        """Return (fraction_complete, remaining_seconds) after the done-th
        library.
        """
        elapsed = time.monotonic() - self.start
        fraction = done / max(1, self.total)
        estimate = (elapsed / done) * (self.total - done) if done else 0.0
        # Slightly pessimistic while few libraries have been timed; the padding
        # fades as the run progresses.
        estimate *= 1.0 + 0.25 * (1.0 - fraction)
        if self.displayed is None:
            self.displayed = estimate
        else:
            self.displayed = min(self.displayed, estimate)
        return fraction, self.displayed


def format_remaining(remaining):
    """Return a human readable estimate, rounded up to a coarse step so the
    label over-promises slightly and steps down steadily.
    """
    if remaining <= 5:
        return "Almost done"
    for step in (10, 15, 20, 30, 45, 60):
        if remaining <= step:
            return "About %d seconds left" % step
    minutes = int(remaining // 60) + (1 if remaining % 60 else 0)
    return "About %d minute%s left" % (minutes, "" if minutes == 1 else "s")


class MacSplash:
    """Native macOS progress window drawn through the Objective-C runtime via
    ctypes: no GUI toolkit dependencies, only system frameworks (always
    resident) are loaded. The window is a clean, chrome-free card -- app icon,
    a title, a two-line explanation that this is a one-time step, a progress
    bar, a live status line with a time estimate, and a Quit button. Any
    failure disables the splash without affecting the warm-up.
    """

    def __init__(self, application_name):
        self.ok = False
        try:
            import ctypes
            self.ctypes = ctypes
            self.objc = ctypes.CDLL("/usr/lib/libobjc.A.dylib")
            ctypes.CDLL("/System/Library/Frameworks/AppKit.framework/AppKit")
            self.objc.objc_getClass.restype = ctypes.c_void_p
            self.objc.objc_getClass.argtypes = [ctypes.c_char_p]
            self.objc.sel_registerName.restype = ctypes.c_void_p
            self.objc.sel_registerName.argtypes = [ctypes.c_char_p]

            class NSRect(ctypes.Structure):
                _fields_ = [("x", ctypes.c_double), ("y", ctypes.c_double),
                            ("w", ctypes.c_double), ("h", ctypes.c_double)]

            self.NSRect = NSRect
            self.quit_requested = False
            width, height = 520.0, 200.0

            # Titled window with a full-size content view, transparent title
            # bar, hidden title and hidden window buttons: a clean card that
            # can still become key (so the Quit button receives clicks).
            TITLED, FULL_SIZE_CONTENT = 1, 1 << 15
            window = self.send(self.cls("NSWindow"), "alloc")
            window = self.send(
                window, "initWithContentRect:styleMask:backing:defer:",
                NSRect(0, 0, width, height), TITLED | FULL_SIZE_CONTENT, 2, False,
                argtypes=[NSRect, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_bool])
            if not window:
                raise RuntimeError("splash window creation failed")
            self.send(window, "setTitleVisibility:", 1,  # NSWindowTitleHidden
                      restype=None, argtypes=[ctypes.c_long])
            self.send(window, "setTitlebarAppearsTransparent:", True,
                      restype=None, argtypes=[ctypes.c_bool])
            self.send(window, "setMovableByWindowBackground:", True,
                      restype=None, argtypes=[ctypes.c_bool])
            for button in (0, 1, 2):  # close, miniaturize, zoom
                handle = self.send(window, "standardWindowButton:", button,
                                   argtypes=[ctypes.c_long])
                if handle:
                    self.send(handle, "setHidden:", True,
                              restype=None, argtypes=[ctypes.c_bool])
            self.send(window, "center")
            self.send(window, "setLevel:", 5, argtypes=[ctypes.c_long])
            content = self.send(window, "contentView")

            # Optional application icon on the left; the text block shifts right
            # when it is present.
            text_left = 28.0
            icon_path = self._find_icon()
            if icon_path:
                image = self.send(self.send(self.cls("NSImage"), "alloc"),
                                  "initWithContentsOfFile:", self.string(icon_path))
                if image:
                    view = self.send(self.cls("NSImageView"), "imageViewWithImage:", image)
                    self.send(view, "setFrame:", NSRect(28, height - 28 - 64, 64, 64),
                              restype=None, argtypes=[NSRect])
                    self.send(content, "addSubview:", view)
                    text_left = 108.0
            text_width = width - text_left - 28

            title = self._label(content, "Setting up %s" % application_name,
                                NSRect(text_left, 150, text_width, 24),
                                size=15.0, bold=True)
            self._label(content,
                        "macOS is checking this build's components for the first time.",
                        NSRect(text_left, 126, text_width, 18), size=12.0, secondary=True)
            self._label(content,
                        "This happens once after an install or update; later launches are fast.",
                        NSRect(text_left, 108, text_width, 18), size=12.0, secondary=True)

            bar = self.send(self.send(self.cls("NSProgressIndicator"), "alloc"), "init")
            self.send(bar, "setStyle:", 0, restype=None, argtypes=[ctypes.c_long])
            self.send(bar, "setIndeterminate:", False, restype=None, argtypes=[ctypes.c_bool])
            self.send(bar, "setMinValue:", ctypes.c_double(0),
                      restype=None, argtypes=[ctypes.c_double])
            self.send(bar, "setMaxValue:", ctypes.c_double(1000),
                      restype=None, argtypes=[ctypes.c_double])
            self.send(bar, "setFrame:", NSRect(28, 76, width - 56, 16),
                      restype=None, argtypes=[NSRect])
            self.send(content, "addSubview:", bar)

            self.status_label = self._label(content, "Preparing...",
                                            NSRect(28, 50, 300, 18),
                                            size=11.0, secondary=True)
            self.remaining_label = self._label(content, "",
                                               NSRect(width - 28 - 220, 50, 220, 18),
                                               size=11.0, secondary=True, right=True)

            self._add_quit_button(content, NSRect(width - 100, 14, 76, 30))

            application = self.send(self.cls("NSApplication"), "sharedApplication")
            self.send(application, "setActivationPolicy:", 1,
                      restype=ctypes.c_bool, argtypes=[ctypes.c_long])
            self.send(window, "makeKeyAndOrderFront:", None)
            self.send(application, "activateIgnoringOtherApps:", True,
                      restype=None, argtypes=[ctypes.c_bool])
            self.window = window
            self.title_label = title
            self.bar = bar
            self.application = application
            self.runloop = self.send(self.cls("NSRunLoop"), "currentRunLoop")
            self.nsdate = self.cls("NSDate")
            self.default_mode = self.string("kCFRunLoopDefaultMode")
            self.ok = True
            self.pump()
        except Exception:
            self.ok = False

    def _find_icon(self):
        """Return a .icns path from the bundle Resources next to this script,
        or None. In an installed bundle this file is at Contents/bin, so the
        icons are at Contents/Resources.
        """
        try:
            resources = os.path.join(
                os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                "Resources")
            for name in sorted(os.listdir(resources)):
                if name.endswith(".icns"):
                    return os.path.join(resources, name)
        except OSError:
            pass
        return None

    def _label(self, content, text, frame, size=12.0, bold=False,
               secondary=False, right=False):
        ctypes = self.ctypes
        label = self.send(self.cls("NSTextField"), "labelWithString:", self.string(text))
        self.send(label, "setFrame:", frame, restype=None, argtypes=[self.NSRect])
        font_selector = "boldSystemFontOfSize:" if bold else "systemFontOfSize:"
        font = self.send(self.cls("NSFont"), font_selector, ctypes.c_double(size),
                         argtypes=[ctypes.c_double])
        self.send(label, "setFont:", font)
        if secondary:
            self.send(label, "setTextColor:", self.send(self.cls("NSColor"),
                                                         "secondaryLabelColor"))
        if right:
            self.send(label, "setAlignment:", 1, restype=None, argtypes=[ctypes.c_long])
        self.send(content, "addSubview:", label)
        return label

    def _add_quit_button(self, content, frame):
        # "Quit" button wired through a minimal Objective-C target class so the
        # user can abandon the launch if the preparation takes longer than they
        # are willing to wait.
        ctypes = self.ctypes
        QuitAction = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p)

        def quit_clicked(_target, _command, _sender):
            self.quit_requested = True

        self.quit_action = QuitAction(quit_clicked)  # keep the callback alive
        self.objc.objc_allocateClassPair.restype = ctypes.c_void_p
        self.objc.objc_allocateClassPair.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t]
        self.objc.class_addMethod.restype = ctypes.c_bool
        self.objc.class_addMethod.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_char_p]
        self.objc.objc_registerClassPair.argtypes = [ctypes.c_void_p]
        target_class = self.objc.objc_allocateClassPair(self.cls("NSObject"),
                                                        b"SlicerPrewarmQuitTarget", 0)
        self.objc.class_addMethod(target_class, self.objc.sel_registerName(b"quit:"),
                                  ctypes.cast(self.quit_action, ctypes.c_void_p), b"v@:@")
        self.objc.objc_registerClassPair(target_class)
        self.quit_target = self.send(target_class, "new")
        button = self.send(self.cls("NSButton"), "buttonWithTitle:target:action:",
                           self.string("Quit"), self.quit_target,
                           self.objc.sel_registerName(b"quit:"))
        self.send(button, "setFrame:", frame, restype=None, argtypes=[self.NSRect])
        self.send(content, "addSubview:", button)

    def cls(self, name):
        return self.objc.objc_getClass(name.encode())

    def send(self, receiver, selector, *args, restype="pointer", argtypes=None):
        # Default: return a pointer and treat arguments as pointers. Pass
        # restype=None for void methods and explicit argtypes for scalars
        # and structures.
        ctypes = self.ctypes
        if restype == "pointer":
            restype = ctypes.c_void_p
        if argtypes is None:
            argtypes = [ctypes.c_void_p] * len(args)
        function = ctypes.cast(
            self.objc.objc_msgSend,
            ctypes.CFUNCTYPE(restype, ctypes.c_void_p, ctypes.c_void_p, *argtypes))
        return function(receiver, self.objc.sel_registerName(selector.encode()), *args)

    def string(self, text):
        return self.send(self.cls("NSString"), "stringWithUTF8String:",
                         text.encode(), argtypes=[self.ctypes.c_char_p])

    def pump(self):
        # Dispatch pending user interface events (required for the Quit button
        # to react) and give the run loop a moment to redraw.
        ctypes = self.ctypes
        while True:
            event = self.send(self.application,
                              "nextEventMatchingMask:untilDate:inMode:dequeue:",
                              0xFFFFFFFFFFFFFFFF, None, self.default_mode, True,
                              argtypes=[ctypes.c_ulong, ctypes.c_void_p,
                                        ctypes.c_void_p, ctypes.c_bool])
            if not event:
                break
            self.send(self.application, "sendEvent:", event)
        until = self.send(self.nsdate, "dateWithTimeIntervalSinceNow:",
                          ctypes.c_double(0.01), argtypes=[ctypes.c_double])
        self.send(self.runloop, "runUntilDate:", until,
                  restype=None, argtypes=[ctypes.c_void_p])

    def update(self, done, total, name, fraction=None, remaining=None):
        if not self.ok:
            return
        try:
            if fraction is None:
                fraction = float(done) / max(1, total)
            self.send(self.bar, "setDoubleValue:", self.ctypes.c_double(fraction * 1000),
                      restype=None, argtypes=[self.ctypes.c_double])
            self.send(self.status_label, "setStringValue:",
                      self.string("Checking component %d of %d" % (done, total)))
            self.send(self.remaining_label, "setStringValue:",
                      self.string(format_remaining(remaining) if remaining is not None else ""))
            self.pump()
        except Exception:
            self.ok = False

    def close(self):
        if not self.ok:
            return
        try:
            self.send(self.window, "orderOut:", None)
            self.pump()
        except Exception:
            pass


def create_splash(application_name):
    """Return the macOS progress splash, or None if it cannot be shown."""
    splash = MacSplash(application_name)
    return splash if splash.ok else None


def main(argv):
    if sys.platform != "darwin":
        return 0  # the first-load scan this warms is macOS-specific
    options = parse_arguments(argv)
    libraries = collect_libraries(options)
    total = len(libraries)
    if total == 0:
        return 0
    if not options["quiet"]:
        print("Pre-warming %d libraries..." % total, flush=True)

    if options["stamp"]:
        # Stamp first so libraries created while the prewarm runs are picked
        # up by the next invocation instead of being missed. The stamp is only
        # an optimization (it lets later launches skip unchanged libraries), so
        # a stamp location that cannot be written -- e.g. a read-only installed
        # bundle -- must never abort the prewarm: disable stamping and continue.
        try:
            os.makedirs(os.path.dirname(options["stamp"]) or ".", exist_ok=True)
            with open(options["stamp"] + ".next", "w"):
                pass
        except OSError as error:
            print("prewarm: stamp disabled (%s)" % error, flush=True)
            options["stamp"] = None

    estimator = RemainingTimeEstimator(total)
    # The splash is created lazily: warm-ups that finish quickly should not
    # flash a window at all. Once the observed pace predicts the whole run will
    # take longer than this, the splash appears and covers the remainder.
    minimum_duration = 2.0
    splash = [None]  # boxed so the closures below see later assignment

    def report(done, total, name):
        fraction, remaining = estimator.update(done)
        if options["gui"] and splash[0] is None and done < total:
            elapsed = time.monotonic() - estimator.start
            predicted_total = (elapsed / done) * total if done else 0.0
            # Show once a couple of libraries have set the pace and the run is
            # predicted to be long, or as a fallback if it already has been --
            # but never so late that the window would only flash briefly.
            long_enough = (done >= 3 and predicted_total >= minimum_duration) \
                or elapsed >= minimum_duration
            if long_enough and remaining >= 0.75:
                splash[0] = create_splash("3D Slicer")
        if splash[0]:
            splash[0].update(done, total, name, fraction, remaining)
            if os.environ.get("SLICER_PREWARM_DEBUG"):
                print(f"ETA {remaining:.1f} shown='{format_remaining(remaining)}'",
                      flush=True)

    def idle():
        if splash[0] is not None:
            splash[0].pump()

    def should_stop():
        return splash[0] is not None and splash[0].quit_requested

    completed = warm_libraries(libraries, report, idle=idle, should_stop=should_stop)
    if splash[0]:
        splash[0].close()
    if not completed:
        # The user chose to quit from the splash: leave the stamp untouched so
        # the next launch resumes (already-warmed libraries stay cached), and
        # tell the caller not to continue starting the application.
        if options["stamp"]:
            try:
                os.unlink(options["stamp"] + ".next")
            except OSError:
                pass
        print("prewarm: cancelled", flush=True)
        return 75
    if options["stamp"]:
        try:
            os.replace(options["stamp"] + ".next", options["stamp"])
        except OSError as error:
            print("prewarm: stamp not updated (%s)" % error, flush=True)
    if not options["quiet"]:
        print("...pre-warm done", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
