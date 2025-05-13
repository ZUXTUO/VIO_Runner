rm -rf build
mkdir build
cd build

cmake ..
make -j$(nproc)

cd ..


cp build/Runner AppDir/usr/bin/
chmod +x AppDir/usr/bin/Runner
cp build/libulocalization.so AppDir/usr/lib/

chmod +x AppDir/*

rm -rf AppDir/AppRun
rm -rf AppDir/AppRun.wrapped

cp $(readlink -f /usr/lib/x86_64-linux-gnu/libOpenGL.so.0) AppDir/usr/lib/
cp $(readlink -f /usr/lib/x86_64-linux-gnu/libGL.so.1) AppDir/usr/lib/
cp $(readlink -f /usr/lib/x86_64-linux-gnu/libX11.so.6)    AppDir/usr/lib/
cp $(readlink -f /usr/lib/x86_64-linux-gnu/libEGL.so.1)    AppDir/usr/lib/
cp $(readlink -f /usr/lib/x86_64-linux-gnu/libGLdispatch.so.0) AppDir/usr/lib/
cp $(readlink -f /usr/lib/x86_64-linux-gnu/libwayland-client.so.0) AppDir/usr/lib/
cp $(readlink -f /usr/lib/x86_64-linux-gnu/libxcb.so.1)    AppDir/usr/lib/

patchelf --set-rpath '$ORIGIN/../lib' AppDir/usr/bin/Runner

export PLUGIN_QT_EXECUTABLE=./linuxdeploy-plugin-qt-x86_64.AppImage

./linuxdeploy-x86_64.AppImage \
    --appdir AppDir \
    --executable AppDir/usr/bin/Runner \
    --desktop-file AppDir/Runner.desktop \
    --icon-file AppDir/myapp.png \
    --output appimage \
    --plugin qt

# ./appimagetool-x86_64.AppImage AppDir \
#   --runtime-file runtime-x86_64
