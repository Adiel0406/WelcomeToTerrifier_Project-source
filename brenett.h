#include <SFML/Graphics.hpp>
#include <cmath>
//using namespace sf;

// Calcular Colisiones de mapa
float close(float val, float min, float max){
    if (val < min) return min;
    if (val > max) return max;
    else return val;
};

int checker(sf::RectangleShape &R1, sf::CircleShape &C1){
    float closeX= close(C1.getPosition().x,R1.getPosition().x,R1.getPosition().x+R1.getSize().x);
    float closeY= close(C1.getPosition().y,R1.getPosition().y,R1.getPosition().y+R1.getSize().y);

    float distanceX = C1.getPosition().x - closeX;
    float distanceY = C1.getPosition().y - closeY;

    float dist = (distanceX * distanceX) + (distanceY * distanceY);
    if (dist<C1.getRadius()*C1.getRadius() && closeX!=C1.getPosition().x && closeY!=C1.getPosition().y){
        return 3;
    }
    else if (dist<C1.getRadius()*C1.getRadius() && closeX==C1.getPosition().x){
        return 2;
    }
    else if (dist<C1.getRadius()*C1.getRadius() && closeY==C1.getPosition().y){
        return 1;
    }
    else return 0;
}

//Calcular límites de cámara
void limitCam(sf::View &view, sf::Sprite &player, sf::FloatRect &mapBounds) {
    sf::Vector2f viewSize = view.getSize();
    sf::Vector2f centerCam = player.getPosition();
    
    // Calcular los limites de la cámara
    float halfWidth = viewSize.x / 2.0f;
    float halfHeight = viewSize.y / 2.0f;
    
    // Limite izquierdo
    if (centerCam.x - halfWidth < mapBounds.left) {
        centerCam.x = mapBounds.left + halfWidth;
    }
    // Limite derecho
    else if (centerCam.x + halfWidth > mapBounds.left + mapBounds.width) {
        centerCam.x = mapBounds.left + mapBounds.width - halfWidth;
    }
    
    // Limite superior
    if (centerCam.y - halfHeight < mapBounds.top) {
        centerCam.y = mapBounds.top + halfHeight;
    }
    // Limite inferior
    else if (centerCam.y + halfHeight > mapBounds.top + mapBounds.height) {
        centerCam.y = mapBounds.top + mapBounds.height - halfHeight;
    }
    
    // Centrar Camara
    if (mapBounds.width <= viewSize.x) {
        centerCam.x = mapBounds.left + (mapBounds.width / 2.0f);
    }
    if (mapBounds.height <= viewSize.y) {
        centerCam.y = mapBounds.top + (mapBounds.height / 2.0f);
    }
    
    // Aplicar la posición limitadaL.setCenter(centerCam);
}