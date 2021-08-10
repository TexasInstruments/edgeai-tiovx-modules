# edgeai-tiovx-modules
Repository to host TI's OpenVx modules used in EdgeAI SDK. These modules serve as the bridge between EdgeAI's custom GStreamer elements and underlying hardware accelerator cores in Jacinto class of devices.

## Dependencies
This OpenVx modules are validated only on TDA4VM/J721E/J7ES board using
EdgeAI image built using PSDK-LINUX and PSDK-RTOS

These modules are used by edgeai-gst-plugins https://github.com/TexasInstruments/edgeai-gst-plugins/

## Steps to clone and build on target
clone the repo under '/opt'
```
/opt# git clone https://github.com/TexasInstruments/edgeai-tiovx-modules.git
/opt/edgeai-tiovx-modules# mkdir build
/opt/edgeai-tiovx-modules# cd build
/opt/edgeai-tiovx-modules/build# cmake ..
/opt/edgeai-tiovx-modules/build# make -j2
/opt/edgeai-tiovx-modules/build# make install
```

This will install,

- Shared library at /opt/lib/egeai-tiovx-modules/libedgeai-tiovx-modules.so
- Headers at /opt/include/edgeai-tiovx-modules/include


