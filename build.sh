#!/bin/zsh
set -e

SRC="main.cpp spawn_object.cpp Art.cpp"
OUT="main"

# Si pkg-config está disponible y puede resolver sfml-all, úsalo (más portable)
if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists sfml-all; then
  echo "Compilando con pkg-config..."
  clang++ -std=c++17 -g main.cpp spawn_object.cpp Art.cpp -o "$OUT" $(pkg-config --cflags --libs sfml-all)
else
  # Detectar Homebrew en Apple Silicon (/opt/homebrew) o Intel (/usr/local)
  if [ -d /opt/homebrew ]; then
    INCL='-I/opt/homebrew/include'
    LIB='-L/opt/homebrew/lib'
  else
    INCL='-I/usr/local/include'
    LIB='-L/usr/local/lib'
  fi
  echo "Compilando con includes: $INCL y libs: $LIB"
  clang++ -std=c++17 -g main.cpp spawn_object.cpp Art.cpp -o "$OUT" $INCL $LIB -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
fi

echo "Compilación completada: ./$OUT"
