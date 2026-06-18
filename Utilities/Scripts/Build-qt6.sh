#!/bin/bash
#
# Builds Qt from source (macOS) for use with 3D Slicer.
#
# Usage:
#   ./Build-qt6.sh [QT_VERSION] [ARCH] [MACOS_DEPLOYMENT_TARGET]
#
#   QT_VERSION defaults to 6.11.1 and must be 6.8 or newer.
#   ARCH selects the macOS architecture to build for: arm64 (default) or
#   x86_64.
#   MACOS_DEPLOYMENT_TARGET is the minimum macOS version to target (default:
#   14.0). Used for both the OpenSSL build and the Qt CMake configuration.
#
# Examples:
#   ./Build-qt6.sh                       # build the default Qt version for arm64
#   ./Build-qt6.sh 6.11.1                # build a specific Qt version for arm64
#   ./Build-qt6.sh 6.11.1 x86_64         # build a specific Qt version for x86_64
#   ./Build-qt6.sh 6.11.1 arm64 15.0     # target a different minimum macOS version
#   ./Build-qt6.sh --help                # show usage
#
# Run this script from the directory where Qt should be downloaded, built,
# and installed (source, build, and install trees are created alongside it,
# one per architecture).

# Increase file descriptor limit for QtWebEngine build (prevents "Too many open files" error)
ulimit -n 4096
echo "File descriptor limit set to: $(ulimit -n)"
echo ""

usage() {
  echo "Usage: $(basename "$0") [QT_VERSION] [ARCH] [MACOS_DEPLOYMENT_TARGET]"
  echo ""
  echo "  QT_VERSION                Qt version to build from source (default: 6.11.1, must be 6.8 or newer)"
  echo "  ARCH                      macOS architecture to build for: arm64 (default) or x86_64"
  echo "  MACOS_DEPLOYMENT_TARGET   Minimum macOS version to target (default: 14.0)"
  echo ""
  echo "Example:"
  echo "  $(basename "$0") 6.11.1 arm64 14.0"
}

if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
  usage
  exit 0
fi

# Configuration
QT_VERSION="${1:-6.11.1}"
ARCH="${2:-arm64}"

MIN_QT_VERSION="6.8"
if [ "$(printf '%s\n' "${MIN_QT_VERSION}" "${QT_VERSION}" | sort -V | head -n 1)" != "${MIN_QT_VERSION}" ]; then
  echo "ERROR: QT_VERSION must be ${MIN_QT_VERSION} or newer (got ${QT_VERSION})."
  exit 1
fi

if [ "${ARCH}" != "arm64" ] && [ "${ARCH}" != "x86_64" ]; then
  echo "ERROR: ARCH must be 'arm64' or 'x86_64' (got ${ARCH})."
  exit 1
fi

# Minimum macOS version to target. Used for both the OpenSSL build and the
# Qt CMake configuration so the resulting libraries/binaries agree and the
# linker doesn't warn about mismatched deployment targets.
MACOS_DEPLOYMENT_TARGET="${3:-14.0}"

echo "Building Qt ${QT_VERSION} for ${ARCH} (macOS ${MACOS_DEPLOYMENT_TARGET}+)"
echo ""

# Modules to build. This is checked further below (once the source tree is
# available) to only pass -DBUILD_<module> flags for modules that are
# actually present, and is also consulted here to decide which environment
# checks are needed before any installs are attempted.
MODULE_BUILD_MATRIX="
qt3d OFF
qt5compat ON
qtactiveqt OFF
qtbase ON
qtcanvaspainter OFF
qtcharts OFF
qtcoap OFF
qtconnectivity OFF
qtdatavis3d OFF
qtdeclarative ON
qtdoc OFF
qtgraphs OFF
qtgrpc OFF
qthttpserver OFF
qtimageformats OFF
qtlanguageserver OFF
qtlocation OFF
qtlottie OFF
qtmqtt OFF
qtmultimedia ON
qtnetworkauth OFF
qtopcua OFF
qtopenapi OFF
qtpositioning ON
qtquick3d OFF
qtquick3dphysics OFF
qtquickeffectmaker OFF
qtquicktimeline OFF
qtremoteobjects OFF
qtscxml ON
qtsensors OFF
qtserialbus OFF
qtserialport OFF
qtshadertools ON
qtspeech OFF
qtsvg ON
qttasktree OFF
qttools ON
qttranslations OFF
qtvirtualkeyboard OFF
qtwayland OFF
qtwebchannel ON
qtwebengine ON
qtwebsockets ON
qtwebview OFF
"

QTWEBENGINE_ENABLED=$(awk '$1 == "qtwebengine" { print $2 }' <<< "${MODULE_BUILD_MATRIX}")

