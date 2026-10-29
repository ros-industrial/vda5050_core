#!/bin/bash
# Install native build deps for vda5050 (fmt, nlohmann_json, Paho MQTT C++).
set -euo pipefail

PAHO_CPP_TAG="${PAHO_CPP_TAG:-c0b43a49d7b7e7b5e7009658ca22f19ac1112c83}"  # v1.6.0
FMT_TAG="${FMT_TAG:-40626af88bd7df9a5fb80be7b25ac85b122d6c21}"  # 11.2.0
NLOHMANN_JSON_TAG="${NLOHMANN_JSON_TAG:-9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03}"  # v3.11.3
PYBIND11_TAG="${PYBIND11_TAG:-a2e59f0e7065404b44dfe92a28aca47ba1378dc4}"  # v2.13.6
PYBIND11_JSON_TAG="${PYBIND11_JSON_TAG:-b02a2ad597d224c3faee1f05a56d81d4c4453092}"  # 0.2.13
INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX:-/usr/local}"

install_linux() {
  if command -v yum >/dev/null 2>&1; then
    # manylinux / Alma Linux images
    # clear stale cache
    dnf clean all && dnf makecache
    yum install -y cmake git openssl-devel
    install_fmt_from_source
    install_json_from_source
    install_pybind11_from_source
    install_pybind11_json_from_source
    install_paho_from_source
  elif command -v apk >/dev/null 2>&1; then
    # musllinux / Alpine images
    apk add --no-cache \
      cmake git openssl-dev fmt-dev
    install_fmt_from_source
    install_json_from_source
    install_pybind11_from_source
    install_pybind11_json_from_source
    install_paho_from_source
  else
    echo "Unsupported Linux package manager" >&2
    exit 1
  fi
}

