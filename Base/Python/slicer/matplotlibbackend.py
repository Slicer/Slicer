"""Interactive Matplotlib backend for 3D Slicer, built on PythonQt.

Slicer embeds Qt through `PythonQt <https://mevislab.github.io/pythonqt/>`_ rather than
through PyQt or PySide, and its Python distribution is built without Tcl/Tk. As a
consequence none of the interactive backends shipped with Matplotlib can be used:

* ``TkAgg`` fails because the ``_tkinter`` extension module is not built.
* ``QtAgg``/``Qt5Agg`` fail because ``PyQt5``/``PySide2``/``PySide6`` are not available,
  and installing one of them would load a second, independent copy of the Qt libraries
  into a process that has already initialized Slicer's own copy.
* ``WXAgg`` requires the extra ``wxPython`` dependency and runs the figure in a window
  driven by a foreign event loop.

This module implements a Matplotlib backend that renders with the Agg rasterizer and
blits the resulting buffer into a ``QWidget`` managed by Slicer's own event loop, so
figures are fully interactive (pan, zoom, pick, hover, widgets, animations) and can be
embedded directly in module panels.

Select the backend the usual way::

    import matplotlib
    matplotlib.use("module://slicer.matplotlibbackend")

    import matplotlib.pyplot as plt
    fig, ax = plt.subplots()
    ax.plot([0, 1, 2], [0, 1, 0])
    plt.show()

or let Slicer configure it for you, which additionally enables interactive mode::

    import slicer.matplotlibbackend
    slicer.matplotlibbackend.enable()

To embed a figure in a scripted module rather than in its own window, take the Qt widget
that belongs to the canvas::

    from matplotlib.figure import Figure
    from slicer.matplotlibbackend import FigureCanvas, NavigationToolbar2Slicer

    figure = Figure()
    canvas = FigureCanvas(figure)
    layout.addWidget(canvas.get_widget())
    layout.addWidget(NavigationToolbar2Slicer(canvas).get_widget())

Requires Matplotlib 3.10 or later.

Notes
-----
The canvas deliberately *contains* a ``QWidget`` instead of inheriting from one. Slicer's
Qt binding wraps C++ classes dynamically, and combining such a wrapper with a second
Python base class in a single ``class`` statement is not part of the binding's supported
surface. Composition relies only on documented behaviour: subclassing a single wrapped
class and overriding its virtual methods.
"""

import sys
import weakref

import matplotlib

# MouseEvent(buttons=...), used to report the pressed buttons on motion, was added in 3.10.
MINIMUM_MATPLOTLIB_VERSION = (3, 10)
if tuple(matplotlib.__version_info__[:2]) < MINIMUM_MATPLOTLIB_VERSION:
    raise ImportError(
        "slicer.matplotlibbackend requires matplotlib >= %d.%d, but %s is installed. "
        "Upgrade it with slicer.packaging.pip_ensure('matplotlib>=%d.%d') and restart Slicer."
        % (*MINIMUM_MATPLOTLIB_VERSION, matplotlib.__version__, *MINIMUM_MATPLOTLIB_VERSION))

from matplotlib import backend_tools, cbook
from matplotlib._pylab_helpers import Gcf
from matplotlib.backend_bases import (
    CloseEvent,
    FigureManagerBase,
    KeyEvent,
    LocationEvent,
    MouseButton,
    MouseEvent,
    NavigationToolbar2,
    ResizeEvent,
    TimerBase,
    _Backend,
)
from matplotlib.backends.backend_agg import FigureCanvasAgg

import qt

__all__ = [
    "FigureCanvas",  #noqa: F822
    "FigureCanvasSlicer",
    "FigureManager",  #noqa: F822
    "FigureManagerSlicer",
    "NavigationToolbar2Slicer",
    "enable",
]

backend_version = matplotlib.__version__

#: Name to pass to :func:`matplotlib.use`.
BACKEND_NAME = "module://slicer.matplotlibbackend"


# ---------------------------------------------------------------------------
# PythonQt compatibility helpers
# ---------------------------------------------------------------------------

