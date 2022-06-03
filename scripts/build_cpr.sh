rm -rf ./cpr

git clone https://github.com/libcpr/cpr
cd cpr
#git apply ../cpr1.patch
cd ..

rm -rf ../cpp/cpr
rm -rf ../cpp/include/cpr
mkdir ../cpp/cpr
mkdir ../cpp/include/cpr

mv ./cpr/cpr/*.cpp ../cpp/cpr
mv ./cpr/include/cpr/*.h ../cpp/include/cpr
rm -rf ./cpr

# build for iPhone
#cd ..
#rm -rf build
#mkdir build
#cd build
#cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake -DPLATFORM=OS64
#cmake --build . --target hv_static --config Release
#mv lib/Release/libhv_static.a ../../build/devices/libhv_static.a
#
#cd ../..
#rm -rf libhv
#
## put everything to xcframework
#
#xcodebuild -create-xcframework \
#    -library build/simulators/libhv_static.a \
#    -library build/devices/libhv_static.a \
#    -output Libhv.xcframework
