#!/usr/bin/env bash

set -euxo pipefail

# See https://github.com/codespell-project/codespell#readme

SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")


cd $SCRIPT_DIR/../../

if ! command -v codespell &> /dev/null; then
  echo >&2 'error: "codespell" command not found in the PATH !'
  exit 1
fi

if ! command -v yq &> /dev/null; then
  echo >&2 'error: "yq" command not found in the PATH !'
  exit 1
fi

github_action_codespell_workflow=${SCRIPT_DIR}/../../.github/workflows/codespell.yml
if [[ ! -e ${github_action_codespell_workflow} ]]; then
  echo >&2 'error: "codespell.yml" file not found at ${github_action_codespell_workflow}'
  exit 1
fi

skip=$(yq ".jobs.codespell.steps[1].with.skip" ${github_action_codespell_workflow} | sed 's/, /,/g')

ignore_words_file=$(yq ".jobs.codespell.steps[1].with.ignore_words_file" ${github_action_codespell_workflow})
if [[ ! -e ${ignore_words_file} ]]; then
  echo >&2 'error: "ignore words file not found at ${ignore_words_file}'
  exit 1
fi

codespell -w -q6 --skip="${skip}" -I ${ignore_words_file}
