#version 430 core

#define PI (3.1416)
#define TAU (6.2832)
#define EPSILON (.0001)
#define MAXDIST (100000.)

#define SAMPLES (16)
#define BOUNCE (8)


#define SPH1 vec3(0.,0.,10.)
#define SPH2 vec3(-.75,-.25,9.)
#define SPH3 vec3(1.2,-.3,8.)
#define SPH4 vec3(-.4,-.35,7.)
#define SPH5 vec3(.8,-.3,12.)
#define SPH6 vec3(.2,-.2,6.)


uniform coherent layout (rgba32f) image2D ichannel;
uniform vec2 u_res;
uniform int iframe;
out vec4 fragcol;


float ran1(inout float seed) {
    return fract(sin(seed+=.1)*43758.545);
}

vec2 ran2(inout float seed) {
    return fract(sin(vec2(seed+=.1,seed+=.1))
        * vec2(43758.545,22578.145));
}

vec3 ran3(inout float seed) {
    return fract(sin(vec3(seed+=.1,seed+=.1,seed+=.1))
        * vec3(43758.545,22578.145,19642.349));
}

vec2 ran_circ(inout float seed, float r) {
    vec2 rn = ran2(seed);
    float tT = TAU * rn.y;
    return sqrt(rn.x)*vec2(cos(tT), sin(tT))*r;
}

vec3 ran_sph(inout float seed) {
    vec3 rn = ran3(seed);
    rn.y *= TAU;
    rn.x = acos(2.*rn.x-1.);
    return pow(rn.z, .333)
        * vec3(sin(rn.y)*sin(rn.x), cos(rn.y)*sin(rn.x), cos(rn.x));
}

float gauss (float x, float a, float mu, float s1, float s2) {
    float sr = (x-mu)/(x<mu? s1 : s2);
    return a * exp(-(sr*sr)/2.);
}

vec3 cie(float l) {
    l = l*3000. + 4000.;
    vec3 col = vec3(
        gauss(l, 1.056, 5998., 379., 310.) + gauss(l, 0.362, 4420., 160., 267.)
            + gauss(l, -0.065, 5011., 204., 262.),
        gauss(l, 0.821, 5688., 469., 405.) + gauss(l, 0.286, 5309., 163., 311.),
        gauss(l, 1.217, 4370., 118., 360.) + gauss(l, 0.681, 4590., 260., 138.)
    );
    return col;
}

float intersect_plane(vec3 ro, vec3 rd, vec3 n, vec3 p) {
	return dot(p-ro,n)/dot(rd,n);
}

float intersect_sphere(vec3 ro, vec3 rd, vec3 p, float r) {
	vec3 fc = ro-p;
    float b = dot(fc, rd);
    float c = dot(fc,fc) - r*r;
    float h = b*b-c;
    if (h<0.) return -1.;
    return -b-sqrt(h);
}

vec3 normal(vec3 p, int obj) {
    switch (obj) {
        case 7:
            return normalize(p-SPH6);
        case 6:
            return normalize(p-SPH5);
        case 5:
            return normalize(p-SPH4);
        case 4:
            return normalize(p-SPH3);
        case 3:
            return normalize(p-SPH2);
        case 2:
            return normalize(p-SPH1);
        case 1:
            return vec3(0.,1.,0.);
        case 0:
        default:
            return vec3(0);
    }
}

vec3 cosreflect(inout float seed, vec3 nor) {
    vec2 rn = ran2(seed);
    float a = TAU * rn.x;
    rn.y = 2.*rn.y - 1.;
    return normalize(nor + vec3(sqrt(1.-rn.y*rn.y) * vec2(cos(a), sin(a)), rn.y));
}

vec3 conereflect(inout float seed, vec3 nor, vec3 inc, float t) {
    return normalize(reflect(inc,nor) + ran_sph(seed)*t);
}

vec3 bsdf(inout float seed, float l, float ior, vec3 ss, vec3 mat,
        vec3 nor, vec3 inc) {
    if (ran1(seed) < mat.x) {
        vec3 rd = refract(inc, nor, ior/(ss.x+ss.y*(1-l)));
        if (length(rd) > 0.) return rd;
        else nor = -nor;
    }
    if (ran1(seed) < mat.y) return cosreflect(seed, nor);
    return conereflect(seed, nor, inc, mat.z);
}

