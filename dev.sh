cmake --build ./build --config Debug --target all -j 12
cp build/mac.xpl dist/FlyWithHTTP/64
cp -r dist/FlyWithHTTP "/Users/sterehov/Library/Application Support/Steam/steamapps/common/X-Plane 11/Resources/plugins"