check_metal_toolchain() {
  echo "Checking Xcode Metal toolchain for QtWebEngine..."

  if ! command -v xcode-select >/dev/null 2>&1 || ! command -v xcrun >/dev/null 2>&1; then
    echo "ERROR: xcode-select/xcrun is not available. Install Xcode and Command Line Tools."
    exit 1
  fi

  XCODE_DEVELOPER_DIR=$(xcode-select -p 2>/dev/null)
  if [ -z "${XCODE_DEVELOPER_DIR}" ] || [ ! -d "${XCODE_DEVELOPER_DIR}" ]; then
    echo "ERROR: Active Xcode developer directory is not configured."
    echo "Run: sudo xcode-select -s /Applications/Xcode.app/Contents/Developer"
    exit 1
  fi

  METAL_BIN=$(xcrun --sdk macosx --find metal 2>/dev/null || true)
  METALLIB_BIN=$(xcrun --sdk macosx --find metallib 2>/dev/null || true)

  if [ -z "${METAL_BIN}" ] || [ -z "${METALLIB_BIN}" ]; then
    echo "ERROR: Xcode Metal toolchain is missing (metal/metallib not found)."
    echo "Install or repair Xcode components with:"
    echo "  sudo xcode-select -s /Applications/Xcode.app/Contents/Developer"
    echo "  sudo xcodebuild -runFirstLaunch"
    echo "  sudo xcodebuild -downloadComponent MetalToolchain"
    exit 1
  fi

  METAL_VERSION_OUTPUT=$(xcrun --sdk macosx metal --version 2>&1 || true)
  if ! echo "${METAL_VERSION_OUTPUT}" | grep -Eq "Apple metal version [0-9]+\.[0-9]+"; then
    echo "ERROR: Xcode Metal toolchain is not usable for QtWebEngine."
    echo "xcrun metal --version output:"
    echo "  ${METAL_VERSION_OUTPUT}"
    echo "Repair with:"
    echo "  sudo xcode-select -s /Applications/Xcode.app/Contents/Developer"
    echo "  sudo xcodebuild -runFirstLaunch"
    echo "  sudo xcodebuild -downloadComponent MetalToolchain"
    exit 1
  fi

  echo "✓ Metal toolchain detected"
  echo "  metal: ${METAL_BIN}"
  echo "  metallib: ${METALLIB_BIN}"
  echo ""
}

# Run all environment/dependency checks up front, before installing or
# downloading anything, so the build fails fast instead of breaking
# confusingly partway through a long brew/pip/Qt build.
if [ "${QTWEBENGINE_ENABLED}" = "ON" ]; then
  check_metal_toolchain
fi

# Check and install required dependencies
echo "Checking required dependencies..."

REQUIRED_DEPS="ninja cmake python@3 nodejs gperf bison flex nss llvm ffmpeg opus libvpx icu4c re2 libtiff libpng harfbuzz glib pkg-config"

for dep in $REQUIRED_DEPS; do
  if ! brew list "$dep" &>/dev/null; then
    echo "Installing $dep..."
    brew install "$dep"
  else
    echo "✓ $dep already installed"
  fi
done

echo ""
echo "Installing Python packages for QtWebEngine..."
# Find the Python that Qt will use
PYTHON_CMD="/opt/homebrew/Frameworks/Python.framework/Versions/3.12/bin/python3.12"
if [ ! -f "$PYTHON_CMD" ]; then
  PYTHON_CMD="/opt/homebrew/bin/python3"
fi
if [ ! -f "$PYTHON_CMD" ]; then
  PYTHON_CMD="/usr/bin/python3"
fi

echo "Using Python: $PYTHON_CMD"

# Install into a dedicated virtual environment rather than the system/Homebrew
# Python (avoids --break-system-packages and keeps these packages isolated).
QT_BUILD_VENV_DIR="$(pwd)/qt-build-venv"
if [ ! -d "$QT_BUILD_VENV_DIR" ]; then
  echo "Creating Python virtual environment at ${QT_BUILD_VENV_DIR}..."
  "$PYTHON_CMD" -m venv "$QT_BUILD_VENV_DIR"
else
  echo "Using existing Python virtual environment at ${QT_BUILD_VENV_DIR}"
fi

# shellcheck disable=SC1091
source "${QT_BUILD_VENV_DIR}/bin/activate"
pip install --upgrade pip >/dev/null
pip install html5lib spdx-tools

# Verify installation
if python3 -c "import html5lib" 2>/dev/null; then
  echo "✓ html5lib and spdx-tools installed successfully"
else
  echo "WARNING: html5lib installation may have failed"
fi

# Keep the venv's bin directory first on PATH so the Qt build (which invokes
# "python3" internally) picks up html5lib/spdx-tools.
export PATH="${QT_BUILD_VENV_DIR}/bin:$PATH"

echo ""
echo "All dependencies are installed."
echo ""

