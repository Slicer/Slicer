#!/bin/bash
#
# Package, verify, sign, and optionally install a macOS Slicer build.
#
# Run this after the Slicer superbuild has completed. The build directory is
# the inner Slicer build (for example, /opt/s/Slicer-build), not the superbuild
# directory itself.

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source_dir="$(cd "${script_dir}/../.." && pwd)"
build_dir=""
output_dir=""
install_dir=""
install_name=""
expected_arch="arm64"
sign_identity="-"
jobs="$(sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
run_package=1
run_smoke_test=1
final_dmg=""

usage()
{
  cat <<EOF
Usage: $(basename "$0") --build-dir <path> [options]

Required:
  --build-dir <path>       Inner Slicer build directory

Options:
  --source-dir <path>      Slicer source directory (default: ${source_dir})
  --output-dir <path>      DMG output directory (default: build directory)
  --architecture <arch>    Required application architecture (default: arm64)
  --sign-identity <name>   codesign identity (default: - for ad-hoc signing)
  --install <directory>    Replace and install the app in this directory
  --install-name <name>    App bundle name to install (default: packaged name)
  --jobs <count>           Parallel package build jobs (default: ${jobs})
  --skip-package           Reuse the newest app already staged by CPack
  --skip-smoke-test        Do not launch the staged app headlessly
  -h, --help               Show this help

Examples:
  $(basename "$0") --build-dir /opt/s/Slicer-build
  $(basename "$0") --build-dir /opt/s/Slicer-build --install /Applications
  $(basename "$0") --build-dir /opt/s/Slicer-build --skip-package \\
    --install /Applications --install-name Slicer-arm64.app
EOF
}

die()
{
  echo "ERROR: $*" >&2
  exit 1
}

require_command()
{
  command -v "$1" >/dev/null 2>&1 || die "Required command not found: $1"
}

newest_path()
{
  local newest=""
  local newest_time=0
  local candidate candidate_time

  for candidate in "$@"; do
    [[ -e "${candidate}" ]] || continue
    candidate_time="$(stat -f '%m' "${candidate}")"
    if [[ -z "${newest}" || "${candidate_time}" -gt "${newest_time}" ]]; then
      newest="${candidate}"
      newest_time="${candidate_time}"
    fi
  done

  [[ -n "${newest}" ]] || return 1
  printf '%s\n' "${newest}"
}

find_staged_app()
{
  local staging_root="${build_dir}/_CPack_Packages"
  local candidate parent package_parent
  local apps=()

  [[ -d "${staging_root}" ]] || die "CPack staging directory not found: ${staging_root}"
  while IFS= read -r -d '' candidate; do
    parent="$(dirname "${candidate}")"
    package_parent="$(basename "$(dirname "${parent}")")"
    if [[ "${package_parent}" == "DragNDrop" ]]; then
      apps+=("${candidate}")
    fi
  done < <(find "${staging_root}" -type d -name '*.app' -print0)

  [[ "${#apps[@]}" -gt 0 ]] || die "No staged application bundle found under ${staging_root}"
  newest_path "${apps[@]}"
}

find_package_dmg()
{
  local app="$1"
  local package_name matching_dmg
  local dmgs=()
  local candidate

  package_name="$(basename "$(dirname "${app}")")"
  matching_dmg="${build_dir}/${package_name}.dmg"
  if [[ -f "${matching_dmg}" ]]; then
    printf '%s\n' "${matching_dmg}"
    return
  fi

  while IFS= read -r -d '' candidate; do
    dmgs+=("${candidate}")
  done < <(find "${build_dir}" -maxdepth 1 -type f -name '*.dmg' -print0)

  [[ "${#dmgs[@]}" -gt 0 ]] || return 1
  newest_path "${dmgs[@]}"
}

verify_bundle_architecture()
{
  local app="$1"
  local file_path architectures
  local checked=0
  local invalid=0

  while IFS= read -r -d '' file_path; do
    architectures="$(lipo -archs "${file_path}" 2>/dev/null)" || continue
    checked=$((checked + 1))
    if [[ " ${architectures} " != *" ${expected_arch} "* ]]; then
      echo "Unexpected architecture '${architectures}': ${file_path}" >&2
      invalid=1
    fi
  done < <(find "${app}" -type f \
    \( -perm -111 -o -name '*.dylib' -o -name '*.so' -o -name '*.bundle' \
       -o -path '*/Frameworks/*.framework/Versions/*/*' \) -print0)

  [[ "${invalid}" -eq 0 ]] ||
    die "One or more Mach-O files do not contain '${expected_arch}'"
  echo "Verified ${checked} Mach-O files contain architecture: ${expected_arch}"
}

