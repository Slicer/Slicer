#!/bin/bash
#
# This script documents how to update a branch
# to apply pre-commit enforcements to every commit

#branch_to_rebase_from=$1
branch_to_rebase_from=jcfr/style-consistently-indent-cpp-files

echo "For each commit that required changes,\n
git add -p # <- Review the changes made and add them
git commit --amend # <- amend the commit
git rebase --continue # <- goto next commit
"
git rebase --exec "pre-commit run --from-ref HEAD~1 --to-ref HEAD --hook-stage=commit" ${branch_to_rebase_from}
