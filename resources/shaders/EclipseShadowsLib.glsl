#ifndef CS_ECLIPSE_SHADOWS_GLSL
#define CS_ECLIPSE_SHADOWS_GLSL

// For texture mode
uniform sampler2D uShadowTextures[16];
uniform float uShadowLength[16];
uniform vec3 uBodyShadowNormals[16];

// For all other modes
uniform vec4 uOccludingBodies[16];
uniform vec3 uSunPosition;
uniform float uSunRadius;

// For all modes
uniform int uNumOccludingBodies;
uniform int uEclipseCalcType;

const double PI_D = 3.14159265358979323846LF;
const double TWO_PI_D = 2.0LF * PI_D;
const float PI_F = 3.141592654;
const float TWO_PI_F = 2.0 * PI_F;

// atan2 approximation for doubles for GLSL
// using http://lolengine.net/wiki/doc/maths/remez
double atan2d(double y, double x) {
    const double atan_tbl[] = {
    -3.333333333333333333333333333303396520128e-1LF,
    1.999999117496509842004185053319506031014e-1LF,
    -1.428514132711481940637283859690014415584e-1LF,
    1.110012236849539584126568416131750076191e-1LF,
    -8.993611617787817334566922323958104463948e-2LF,
    7.212338962134411520637759523226823838487e-2LF,
    -5.205055255952184339031830383744136009889e-2LF,
    2.938542391751121307313459297120064977888e-2LF,
    -1.079891788348568421355096111489189625479e-2LF,
    1.858552116405489677124095112269935093498e-3LF
    };

    /* argument reduction:
       arctan (-x) = -arctan(x);
       arctan (1/x) = 1/2 * pi - arctan (x), when x > 0
    */
    double ax = abs(x);
    double ay = abs(y);
    double t0 = max(ax, ay);
    double t1 = min(ax, ay);

    double a = 1 / t0;
    a *= t1;

    double s = a * a;
    double p = atan_tbl[9];

    p = fma(fma(fma(fma(fma(fma(fma(fma(fma(fma(p, s,
    atan_tbl[8]), s,
    atan_tbl[7]), s,
    atan_tbl[6]), s,
    atan_tbl[5]), s,
    atan_tbl[4]), s,
    atan_tbl[3]), s,
    atan_tbl[2]), s,
    atan_tbl[1]), s,
    atan_tbl[0]), s * a, a);

    double r = ay > ax ? (1.57079632679489661923LF - p) : p;

    r = x < 0 ?  3.14159265358979323846LF - r : r;
    r = y < 0 ? -r : r;

    return r;
}

double sind(double x) {
    //minimax coefs for sin for 0..pi/2 range
    const double a3 = -1.666666660646699151540776973346659104119e-1LF;
    const double a5 =  8.333330495671426021718370503012583606364e-3LF;
    const double a7 = -1.984080403919620610590106573736892971297e-4LF;
    const double a9 =  2.752261885409148183683678902130857814965e-6LF;
    const double ab = -2.384669400943475552559273983214582409441e-8LF;

    const double m_2_pi = 0.636619772367581343076LF;
    const double m_pi_2 = 1.57079632679489661923LF;

    double y = abs(x * m_2_pi);
    double q = floor(y);
    int quadrant = int(q);

    double t = (quadrant & 1) != 0 ? 1 - y + q : y - q;
    t *= m_pi_2;

    double t2 = t * t;
    double r = fma(fma(fma(fma(fma(ab, t2, a9), t2, a7), t2, a5), t2, a3), t2 * t, t);

    r = x < 0 ? -r : r;

    return (quadrant & 2) != 0 ? -r : r;
}

//cos approximation, error < 5e-11
double cosd(double x) {
    //sin(x + PI/2) = cos(x)
    return sind(x + 1.57079632679489661923LF);
}

/* compute arcsin (a) for a in [-9/16, 9/16] */
double asin_core(double a) {
    double s = a * a;
    double q = s * s;
    double r =      5.5579749017470502e-2LF;
    double t =     -6.2027913464120114e-2LF;
    r = fma (r, q, 5.4224464349245036e-2LF);
    t = fma (t, q, -1.1326992890324464e-2LF);
    r = fma (r, q, 1.5268872539397656e-2LF);
    t = fma (t, q, 1.0493798473372081e-2LF);
    r = fma (r, q, 1.4106045900607047e-2LF);
    t = fma (t, q, 1.7339776384962050e-2LF);
    r = fma (r, q, 2.2372961589651054e-2LF);
    t = fma (t, q, 3.0381912707941005e-2LF);
    r = fma (r, q, 4.4642857881094775e-2LF);
    t = fma (t, q, 7.4999999991367292e-2LF);
    r = fma (r, s, t);
    r = fma (r, s, 1.6666666666670193e-1LF);
    t = a * s;
    r = fma (r, t, a);

    return r;
}