def _value(obj, name, *args):
    """Return ``obj.name``, calling it when the binding exposes it as a method.

    PythonQt surfaces ``Q_PROPERTY`` members such as ``QWidget::width`` as plain
    attributes while PyQt/PySide surface them as methods. Accessing them through this
    helper keeps the backend working under either convention.
    """
    attr = getattr(obj, name)
    return attr(*args) if callable(attr) else attr


def _connect_weakly(sender, signal, method, *args):
    """Connect ``signal`` to the bound ``method`` without keeping its object alive.

    PythonQt connections hold a strong reference to the Python callable, and that
    reference lives on the C++ side where the garbage collector cannot see it. A
    connection to a bound method of an object that also owns the sender therefore
    forms a cycle that is never collected. The slot is a no-op once the object is gone.
    """
    ref = weakref.WeakMethod(method)

    def slot(*signal_args):
        target = ref()
        if target is not None:
            target(*args)

    sender.connect(signal, slot)


def _int(value):
    """Coerce a Qt enum/flag wrapper to a plain ``int``."""
    try:
        return int(value)
    except (TypeError, ValueError):
        return int(getattr(value, "value", 0))


# ---------------------------------------------------------------------------
# Key and button translation
# ---------------------------------------------------------------------------

# Qt keys that do not map onto their Unicode name and therefore need an explicit
# Matplotlib name. Mirrors ``matplotlib.backends.backend_qt.SPECIAL_KEYS``.
SPECIAL_KEYS = {
    _int(getattr(qt.Qt, name)): mpl_name
    for name, mpl_name in [
        ("Key_Escape", "escape"),
        ("Key_Tab", "tab"),
        ("Key_Backspace", "backspace"),
        ("Key_Return", "enter"),
        ("Key_Enter", "enter"),
        ("Key_Insert", "insert"),
        ("Key_Delete", "delete"),
        ("Key_Pause", "pause"),
        ("Key_SysReq", "sysreq"),
        ("Key_Clear", "clear"),
        ("Key_Home", "home"),
        ("Key_End", "end"),
        ("Key_Left", "left"),
        ("Key_Up", "up"),
        ("Key_Right", "right"),
        ("Key_Down", "down"),
        ("Key_PageUp", "pageup"),
        ("Key_PageDown", "pagedown"),
        ("Key_Shift", "shift"),
        # macOS swaps the control and command keys.
        ("Key_Control", "control" if sys.platform != "darwin" else "cmd"),
        ("Key_Meta", "meta" if sys.platform != "darwin" else "control"),
        ("Key_Alt", "alt"),
        ("Key_CapsLock", "caps_lock"),
        ("Key_F1", "f1"),
        ("Key_F2", "f2"),
        ("Key_F3", "f3"),
        ("Key_F4", "f4"),
        ("Key_F5", "f5"),
        ("Key_F6", "f6"),
        ("Key_F7", "f7"),
        ("Key_F8", "f8"),
        ("Key_F9", "f9"),
        ("Key_F10", "f10"),
        ("Key_F11", "f11"),
        ("Key_F12", "f12"),
        ("Key_Super_L", "super"),
        ("Key_Super_R", "super"),
    ]
    if hasattr(qt.Qt, name)
}

# (Qt::KeyboardModifier, Qt::Key) pairs. The order fixes the order in which Matplotlib
# reports combined modifiers, e.g. ``ctrl+alt+a``.
_MODIFIER_KEYS = [
    (_int(getattr(qt.Qt, modifier)), _int(getattr(qt.Qt, key)))
    for modifier, key in [
        ("ControlModifier", "Key_Control"),
        ("AltModifier", "Key_Alt"),
        ("ShiftModifier", "Key_Shift"),
        ("MetaModifier", "Key_Meta"),
    ]
    if hasattr(qt.Qt, modifier) and hasattr(qt.Qt, key)
]

