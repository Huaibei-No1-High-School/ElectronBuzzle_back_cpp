FROM debian:12.0
COPY ./ /ele
RUN ["apt", "update"]
RUN ["apt", "upgrade", "-y"]
RUN ["apt", "install", "git", "pkg-config", "g++", "cmake", "make", "curl", "zip", "unzip", "tar", "ninja-build","-y"]
RUN ["git", "clone", "https://github.com/Microsoft/vcpkg.git"]
RUN ["./vcpkg/bootstrap-vcpkg.sh"]
RUN ["/vcpkg/vcpkg", "install", "libhv"]
RUN ["/vcpkg/vcpkg", "install", "glog"]
RUN ["/vcpkg/vcpkg", "install", "yaml-cpp"]
WORKDIR /ele/build
RUN ["cmake", "-B", ".", "-S", "..", "-DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake"]
RUN ["cmake", "--build", "."]
CMD /ele/build/electronBuzzer
