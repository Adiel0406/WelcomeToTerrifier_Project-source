uniform vec2 lightPos;        // posición de la luz en SCREEN coords
uniform vec2 lightDir;        // dirección normalizada
uniform float lightRadius;
uniform float innerRadius;
uniform float time;           // <-- NUEVO para flickering
uniform vec2 resolution;

// ---- Ruido simple (sin texturas) ----
float noise(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

// ---- Suaviza bordes ----
float smoothCone(float x, float minAngle, float maxAngle)
{
    return smoothstep(minAngle, maxAngle, x);
}

void main()
{
    vec2 frag = gl_FragCoord.xy;

    // Distancia del fragmento a la luz
    float dist = distance(frag, lightPos);
    if (dist > lightRadius)
    {
        gl_FragColor = vec4(0,0,0,1);
        return;
    }

    // Dirección hacia el fragmento
    vec2 toPixel = normalize(frag - lightPos);

    // Ángulo respecto a la dirección de la linterna
    float angle = dot(toPixel, normalize(lightDir));

    // CONO REALISTA (más ancho y con penumbra)
    float innerCone = 1.5;  // centro brillante
    float outerCone = 0.55;  // penumbra suave
    float cone = smoothCone(angle, outerCone, innerCone);

    // Atenuación por distancia (realista)
    float falloff = pow(1.0 - (dist / lightRadius), 3.0);

    // Halo cercano al jugador
    float halo = smoothstep(innerRadius, innerRadius * 1.6, dist);

    // Flickering con ruido suave
    float flicker = 0.93 + 0.07 * noise(vec2(time * 5.0, dist * 0.1));

    float intensity = 1.9 * cone * falloff * flicker;

    // Color de la luz: cálido suave
    vec3 lightColor = vec3(1.0, 0.92, 0.75);

    // Mezclamos halo
    float finalAlpha = clamp(intensity + (halo * 0.12), 0.0, 1.0);

    // shader ilumina reduciendo oscuridad → alpha inverso
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.7 - finalAlpha);
}
