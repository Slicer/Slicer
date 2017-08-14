#!/usr/bin/env bash

set -ex

thirdparty_module_name='qtstyleplugins'

upstream_git_url='git://code.qt.io/qt/qtstyleplugins.git'
upstream_git_branch='master'

snapshot_relative_path='src'

rm -rf /tmp/$thirdparty_module_name
pushd /tmp
git clone $upstream_git_url -b $upstream_git_branch
cd $thirdparty_module_name
sha=$(git rev-parse --short HEAD)
popd
cp /tmp/$thirdparty_module_name/src/plugins/styles/shared/* $snapshot_relative_path/
cp /tmp/$thirdparty_module_name/src/plugins/styles/cleanlooks/* $snapshot_relative_path/
rm -f $snapshot_relative_path/shared.pri
rm -f $snapshot_relative_path/cleanlooks.pro
rm -f $snapshot_relative_path/cleanlooks.json
rm -f $snapshot_relative_path/plugin.cpp

echo "Example of commit message:

This commit integrates $thirdparty_module_name files based of commit $sha
from $upstream_git_url
"