_BUTTON_MAP = {
    _int(getattr(qt.Qt, name)): button
    for name, button in [
        ("LeftButton", MouseButton.LEFT),
        ("MiddleButton", MouseButton.MIDDLE),
        ("RightButton", MouseButton.RIGHT),
        # Qt 5 spells the extra mouse buttons XButton1/XButton2; Qt 6 keeps those
        # names as aliases but prefers BackButton/ForwardButton.
        ("XButton1", MouseButton.BACK),
        ("XButton2", MouseButton.FORWARD),
        ("BackButton", MouseButton.BACK),
        ("ForwardButton", MouseButton.FORWARD),
    ]
    if hasattr(qt.Qt, name)
}

_CURSOR_MAP = {
    cursor: getattr(qt.Qt, name)
    for cursor, name in [
        (backend_tools.Cursors.MOVE, "SizeAllCursor"),
        (backend_tools.Cursors.HAND, "PointingHandCursor"),
        (backend_tools.Cursors.POINTER, "ArrowCursor"),
        (backend_tools.Cursors.SELECT_REGION, "CrossCursor"),
        (backend_tools.Cursors.WAIT, "WaitCursor"),
        (backend_tools.Cursors.RESIZE_HORIZONTAL, "SizeHorCursor"),
        (backend_tools.Cursors.RESIZE_VERTICAL, "SizeVerCursor"),
    ]
    if hasattr(qt.Qt, name)
}


def _keyboard_modifiers(modifiers=None, *, exclude=None):
    """Return the Matplotlib names of the currently pressed modifier keys."""
    if modifiers is None:
        modifiers = qt.QApplication.keyboardModifiers()
    modifiers = _int(modifiers)
    # Matplotlib spells the standalone key "control" but the modifier "ctrl".
    return [
        SPECIAL_KEYS[key].replace("control", "ctrl")
        for mask, key in _MODIFIER_KEYS
        if key != exclude and modifiers & mask and key in SPECIAL_KEYS
    ]


def _pressed_buttons(buttons):
    buttons = _int(buttons)
    return {button for mask, button in _BUTTON_MAP.items() if mask & buttons}


# ---------------------------------------------------------------------------
# Timer
# ---------------------------------------------------------------------------

class TimerSlicer(TimerBase):
    """`.TimerBase` implementation driven by ``QTimer``, used for animations."""

    def __init__(self, *args, parent=None, **kwargs):
        # Parent the QTimer to the canvas widget so that it is destroyed with it.
        self._timer = qt.QTimer() if parent is None else qt.QTimer(parent)
        _connect_weakly(self._timer, "timeout()", self._on_qt_timeout)
        super().__init__(*args, **kwargs)

    def __del__(self):
        # A parented QTimer outlives this object, so make sure it stops firing.
        try:
            self._timer.stop()
        except (AttributeError, RuntimeError, ValueError):
            pass

    def _on_qt_timeout(self):
        self._on_timer()

    def _timer_set_single_shot(self):
        self._timer.setSingleShot(self._single)

    def _timer_set_interval(self):
        self._timer.setInterval(int(self._interval))

    def _timer_start(self):
        self._timer.start()

    def _timer_stop(self):
        self._timer.stop()


# ---------------------------------------------------------------------------
# Canvas widget
# ---------------------------------------------------------------------------