/* Compute arccosine (a), maximum error observed: 1.4316 ulp
   Double-precision factorization of π courtesy of Tor Myklebust
*/
double acosd(double a) {
    double r = (a > 0.0LF) ? -a : a;// avoid modifying the "sign" of NaNs
    if (r > -0.5625LF) {
        /* arccos(x) = pi/2 - arcsin(x) */
        r = fma (9.3282184640716537e-1LF, 1.6839188885261840e+0LF, asin_core(r));
    } else {
        /* arccos(x) = 2 * arcsin (sqrt ((1-x) / 2)) */
        r = 2.0LF * asin_core(sqrt(fma(0.5LF, r, 0.5LF)));
    }
    if (!(a > 0.0LF) && (a >= -1.0LF)) { // avoid modifying the "sign" of NaNs
        /* arccos (-x) = pi - arccos(x) */
        r = fma (1.8656436928143307e+0LF, 1.6839188885261840e+0LF, -r);
    }
    return r;
}

double asind(double a) {
    return (PI_D / 2.0LF) - acosd(a);
}

float areaOfSphericalCapIntersectionApprox(float radiusA, float radiusB, float centerDistance) {
    // No intersection
    if (centerDistance >= radiusA + radiusB) {
        return 0.0;
    }

    // One circle fully in the other
    if (min(radiusA, radiusB) <= max(radiusA, radiusB) - centerDistance) {
        return TWO_PI_F - (TWO_PI_F * cos(min(radiusA, radiusB)));
    }

    const float diff = abs(radiusA - radiusB);
    const float interp = smoothstep(0.0, 1.0, 1.0 - clamp((centerDistance-diff) / (radiusA + radiusB - diff), 0.0, 1.0));
    const float area = interp * (TWO_PI_F - TWO_PI_F * cos(min(radiusA, radiusB)));
    return area;
}

double angularRadOfSphereD(double distance, double radius) {
    return asind(radius / distance);
}

float angularRadOfSphere(float distance, float radius) {
    return asin(radius / distance);
}

double angleD(dvec3 v1, dvec3 v2) {
    double a = length(v1);
    double b = length(v2);
    double c = length(v1 - v2);

    double mu;
    if (b >= c) {
        mu = c - (a - b);
    } else {
        mu = b - (a - c);
    }

    double top = ((a - b) + c) * mu;
    double bot = (a + (b + c)) * ((a - c) + b);

    return 2.0LF * atan2d(sqrt(top), sqrt(bot));
}

float angle(vec3 v1, vec3 v2) {
    float a = length(v1);
    float b = length(v2);
    float c = length(v1 - v2);

    float mu;
    if (b >= c) {
        mu = c - (a - b);
    } else {
        mu = b - (a - c);
    }

    float top = ((a - b) + c) * mu;
    float bot = (a + (b + c)) * ((a - c) + b);

    return 2.0 * atan(sqrt(top), sqrt(bot));
}

float calcEclipseApprox(vec4 occludingBody, vec3 fragmentPosition) {
    vec3 sunLocationRelative = uSunPosition - fragmentPosition;
    vec3 sunLocationNormalized = normalize(sunLocationRelative);
    float sunAngularRadius = angularRadOfSphere(length(sunLocationRelative), uSunRadius);
    float sunSolidAngle = PI_F * sunAngularRadius * sunAngularRadius;

    vec3 bodyLocation = occludingBody.xyz;
    float bodyRadius = occludingBody.w;

    vec3 bodyLocationRelative = bodyLocation - fragmentPosition;
    vec3 bodyLocationNormalized = normalize(bodyLocationRelative);

    float bodyAngularRadius = angularRadOfSphere(length(bodyLocationRelative), bodyRadius);
    float angularDistanceToSun = angle(sunLocationNormalized, bodyLocationNormalized);

    float intersect = areaOfSphericalCapIntersectionApprox(sunAngularRadius,
    bodyAngularRadius,
    angularDistanceToSun);

    return (sunSolidAngle - clamp(intersect, 0.0, sunSolidAngle)) / sunSolidAngle;
}

