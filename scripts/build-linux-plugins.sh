#!/bin/bash


################################################################################
# build-linux-plugins.sh
#
# Author: Alex Leutgöb <alexleutgoeb@gmail.com>
#
# Run this script to create builds of the most commong plugins of the DLVHEX
# project for Linux platforms. The script clones a clean copy of the
# repositories and builds the binaries. The script requires a valid build of
# DLVHEX core. Tested on Ubuntu and Debian.
#
################################################################################


################################################################################
# Configuration parameters

BASE_CLONE_URL="https://github.com/hexhex/"
# TODO: Allow to override from command line to build a specific plugin only
# actionplugin and addons order is important here for collect step
PLUGINS="nestedhexplugin mcsieplugin stringplugin actionplugin actionplugin-addons"
# Plugins TODO: dlliteplugin, caspplugin, wordnetplugin
# Don't change the following ones, should match directories from core build
ROOT_DIR=`pwd`
OUTPUT_IO=/dev/null
# TODO: Check if already set from core build script
BUILD_DIR=$ROOT_DIR/build-linux
OUT_DIR=$ROOT_DIR/out-linux
LIB_DIR=$BUILD_DIR/out

# Color output
C_WHITE='\033[1;37m'
C_GREEN='\033[0;32m'
C_RED='\033[0;31m'
C_YELLOW='\033[1;33m'
C_NC='\033[0m'
T_ERROR="${C_RED}Error${C_NC}:"
T_WARN="${C_YELLOW}Warning${C_NC}:"
T_INFO="${C_WHITE}Info${C_NC}:"
T_DONE="${C_GREEN}Done${C_NC}:"



################################################################################
# Helper functions

function usage {
  echo -e "usage: $0 [-v -h]"
  echo -e
  echo -e "Parameters:"
  echo -e "  -h     print this help"
  echo -e "  -v     Enable verbose output"
}

function prepare {
  echo -e "==> Create directories"
  mkdir -p $BUILD_DIR
  mkdir -p $OUT_DIR
}

# Check for missing depdencies
function checkDependencies {
  echo -e "==> Check build dependencies"

  deps="unzip sed git build-essential autoconf autotools-dev libtool wget scons bison re2c python-dev libpython-all-dev libcurl4-openssl-dev libbz2-dev"
  missing_deps=""

  for dep in `echo -e $deps`; do
    if ! dpkg -s $dep &> /dev/null; then
      missing_deps="$missing_deps $dep"
    fi
  done

  if [ ! -z "$missing_deps" ]; then
    # TODO: Check for other distros and replace apt-get command
    echo -e "${T_ERROR} Missing build dependencies, use:"
    echo -e "apt-get install${missing_deps}"
    exit 1
  fi
}

function processPlugin {
  # Name of currently built plugin
  plugin_name=$1

  echo -e "\n==> ${T_INFO} Process <$plugin_name> repository"

  cd $BUILD_DIR

  # Check if directory already exists
  if [ -d $plugin_name ]; then
    echo -e "===> ${T_WARN} Source directory already exists"
    # Do not delete already existing repository but re-use it
    # rm -rf $plugin_name
  else
    echo -e "===> Clone repository"

    # Clone latest sources
    plugin_url="${BASE_CLONE_URL}${plugin_name}.git"
    git clone --recursive $plugin_url &> $OUTPUT_IO
    if [ $? -ne 0 ]; then
      echo -e "===> ${T_ERROR} Can't clone repository from url: <$plugin_url>"
      exit 1
    fi
  fi

  cd $plugin_name

  # Check if there exists a bootstrap file, if not try to iterate over subdirs,
  # needed for plugins like actionplugin-addons
  if [ ! -f bootstrap.sh ]; then
    echo -e "===> ${T_INFO} No bootstrap.sh file found in root directory, look for subdirectories"

    for subdir in */ ; do

      if [ -d $subdir ]; then
        cd $subdir
        subdir_name=${subdir%/}
        buildPlugin $subdir_name
        cd ..
      fi
    done

  # bootstrap.sh file found, build
  else
    buildPlugin $plugin_name
  fi

  cd $OUT_DIR
  collect $plugin_name
}

# Make sure to dir to plugin directory before calling this method
function buildPlugin {
  # Name of currently built plugin
  local plugin_name=$1

  echo -e "===> ${T_INFO} Build <$plugin_name> plugin"

  if [ -f bootstrap.sh ]; then
    echo -e "===> Configure"

    # Configure build
    ./bootstrap.sh &> $OUTPUT_IO
    ./configure --prefix $LIB_DIR PKG_CONFIG_PATH=$LIB_DIR/lib/pkgconfig --with-boost=$LIB_DIR LDFLAGS="-Wl,-rpath,'\$\$ORIGIN' -Wl,-z,origin" CPPFLAGS="-DNDEBUG=1" &> $OUTPUT_IO

    echo -e "===> Build"

    # Build plugin
    make &> $OUTPUT_IO
    if [ $? -ne 0 ]; then
      echo -e "===> ${T_ERROR} Error building library"
    else
      make install &> $OUTPUT_IO

      # TODO: Run make check?

      echo -e "===> ${T_DONE} Build finished"
    fi

    # Clean up
    make clean &> $OUTPUT_IO
  else
    echo -e "===> ${T_WARN} No bootstrap.sh file found, skipping build"
  fi
}

# Collects build artifacts used for distribution later on
function collect {
  # Name of currently built plugin, we remove the "plugin" part from the name to find built libs
  local plugin_name=${1//"plugin"/""}

  echo -e "===> Collect build files for <$plugin_name> plugin"

  # This is a temporary solution for collection the action addon libs
  if [[ $plugin_name == *"addons"* ]]; then
    plugin_name=${plugin_name/"-addons"/""}
    plugin_name="_"$plugin_name
  else
    plugin_name="-"$plugin_name
  fi

  echo -e "====> Strip binary files"
  cd $BUILD_DIR/out/lib
  find . -type f -iname "lib*$plugin_name*.so*" -exec strip {} \; &> $OUTPUT_IO

  echo -e "====> Archive"
  tar_name="dlvhexplugin-${1//"plugin"/""}.tar"

  # Find all plugin related libs and tar them, we look at two paths for now
  cd $BUILD_DIR/out/lib
  find * -maxdepth 1 \( -type f -or -type l \) -iname "lib*$plugin_name*.so*" -print0 | tar -cf $OUT_DIR/$tar_name --null -T - &> $OUTPUT_IO
  cd $BUILD_DIR/out/lib/dlvhex2/plugins
  find * -maxdepth 1 \( -type f -or -type l \) -iname "lib*$plugin_name*.so*" -print0 | tar -uf $OUT_DIR/$tar_name --null -T - &> $OUTPUT_IO

  # Compress archive
  rm -f $OUT_DIR/$tar_name.gz &> $OUTPUT_IO
  gzip $OUT_DIR/$tar_name &> $OUTPUT_IO

  cd $ROOT_DIR

  echo -e "===> ${T_DONE} Collect finished"
}


################################################################################
# Script execution

# Check given parameters
while [ "$1" != "" ]; do
  case $1 in
    -v | --verbose )            OUTPUT_IO=/dev/stdout
                                ;;
    -h | --help )               usage
                                exit
                                ;;
    * )                         usage
                                exit 1
  esac
  shift
done

checkDependencies
prepare

# Build plugins
# Iterate over plugins to be built
for plugin in `echo -e $PLUGINS`; do
  processPlugin $plugin
done

echo -e "==> ${T_DONE} All builds finished"