git_shallow_clone() {
  if [ $# -lt 3 ]; then
    echo "Error: git_shallow_clone"
    echo "Bad parameters: $*"
    echo "Usage: git_shallow_clone <repo_url> <repo_dir> <repo_tag>"
    exit 1
  fi
  local repo_url=$1; shift
  local repo_dir=$1; shift
  local repo_tag=$1; shift

  mkdir "$repo_dir"
  git -C "$repo_dir" init -q
  git -C "$repo_dir" remote add origin "$repo_url"
  git -C "$repo_dir" fetch --depth 1 origin "$repo_tag"
  git -C "$repo_dir" checkout -q "$repo_tag"
}

git_shallow_clone_recursive() {
  git_shallow_clone "$@"
  local repo_dir=${2:-.}
  git -C "$repo_dir" submodule update --init --recursive --depth 1
}

install_fmt_from_source() {
  local work
  work="$(mktemp -d)"

  git_shallow_clone \
    https://github.com/fmtlib/fmt.git "$work/fmt" "$FMT_TAG"
  cmake -S "$work/fmt" -B "$work/fmt/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DFMT_TEST=OFF
  cmake --build "$work/fmt/build" --parallel
  cmake --install "$work/fmt/build"

  rm -rf "$work"
}

install_json_from_source() {
  local work
  work="$(mktemp -d)"

  git_shallow_clone \
    https://github.com/nlohmann/json.git "$work/json" "$NLOHMANN_JSON_TAG"
  cmake -S "$work/json" -B "$work/json/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DJSON_BuildTests=OFF
  cmake --install "$work/json/build"

  rm -rf "$work"
}

# Header-only pybind11 CMake package for hosts without a distro package
# (manylinux). PYBIND11_NOPYTHON avoids baking the container interpreter;
# the wheel build still gets pybind11 from the build-system requires (v3 instead).
install_pybind11_from_source() {
  local work
  work="$(mktemp -d)"

  git_shallow_clone \
    https://github.com/pybind/pybind11.git "$work/pybind11" "$PYBIND11_TAG"
  cmake -S "$work/pybind11" -B "$work/pybind11/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DPYBIND11_TEST=OFF \
    -DPYBIND11_NOPYTHON=ON
  cmake --build "$work/pybind11/build" --parallel
  cmake --install "$work/pybind11/build"

  rm -rf "$work"
}

# pybind11_json: header-only bridge between nlohmann::json and pybind11.
install_pybind11_json_from_source() {
  local work
  work="$(mktemp -d)"

  git_shallow_clone \
    https://github.com/pybind/pybind11_json.git "$work/pb11j" "$PYBIND11_JSON_TAG"
  # pybind11_json 0.2.13 still declares cmake_minimum_required(< 3.5);
  # CMake 4+ rejects that unless CMAKE_POLICY_VERSION_MINIMUM is set.
  # Empty PYTHON_INCLUDE_DIRS so the INTERFACE target does not bake the
  # host interpreter's include path (wheel builds use a different Python).
  cmake -S "$work/pb11j" -B "$work/pb11j/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_PREFIX_PATH="${INSTALL_PREFIX}${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DPYTHON_INCLUDE_DIRS=
  cmake --build "$work/pb11j/build" --parallel
  cmake --install "$work/pb11j/build"

  rm -rf "$work"
}

# Paho is built from source with PAHO_WITH_MQTT_C=ON into PREFIX.
# A second copy of the same dylib basename (Homebrew formula, leftover
# /usr/local install, etc.) makes delocate fail with:
#   Already planning to copy library with same basename as: libpaho-mqtt3as...
ensure_single_macos_paho_prefix() {
  local prefix="$1"
  local other

  if brew list --formula libpaho-mqtt >/dev/null 2>&1; then
    echo "Uninstalling Homebrew libpaho-mqtt (conflicts with source Paho install)."
    brew uninstall --ignore-dependencies libpaho-mqtt || true
  fi

  for other in /usr/local /opt/homebrew; do
    [[ "${other}" == "${prefix}" ]] && continue
    if compgen -G "${other}/lib/libpaho-mqtt*" >/dev/null 2>&1; then
      echo "error: found conflicting Paho libs under ${other}/lib while PREFIX=${prefix}" >&2
      echo "delocate cannot vendor two dylibs with the same basename." >&2
      echo "Remove the extras, e.g.:" >&2
      echo "  sudo rm -f ${other}/lib/libpaho-mqtt*" >&2
      echo "  sudo rm -rf ${other}/include/mqtt ${other}/lib/cmake/PahoMqttCpp" >&2
      exit 1
    fi
  done
}

install_macos() {
  brew install cmake openssl@3

  INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX:-$(brew --prefix)}"
  ensure_single_macos_paho_prefix "$INSTALL_PREFIX"
  export CMAKE_PREFIX_PATH="$INSTALL_PREFIX"
  REPAIR_LIBRARY_PATH="$INSTALL_PREFIX/lib:$(brew --prefix openssl@3)/lib"; export REPAIR_LIBRARY_PATH

  install_fmt_from_source
  install_json_from_source
  install_pybind11_from_source
  install_pybind11_json_from_source
  install_paho_from_source
}

install_paho_from_source() {
  local work
  work="$(mktemp -d)"

  git_shallow_clone_recursive \
    https://github.com/eclipse/paho.mqtt.cpp.git "$work/paho.mqtt.cpp" "$PAHO_CPP_TAG"

  cmake -S "$work/paho.mqtt.cpp" -B "$work/paho.mqtt.cpp/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_INSTALL_NAME_DIR="$INSTALL_PREFIX/lib" \
    -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
    -DPAHO_WITH_MQTT_C=ON \
    -DPAHO_BUILD_EXAMPLES=OFF \
    -DPAHO_WITH_SSL=ON

  cmake --build "$work/paho.mqtt.cpp/build" --parallel
  cmake --install "$work/paho.mqtt.cpp/build"

  rm -rf "$work"
}

case "$(uname -s)" in
  Linux)
    install_linux
    ;;
  Darwin)
    install_macos
    ;;
  *)
    echo "Unsupported OS: $(uname -s)" >&2
    exit 1
    ;;
esac

echo "All Native dependencies installed!"
