const char *frag_source = "#version 430 core\n"
"\n"
"#define PI (3.1416)\n"
"#define TAU (6.2832)\n"
"#define EPSILON (.0001)\n"
"#define MAXDIST (100000.)\n"
"\n"
"#define SAMPLES (16)\n"
"#define BOUNCE (8)\n"
"\n"
"\n"
"#define SPH1 vec3(0.,0.,10.)\n"
"#define SPH2 vec3(-.75,-.25,9.)\n"
"#define SPH3 vec3(1.2,-.3,8.)\n"
"#define SPH4 vec3(-.4,-.35,7.)\n"
"#define SPH5 vec3(.8,-.3,12.)\n"
"#define SPH6 vec3(.2,-.2,6.)\n"
"\n"
"\n"
"uniform coherent layout (rgba32f) image2D ichannel;\n"
"uniform vec2 u_res;\n"
"uniform int iframe;\n"
"out vec4 fragcol;\n"
"\n"
"\n"
"float ran1(inout float seed) {\n"
"return fract(sin(seed+=.1)*43758.545);\n"
"}\n"
"\n"
"vec2 ran2(inout float seed) {\n"
"return fract(sin(vec2(seed+=.1,seed+=.1))\n"
"* vec2(43758.545,22578.145));\n"
"}\n"
"\n"
"vec3 ran3(inout float seed) {\n"
"return fract(sin(vec3(seed+=.1,seed+=.1,seed+=.1))\n"
"* vec3(43758.545,22578.145,19642.349));\n"
"}\n"
"\n"
"vec2 ran_circ(inout float seed, float r) {\n"
"vec2 rn = ran2(seed);\n"
"float tT = TAU * rn.y;\n"
"return sqrt(rn.x)*vec2(cos(tT), sin(tT))*r;\n"
"}\n"
"\n"
"vec3 ran_sph(inout float seed) {\n"
"vec3 rn = ran3(seed);\n"
"rn.y *= TAU;\n"
"rn.x = acos(2.*rn.x-1.);\n"
"return pow(rn.z, .333)\n"
"* vec3(sin(rn.y)*sin(rn.x), cos(rn.y)*sin(rn.x), cos(rn.x));\n"
"}\n"
"\n"
"float gauss (float x, float a, float mu, float s1, float s2) {\n"
"float sr = (x-mu)/(x<mu? s1 : s2);\n"
"return a * exp(-(sr*sr)/2.);\n"
"}\n"
"\n"
"vec3 cie(float l) {\n"
"l = l*3000. + 4000.;\n"
"vec3 col = vec3(\n"
"gauss(l, 1.056, 5998., 379., 310.) + gauss(l, 0.362, 4420., 160., 267.)\n"
"+ gauss(l, -0.065, 5011., 204., 262.),\n"
"gauss(l, 0.821, 5688., 469., 405.) + gauss(l, 0.286, 5309., 163., 311.),\n"
"gauss(l, 1.217, 4370., 118., 360.) + gauss(l, 0.681, 4590., 260., 138.)\n"
");\n"
"return col;\n"
"}\n"
"\n"
"float intersect_plane(vec3 ro, vec3 rd, vec3 n, vec3 p) {\n"
"return dot(p-ro,n)/dot(rd,n);\n"
"}\n"
"\n"
"float intersect_sphere(vec3 ro, vec3 rd, vec3 p, float r) {\n"
"vec3 fc = ro-p;\n"
"float b = dot(fc, rd);\n"
"float c = dot(fc,fc) - r*r;\n"
"float h = b*b-c;\n"
"if (h<0.) return -1.;\n"
"return -b-sqrt(h);\n"
"}\n"
"\n"
"vec3 normal(vec3 p, int obj) {\n"
"switch (obj) {\n"
"case 7:\n"
"return normalize(p-SPH6);\n"
"case 6:\n"
"return normalize(p-SPH5);\n"
"case 5:\n"
"return normalize(p-SPH4);\n"
"case 4:\n"
"return normalize(p-SPH3);\n"
"case 3:\n"
"return normalize(p-SPH2);\n"
"case 2:\n"
"return normalize(p-SPH1);\n"
"case 1:\n"
"return vec3(0.,1.,0.);\n"
"case 0:\n"
"default:\n"
"return vec3(0);\n"
"}\n"
"}\n"
"\n"
"vec3 cosreflect(inout float seed, vec3 nor) {\n"
"vec2 rn = ran2(seed);\n"
"float a = TAU * rn.x;\n"
"rn.y = 2.*rn.y - 1.;\n"
"return normalize(nor + vec3(sqrt(1.-rn.y*rn.y) * vec2(cos(a), sin(a)), rn.y));\n"
"}\n"
"\n"
"vec3 conereflect(inout float seed, vec3 nor, vec3 inc, float t) {\n"
"return normalize(reflect(inc,nor) + ran_sph(seed)*t);\n"
"}\n"
"\n"
"vec3 bsdf(inout float seed, float l, float ior, vec3 ss, vec3 mat,\n"
"vec3 nor, vec3 inc) {\n"
"if (ran1(seed) < mat.x) {\n"
"vec3 rd = refract(inc, nor, ior/(ss.x+ss.y*(1-l)));\n"
"if (length(rd) > 0.) return rd;\n"
"else nor = -nor;\n"
"}\n"
"if (ran1(seed) < mat.y) return cosreflect(seed, nor);\n"
"return conereflect(seed, nor, inc, mat.z);\n"
"}\n"
"\n"
"float intersect_scene(vec3 ro, vec3 rd, inout int obj) {\n"
"obj = 0;\n"
"float dist = MAXDIST;\n"
"float d = 0.;\n"
"\n"
"d = intersect_plane(ro, rd, vec3(0.,1.,0.), vec3(0.,-.5,0.));\n"
"if (d>0. && d<dist) {\n"
"dist = d;\n"
"obj = 1;\n"
"if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;\n"
"}\n"
"\n"
"d = intersect_sphere(ro, rd, SPH1, .5);\n"
"if (d>0. && d<dist) {\n"
"dist = d;\n"
"obj = 2;\n"
"if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;\n"
"}\n"
"\n"
"d = intersect_sphere(ro, rd, SPH2, .25);\n"
"if (d>0. && d<dist) {\n"
"dist = d;\n"
"obj = 3;\n"
"if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;\n"
"}\n"
"\n"
"d = intersect_sphere(ro, rd, SPH3, .2);\n"
"if (d>0. && d<dist) {\n"
"dist = d;\n"
"obj = 4;\n"
"if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;\n"
"}\n"
"\n"
"d = intersect_sphere(ro, rd, SPH4, .15);\n"
"if (d>0. && d<dist) {\n"
"dist = d;\n"
"obj = 5;\n"
"if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;\n"
"}\n"
"\n"
"d = intersect_sphere(ro, rd, SPH5, .2);\n"
"if (d>0. && d<dist) {\n"
"dist = d;\n"
"obj = 6;\n"
"if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;\n"
"}\n"
"\n"
"d = intersect_sphere(ro, rd, SPH6, .3);\n"
"if (d>0. && d<dist) {\n"
"dist = d;\n"
"obj = 7;\n"
"if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;\n"
"}\n"
"\n"
"return dist;\n"
"}\n"
"\n"
"vec3 getsurface(int obj) {\n"
"switch (obj) {\n"
"case 6:\n"
"return vec3(.35,.65,1.);\n"
"case 5:\n"
"return vec3(.6,.95,1.);\n"
"case 7:\n"
"return vec3(.75,.75,1.);\n"
"case 4:\n"
"return vec3(.35,.7,.95);\n"
"case 3:\n"
"return vec3(1.,1.,.85);\n"
"case 2:\n"
"return vec3(1.,.1,.1);\n"
"case 1:\n"
"return vec3(.0,.65,.35);\n"
"case 0:\n"
"return vec3(.0,.0,.0);\n"
"default:\n"
"return vec3(1.,0.,1.);\n"
"}\n"
"}\n"
"\n"
"// vec3(transmittance, diffuse, tan(gloss angle))\n"
"vec3 getmaterial(int obj) {\n"
"switch (obj) {\n"
"case 7:\n"
"return vec3(.85,.05,.05);\n"
"case 6:\n"
"return vec3(0.,0.,0.);\n"
"case 5:\n"
"return vec3(.95,.01,.02);\n"
"case 4:\n"
"return vec3(0.,.5,.2);\n"
"case 3:\n"
"return vec3(0.,0.,0.);\n"
"case 2:\n"
"return vec3(0.,.75,.4);\n"
"case 1:\n"
"return vec3(0.,.97,.35);\n"
"case 0:\n"
"return vec3(.95,0.,0.);\n"
"default:\n"
"return vec3(.5);\n"
"}\n"
"}\n"
"\n"
"// vec3(index of refraction, dispersion, scattering coefficient)\n"
"vec3 getssprops(int obj) {\n"
"switch (obj) {\n"
"case 7:\n"
"return vec3(1.45, .5, 0.);\n"
"case 5:\n"
"return vec3(1.31, .1, 0.);\n"
"case 0:\n"
"default:\n"
"return vec3(1.,0.,0.);\n"
"}\n"
"}\n"
"\n"
"bool islight(int obj) {\n"
"return (obj == 3) || (obj == 6); // || (obj == 4);\n"
"}\n"
"\n"
"vec3 getcolour(inout float seed, float l, vec3 fcol, vec3 ro, vec3 rd, int n) {\n"
"int obj = 0;\n"
"float ior = 1.;\n"
"do {\n"
"float d = intersect_scene(ro,rd,obj);\n"
"\n"
"fcol *= getsurface(obj);\n"
"if (islight(obj)) return (fcol);\n"
"vec3 mat = getmaterial(obj);\n"
"vec3 ss = getssprops(obj);\n"
"\n"
"ro = ro + d*rd;\n"
"vec3 nor = normal(ro,obj);\n"
"rd = bsdf(seed,l,ior,ss,mat,nor,rd);\n"
"ro = ro + rd*EPSILON;\n"
"ior = ss.x;\n"
"} while(--n > 0);\n"
"return vec3(0);\n"
"}\n"
"\n"
"vec3 calcsample(inout float seed, vec2 uv) {\n"
"const float sens = .035;\n"
"const float focal_length = .05;\n"
"const float focal_dist = 10.;\n"
"const float f_stop = 2.8;\n"
"const float aperture = focal_length / f_stop;\n"
"const float aperad = aperture / 2.;\n"
"\n"
"const float c_aberration = 1.5;\n"
"\n"
"vec3 princ = normalize(vec3(uv*.5*-sens, -focal_length));\n"
"float d = intersect_plane(vec3(0), princ, vec3(0,0,-1),\n"
"vec3(0,0,focal_dist));\n"
"vec3 focal_point = princ*d;\n"
"\n"
"vec3 col = vec3(0);\n"
"int i = SAMPLES;\n"
"float ml = 0.;\n"
"do {\n"
"float l = ran1(seed);\n"
"vec2 r_c = ran_circ(seed,aperad);\n"
"vec3 sc = cie(l);\n"
"vec3 lens_pos = vec3(r_c,0.);\n"
"vec3 lens_ray = normalize(\n"
"focal_point\n"
"* (vec3(1)+vec3(l*length(r_c)*c_aberration)\n"
"* vec3(focal_length,focal_length,-.5))\n"
"-lens_pos\n"
");\n"
"col += getcolour(seed, l, sc, lens_pos, lens_ray, BOUNCE);\n"
"ml += (sc.x + sc.y + sc.z)/3.;\n"
"} while(--i > 0);\n"
"return col/ml;\n"
"}\n"
"\n"
"void main() {\n"
"vec4 inp = imageLoad(ichannel, ivec2(gl_FragCoord.xy));\n"
"if (iframe < 1024) {\n"
"vec2 uv = (gl_FragCoord.xy-.5*u_res)/u_res.y;\n"
"\n"
"inp.xyz += calcsample(inp.w, uv);\n"
"\n"
"vec3 col = inp.xyz/float(iframe);\n"
"fragcol = vec4(pow(col,vec3(.45)),1.);\n"
"\n"
"imageStore(ichannel, ivec2(gl_FragCoord.xy), inp);\n"
"} else {\n"
"inp.xyz = pow(inp.xyz/float(1024),vec3(.45));\n"
"fragcol = vec4(inp.xyz,1.);\n"
"}\n"
"}\0\n"
;