sign_bundle()
{
  local app="$1"
  local sign_args=(--force --sign "${sign_identity}")
  local nested_apps=()
  local file_path nested_app index

  if [[ "${sign_identity}" == "-" ]]; then
    sign_args+=(--timestamp=none)
  else
    sign_args+=(--options runtime --timestamp)
  fi

  echo "Signing Mach-O files with identity: ${sign_identity}"
  while IFS= read -r -d '' file_path; do
    lipo -archs "${file_path}" >/dev/null 2>&1 || continue
    /usr/bin/codesign "${sign_args[@]}" "${file_path}" >/dev/null
  done < <(find "${app}" -type f \
    \( -perm -111 -o -name '*.dylib' -o -name '*.so' -o -name '*.bundle' \
       -o -path '*/Frameworks/*.framework/Versions/*/*' \) -print0)

  while IFS= read -r -d '' nested_app; do
    nested_apps+=("${nested_app}")
  done < <(find "${app}/Contents" -mindepth 1 -type d -name '*.app' -print0)

  for ((index=${#nested_apps[@]} - 1; index >= 0; index--)); do
    /usr/bin/codesign "${sign_args[@]}" --deep "${nested_apps[index]}" >/dev/null
  done
  /usr/bin/codesign "${sign_args[@]}" --deep "${app}" >/dev/null
}

create_dmg()
{
  local app="$1"
  local cpack_dmg="$2"
  local destination="${output_dir}/$(basename "${cpack_dmg}")"
  local temporary_dmg="${destination%.dmg}.tmp.dmg"
  local package_dir applications_link link_created=0

  package_dir="$(dirname "${app}")"
  applications_link="${package_dir}/Applications"
  if [[ ! -e "${applications_link}" && ! -L "${applications_link}" ]]; then
    ln -s /Applications "${applications_link}"
    link_created=1
  fi

  rm -f "${temporary_dmg}"
  echo "Creating disk image: ${destination}"
  if ! hdiutil create \
      -volname "$(basename "${app}" .app)" \
      -srcfolder "${package_dir}" \
      -format UDZO \
      "${temporary_dmg}"; then
    [[ "${link_created}" -eq 0 ]] || rm -f "${applications_link}"
    rm -f "${temporary_dmg}"
    die "Failed to create disk image"
  fi

  if [[ "${link_created}" -eq 1 ]]; then
    rm -f "${applications_link}"
  fi
  mv -f "${temporary_dmg}" "${destination}"
  hdiutil verify "${destination}"
  printf 'SHA-256: '
  shasum -a 256 "${destination}" | awk '{print $1}'
  final_dmg="${destination}"
}

install_bundle()
{
  local app="$1"
  local destination="${install_dir}/${install_name:-$(basename "${app}")}"

  [[ -d "${install_dir}" ]] || die "Install directory does not exist: ${install_dir}"
  [[ "${destination}" == "${install_dir}/"*.app ]] || die "Unsafe install destination: ${destination}"
  require_command pgrep

  if [[ -d "${destination}" ]] &&
      pgrep -f "${destination}/Contents/MacOS/" >/dev/null 2>&1; then
    die "Cannot replace a running application: ${destination}"
  fi

  echo "Installing application: ${destination}"
  if [[ ! -w "${install_dir}" ]]; then
    require_command sudo
    sudo rm -rf "${destination}"
    sudo /usr/bin/ditto "${app}" "${destination}"
  else
    rm -rf "${destination}"
    /usr/bin/ditto "${app}" "${destination}"
  fi
  /usr/bin/codesign --verify --deep --strict --verbose=1 "${destination}"
  file "${destination}/Contents/MacOS/${bundle_executable}"
}

while [[ "$#" -gt 0 ]]; do
  case "$1" in
    --build-dir)
      [[ "$#" -ge 2 ]] || die "--build-dir requires a path"
      build_dir="$2"
      shift 2
      ;;
    --source-dir)
      [[ "$#" -ge 2 ]] || die "--source-dir requires a path"
      source_dir="$2"
      shift 2
      ;;
    --output-dir)
      [[ "$#" -ge 2 ]] || die "--output-dir requires a path"
      output_dir="$2"
      shift 2
      ;;
    --architecture)
      [[ "$#" -ge 2 ]] || die "--architecture requires a value"
      expected_arch="$2"
      shift 2
      ;;
    --sign-identity)
      [[ "$#" -ge 2 ]] || die "--sign-identity requires a value"
      sign_identity="$2"
      shift 2
      ;;
    --install)
      [[ "$#" -ge 2 ]] || die "--install requires a directory"
      install_dir="$2"
      shift 2
      ;;
    --install-name)
      [[ "$#" -ge 2 ]] || die "--install-name requires a value"
      install_name="$2"
      shift 2
      ;;
    --jobs)
      [[ "$#" -ge 2 ]] || die "--jobs requires a value"
      jobs="$2"
      shift 2
      ;;
    --skip-package)
      run_package=0
      shift
      ;;
    --skip-smoke-test)
      run_smoke_test=0
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      die "Unknown argument: $1"
      ;;
  esac
