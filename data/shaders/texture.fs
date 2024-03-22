
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform float u_time;
uniform float u_iRes;
uniform vec3 u_camera_position;

uniform vec3 u_light_color;
uniform vec3 u_light_pos;
uniform float u_light_max_distance;
uniform vec3 u_light_direction;
void main()
{
	vec2 uv = v_uv;
	vec3 N = normalize(v_normal);
	
	vec3 light_color = vec3(1.0, 0.84, 0.61);
	vec3 light = vec3(0.8, 0.6, 0.4); // AMBIENT LIGHT

	// L vector is the same for directional light, no matter the pixel position in world coords
	vec3 L = normalize(vec3(0.0, 500.0, 40));
	//vec3 L = normalize(u_light_direction);
	
	float NdotL = clamp(dot(N,L), 0.0, 1.0);
	light += NdotL * light_color;

	vec4 color = u_color * texture2D( u_texture, uv );
	color.xyz *= light * 1.4;
	
	gl_FragColor = color;
	
		
}