# Set up environment for finding libraries
export PKG_CONFIG_PATH="/opt/homebrew/opt/ffmpeg/lib/pkgconfig:/opt/homebrew/opt/opus/lib/pkgconfig:/opt/homebrew/opt/libvpx/lib/pkgconfig:/opt/homebrew/opt/icu4c/lib/pkgconfig:/opt/homebrew/opt/re2/lib/pkgconfig:/opt/homebrew/opt/libtiff/lib/pkgconfig:/opt/homebrew/opt/libpng/lib/pkgconfig:/opt/homebrew/opt/harfbuzz/lib/pkgconfig:/opt/homebrew/opt/glib/lib/pkgconfig:$PKG_CONFIG_PATH"
echo "PKG_CONFIG_PATH set for Homebrew libraries"
echo ""

QT_SOURCE_DIR="qt-everywhere-src-${QT_VERSION}"
QT_BUILD_DIR="qt-everywhere-build-${QT_VERSION}-${ARCH}"
QT_INSTALL_DIR="$(pwd)/qt-everywhere-install-${QT_VERSION}-${ARCH}"
USE_GIT=false  # Set to true to clone from git, false to use tarball

# Get Qt source
if [ ! -d "$QT_SOURCE_DIR" ]; then
  if [ "$USE_GIT" = true ]; then
    echo "Cloning Qt ${QT_VERSION} from GitHub..."
    git clone --branch v${QT_VERSION} --depth 1 https://github.com/qt/qt5.git "$QT_SOURCE_DIR" # Despite the repo name "qt5", it contains Qt 6 as well.
    cd "$QT_SOURCE_DIR"

    # Initialize required submodules
    echo "Initializing Qt submodules..."
    perl init-repository --module-subset=qtbase,qt5compat,qtdeclarative,qtshadertools,qtwebchannel,qtpositioning,qtmultimedia,qtwebengine

    cd ..
    echo "Qt source cloned successfully"
    SBOM_ENABLE="ON"
  else
    echo "Downloading Qt ${QT_VERSION} source tarball..."
    QT_TARBALL="qt-everywhere-src-${QT_VERSION}.tar.xz"
    if [ ! -f "$QT_TARBALL" ]; then
      curl -L -O "https://download.qt.io/official_releases/qt/${QT_VERSION%.*}/${QT_VERSION}/single/${QT_TARBALL}"
    fi

    echo "Extracting Qt source..."
    tar xf "$QT_TARBALL"
    echo "Qt source extracted successfully"
    SBOM_ENABLE="OFF"
  fi
else
  echo "Using existing Qt source at ${QT_SOURCE_DIR}"
  # Determine if it's a git repo
  if [ -d "$QT_SOURCE_DIR/.git" ]; then
    SBOM_ENABLE="ON"
  else
    SBOM_ENABLE="OFF"
  fi
fi

echo ""

# Build OpenSSL from source
OPENSSL_VERSION="3.6.3"
OPENSSL_INSTALL_DIR="$(pwd)/openssl-install-${ARCH}"

if [ ! -d "$OPENSSL_INSTALL_DIR" ]; then
  echo "Building OpenSSL ${OPENSSL_VERSION}..."

  # Download OpenSSL
  curl -L -O "https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz"
  tar xzf "openssl-${OPENSSL_VERSION}.tar.gz"
  cd "openssl-${OPENSSL_VERSION}"

  # Configure and build OpenSSL. Set MACOSX_DEPLOYMENT_TARGET so OpenSSL's
  # Configure passes a matching -mmacosx-version-min to the compiler/linker;
  # otherwise it defaults to the SDK's version and the resulting dylibs
  # trigger "linking with dylib ... built for newer version" warnings when
  # linked into the Qt build below.
  export MACOSX_DEPLOYMENT_TARGET="${MACOS_DEPLOYMENT_TARGET}"
  ./Configure \
    darwin64-${ARCH}-cc \
    --prefix="${OPENSSL_INSTALL_DIR}" \
    --openssldir="${OPENSSL_INSTALL_DIR}/ssl" \
    no-ssl3 \
    no-weak-ssl-ciphers \
    shared

  make -j$(sysctl -n hw.ncpu)
  make install_sw install_ssldirs

  cd ..
  rm -rf "openssl-${OPENSSL_VERSION}" "openssl-${OPENSSL_VERSION}.tar.gz"
  echo "OpenSSL built successfully at ${OPENSSL_INSTALL_DIR}"
else
  echo "Using existing OpenSSL at ${OPENSSL_INSTALL_DIR}"
fi

OPENSSL_SSL_LIB="${OPENSSL_INSTALL_DIR}/lib/libssl.dylib"
OPENSSL_CRYPTO_LIB="${OPENSSL_INSTALL_DIR}/lib/libcrypto.dylib"