done

[[ "$(uname -s)" == "Darwin" ]] || die "This script only supports macOS"
[[ -n "${build_dir}" ]] || die "--build-dir is required"
[[ -d "${build_dir}" ]] || die "Build directory not found: ${build_dir}"
[[ -f "${build_dir}/CMakeCache.txt" ]] || die "CMakeCache.txt not found in: ${build_dir}"
[[ -f "${source_dir}/CMake/SlicerCPackBundleVerify.cmake" ]] ||
  die "Slicer source directory is invalid: ${source_dir}"
[[ "${jobs}" =~ ^[1-9][0-9]*$ ]] || die "--jobs must be a positive integer"
if [[ -n "${install_name}" ]]; then
  [[ -n "${install_dir}" ]] || die "--install-name requires --install"
  [[ "${install_name}" != */* && "${install_name}" == *.app ]] ||
    die "--install-name must be a single .app bundle name"
fi

output_dir="${output_dir:-${build_dir}}"
mkdir -p "${output_dir}"

for command_name in cmake file find hdiutil lipo shasum stat; do
  require_command "${command_name}"
done

if [[ "${run_package}" -eq 1 ]]; then
  echo "Building the CPack DragNDrop package"
  cmake --build "${build_dir}" --target package --parallel "${jobs}"
fi

staged_app="$(find_staged_app)"
echo "Staged application: ${staged_app}"

bundle_executable="$(/usr/libexec/PlistBuddy \
  -c 'Print :CFBundleExecutable' "${staged_app}/Contents/Info.plist")"
main_executable="${staged_app}/Contents/MacOS/${bundle_executable}"
[[ -f "${main_executable}" ]] || die "Bundle executable not found: ${main_executable}"

architectures="$(lipo -archs "${main_executable}")"
if [[ " ${architectures} " != *" ${expected_arch} "* ]]; then
  die "Expected architecture '${expected_arch}', found '${architectures}'"
fi
echo "Verified application architecture: ${architectures}"
verify_bundle_architecture "${staged_app}"

cmake \
  "-DSlicer_INSTALL_DIR=$(dirname "${staged_app}")" \
  -P "${source_dir}/CMake/SlicerCPackBundleVerify.cmake"

sign_bundle "${staged_app}"
/usr/bin/codesign --verify --deep --strict --verbose=1 "${staged_app}"

if [[ "${run_smoke_test}" -eq 1 ]]; then
  smoke_dir="$(mktemp -d "${TMPDIR:-/tmp}/slicer-package-smoke.XXXXXX")"
  smoke_contents_before=()
  while IFS= read -r -d '' content_path; do
    smoke_contents_before+=("${content_path}")
  done < <(find "${staged_app}/Contents" -mindepth 1 -maxdepth 1 -print0)

  echo "Running a headless package smoke test"
  smoke_succeeded=0
  if (
      cd "${smoke_dir}"
      PYTHONDONTWRITEBYTECODE=1 "${main_executable}" \
        --testing \
        --no-main-window \
        --disable-modules \
        --python-code \
        "import platform, slicer; assert platform.machine() == '${expected_arch}'; print('SLICER_PACKAGE_VERSION=' + slicer.app.applicationVersion); slicer.app.quit()"
    ); then
    smoke_succeeded=1
  fi
  rm -rf "${smoke_dir}"

  while IFS= read -r -d '' content_path; do
    content_existed=0
    for original_path in "${smoke_contents_before[@]}"; do
      if [[ "${content_path}" == "${original_path}" ]]; then
        content_existed=1
        break
      fi
    done
    if [[ "${content_existed}" -eq 0 ]]; then
      echo "Removing smoke-test artifact: ${content_path}"
      rm -rf "${content_path}"
    fi
  done < <(find "${staged_app}/Contents" -mindepth 1 -maxdepth 1 -print0)

  [[ "${smoke_succeeded}" -eq 1 ]] || die "Headless package smoke test failed"
  /usr/bin/codesign --verify --deep --strict --verbose=1 "${staged_app}"
fi

cpack_dmg="$(find_package_dmg "${staged_app}")" ||
  die "CPack did not produce a DMG in ${build_dir}"
create_dmg "${staged_app}" "${cpack_dmg}"

if [[ -n "${install_dir}" ]]; then
  install_bundle "${staged_app}"
fi

echo "Package ready: ${final_dmg}"
