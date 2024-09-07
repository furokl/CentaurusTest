# Тестовое задание

![Компания](https://avatars.mds.yandex.net/i?id=1bbb8bceaf2dd0636077cc070929d18c4f34588c-9290726-images-thumbs&n=13)

## Кратко

Было написано полноценное клиент-серверное приложение


## Описание задачи

Client-Server application for monitoring work activity

Explanation
Simple application to show current work activity of all employers in organisation

Example applications 
https://www.teramind.co/solutions/employee-monitoring-software
https://veriato.com/product/

Client (windows) - `c`/`c++`
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

Основные инструкции файла:
```bat
rd /s /q .\build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
powershell [create lnk .sln]
powershell [create lnk .exe]
```