float intersect_scene(vec3 ro, vec3 rd, inout int obj) {
    obj = 0;
    float dist = MAXDIST;
    float d = 0.;

    d = intersect_plane(ro, rd, vec3(0.,1.,0.), vec3(0.,-.5,0.));
    if (d>0. && d<dist) {
        dist = d;
        obj = 1;
        if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;
    }

    d = intersect_sphere(ro, rd, SPH1, .5);
    if (d>0. && d<dist) {
        dist = d;
        obj = 2;
        if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;
    }

    d = intersect_sphere(ro, rd, SPH2, .25);
    if (d>0. && d<dist) {
        dist = d;
        obj = 3;
        if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;
    }

    d = intersect_sphere(ro, rd, SPH3, .2);
    if (d>0. && d<dist) {
        dist = d;
        obj = 4;
        if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;
    }

    d = intersect_sphere(ro, rd, SPH4, .15);
    if (d>0. && d<dist) {
        dist = d;
        obj = 5;
        if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;
    }

    d = intersect_sphere(ro, rd, SPH5, .2);
    if (d>0. && d<dist) {
        dist = d;
        obj = 6;
        if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;
    }

    d = intersect_sphere(ro, rd, SPH6, .3);
    if (d>0. && d<dist) {
        dist = d;
        obj = 7;
        if (dot(normal(vec3(d-EPSILON)*rd + ro, obj), rd) > 0.) obj = 0;
    }

    return dist;
}

vec3 getsurface(int obj) {
    switch (obj) {
        case 6:
            return vec3(.35,.65,1.);
        case 5:
            return vec3(.6,.95,1.);
        case 7:
            return vec3(.75,.75,1.);
        case 4:
            return vec3(.35,.7,.95);
        case 3:
            return vec3(1.,1.,.85);
        case 2:
            return vec3(1.,.1,.1);
        case 1:
            return vec3(.0,.65,.35);
        case 0:
            return vec3(.0,.0,.0);
        default:
            return vec3(1.,0.,1.);
    }
}

// vec3(transmittance, diffuse, tan(gloss angle))
vec3 getmaterial(int obj) {
    switch (obj) {
        case 7:
            return vec3(.85,.05,.05);
        case 6:
            return vec3(0.,0.,0.);
        case 5:
            return vec3(.95,.01,.02);
        case 4:
            return vec3(0.,.5,.2);
        case 3:
            return vec3(0.,0.,0.);
        case 2:
            return vec3(0.,.75,.4);
        case 1:
            return vec3(0.,.97,.35);
        case 0:
            return vec3(.95,0.,0.);
        default:
            return vec3(.5);
    }
}

// vec3(index of refraction, dispersion, scattering coefficient)
vec3 getssprops(int obj) {
    switch (obj) {
        case 7:
            return vec3(1.45, .5, 0.);
        case 5:
            return vec3(1.31, .1, 0.);
        case 0:
        default:
            return vec3(1.,0.,0.);
    }
}

bool islight(int obj) {
    return (obj == 3) || (obj == 6); // || (obj == 4);
}

vec3 getcolour(inout float seed, float l, vec3 fcol, vec3 ro, vec3 rd, int n) {
    int obj = 0;
    float ior = 1.;
    do {
        float d = intersect_scene(ro,rd,obj);

        fcol *= getsurface(obj);
        if (islight(obj)) return (fcol);
        vec3 mat = getmaterial(obj);
        vec3 ss = getssprops(obj);

        ro = ro + d*rd;
        vec3 nor = normal(ro,obj);
        rd = bsdf(seed,l,ior,ss,mat,nor,rd);
        ro = ro + rd*EPSILON;
        ior = ss.x;
    } while(--n > 0);
    return vec3(0);
}

vec3 calcsample(inout float seed, vec2 uv) {
    const float sens = .035;
    const float focal_length = .05;
    const float focal_dist = 10.;
    const float f_stop = 2.8;
    const float aperture = focal_length / f_stop;
    const float aperad = aperture / 2.;

    const float c_aberration = 1.5;

    vec3 princ = normalize(vec3(uv*.5*-sens, -focal_length));
    float d = intersect_plane(vec3(0), princ, vec3(0,0,-1),
        vec3(0,0,focal_dist));
    vec3 focal_point = princ*d;

    vec3 col = vec3(0);
    int i = SAMPLES;
    float ml = 0.;
    do {
        float l = ran1(seed);
        vec2 r_c = ran_circ(seed,aperad);
        vec3 sc = cie(l);
        vec3 lens_pos = vec3(r_c,0.);
        vec3 lens_ray = normalize(
            focal_point
            * (vec3(1)+vec3(l*length(r_c)*c_aberration)
                * vec3(focal_length,focal_length,-.5))
            -lens_pos
        );
        col += getcolour(seed, l, sc, lens_pos, lens_ray, BOUNCE);
        ml += (sc.x + sc.y + sc.z)/3.;
    } while(--i > 0);
    return col/ml;
}

void main() {
    vec4 inp = imageLoad(ichannel, ivec2(gl_FragCoord.xy));
    if (iframe < 1024) {
        vec2 uv = (gl_FragCoord.xy-.5*u_res)/u_res.y;

        inp.xyz += calcsample(inp.w, uv);

        vec3 col = inp.xyz/float(iframe);
        fragcol = vec4(pow(col,vec3(.45)),1.);

        imageStore(ichannel, ivec2(gl_FragCoord.xy), inp);
    } else {
        inp.xyz = pow(inp.xyz/float(1024),vec3(.45));
        fragcol = vec4(inp.xyz,1.);
    }
}
