#!/bin/bash

export RG="ResourceGroup1"
export BlobStorage="myblobstorage" 
export BlobSAStoken="secret key for the blob storage"
export homebase=`pwd`
export GREEN='\033[0;32m' 
export RED='\033[0;31m' 
export GRAY='\033[0;37m' 
export base_dir=/root/dev_simulation

[ "${DEVICENAME}" == "" ] && echo -e "\n${GREEN}use $0 [devicename] ${GRAY}\n" && exit 1

export BLOBDIR=`echo ${DEVICENAME} | sed 's/-/\//g'`

azcopy copy "https://${BlobStorage}.blob.core.windows.net/certs/${RG}/devices/${BLOBDIR}/${DEVICENAME}/?${BlobSAStoken}" ${homebase}/ --recursive 

[ -d "/etc/aziot" ] || mkdir /etc/aziot
[ -d "/etc/aziot/secrets" ] || mkdir /etc/aziot/secrets

mv ${homebase}/${DEVICENAME}/sim_config.json /etc/aziot
mv ${homebase}/${DEVICENAME}/* /etc/aziot/secrets
rmdir ${homebase}/${DEVICENAME}

export APP_RPT=`cat /etc/aziot/sim_config.json | grep "APP_RPT" | head -1 | sed 's/"APP_RPT"//g' | sed 's/://g' | sed 's/,//g'`
export APP_SLP_MIN=`cat /etc/aziot/sim_config.json | grep "APP_SLP_MIN" | head -1 | sed 's/"APP_SLP_MIN"//g' | sed 's/://g' | sed 's/,//g'`
export APP_SLP_RDM=`cat /etc/aziot/sim_config.json | grep ""APP_SLP_RDM"" | head -1 | sed 's/"APP_SLP_RDM"//g' | sed 's/://g' | sed 's/,//g'`


${base_dir}/prov_dev_client_sample

${base_dir}/iothub_ll_client_x509_sample

for (( c=1; c<=$APP_RPT; c++ ))
do
	echo "Welcome $c times"
	export SLEEP=`echo $(($APP_SLP_MIN + $RANDOM % $APP_SLP_RDM))`
	echo "Sleep $SLEEP" 
	sleep $SLEEP
	${base_dir}/iothub_ll_client_x509_sample
done


exit 





