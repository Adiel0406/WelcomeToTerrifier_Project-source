#include "Art.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

Art::Art(float startX, float startY)
    : x(startX), y(startY), velocidad(150.f), rangoDeteccion(300.f),
      estadoActual(PATRULLANDO), direccionActual(ABAJO), frameActual(0),
      tiempoPorFrame(0.15f), tiempoAnimacion(0.f), caminando(false),
      waypointActual(0), distanciaUmbral(15.f), temporizadorEspera(0.f),
      esperando(false), patrullaCiclica(true)
{
    sf::Texture tex; tex.create(45,45);
    sprite.setTexture(tex);
    sprite.setPosition(x,y);
    sprite.setOrigin(22.5f,22.5f);
}

void Art::actualizar(float deltaTime, float playerX, float playerY) {
    float distanciaAlJugador = calcularDistancia(playerX, playerY);

    if (distanciaAlJugador <= rangoDeteccion) {
        if (estadoActual != PERSEGUIR) {
            // guardar estado si hace falta
        }
        estadoActual = PERSEGUIR;
    } else {
        if (estadoActual == PERSEGUIR) estadoActual = REGRESANDO;
    }

    switch (estadoActual) {
        case PATRULLANDO: actualizarPatrulla(deltaTime); break;
        case PERSEGUIR: actualizarPersecucion(deltaTime, playerX, playerY); break;
        case REGRESANDO: actualizarRegreso(deltaTime); break;
    }

    actualizarAnimacion(deltaTime);
    sprite.setPosition(x,y);
}

void Art::actualizarPatrulla(float deltaTime) {
    if (waypoints.empty()) { caminando=false; return; }
    if (esperando) {
        temporizadorEspera -= deltaTime;
        if (temporizadorEspera <= 0.f) {
            esperando = false;
            waypointActual++;
            if (waypointActual >= waypoints.size()) {
                if (patrullaCiclica) waypointActual = 0;
                else { waypointActual = waypoints.size()-1; caminando=false; return; }
            }
        }
        caminando = false; return;
    }

    Waypoint& wp = waypoints[waypointActual];
    float distancia = calcularDistanciaAlPunto(wp.posicion);
    if (distancia <= distanciaUmbral) {
        esperando = true; temporizadorEspera = wp.tiempoEspera; caminando=false; return;
    }
    moverHaciaPunto(wp.posicion.x, wp.posicion.y, deltaTime);
    caminando = true;
}

void Art::actualizarPersecucion(float deltaTime, float playerX, float playerY) {
    float diffX = playerX - x; float diffY = playerY - y;
    float distancia = calcularDistancia(playerX, playerY);
    actualizarDireccion(diffX, diffY);
    if (distancia > 20.f) {
        float dirX = diffX / distancia; float dirY = diffY / distancia;
        x += dirX * velocidad * 1.3f * deltaTime;
        y += dirY * velocidad * 1.3f * deltaTime;
        caminando = true;
    } else caminando = false;
}

void Art::actualizarRegreso(float deltaTime) {
    if (waypoints.empty()) { estadoActual = PATRULLANDO; return; }
    Waypoint& wp = waypoints[waypointActual];
    float distancia = calcularDistanciaAlPunto(wp.posicion);
    if (distancia <= distanciaUmbral) { estadoActual = PATRULLANDO; esperando=true; temporizadorEspera=wp.tiempoEspera; caminando=false; return; }
    moverHaciaPunto(wp.posicion.x, wp.posicion.y, deltaTime); caminando=true;
}

size_t Art::encontrarWaypointMasCercano() const {
    if (waypoints.empty()) return 0;
    size_t mejor = 0; float mejorD = calcularDistanciaAlPunto(waypoints[0].posicion);
    for (size_t i=1;i<waypoints.size();++i){ float d=calcularDistanciaAlPunto(waypoints[i].posicion); if (d<mejorD){ mejorD=d; mejor=i; }}
    return mejor;
}

void Art::moverHaciaPunto(float targetX, float targetY, float deltaTime) {
    float diffX = targetX - x; float diffY = targetY - y;
    float distancia = std::sqrt(diffX*diffX + diffY*diffY);
    actualizarDireccion(diffX, diffY);
    if (distancia>0.f){ float dirX=diffX/distancia, dirY=diffY/distancia; x += dirX*velocidad*deltaTime; y += dirY*velocidad*deltaTime; }
}

void Art::agregarWaypoint(float px, float py, float tiempoEspera, const std::string& accion) {
    waypoints.emplace_back(px, py, tiempoEspera, accion);
}

