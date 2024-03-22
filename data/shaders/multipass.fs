varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec3 u_color;
uniform float u_time;
uniform float u_iRes;
uniform vec3 u_light_direction;
uniform vec3 u_ambient_light;
uniform vec3 u_light_color;
uniform vec3 u_light_pos;
uniform float u_light_max_distance;

uniform float u_light_cutoff;
uniform float u_light_exp;

uniform int u_light_type;


void main()
{
	vec3 N = normalize(v_normal);

	vec2 uv = v_uv;
	vec4 color = vec4(u_color, 1.0);
		
	vec2 uv_screen = gl_FragCoord.xy * u_iRes;
	uv_screen.x = 1.0 - uv_screen.x;


	vec3 light_color = vec3(0.5, 0.4, 0.7);
	vec3 light;

	
	// L vector is the same for directional light, no matter the pixel position in world coords
	vec3 L = normalize(vec3(-50.0 + 0.5*sin(u_time)*100.0, 100.0, 0.0));
	float NdotL = clamp(dot(N,L), 0.0, 1.0);
	light += NdotL * light_color ;


	color.xyz *= light;
	
	gl_FragColor = color;
}