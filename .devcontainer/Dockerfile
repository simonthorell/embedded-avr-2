FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

# Set the CMake version to be reinstalled from source if needed.
ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.22.2"

# Reinstall CMake the specific version if required.
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# Install AVR toolchain
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends avr-libc avrdude binutils-avr gcc-avr gdb-avr make \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*