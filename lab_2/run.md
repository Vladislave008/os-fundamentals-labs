# Сборка
- Windows:  cmake -S . -B build-windows
            cmake --build build-windows
- Linux:    cmake --build build-linux
            cmake -S . -B build-linux
# Бинарники:
- Windows:  ./build-windows/bin/Debug/parent.exe
            ./build-windows/bin/Debug/child.exe
- Linux:    ./build-linux/bin/parent
            ./build-linux/bin/child

# Запуск из папки с бинарниками:
- Windows:  cd build-windows/bin/Debug
            ./parent.exe  
- Linux:    cd build-linux/bin
            ./parent  

# Скрыть ошибки/логи: 
- Windows: ./parent 2>$nul
- Linux: ./parent 2>/dev/null