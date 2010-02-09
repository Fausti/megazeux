uniform sampler2D baseMap;

varying vec2 Texcoord;
#define XS 1024.0
#define YS 512.0
#define AX 0.5/XS
#define AY 0.5/YS
void main( void )
{
    gl_FragColor = texture2D( baseMap, vec2(floor(Texcoord.x*XS)/XS+AX, floor(Texcoord.y*YS)/YS+AY));
}
