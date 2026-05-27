FROM ubuntu:18.04

ARG NUM_JOBS=8
ARG BOOST_MAJOR=1
ARG BOOST_MINOR=83
ARG CMAKE_VERSION=3.27.7
ARG POSTGRES_VERSION=15.3
ARG NINJA_VERSION=1.11.1
ARG CROSSTOOL_VERSION=1.25.0

RUN dpkg --add-architecture i386

RUN apt-get update && \
    apt-get install -y git curl build-essential \
            libc6-i386 lib32z1 lib32ncurses5 \
            libxml-simple-perl \
            bc rsync cpio file net-tools liblzo2-2 \
            zlib1g-dev libcurl4-openssl-dev \
            gcc g++ gperf bison flex texinfo help2man make libncurses5-dev \
            python3-dev autoconf automake libtool libtool-bin gawk wget bzip2 xz-utils unzip \
            patch libstdc++6 rsync meson

RUN apt-get clean && rm -rf /var/lib/apt/lists/*

RUN useradd -ms /bin/bash century

USER century
WORKDIR /home/century/
RUN mkdir .ssh && chmod 0600 .ssh
ADD id_ed25519 .ssh/

USER root
WORKDIR /
RUN mkdir /usr/local/crosstool-ng/
RUN chown century: /opt /usr/local/crosstool-ng

USER century
WORKDIR /opt/

# Usando o Git da Montage
#RUN git clone --branch MTSDK-L01-2.2.0-01639-S2-00-0 --depth 1 ssh://century@socguestgerrit.montage-lz.com:29418/linux && cd linux && git checkout -b MTSDK-L01-2.0.0-01639-S2-00-0

# Usando um tar já gerado pelo Git da Monatge e hospedado localmente
RUN curl http://192.168.30.7/montage/sdk-rt720-20231024.tar.gz | tar xzv && mv sdk montage

USER root
WORKDIR /opt/
RUN chown root: /opt

USER root
WORKDIR /usr/local/crosstool-ng/
RUN tar xf /opt/montage/linux/tools/toolchain/gcc-6.3-mipsel-mt-linux-gnu-rm2.0.tar.xz
#RUN curl http://192.168.30.7/montage/gcc-11-mipsel-mt-linux-gnu-rm2.0.tar.gz | tar xzv

ENV PATH=$PATH:/usr/local/crosstool-ng/mipsel-mt-linux-gnu/bin/

USER century

WORKDIR /opt/montage/linux
RUN bash -c "source envsetup lotus_symphony2.cfg && make clean && make"

WORKDIR /opt/montage/sdkproduct/lotus/
RUN bash -c "source setenv.sh customer=demo chipsub=symphony2 && ./auto_build_symphony_mdl_glibc.sh && ./auto_build_symphony_mmi_glibc.sh && make && make flash"

WORKDIR /home/century
RUN mkdir work
WORKDIR /home/century/work

RUN curl -kv https://codeload.github.com/Kitware/CMake/tar.gz/refs/tags/v${CMAKE_VERSION} | tar xzv && \
    cd CMake-${CMAKE_VERSION} && \
    mkdir build && \
    cd build && \
    ../configure --prefix=/usr --no-qt-gui --system-curl --parallel=$NUM_JOBS && \
    make -j $NUM_JOBS

USER root
WORKDIR /home/century/work/CMake-${CMAKE_VERSION}/build
RUN make install && cd /home/century/work/ && rm -rf CMake-${CMAKE_VERSION}

USER century
WORKDIR /home/century/work
RUN curl -kL https://boostorg.jfrog.io/artifactory/main/release/${BOOST_MAJOR}.${BOOST_MINOR}.0/source/boost_${BOOST_MAJOR}_${BOOST_MINOR}_0.tar.bz2 | tar xjv
WORKDIR /home/century/work/boost_${BOOST_MAJOR}_${BOOST_MINOR}_0
RUN ./bootstrap.sh
COPY project-config.jam ./

RUN ./b2 variant=release link=static --prefix=/opt/montage/boost --build-dir=./build --without-python --reconfigure --layout=system -j$NUM_JOBS --toolset=gcc-mips --threading=single install && \
    cd .. && \
    rm -rf boost_${BOOST_MAJOR}_${BOOST_MINOR}_0

USER century
WORKDIR /home/century/work
RUN curl -kL https://ftp.postgresql.org/pub/source/v${POSTGRES_VERSION}/postgresql-${POSTGRES_VERSION}.tar.bz2 | tar xjv
WORKDIR /home/century/work/postgresql-${POSTGRES_VERSION}/
RUN ./configure --prefix=/opt/montage/pgsql --host=mips-linux-gnu CC=mips-linux-gnu-gcc AR=mips-linux-gnu-ar CPP=mips-linux-gnu-cpp LDFLAGS="-L/opt/montage/linux/user_fs/lib/" CPPFLAGS="-I/opt/montage/linux/opensource-2nd/output/mips/soft-float/el/include/" --without-readline --without-python && \
    make -j$NUM_JOBS && \
    cd ./src/include && \
    make install && \
    cd /home/century/work/postgresql-${POSTGRES_VERSION}/src/interfaces/libpq/ && \
    make install && \
    cd /home/century/work/postgresql-${POSTGRES_VERSION}/src/bin && \
    make install && \
    cd /opt/montage/pgsql/include && \
    ln -s libpq-fe.h libpq.h && \
    cd /home/century/work && \
    rm -rf postgresql-${POSTGRES_VERSION}

USER century
WORKDIR /home/century/work
RUN curl -kL https://github.com/ninja-build/ninja/archive/refs/tags/v${NINJA_VERSION}.tar.gz | tar xzv
WORKDIR /home/century/work/ninja-${NINJA_VERSION}/
RUN cmake -DCMAKE_BUILD_TYPE=Release . && cmake --build . -j 12

USER root
WORKDIR /home/century/work/ninja-${NINJA_VERSION}/
RUN cmake --install . --prefix /usr  && \
    cd .. && \
    rm -rf ninja-${NINJA_VERSION}

# USER century
# WORKDIR /home/century/work
#
# RUN curl http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-$CROSSTOOL_VERSION.tar.bz2 | tar xjv && \
#     cd crosstool-ng-$CROSSTOOL_VERSION && \
#     ./configure --enable-local && \
#     make -j20 && \
#     bzcat ../../build.log.bz2 | ./ct-ng extractconfig > .config

### TMux

# USER century
# WORKDIR /home/century/work
#
#
# USER century
# WORKDIR /home/century/work
# RUN curl -kL https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz | tar xzv
# RUN ./configure --prefix=/opt/montage/tmux --host=mips-linux-gnu CC=mips-linux-gnu-gcc AR=mips-linux-gnu-ar CPP=mips-linux-gnu-cpp LDFLAGS="-L/opt/montage/linux/user_fs/lib/" CPPFLAGS="-I/opt/montage/linux/opensource-2nd/output/mips/soft-float/el/include/" --disable-openssl && make -j 20 && make install
#
# WORKDIR /home/century/work
# RUN curl -kL https://github.com/tmux/tmux/releases/download/3.3a/tmux-3.3a.tar.gz | tar xzv
# WORKDIR /home/century/work/tmux-3.3a/
# RUN ./configure --prefix=/opt/montage/tmux --host=mips-linux-gnu CC=mips-linux-gnu-gcc AR=mips-linux-gnu-ar CPP=mips-linux-gnu-cpp LDFLAGS="-L/opt/montage/linux/user_fs/lib/ -L/opt/montage/tmux/lib/" CPPFLAGS="-I/opt/montage/linux/opensource-2nd/output/mips/soft-float/el/include/ -I/opt/montage/tmux/include/" && make -j20
