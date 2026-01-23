#!/bin/bash

func_usage() {
	echo "Usage: $0 [hp|mic] [44100|48000]"
	exit 1
}

if [ $# -ne 2 ]; then
	func_usage
fi

sample_rate=$2

# Select playback/capture
if [[ "$1" == "hp" ]]; then
	amixer -c APE -q sset 'I2S1 Mux' 'ADMAIF1'
	amixer -c APE -q sset "I2S1 Sample Rate" ${sample_rate}

	amixer -c APE -q cset name='DA72 Headphone Switch' on
	amixer -c APE -q cset name='DA72 Headphone Volume' 60%

	amixer -c APE -q cset name='DA72 Lineout Switch' on
	amixer -c APE -q cset name='DA72 Lineout Volume' 80%

	amixer -c APE -q cset name='DA72 Mixout Left DAC Left Switch' on
	amixer -c APE -q cset name='DA72 Mixout Right DAC Right Switch' on

	echo "!!!!!! star playback !!!!!!"
	aplay -D hw:APE,0 test.wav

elif [[ "$1" == "mic" ]]; then
	amixer -c APE -q sset 'ADMAIF1 Mux' 'I2S1'
	amixer -c APE -q sset "I2S1 Sample Rate" ${sample_rate}

	amixer -c APE -q cset name='DA72 Mixin PGA Switch' on
	amixer -c APE -q cset name='DA72 Mixin PGA Volume' 80%

	amixer -c APE -q cset name='DA72 ADC Switch' on
	amixer -c APE -q cset name='DA72 ADC Volume' 80%

	amixer -c APE -q cset name='DA72 DAI Left Source MUX' 'ADC Left'
	amixer -c APE -q cset name='DA72 DAI Right Source MUX' 'ADC Right'
#	amixer -c APE -q cset name='DA72 DAI Right Source MUX' 'ADC Left'

	amixer -c APE -q cset name='DA72 Mic 1 Switch' on
	amixer -c APE -q cset name='DA72 Mic 1 Volume' 50%
#	amixer -c APE -q cset name='DA72 Mic 1 Amp Source MUX' 'Differential'
	amixer -c APE -q cset name='DA72 Mic 1 Amp Source MUX' 'MIC_P'
	amixer -c APE -q cset name='DA72 Mixin Left Mic 1 Switch' on
	amixer -c APE -q cset name='DA72 Mixin Right Mic 1 Switch' on

	amixer -c APE -q sset 'DA72 ADC Voice Cutoff' '200Hz'

	if [[ -f test.wav ]]; then
		rm test.wav
	fi
	echo "!!!!!! star recording, press Ctrl+c to exit !!!!!!"
	arecord -D hw:APE,0 -f S16_LE -r ${sample_rate} -c 2 test.wav
else
	func_usage
fi

