# edgeai-tiovx-modules
Repository to host TI's OpenVx modules used in EdgeAI SDK.

## Dependencies
This OpenVx modules are validated only on TDA4VM/J721E/J7ES board using
EdgeAI image built using PSDK-LINUX and PSDK-RTOS

These modules are used by edgeai-gst-plugins https://github.com/TexasInstruments/edgeai-gst-plugins/

## Steps to clone and build on target
clone the repo under '/opt'
```
/opt# git clone https://github.com/TexasInstruments/edgeai-tiovx-modules.git
```

### Compilation on the target
The library can be built directly on the target as follows.

```
/opt# cd /opt/edgeai-tiovx-modules
/opt/edgeai-tiovx-modules# mkdir build
/opt/edgeai-tiovx-modules# cd build
/opt/edgeai-tiovx-modules/build# cmake ..
/opt/edgeai-tiovx-modules/build# make -j2
```

### Installation
The following command installs the library and header files under /usr dirctory. The headers
and library will be placed as follows

```
/opt/edgeai-tiovx-modules/build# make install
```

The headers and library will be placed as follows

- **Headers**: /usr/**include**/edgeai-tiovx-modules/
- **Library**: /usr/**lib**/

The desired install location can be specified as follows

```
/opt/edgeai-tiovx-modules/build# cmake -DCMAKE_INSTALL_PREFIX=<path/to/install> ..
/opt/edgeai-tiovx-modules/build# make -j2
/opt/edgeai-tiovx-modules/build# make install
```

- **Headers**: path/to/install/**include**/edgeai-tiovx-modules/
- **Library**: path/to/install/**lib**/

### Cross-Compilation for the target
The library can be cross-compiled on an x86_64 machine for the target. Here are the steps for cross-compilation.
Here 'work_area' is used as the root directory for illustration.

```
cd work_area
git clone https://github.com/TexasInstruments/edgeai-tiovx-modules.git
cd edgeai-tiovx-modules
# Update cmake/setup_cross_compile.sh to specify tool paths and settings
mkdir build
cd build
source ../cmake/setup_cross_compile.sh
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/cross_compile_aarch64.cmake ..
make -j2
make install DESTDIR=<path/to/targetfs>
```
The library and headers will be placed under the following directory structire. Here we use the default
'/usr/' install previx and we prepend the tarfetfs directory location

- **Headers**: path/to/targetfs/usr/**include**/edgeai-tiovx-modules/
- **Library**: path/to/targetfs/usr/**lib**/

