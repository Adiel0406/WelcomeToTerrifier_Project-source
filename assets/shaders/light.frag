uniform vec2 lightPos;
uniform vec2 lightDir;
uniform float lightRadius;
uniform float innerRadius;
uniform vec2 resolution;

void main()
{
    // Convertir coordenadas de píxel a coordenadas del mundo
    // gl_FragCoord son coordenadas de ventana (0,0 a resolución)
    // Necesitamos convertirlas a coordenadas del mundo donde está el jugador
    vec2 worldPos = vec2(gl_FragCoord.x, gl_FragCoord.y);
    
    float dist = distance(worldPos, lightPos);

    // Zona donde NO hay sombra (tolerancia alrededor del jugador)
    if (dist < innerRadius) {
        gl_FragColor = vec4(0.0); // Transparente completamente
        return;
    }

    // Si está fuera del rango de luz, queda oscuro
    if (dist > lightRadius) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Negro opaco
        return;
    }

    // Dirección del fragmento respecto a la luz
    vec2 toPixel = worldPos - lightPos;
    vec2 toPixelNorm = normalize(toPixel);

    // Asegurar que lightDir esté normalizado (ya lo haces en C++)
    vec2 lightDirNorm = normalize(lightDir);

    // Ángulo entre la dirección de la linterna y la dirección al píxel
    float angle = dot(toPixelNorm, lightDirNorm);

    // Cono de luz - ajusta estos valores para cambiar el ángulo
    float coneAngle = 0.6; // Más pequeño = cono más estrecho
    float brightness = smoothstep(-0.2, coneAngle, angle);

    // Atenuación por distancia
    float falloff = 1.0 - smoothstep(innerRadius, lightRadius, dist);

    // Color de la luz (puedes cambiarlo)
    vec3 lightColor = vec3(0.9, 0.8, 0.6); // Amarillo cálido

    // Combinar todo
    float finalBrightness = brightness * falloff;
    gl_FragColor = vec4(lightColor * finalBrightness, finalBrightness);
}