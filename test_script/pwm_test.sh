#!/bin/bash

declare -A PWM_DICT=(
 [pwm1]=pwmchip3
 [pwm2]=pwmchip0
 [pwm3]=pwmchip2
)

if [ $# -ne 2 ]; then
    echo "Usage: $0 [pwm1|pwm2|pwm3] 5"
    exit 1
fi
timeout=$2
key=$1
pwmchip=${PWM_DICT[$key]}

if [ -z ${key} ]; then
    echo "need input pwm1 or pwm2 or pwm3"
    exit 1
fi

if [ -z ${pwmchip} ]; then
    echo "do not support $key as pwm"
    exit 1
fi

echo "key=$key, pwmchip=$pwmchip"
classfs="/sys/class/pwm/${pwmchip}"

echo 0 > ${classfs}/export
sleep 1
echo 100000000 > ${classfs}/pwm0/period
echo 50000000 > ${classfs}/pwm0/duty_cycle
echo 1 > ${classfs}/pwm0/enable

sleep $timeout

echo 0 > ${classfs}/pwm0/enable
echo 0 > ${classfs}/unexport