class _CanvasWidget(qt.QWidget):
    """``QWidget`` that displays a figure and forwards input to its canvas."""

    def __init__(self, canvas):
        qt.QWidget.__init__(self)
        self._canvas = canvas
        self._size_hint = None
        self.setMouseTracking(True)
        self.setFocusPolicy(qt.Qt.StrongFocus)
        self.setAttribute(qt.Qt.WA_OpaquePaintEvent)
        self.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
        self.setMinimumSize(qt.QSize(10, 10))

    # -- painting ----------------------------------------------------------
    def paintEvent(self, event):
        canvas = self._canvas
        if canvas is None or canvas.figure is None:
            return
        image = canvas._qimage()
        painter = qt.QPainter()
        painter.begin(self)
        try:
            if image is None or image.isNull():
                painter.fillRect(_value(self, "rect"), qt.QColor(255, 255, 255))
            else:
                painter.drawImage(qt.QPointF(0.0, 0.0), image)
                # The zoom-to-rect rubberband is painted on top of the figure.
                if canvas._rubberband_rect is not None:
                    x0, y0, x1, y1 = canvas._rubberband_rect
                    pen = qt.QPen(qt.QColor(0, 0, 0), 1.0 / canvas.device_pixel_ratio)
                    pen.setStyle(qt.Qt.DotLine)
                    painter.setPen(pen)
                    painter.drawRect(qt.QRectF(x0, y0, x1 - x0, y1 - y0))
        finally:
            painter.end()

    # -- geometry ----------------------------------------------------------
    def resizeEvent(self, event):
        canvas = self._canvas
        if canvas is None or canvas.figure is None:
            return
        canvas._on_resize(event.size().width(), event.size().height())

    def sizeHint(self):
        if self._size_hint is not None:
            return self._size_hint
        canvas = self._canvas
        if canvas is None:
            return qt.QSize(640, 480)
        width, height = canvas.get_width_height()
        return qt.QSize(width, height)

    def minimumSizeHint(self):
        return qt.QSize(10, 10)

    # -- input -------------------------------------------------------------
    def enterEvent(self, event):
        canvas = self._canvas
        if canvas is None or canvas.figure is None:
            return
        LocationEvent(
            "figure_enter_event", canvas, *canvas._event_coords(),
            modifiers=_keyboard_modifiers(), guiEvent=event)._process()

    def leaveEvent(self, event):
        canvas = self._canvas
        if canvas is None or canvas.figure is None:
            return
        LocationEvent(
            "figure_leave_event", canvas, *canvas._event_coords(),
            modifiers=_keyboard_modifiers(), guiEvent=event)._process()

    def mousePressEvent(self, event):
        self._mouse_event("button_press_event", event)

    def mouseDoubleClickEvent(self, event):
        self._mouse_event("button_press_event", event, dblclick=True)

    def mouseReleaseEvent(self, event):
        self._mouse_event("button_release_event", event)

    def mouseMoveEvent(self, event):
        canvas = self._canvas
        if canvas is None or canvas.figure is None:
            return
        MouseEvent(
            "motion_notify_event", canvas, *canvas._event_coords(event),
            buttons=_pressed_buttons(event.buttons()),
            modifiers=_keyboard_modifiers(event.modifiers()),
            guiEvent=event)._process()

    def wheelEvent(self, event):
        canvas = self._canvas
        if canvas is None or canvas.figure is None:
            return
        steps = self._wheel_steps(event)
        if not steps:
            return
        MouseEvent(
            "scroll_event", canvas, *canvas._event_coords(event), step=steps,
            modifiers=_keyboard_modifiers(event.modifiers()),
            guiEvent=event)._process()

    def keyPressEvent(self, event):
        self._key_event("key_press_event", event)

    def keyReleaseEvent(self, event):
        self._key_event("key_release_event", event)

    # -- input helpers -----------------------------------------------------
    def _mouse_event(self, name, event, dblclick=False):
        canvas = self._canvas
        if canvas is None or canvas.figure is None:
            return
        button = _BUTTON_MAP.get(_int(event.button()))
        if button is None:
            return
        MouseEvent(
            name, canvas, *canvas._event_coords(event), button, dblclick=dblclick,
            modifiers=_keyboard_modifiers(event.modifiers()),
            guiEvent=event)._process()

    def _key_event(self, name, event):
        canvas = self._canvas
        if canvas is None or canvas.figure is None:
            return
        key = self._mpl_key(event)
        if key is None:
            return
        KeyEvent(name, canvas, key, *canvas._event_coords(), guiEvent=event)._process()

    @staticmethod
    def _wheel_steps(event):
        """Return the scroll amount in Matplotlib "steps"."""
        try:
            pixel_delta = event.pixelDelta()
            # pixelDelta() is unset on most platforms and unreliable under X11.
            if not pixel_delta.isNull() and qt.QApplication.platformName() != "xcb":
                return pixel_delta.y()
        except AttributeError:
            pass
        try:
            return event.angleDelta().y() / 120
        except AttributeError:
            return event.delta() / 120

    @staticmethod
    def _mpl_key(event):
        key_code = _int(event.key())
        modifiers = _keyboard_modifiers(event.modifiers(), exclude=key_code)
        if key_code in SPECIAL_KEYS:
            key = SPECIAL_KEYS[key_code]
        else:
            # Qt uses key codes beyond the Unicode range for non-character keys
            # (multimedia keys and similar); those have no Matplotlib name.
            if key_code > sys.maxunicode:
                return None
            key = chr(key_code)
            # Qt always reports capital letters, so restore the real case.
            if "shift" in modifiers:
                modifiers.remove("shift")
            else:
                key = key.lower()
        return "+".join(modifiers + [key])


