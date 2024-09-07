# Тестовое задание

![Компания](https://avatars.mds.yandex.net/i?id=1bbb8bceaf2dd0636077cc070929d18c4f34588c-9290726-images-thumbs&n=13)

## Кратко

Было написано полноценное клиент-серверное приложение под Windows с применением C++ и WinApi.

Клиентов может быть несколько, сервер - один.
Все решения настроены при помощи CMake на стандарт C++17.

Соединение настраивается через файл `server.properties`

Основные возможности сервера:
* Получать информацию о времени подключения, ip-адрес, Имя пользователя, Имя компьютера.
* Делать снимок экрана клиента.

Основные команды сервера:

`/scn {id}`  - сделать снимок экрана.

`/list`      - отобразить список активных клиентов.

`/exit`      - завершить сессию.

`/help`      - отобразить возможные команды.

Если сервер отключен, клиент будет стараться возобновить соединение.

Требования: Windows, MSVC.

Вес приложения около ~80 КБ

Ниже будет информация про сборку и установки клиента на автозапуск.

## Описание задачи

Client-Server application for monitoring work activity

Explanation
Simple application to show current work activity of all employers in organisation

Example applications 

https://www.teramind.co/solutions/employee-monitoring-software

https://veriato.com/product/

Client (windows) - `c/c++`
Silent launches on user logon and work in background
Communicates with server at any protocol
You can't use third-party libraries like boost and others, and you can't use frameworks like Qt and others. 

Server - desktop or web interface - any language 
List all connected clients - domain/machine/ip/user
Show client’s last active time
Ability to get screenshot from client’s desktop 

In response send link to github.com project page, which contains all Visual Studio solution files with full source code and dependencies if any.

## Сборка и запуск

Для построения решения и готового приложения необходимо запустить `build vs22.bat`.

В корне появятся ярлыки на решение `*sln` и исполняемые файлы: клиент, сервер `*exe`, также папка Build со сборкой.

Подойдет для Visual Studio 2022,
для 19 версии отредактируйте строку на `cmake -G "Visual Studio 16 2019" -A x64`.

Также cmake подскажет другие среды разработки, если ввести `cmake -G`


Основные инструкции файла:
```bat
rd /s /q .\build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
powershell [create lnk .sln]
powershell [create lnk .exe]
```

Чтобы установить приложение на автозапуск необходимо запустить `autostart-client.bat`.
```bat
set "scriptPath=%~dp0"
set "shortcutPath=%scriptPath%Centaurus-Client.exe.lnk"
reg add [HKEY_CURRENT_USER]
```