if [ ! -f "${OPENSSL_SSL_LIB}" ]; then
  OPENSSL_SSL_LIB=$(ls "${OPENSSL_INSTALL_DIR}"/lib/libssl*.dylib 2>/dev/null | head -n 1)
fi
if [ ! -f "${OPENSSL_CRYPTO_LIB}" ]; then
  OPENSSL_CRYPTO_LIB=$(ls "${OPENSSL_INSTALL_DIR}"/lib/libcrypto*.dylib 2>/dev/null | head -n 1)
fi

if [ ! -f "${OPENSSL_SSL_LIB}" ] || [ ! -f "${OPENSSL_CRYPTO_LIB}" ]; then
  echo "ERROR: Could not locate OpenSSL shared libraries in ${OPENSSL_INSTALL_DIR}/lib"
  exit 1
fi

# Detect LLVM path (for QDoc)
if [ -d "/opt/homebrew/opt/llvm" ]; then
  LLVM_DIR="/opt/homebrew/opt/llvm"
elif [ -d "/usr/local/opt/llvm" ]; then
  LLVM_DIR="/usr/local/opt/llvm"
fi

# Build Qt with custom OpenSSL
# Set macOS SDK path
MACOS_SDK_PATH=$(xcrun --sdk macosx --show-sdk-path)
echo "Using macOS SDK: ${MACOS_SDK_PATH}"

# MODULE_BUILD_MATRIX and the Metal toolchain check were already handled up
# front (see top of script); here we just filter the module list down to
# what's actually present in the checked-out source tree.
BUILD_MODULE_ARGS=()
while read -r module state; do
  [ -z "$module" ] && continue
  if [ -d "${QT_SOURCE_DIR}/${module}" ]; then
    BUILD_MODULE_ARGS+=("-DBUILD_${module}=${state}")
  else
    echo "Skipping ${module}: not present in source tree"
  fi
done <<< "${MODULE_BUILD_MATRIX}"

cmake \
  -G Ninja \
  -DCMAKE_OSX_ARCHITECTURES:STRING=${ARCH} \
  -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${MACOS_DEPLOYMENT_TARGET} \
  -DCMAKE_OSX_SYSROOT:PATH="${MACOS_SDK_PATH}" \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DCMAKE_INSTALL_PREFIX:PATH="${QT_INSTALL_DIR}" \
  -DQT_NO_HANDLE_APPLE_SINGLE_ARCH_CROSS_COMPILING=ON \
  -DFEATURE_framework=ON \
  -DFEATURE_ffmpeg=OFF \
  -DQT_BUILD_EXAMPLES:BOOL=OFF \
  -DQT_BUILD_TESTS:BOOL=OFF \
  -DQT_BUILD_BENCHMARKS:BOOL=OFF \
  -DLLVM_INSTALL_DIR="${LLVM_DIR}" \
  -DQT_GENERATE_SBOM=${SBOM_ENABLE} \
  -DINPUT_openssl=linked \
  -DOPENSSL_ROOT_DIR="${OPENSSL_INSTALL_DIR}" \
  -DOPENSSL_INCLUDE_DIR="${OPENSSL_INSTALL_DIR}/include" \
  -DOPENSSL_SSL_LIBRARY="${OPENSSL_SSL_LIB}" \
  -DOPENSSL_CRYPTO_LIBRARY="${OPENSSL_CRYPTO_LIB}" \
  "${BUILD_MODULE_ARGS[@]}" \
  -B ./${QT_BUILD_DIR} \
  -S ./${QT_SOURCE_DIR}

if [ $? -ne 0 ]; then
  echo "ERROR: Qt configuration failed!"
  exit 1
fi

echo ""
echo "Building Qt..."
cmake --build ./${QT_BUILD_DIR} --parallel

if [ $? -ne 0 ]; then
  echo "ERROR: Qt build failed!"
  exit 1
fi

echo ""
echo "Installing Qt..."
# If SBOM is disabled, remove SBOM install scripts to avoid installation errors
if [ "${SBOM_ENABLE}" = "OFF" ]; then
  echo "Removing SBOM install scripts..."
  # Remove all SBOM-related CMake scripts
  find ./${QT_BUILD_DIR}/qtwebengine -name "*sbom*.cmake" -delete 2>/dev/null || true
  # Remove SBOM-related include lines in cmake_install.cmake files
  find ./${QT_BUILD_DIR}/qtwebengine -name "cmake_install.cmake" -exec sed -i.bak -e '/sbom/d' -e '/SBOM/d' {} \; 2>/dev/null || true
  echo "SBOM install scripts removed."
fi

cmake --install ./${QT_BUILD_DIR}

if [ $? -ne 0 ]; then
  echo "ERROR: Qt installation failed!"
  exit 1
fi

echo ""
echo "Qt ${QT_VERSION} build and installation complete!"
echo "Install location: ${QT_INSTALL_DIR}"
