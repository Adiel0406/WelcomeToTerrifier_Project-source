@echo off
echo BUILDING G++ GAME
g++ main.cpp spawn_object.cpp -IC:\SFML\include -LC:\SFML\lib -lsfml-graphics -lsfml-window -lsfml-system -o sfml_app.exe
echo.
echo done.
pause
sfml_app