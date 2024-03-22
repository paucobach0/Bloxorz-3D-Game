
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_sky_texture;
uniform float u_time;
uniform float u_tex_tiling;
uniform vec3 u_cam_pos;
uniform sampler2D u_normal_map;


void main()
{
	vec2 uv = v_world_position.xz * 0.05;
	
	uv.x += 6.0 * sin(u_time * 0.03);
	
	vec2 uv2 = v_world_position.xz * 0.13;
	uv2.y += 2.3 * sin(u_time * 0.13);

	
	vec3 N1 = normalize(texture2D(u_normal_map, uv).xzy * 2.0 - vec3(1));
	vec3 N2 = normalize(texture2D(u_normal_map, uv2).xzy * 2.0 - vec3(1));

	vec3 N = mix(N1, N2, 0.5);
	
	vec3 distVec = u_cam_pos - v_world_position;
	float distToCam = length(distVec);
	vec3 E = -distVec/distToCam; 
	
	
	vec3 R = reflect(E,N);	

	float yaw = atan(R.x,R.z) / 6.28318531;
	yaw += -0.25; //in case the 0deg doesn’t match our Z axis, we can rotate 90 degrees

	float pitch = asin(R.y) / 1.57079633; //0 means bottom, 1 means top

	vec2 uv_reflection = vec2(yaw, clamp(pitch, 0.0, 1.0) );

	vec4 sky_color = texture2DLod( u_sky_texture, uv_reflection, distToCam * 0.003);
	
	sky_color.a = .9;
	
	gl_FragColor = sky_color;
}