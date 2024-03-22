attribute vec3 a_vertex;
attribute vec2 a_uv;


void main()
{	
	v_uv = a_uv;
	gl_Position = vec4( a_vertex, 1.0 );
}