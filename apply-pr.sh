#!/bin/bash
set -x

function applyPR()
{
	branch="`git branch --show-current`"
	git checkout --detach upstream/master
	curl -L $1$2.patch > /tmp/autopatch$2.patch
	git am /tmp/autopatch$2.patch
	rm -f /tmp/autopatch$2.patch
	git checkout $branch
	echo $branch
	git merge --commit --no-ff HEAD@{1} -m "Merge the pending PR #$2 on top of upstream/master"
}

## SofaPython3
applyPR $1 $2 
