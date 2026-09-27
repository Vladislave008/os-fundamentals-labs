# Сборка

## Windows

```sh
cmake -S . -B build-windows
cmake --build build-windows --config Release
```

## Linux

```sh
cmake -S . -B build-linux
cmake --build build-linux
```

# Бинарники

- Windows: `build-windows/bin/Release/parent.exe`, `build-windows/bin/Release/child.exe`
- Linux: `build-linux/bin/parent`, `build-linux/bin/child`

# Запуск из папки с бинарниками

## Windows

```sh
cd build-windows/bin/Release
./parent.exe
```
- Ctrl+Z для EOF

## Linux

```sh
cd build-linux/bin
./parent
```
- Ctrl+D для EOF

# Скрыть сообщения программ

```sh
# Windows (PowerShell)
./parent 2>$null

# Windows (cmd.exe)
./parent 2>nul

# Linux
./parent 2>/dev/null
```