uniform vec2 lightPos;
uniform vec2 lightDir;
uniform float lightRadius;
uniform float innerRadius;
uniform vec2 resolution;

void main()
{
    vec2 uv = gl_FragCoord.xy;

    float dist = distance(uv, lightPos);

    // zona donde NO hay sombra (tolerancia)
    if (dist < innerRadius) {
        gl_FragColor = vec4(1.0);
        return;
    }

    // si está fuera del rango de luz, queda muy oscuro
    if (dist > lightRadius) {
        gl_FragColor = vec4(0.0);
        return;
    }

    // dirección del fragmento respecto a la luz
    vec2 toPixel = normalize(uv - lightPos);

    float angle = dot(toPixel, normalize(lightDir));

    // permite el cono de luz
    float brightness = smoothstep(0.4, 1.0, angle);

    // luz que baja con la distancia
    float falloff = 1.0 - (dist / lightRadius);

    gl_FragColor = vec4(vec3(brightness * falloff), 1.0);
}