class FigureCanvasSlicer(FigureCanvasAgg):
    """Agg-rendered Matplotlib canvas displayed in a Slicer ``QWidget``."""

    required_interactive_framework = "qt"
    manager_class = None  # Set to FigureManagerSlicer once it is defined.

    def __init__(self, figure=None):
        super().__init__(figure=figure)
        self._draw_pending = False
        self._is_drawing = False
        self._in_resize_event = False
        self._rubberband_rect = None
        # QImage does not copy the buffer it is constructed from, so the backing
        # bytes must be kept alive for as long as the image may be painted.
        self._image_buffer = None
        self._image = None
        self._event_loop = None

        self._widget = _CanvasWidget(self)
        self._idle_timer = qt.QTimer(self._widget)
        self._idle_timer.setSingleShot(True)
        self._idle_timer.setInterval(0)
        _connect_weakly(self._idle_timer, "timeout()", self._draw_idle)

        self._update_device_pixel_ratio()
        width, height = self.get_width_height()
        self._widget.resize(width, height)

    # -- widget access -----------------------------------------------------
    def get_widget(self):
        """Return the ``QWidget`` that displays this canvas."""
        return self._widget

    #: The ``QWidget`` that displays this canvas.
    widget = property(get_widget)

    def set_size_hint(self, width=None, height=None):
        """Override the preferred size the canvas widget reports to Qt.

        By default the widget reports the figure size, which makes it claim a
        large share of the space when it is embedded next to other widgets.
        Pass a small size to let the surrounding layout drive the geometry, or
        ``None`` to restore the default behavior.
        """
        if width is None or height is None:
            self._widget._size_hint = None
        else:
            self._widget._size_hint = qt.QSize(int(width), int(height))
        self._widget.updateGeometry()

    # -- rendering ---------------------------------------------------------
    def _update_device_pixel_ratio(self):
        ratio = _value(self._widget, "devicePixelRatioF")
        return self._set_device_pixel_ratio(ratio)

    def _qimage(self):
        """Return the ``QImage`` holding the most recent Agg rendering."""
        if self._image is None:
            self._rebuild_image()
        return self._image

    def _rebuild_image(self):
        try:
            buffer = memoryview(self.buffer_rgba()).tobytes()
        except (AttributeError, RuntimeError, ValueError):
            # Nothing has been rendered yet.
            self._image_buffer = None
            self._image = None
            return
        width, height = self.get_width_height(physical=True)
        if width <= 0 or height <= 0 or len(buffer) < width * height * 4:
            self._image_buffer = None
            self._image = None
            return
        self._image_buffer = buffer
        image = qt.QImage(buffer, width, height, qt.QImage.Format_RGBA8888)
        image.setDevicePixelRatio(self.device_pixel_ratio)
        self._image = image

    def draw(self):
        """Render the figure with Agg and schedule a Qt repaint."""
        if self._is_drawing:
            return
        with cbook._setattr_cm(self, _is_drawing=True):
            super().draw()
        self._image = None
        self._widget.update()

    def draw_idle(self):
        """Coalesce redraw requests and service them from the Qt event loop."""
        if not (self._draw_pending or self._is_drawing):
            self._draw_pending = True
            self._idle_timer.start()

    def _draw_idle(self):
        with self._idle_draw_cntx():
            if not self._draw_pending:
                return
            self._draw_pending = False
            if _value(self._widget, "width") <= 0 or _value(self._widget, "height") <= 0:
                return
            try:
                self.draw()
            except Exception:
                # An exception escaping a Qt slot would terminate the application.
                import traceback
                traceback.print_exc()

    def blit(self, bbox=None):
        # docstring inherited
        if bbox is None and self.figure is not None:
            bbox = self.figure.bbox
        if bbox is None:
            return
        self._image = None
        ratio = self.device_pixel_ratio
        left, bottom, width, height = (int(value / ratio) for value in bbox.bounds)
        top = bottom + height
        self._widget.repaint(left, _value(self._widget, "height") - top, width, height)

    def drawRectangle(self, rect):
        """Show or hide the zoom-to-rectangle rubberband."""
        if rect is None:
            self._rubberband_rect = None
        else:
            ratio = self.device_pixel_ratio
            x0, y0, width, height = (value / ratio for value in rect)
            self._rubberband_rect = (x0, y0, x0 + width, y0 + height)
        self._widget.update()

    # -- geometry ----------------------------------------------------------
    def _on_resize(self, width, height):
        # Qt can re-enter resizeEvent while the figure is being resized.
        if self._in_resize_event:
            return
        self._in_resize_event = True
        try:
            self._update_device_pixel_ratio()
            ratio = self.device_pixel_ratio
            dpi = self.figure.dpi
            self.figure.set_size_inches(
                width * ratio / dpi, height * ratio / dpi, forward=False)
            ResizeEvent("resize_event", self)._process()
            self.draw_idle()
        finally:
            self._in_resize_event = False

    def _event_coords(self, event=None):
        """Return the physical-pixel figure coordinates of a Qt mouse position.

        Qt reports logical pixels with the origin in the top-left corner, whereas
        Matplotlib expects physical pixels with the origin in the bottom-left corner.
        """
        if event is None:
            position = self._widget.mapFromGlobal(qt.QCursor.pos())
            x, y = position.x(), position.y()
        else:
            x, y = self._position_of(event)
        ratio = self.device_pixel_ratio
        # Flip y so that y=0 is the bottom of the canvas.
        return x * ratio, (self.get_width_height()[1] - y) * ratio

    @staticmethod
    def _position_of(event):
        """Return the widget-relative ``(x, y)`` of a Qt mouse event.

        Qt 6 removed ``QMouseEvent::x()``/``y()`` in favour of ``position()``, so the
        accessors are tried in the order that works across both major versions.
        """
        for name in ("position", "pos"):
            accessor = getattr(event, name, None)
            if accessor is None:
                continue
            try:
                position = accessor()
            except TypeError:
                continue
            return position.x(), position.y()
        return event.x(), event.y()

    # -- interaction -------------------------------------------------------
    def set_cursor(self, cursor):
        # docstring inherited
        shape = _CURSOR_MAP.get(cursor, qt.Qt.ArrowCursor)
        self._widget.setCursor(qt.QCursor(shape))

    def new_timer(self, *args, **kwargs):
        # docstring inherited
        return TimerSlicer(*args, parent=self._widget, **kwargs)

    def flush_events(self):
        # docstring inherited
        qt.QApplication.processEvents()

    def start_event_loop(self, timeout=0):
        # docstring inherited
        if self._event_loop is not None and self._event_loop.isRunning():
            raise RuntimeError("Event loop already running")
        self._event_loop = qt.QEventLoop()
        timer = None
        if timeout > 0:
            timer = qt.QTimer(self._widget)
            timer.setSingleShot(True)
            timer.setInterval(int(timeout * 1000))
            timer.connect("timeout()", self._event_loop.quit)
            timer.start()
        try:
            # Qt 6 renamed QEventLoop::exec_() back to exec().
            executor = getattr(self._event_loop, "exec_", None) or self._event_loop.exec
            executor()
        finally:
            if timer is not None:
                timer.stop()
            self._event_loop = None

    def stop_event_loop(self, event=None):
        # docstring inherited
        if self._event_loop is not None:
            self._event_loop.quit()


