varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform float u_time;

void main()
{
	vec2 uv = v_world_position.xz * 0.005;
	uv.x += 20.0 * sin(u_time * 0.03);
	
	vec2 uv2 = v_world_position.xz * 0.0013;
	uv2.y += 2.0 * sin(u_time * 0.13);
	
	vec4 color1 = u_color * texture2D( u_texture, uv );
	vec4 color2 = u_color * texture2D( u_texture, uv2 );
	
	color2.z = color2.y * atan(abs(sin(u_time)));
	
	gl_FragColor = mix(color1, color2, 0.9);
	
	//gl_FragColor = u_color * texture2D( u_texture, uv );
}