void Art::limpiarWaypoints(){ waypoints.clear(); waypointActual=0; esperando=false; }

bool Art::cargarWaypointsDesdeArchivo(const std::string& rutaArchivo){
    std::ifstream archivo(rutaArchivo);
    if(!archivo.is_open()){ std::cerr<<"Error: no se pudo abrir "+rutaArchivo<<std::endl; return false; }
    limpiarWaypoints(); std::string linea; int num=0;
    while(std::getline(archivo,linea)){
        num++; if(linea.empty()||linea[0]=='#') continue;
        std::istringstream ss(linea);
        float px, py; float wait=0.f; std::string accion="";
        if(!(ss>>px>>py)) { std::cerr<<"Error en linea "<<num<<" formato incorrecto\n"; continue; }
        if(!(ss>>wait)) wait=0.f;
        if(!(ss>>accion)) accion="";
        agregarWaypoint(px,py,wait,accion);
    }
    archivo.close(); std::cout<<"Waypoints cargados: "<<waypoints.size()<<" desde "<<rutaArchivo<<std::endl; return true;
}

void Art::actualizarDireccion(float diffX, float diffY){
    if (std::abs(diffX)>std::abs(diffY)) direccionActual = diffX>0?DERECHA:IZQUIERDA;
    else direccionActual = diffY>0?ABAJO:ARRIBA;
}

void Art::actualizarAnimacion(float deltaTime){
    tiempoAnimacion += deltaTime;
    if (caminando){
        if (tiempoAnimacion >= tiempoPorFrame){
            std::vector<sf::Texture>* texs = nullptr;
            switch(direccionActual){ case ABAJO: texs=&caminarAbajo; break; case ARRIBA: texs=&caminarArriba; break; case IZQUIERDA: texs=&caminarIzquierda; break; case DERECHA: texs=&caminarDerecha; break; }
            if (texs && !texs->empty()){ frameActual = (frameActual+1)%texs->size(); sprite.setTexture((*texs)[frameActual]); }
            tiempoAnimacion = 0.f;
        }
    } else {
        switch(direccionActual){ case ABAJO: if(quietoAbajo.getSize().x>0) sprite.setTexture(quietoAbajo); break; case ARRIBA: if(quietoArriba.getSize().x>0) sprite.setTexture(quietoArriba); break; case IZQUIERDA: if(quietoIzquierda.getSize().x>0) sprite.setTexture(quietoIzquierda); break; case DERECHA: if(quietoDerecha.getSize().x>0) sprite.setTexture(quietoDerecha); break; }
    }
}

float Art::calcularDistancia(float px, float py) const{ float dx=px-x, dy=py-y; return std::sqrt(dx*dx+dy*dy); }
float Art::calcularDistanciaAlPunto(const sf::Vector2f& p) const{ float dx=p.x-x, dy=p.y-y; return std::sqrt(dx*dx+dy*dy); }

void Art::cargarSprites(const std::string& quietoAbajoPath, const std::string& quietoArribaPath,
                        const std::string& quietoIzquierdaPath, const std::string& quietoDerechaPath,
                        const std::vector<std::string>& caminarAbajoPaths,const std::vector<std::string>& caminarArribaPaths,
                        const std::vector<std::string>& caminarIzquierdaPaths,const std::vector<std::string>& caminarDerechaPaths)
{
    quietoAbajo.loadFromFile(quietoAbajoPath);
    quietoArriba.loadFromFile(quietoArribaPath);
    quietoIzquierda.loadFromFile(quietoIzquierdaPath);
    quietoDerecha.loadFromFile(quietoDerechaPath);

    caminarAbajo.clear(); for (auto &p: caminarAbajoPaths){ sf::Texture t; t.loadFromFile(p); caminarAbajo.push_back(t); }
    caminarArriba.clear(); for (auto &p: caminarArribaPaths){ sf::Texture t; t.loadFromFile(p); caminarArriba.push_back(t); }
    caminarIzquierda.clear(); for (auto &p: caminarIzquierdaPaths){ sf::Texture t; t.loadFromFile(p); caminarIzquierda.push_back(t); }
    caminarDerecha.clear(); for (auto &p: caminarDerechaPaths){ sf::Texture t; t.loadFromFile(p); caminarDerecha.push_back(t); }

    if (quietoAbajo.getSize().x>0) sprite.setTexture(quietoAbajo);
    sprite.setOrigin(sprite.getLocalBounds().width/2.f, sprite.getLocalBounds().height/2.f);
}

void Art::setPosicion(float nuevaX, float nuevaY){ x=nuevaX; y=nuevaY; sprite.setPosition(x,y); }
