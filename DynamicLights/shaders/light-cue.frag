uniform lowp sampler2D source;
// TODO: add reactive halo
// uniform vec4 bgColor;
// uniform vec4 haloColor;
// uniform float luminosity;

varying highp vec2 qt_TexCoord0;

void main(void)
{
    gl_FragColor = texture2D(source, qt_TexCoord0.st);
}