# ---------------------------------------------------------------------------
# Navigation toolbar
# ---------------------------------------------------------------------------

class NavigationToolbar2Slicer(NavigationToolbar2):
    """Standard Matplotlib navigation toolbar rendered as a ``QToolBar``."""

    def __init__(self, canvas, parent=None, coordinates=True):
        self._coordinates = coordinates
        self._actions = {}
        self._toolbar = qt.QToolBar(parent)
        self._toolbar.setIconSize(qt.QSize(20, 20))
        self._message_label = None

        for text, tooltip, image, callback in self.toolitems:
            if text is None:
                self._toolbar.addSeparator()
                continue
            action = self._toolbar.addAction(text)
            action.setToolTip(tooltip or text)
            icon = self._icon(image)
            if icon is not None:
                action.setIcon(icon)
            if callback in ("zoom", "pan"):
                action.setCheckable(True)
            _connect_weakly(action, "triggered(bool)", getattr(self, callback))
            self._actions[callback] = action

        if coordinates:
            spacer = qt.QWidget()
            spacer.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
            self._toolbar.addWidget(spacer)
            self._message_label = qt.QLabel("")
            self._message_label.setAlignment(qt.Qt.AlignRight | qt.Qt.AlignVCenter)
            self._toolbar.addWidget(self._message_label)

        super().__init__(canvas)

    def get_widget(self):
        """Return the ``QToolBar`` backing this toolbar."""
        return self._toolbar

    #: The ``QToolBar`` backing this toolbar.
    widget = property(get_widget)

    @staticmethod
    def _icon(name):
        if not name:
            return None
        path = cbook._get_data_path("images", f"{name}_large.png")
        if not path.exists():
            path = cbook._get_data_path("images", f"{name}.png")
        if not path.exists():
            return None
        pixmap = qt.QPixmap(str(path))
        return qt.QIcon(pixmap) if not pixmap.isNull() else None

    # -- NavigationToolbar2 interface --------------------------------------
    def set_message(self, s):
        # docstring inherited
        if self._message_label is not None:
            self._message_label.setText(s)

    def set_history_buttons(self):
        # docstring inherited
        can_backward = self._nav_stack._pos > 0
        can_forward = self._nav_stack._pos < len(self._nav_stack) - 1
        if "back" in self._actions:
            self._actions["back"].setEnabled(can_backward)
        if "forward" in self._actions:
            self._actions["forward"].setEnabled(can_forward)

    def pan(self, *args):
        super().pan(*args)
        self._update_checked_actions()

    def zoom(self, *args):
        super().zoom(*args)
        self._update_checked_actions()

    def _update_checked_actions(self):
        mode = str(self.mode)
        if "pan" in self._actions:
            self._actions["pan"].setChecked(mode == "pan/zoom")
        if "zoom" in self._actions:
            self._actions["zoom"].setChecked(mode == "zoom rect")

    def draw_rubberband(self, event, x0, y0, x1, y1):
        # docstring inherited
        height = self.canvas.figure.bbox.height
        self.canvas.drawRectangle(
            (min(x0, x1), height - max(y0, y1), abs(x1 - x0), abs(y1 - y0)))

    def remove_rubberband(self):
        # docstring inherited
        self.canvas.drawRectangle(None)

    def save_figure(self, *args):
        # docstring inherited
        import os

        filetypes = self.canvas.get_supported_filetypes_grouped()
        default_filetype = self.canvas.get_default_filetype()
        filters = []
        selected_filter = None
        for name, extensions in sorted(filetypes.items()):
            pattern = " ".join(f"*.{extension}" for extension in extensions)
            filter_ = f"{name} ({pattern})"
            if default_filetype in extensions:
                selected_filter = filter_
            filters.append(filter_)

        start = self.canvas.get_default_filename()
        directory = matplotlib.rcParams["savefig.directory"]
        if directory:
            start = os.path.join(os.path.expanduser(directory), start)

        filename = qt.QFileDialog.getSaveFileName(
            self._toolbar, "Choose a filename to save to", start,
            ";;".join(filters), selected_filter)
        if isinstance(filename, (tuple, list)):
            filename = filename[0]
        if not filename:
            return
        if directory:
            matplotlib.rcParams["savefig.directory"] = os.path.dirname(filename)
        try:
            self.canvas.figure.savefig(filename)
        except Exception as exception:
            qt.QMessageBox.critical(self._toolbar, "Error saving file", str(exception))

    def configure_subplots(self, *args):
        # docstring inherited: the standard subplot tool window is not provided, so
        # apply the layout engine that produces an equivalent result instead.
        self.canvas.figure.tight_layout()
        self.canvas.draw_idle()


