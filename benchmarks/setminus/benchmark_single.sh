# $1: instance
# $2: timeout

# default parameters
if [ $# -le 1 ]; then
	echo "Error: invalid parameters"
	exit 1
else
	instance=$1
	to=$2
fi

export LD_LIBRARY_PATH=/mnt/lion/home/redl/local/lib
export PATH=$PATH:/mnt/lion/home/redl/local/bin

confstr="--flpcheck=explicit;--flpcheck=explicit --extlearn;--flpcheck=ufsm;--flpcheck=ufsm --extlearn;--flpcheck=ufsm --extlearn --ufslearn;--flpcheck=ufs;--flpcheck=ufs --extlearn;--flpcheck=ufs --extlearn --ufslearn;--flpcheck=aufs;--flpcheck=aufs --extlearn;--flpcheck=aufs --extlearn --ufslearn;--flpcheck=explicit -n=1;--flpcheck=explicit --extlearn -n=1;--flpcheck=ufsm -n=1;--flpcheck=ufsm --extlearn -n=1;--flpcheck=ufsm --extlearn --ufslearn -n=1;--flpcheck=ufs -n=1;--flpcheck=ufs --extlearn -n=1;--flpcheck=ufs --extlearn --ufslearn -n=1;--flpcheck=aufs -n=1;--flpcheck=aufs --extlearn -n=1;--flpcheck=aufs --extlearn --ufslearn -n=1"

# split configurations
IFS=';' read -ra confs <<< "$confstr"
header="#size"
i=0
for c in "${confs[@]}"
do
	header="$header   \"$c\""
	let i=i+1
done
echo $header

# do benchmark
echo -ne "$instance "

# write HEX program
prog="
	nsel(X) :- domain(X), &testSetMinus[domain, sel](X)<monotonic domain,antimonotonic sel>.
	sel(X) :- domain(X), &testSetMinus[domain, nsel](X)<monotonic domain, antimonotonic nsel>.
	:- sel(X), sel(Y), sel(Z), X != Y, X != Z, Y != Z."
for (( j = 1; j <= instance; j++ ))
do
	prog="domain($j). $prog"
done
echo $prog > prog$instance.hex

# for all configurations
i=0
for c in "${confs[@]}"
do
	echo -ne -e " "
	output=$(timeout $to time -f %e dlvhex2 $c --plugindir=../../testsuite/ prog$instance.hex 2>&1 >/dev/null)
	if [[ $? == 124 ]]; then
		output="---"
	fi
	echo -ne $output
	let i=i+1
done
echo -e -ne "\n"

rm prog$instance.hex
