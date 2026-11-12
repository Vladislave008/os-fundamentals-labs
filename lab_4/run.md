# Сборка и запуск

Выполнять из каталога `lab_4` под Linux. Нужны CMake 3.15+ и компилятор C11.

```sh
cmake -S . -B build-linux
cmake --build build-linux
cd build-linux/bin
./parent
```

На первой строке введите имя файла результата, затем числа через пробел. Пустая строка или Ctrl+D завершают ввод.
