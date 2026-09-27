# Сборка
- Windows:  cmake -S . -B build-windows
            cmake --build build-windows --config Release
- Linux:    cmake -S . -B build-linux
            cmake --build build-linux

# Бинарники:
- Windows:  ./build-windows/bin/Release/parent.exe
            ./build-windows/bin/Release/child.exe
- Linux:    ./build-linux/bin/parent
            ./build-linux/bin/child

# Запуск из папки с бинарниками:
- Windows:  cd build-windows/bin/Release
            ./parent.exe  
- Linux:    cd build-linux/bin
            ./parent  

# Скрыть ошибки/логи: 
- Windows: ./parent 2>$nul
- Linux: ./parent 2>/dev/null