rm -rf linuxdeploy-x86_64.AppImage
rm -rf linuxdeploy-plugin-qt-x86_64.AppImage
rm -rf appimagetool-x86_64.AppImage

wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage

chmod +x linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage