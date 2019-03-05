FROM faustbase:1.00

WORKDIR /grame
RUN git clone --single-branch -b master-dev --recurse-submodules --depth 1 https://github.com/grame-cncm/faust.git
RUN git clone --single-branch -b dev        --recurse-submodules --depth 1 https://github.com/grame-cncm/faustlive.git

COPY   fl-backends.cmake /grame/faust/build/backends
COPY   fl-targets.cmake /grame/faust/build/targets


RUN make -C faust/build BACKENDS=fl-backends.cmake TARGETS=fl-targets.cmake && make -C faust/build install
RUN make -C faustlive/Build 