double areaOfCircleIntersectionDouble(double radiusA, double radiusB, double centerDistance) {
    // No intersection
    if (centerDistance >= radiusA + radiusB) {
        return 0.0LF;
    }

    // One circle fully in the other (total eclipse)
    if (min(radiusA, radiusB) <= max(radiusA, radiusB) - centerDistance) {
        return PI_D * min(radiusA, radiusB) * min(radiusA, radiusB);
    }

    const double d = centerDistance;

    double rrA = radiusA * radiusA;
    double rrB = radiusB * radiusB;
    double dd = d * d;

    double d1 = fma(radiusA, radiusA, fma(-radiusB, radiusB, dd)) / (2 * d);
    double d2 = d - d1;

    double fourth = -d2 * sqrt(fma(-d2, d2, rrB));
    double third = fma(rrB, acosd(d2 / radiusB), fourth);
    double second = fma(-d1, sqrt(fma(-d1, d1, rrA)), third);
    return fma(rrA, acosd(d1 / radiusA), second);
}

float calcEclipseCircleDouble(vec4 occludingBody, vec3 fragmentPosition) {
    dvec3 sunLocationRelative = dvec3(uSunPosition - fragmentPosition);
    dvec3 sunLocationNormalized = normalize(sunLocationRelative);
    double sunAngularRadius = angularRadOfSphereD(length(sunLocationRelative), double(uSunRadius));
    double sunSolidAngle = PI_D * sunAngularRadius * sunAngularRadius;

    dvec3 bodyLocation = dvec3(occludingBody.xyz);
    double bodyRadius = double(occludingBody.w);

    dvec3 bodyLocationRelative = bodyLocation - dvec3(fragmentPosition);
    dvec3 bodyLocationNormalized = normalize(bodyLocationRelative);

    double bodyAngularRadius = angularRadOfSphereD(length(bodyLocationRelative), bodyRadius);
    double angularDistanceToSun = angleD(sunLocationNormalized, bodyLocationNormalized);

    double intersect = areaOfCircleIntersectionDouble(sunAngularRadius, bodyAngularRadius, angularDistanceToSun);

    return float((sunSolidAngle - clamp(intersect, 0.0LF, sunSolidAngle)) / sunSolidAngle);
}

vec3 projectPointOnRay(vec3 origin, vec3 direction, vec3 p) {
    const vec3 ap = p - origin;
    const vec3 ab = direction - origin;
    return origin + (dot(ap, ab) / dot(ab, ab)) * ab;
}

const float TEX_HEIGHT_TO_RADIUS_FACTOR = 4.0;
const float TEX_SHADOW_WIDTH_EXPONENT = 2.0;

vec3 calcEclipseTextureLookup(int i, vec3 fragmentPosition) {
    vec3 pos = uOccludingBodies[i].xyz;
    vec3 shadowNormal = uBodyShadowNormals[i];
    vec3 xPos = projectPointOnRay(pos, shadowNormal, fragmentPosition);
    float x = distance(pos, xPos);
    float y = distance(xPos, fragmentPosition);

    x = pow(x / uShadowLength[i], 1.0/TEX_SHADOW_WIDTH_EXPONENT);
    y = y / (uOccludingBodies[i].w * TEX_HEIGHT_TO_RADIUS_FACTOR);

    if (x <= 0.0 || x >= 1.0 || y <= 0.0 || y >= 1.0) {
        return vec3(1.0);
    }

    return texture(uShadowTextures[i], vec2(x, y)).rgb;
}

const int ECLIPSE_LOOKUP = 1;
const int ECLIPSE_CIRCLE = 2;
const int ECLIPSE_APPROX = 3;

vec3 applyEclipseShadows(vec3 fragmentPosition, vec3 fragmentNormal) {
    vec3 light = vec3(1.0);

    bool facingSun = dot(fragmentNormal, normalize(uSunPosition - fragmentPosition)) > 0.0;
    if (facingSun) {
        for (int i = 0; i < uNumOccludingBodies; ++i) {
            switch (uEclipseCalcType) {
                case ECLIPSE_LOOKUP:
                light *= calcEclipseTextureLookup(i, fragmentPosition);
                break;

                case ECLIPSE_CIRCLE:
                light *= calcEclipseCircleDouble(uOccludingBodies[i], fragmentPosition);
                break;

                case ECLIPSE_APPROX:
                light *= calcEclipseApprox(uOccludingBodies[i], fragmentPosition);
                break;

                default :
                break;
            }
        }
    }

    return light;
}

#endif // CS_ECLIPSE_SHADOWS_GLSL
