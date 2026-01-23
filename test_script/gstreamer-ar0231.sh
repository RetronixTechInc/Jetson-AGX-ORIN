#!/bin/bash

gst-launch-1.0 nvcompositor name=comp \
  sink_0::xpos=0       sink_0::ypos=0       sink_0::width=1920 sink_0::height=1024 \
  sink_1::xpos=1920 sink_1::ypos=0       sink_1::width=1920 sink_1::height=1024 \
  sink_2::xpos=0       sink_2::ypos=1024 sink_2::width=1920 sink_2::height=1024 \
  sink_3::xpos=1920 sink_3::ypos=1024 sink_3::width=1920 sink_3::height=1024 ! \
  'video/x-raw(memory:NVMM), width=3840, height=2048, format=(string)RGBA, framerate=(fraction)28/1' ! queue ! nv3dsink \
  nvarguscamerasrc sensor-id=0 ! 'video/x-raw(memory:NVMM),width=(int)3840, height=(int)1024, format=NV12, framerate=(fraction)28/1' ! \
  nvvidconv ! 'video/x-raw(memory:NVMM), width=1920, height=1024, format=(string)RGBA, framerate=(fraction)28/1' ! comp.sink_2 \
  nvarguscamerasrc sensor-id=1 ! 'video/x-raw(memory:NVMM),width=(int)3840, height=(int)1024, format=NV12, framerate=(fraction)28/1' ! \
  nvvidconv ! 'video/x-raw(memory:NVMM), width=1920, height=1024, format=(string)RGBA, framerate=(fraction)28/1' ! comp.sink_0 \
  nvarguscamerasrc sensor-id=2 ! 'video/x-raw(memory:NVMM),width=(int)3840, height=(int)1024, format=NV12, framerate=(fraction)28/1' ! \
  nvvidconv ! 'video/x-raw(memory:NVMM), width=1920, height=1024, format=(string)RGBA, framerate=(fraction)28/1' ! comp.sink_1 \
  nvarguscamerasrc sensor-id=3 ! 'video/x-raw(memory:NVMM),width=(int)3840, height=(int)1024, format=NV12, framerate=(fraction)28/1' ! \
  nvvidconv ! 'video/x-raw(memory:NVMM), width=1920, height=1024, format=(string)RGBA, framerate=(fraction)28/1' ! comp.sink_3
