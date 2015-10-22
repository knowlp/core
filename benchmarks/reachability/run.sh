#!/bin/bash

runheader=$(which run_header.sh)
if [[ $runheader == "" ]] || [ $(cat $runheader | grep "run_header.sh Version 1." | wc -l) == 0 ]; then
        echo "Could not find run_header.sh (version 1.x); make sure that the benchmark scripts directory is in your PATH"
        exit 1
fi
source $runheader

# run instances
if [[ $all -eq 1 ]]; then
	# run all instances using the benchmark script run insts
	$bmscripts/runinsts.sh "instances/*.graph" "$mydir/run.sh" "$mydir" "$to" "" "" "$req"
else
	# run single instance
	confstr="--extlearn --flpcheck=aufs --ufslearn=none --liberalsafety reachability.hex -n=1;--extlearn --flpcheck=aufs --ufslearn=none --strongsafety reachability_strongsafety.hex -n=1"

	$bmscripts/runconfigs.sh "dlvhex2 --plugindir=../../testsuite --verbose=8 CONF INST" "$confstr" "$instance" "$to" "$bmscripts/gstimeoutputbuilder.sh"
fi