# ---------------------------------------------------------------------------
# Figure manager
# ---------------------------------------------------------------------------

class _FigureWindow(qt.QWidget):
    """Top-level window hosting a figure canvas and its navigation toolbar."""

    def __init__(self, manager):
        qt.QWidget.__init__(self)
        self._manager = manager

    def closeEvent(self, event):
        manager = self._manager
        if manager is not None:
            manager._on_window_closed()
        event.accept()


class FigureManagerSlicer(FigureManagerBase):
    """Figure manager that shows a figure in its own Slicer-owned window."""

    # ``FigureManagerBase.__init__`` instantiates this to populate ``self.toolbar``.
    _toolbar2_class = NavigationToolbar2Slicer

    def __init__(self, canvas, num):
        self._destroying = False
        self.window = _FigureWindow(self)
        self.window.setWindowTitle(f"Figure {num}")

        self._layout = qt.QVBoxLayout(self.window)
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._layout.setSpacing(0)
        self._layout.addWidget(canvas.get_widget())

        # Creates ``self.toolbar`` when the ``toolbar`` rcParam asks for one.
        super().__init__(canvas, num)

        if self.toolbar is not None:
            self._layout.addWidget(self.toolbar.get_widget())

        width, height = canvas.get_width_height()
        self.window.resize(width, height + self._toolbar_height())

    def _toolbar_height(self):
        if self.toolbar is None:
            return 0
        return max(_value(self.toolbar.get_widget(), "sizeHint").height(), 0)

    # -- window operations -------------------------------------------------
    def show(self):
        # docstring inherited
        self.window.show()
        self.window.raise_()
        self.canvas.draw_idle()

    def destroy(self, *args):
        # docstring inherited
        if self._destroying:
            return
        self._destroying = True
        # Emitted here so that ``close_event`` fires exactly once whether the figure
        # is closed programmatically or by the user closing the window.
        CloseEvent("close_event", self.canvas)._process()
        window, self.window = self.window, None
        if window is not None:
            window._manager = None
            window.close()
            window.setParent(None)
            window.deleteLater()
        super().destroy()

    def resize(self, width, height):
        # docstring inherited: the requested size is in physical pixels.
        ratio = self.canvas.device_pixel_ratio
        self.window.resize(
            int(width / ratio), int(height / ratio) + self._toolbar_height())

    def full_screen_toggle(self):
        # docstring inherited
        if _value(self.window, "isFullScreen"):
            self.window.showNormal()
        else:
            self.window.showFullScreen()

    def get_window_title(self):
        # docstring inherited
        return _value(self.window, "windowTitle")

    def set_window_title(self, title):
        # docstring inherited
        self.window.setWindowTitle(title)

    @classmethod
    def start_main_loop(cls):
        """Do nothing: Slicer's application event loop is already running."""

    def _on_window_closed(self):
        # Routed through Gcf so that the figure is dropped from pyplot's registry;
        # Gcf calls back into destroy(), which emits the close event.
        Gcf.destroy(self)


FigureCanvasSlicer.manager_class = FigureManagerSlicer


# ---------------------------------------------------------------------------
# Backend export
# ---------------------------------------------------------------------------

@_Backend.export
class _BackendSlicer(_Backend):
    backend_version = backend_version
    FigureCanvas = FigureCanvasSlicer
    FigureManager = FigureManagerSlicer

    @staticmethod
    def mainloop():
        """Do nothing: Slicer's application event loop is already running."""


def enable(interactive=True):
    """Select this backend and, by default, turn on Matplotlib's interactive mode.

    Parameters
    ----------
    interactive : bool, default: True
        Whether to call :func:`matplotlib.pyplot.ion`, so that figures are shown and
        updated as soon as they are modified. This is usually what is wanted inside
        Slicer, where the application event loop is always running.
    """
    matplotlib.use(BACKEND_NAME, force=True)
    if interactive:
        import matplotlib.pyplot as plt
        plt.ion()
