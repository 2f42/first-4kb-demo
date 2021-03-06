#ifndef __SHADER_H
#define __SHADER_H

const char *vertex_source = "#version 430 core\n"
    "layout(location=0) in vec3 p;"
    "out vec2 texcoords;"
    "void main(){"
        "gl_Position=vec4(p,1.);"
        "texcoords=(p.xy+1.)*.5;"
    "}\0";

#include "frag_.h"
/*const char *frag_source = "#version 430 core\n"
"#define PI (3.142)\n"
"#define TAU (6.283)\n"
"#define EPSILON (.001)\n"
"#define MAXDIST (100000.)\n"
"#define SAMPLES (16)\n"
"#define BOUNCE (4)\n"
"#define SPH1 vec3(0.,-.5,8.)\n"
"#define SPH2 vec3(-.75,-.75,7.)\n"
"#define SPH3 vec3(1.2,-.8,8.)\n"
"in vec2 texcoords;"
"uniform coherent layout (rgba32f) image2D ichannel;"
"uniform vec2 u_res;"
"uniform int iframe;"
"out vec4 fragcol;"
"float ran1(inout float seed) {"
"return fract(sin(seed+=.1)*36973.45482);"
"}"
"vec2 ran2(inout float seed) {"
"return vec2(ran1(), ran1());"
"}"
"float ran_circ(inout float seed, float r) {"
"vec2 rn = ran2(seed);"
"float sr = sqrt(rn.x) * r;"
"float tT = TAU * rn.y;"
"return vec2(cos(tT), sin(tT))*sr;"
"}"
"float intersect_plane(vec3 ro, vec3 rd, vec3 n, vec3 p) {"
"return dot(p-ro,n)/dot(rd,n);"
"}"
"float intersect_sphere(vec3 ro, vec3 rd, vec3 p, float r) {"
"vec3 fc = ro-p;"
"float b = dot(fc, rd);"
"float c = dot(fc,fc) - r*r;"
"float h = b*b-c;"
"if (h<0.) return -1.;"
"return -b-sqrt(h);"
"}"
"vec2 intersect_scene(vec3 ro, vec3 rd, inout int obj) {"
"obj = 0;"
"float dist = MAXDIST;"
"float d = 0.;"
"d = intersect_plane(ro, rd, vec3(0.,1.,0.), vec3(0.,-1.0.));"
"if (d>0. && d<dist) {"
"dist = d;"
"obj = 1;"
"}"
"d = intersect_sphere(ro, rd, SPH1, .5);"
"if (d>0. && d<dist) {"
"dist = d;"
"obj = 2;"
"}"
"d = intersect_sphere(ro, rd, SPH2, .25);"
"if (d>0. && d<dist) {"
"dist = d;"
"obj = 3;"
"}"
"d = intersect_sphere(ro, rd, SPH3, .2);"
"if (d>0. && d<dist) {"
"dist = d;"
"obj = 4;"
"}"
"return dist;"
"}"
"vec3 normal(vec3 p, int obj) {"
"switch (obj) {"
"case 4:"
"return normalize(p-SPH3);"
"case 3:"
"return normalize(p-SPH2);"
"case 2:"
"return normalize(p-SPH1);"
"case 1:"
"return vec3(0.,1.,0.);"
"case 0:"
"default:"
"return vec3(0);"
"}"
"}"
"vec3 cosreflect(inout float seed, vec3 nor) {"
"vec2 rn = ran2(seed);"
"float a = TAU * rn.x;"
"rn.y = 2.*rn.y - 1.;"
"return normalize(nor + vec3(sqrt(1.-rn.y*rn.y) * vec2(cos(a), sin(a)), u));"
"}"
"vec3 bdrf(inout seed, vec3 nor, vec3 inc) {"
"if (ran1() < .95) return cosreflect(seed, nor);"
"return normalize(reflect(inc, nor));"
"}"
"vec3 getsurface(int obj) {"
"switch (obj) {"
"case 4:"
"return vec3(.35,.7,.95);"
"case 3:"
"return vec3(1.,1.,.85);"
"case 2:"
"return vec3(.75,.1,.1);"
"case 1:"
"return vec3(.0,.65,.35);"
"case 0:"
"return vec3(.0,.0,.0);"
"default:"
"return vec3(1.,0.,1.);"
"}"
"}"
"int islight(int obj) {"
"return (obj == 3) || (obj == 4);"
"}"
"vec3 getcolour(inout seed, vec3 ro, vec3 rd, int n) {"
"vec3 fcol = vec3(0);"
"do {"
"int obj = 0;"
"float d = intersect_scene(ro,rd,obj);"
"vec3 col = get_surface(obj);"
"if (islight(obj)) {"
"return (fcol * col);"
"}"
"fcol += col;"
"ro = ro + d*rd;"
"vec3 nor = normal(ro);"
"rd = bdrf(seed,nor,rd);"
"ro = ro + rd*EPSILON;"
"} while(--n);"
"return vec3(0);"
"}"
"vec3 sample(inout seed, vec2 uv) {"
"const float sens = .035;"
"const float focal_length = .08;"
"const float focal_dist = 7.8;"
"const float f_stop = .2;"
"const float aperture = focal_length / f_stop;"
"const float aperad = aperture / 2.;"
"vec3 princ = normalize(vec3(uv*-sens, -focal_length));"
"float d = intersect_plane(vec3(0), princ, vec3(0,0,-1),"
"vec3(0,0,focal_dist));"
"vec3 focal_point = princ*d;"
"vec3 col = vec3(0);"
"int i = SAMPLES;"
"do {"
"vec2 r_c = ran_circ(seed, aperad);"
"vec3 lens_pos = vec3(r_c,0.);"
"vec3 lens_ray = normalize(focal_point-lens_pos);"
"col += getcolour(seed, lens_pos, lens_ray, BOUNCE);"
"} while(--i);"
"return col/float(SAMPLES);"
"}"
"void main() {"
"if (iframe < 4096) {"
"float seed = imageLoad(ichannel, ivec2(gl_FragCoord.xy)).w;"
"vec2 uv = (gl_FragCoord.xy-.5*u_res)/u_res.y;"
"vec3 col = vec3(0);"
"col += sample(seed, uv);"
"fragCol = vec4(col/float(iframe),1.);"
"imageStore(ichannel, ivec2(gl_FragCoord.xy), vec4(col,seed));"
"} else {"
"fragCol = imageLoad(ichannel, ivec2(gl_FragCoord.xy));"
"}"
"}\0";*/

#endif
