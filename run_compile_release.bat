cmake -S . -B build ^
  -D CMAKE_TOOLCHAIN_FILE=%ANDROID_NDK_HOME%\build\cmake\android.toolchain.cmake ^
  -D ANDROID_PLATFORM=26 ^
  -D ANDROID_ABI=arm64-v8a ^
  -D CMAKE_ANDROID_ARCH_ABI=arm64-v8a ^
  -D CMAKE_BUILD_TYPE=Release ^
  -G Ninja

cmake --build build

pause