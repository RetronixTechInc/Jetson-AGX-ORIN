#!/bin/bash

PIN_ARRAY=(
 NONE 	  
 VDD	VDD   
 PDD.02 VDD	 
 PDD.01	GND	 
 PQ.06	PR.02 
 GND	PR.03 
 PR.04	PH.07 
 PR.00	GND   
 PN.01	PBB.01
 VDD	PH.00 
 PZ.05	GND   
 PZ.04 	PP.04 
 PZ.03 	PZ.06 
 GND	PZ.07 
 PDD.01	PCC.07
 NONE	GND   
 NONE	PBB.00
 PAA.02	GND   
 PI.02	PR.05 
 PAA.03	PI.01 
 GND	PI.00  
)

input_flag=0
output_flag=0
output_value=2

while getopts "i:o:v:" opt; do
    if [ ${opt} == "i" ];then
	input_flag=1
	input_idx=$OPTARG
	input_pin=${PIN_ARRAY[$input_idx]}
    elif [ ${opt} == "o" ];then
	output_flag=1
	output_idx=$OPTARG
	output_pin=${PIN_ARRAY[$output_idx]}
    elif [ ${opt} == "v" ];then
	output_value=$OPTARG
    fi
done

if [ $output_flag = 1  ];then
    output_info=$(gpiofind ${output_pin})
    rst=$?
    # echo "rst=$rst"
    if [ $rst != 0 ];then
	echo "-o ${output_idx} is not a gpio"
	exit 1
    fi
    if [ $output_value = 2 ];then
	echo "need -v with 0 or 1 as output value"
	exit 1
    fi
    gpioset -b -m time -s 2 ${output_info}=${output_value}
fi

if [ $input_flag = 1  ];then
    input_info=$(gpiofind ${input_pin})
    rst=$?
    # echo "rst=$rst"
    if [ $rst != 0  ];then
	echo "-i ${input_idx} is not a gpio"
	exit 1
    fi
    input_value=$(gpioget ${input_info}) 
fi

echo "output_flag=${output_flag}: output_idx=${output_idx}, output_pin=${output_pin} , output_info=${output_info}, output_value=${output_value}"
echo "input_flag=${input_flag}: input_idx=${input_idx}, input_pin=${input_pin}, input_info=${input_info}, input_value=${input_value}"

#idx=0
#for pin in ${PIN_ARRAY[@]}; do
#    if [[ ${pin} != NONE && ${pin} != VDD && ${pin} != GND ]];then
#	rst=$(gpiofind ${pin})
#	echo "[$idx]=${pin}=$rst"
#    fi

#    idx=$(expr $idx + 1)
#done

