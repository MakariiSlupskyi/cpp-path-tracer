#version 410 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texAccum; // The previous frames
uniform float u_time;
uniform int u_frame;
uniform vec2 u_resolution;

struct Material {
  vec3 color;
};

Material getMaterial(float id) {
  if (id == 0.0) return Material(vec3(1.0, 0.5, 0.5));
  if (id == 1.0) return Material(vec3(0.5, 1.0, 0.5));
  if (id == 2.0) return Material(vec3(0.5, 0.5, 1.0));

  return Material(vec3(0.0)); // no such material
}

mat3 rotateX(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(1, 0, 0),
        vec3(0, c, -s),
        vec3(0, s, c)
    );
}

mat3 rotateY(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, 0, s),
        vec3(0, 1, 0),
        vec3(-s, 0, c)
    );
}

mat3 rotateZ(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, -s, 0),
        vec3(s, c, 0),
        vec3(0, 0, 1)
    );
}

float sdSphere(vec3 p, float r) {
  return length(p) - r;
}

float sdBox( vec3 p, vec3 b ) {
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdPlane( vec3 p, vec3 n, float h ) {
  return dot(p,n) + h;
}

vec2 minObj(vec2 first, vec2 second) { return (first.x > second.x) ? second : first; }
vec2 maxObj(vec2 first, vec2 second) { return (first.x < second.x) ? second : first; }

vec2 sceneSDF(vec3 p) {
  vec3 pBox = p;
  pBox.x += 1.1 * sin(u_frame / 40.0);
  pBox *= rotateY(u_frame / 60.0);

  vec2 sphere = vec2(sdSphere(p, 0.5), 0.0);
  vec2 plane = vec2(sdPlane(p, vec3(0.0, 1.0, 0.0), 1.0), 1.0);
  vec2 box = vec2(sdBox(pBox, vec3(0.4)), 2.0);

  return minObj(sphere, minObj(plane, box));
}

vec3 calcNormal(vec3 p) {
    float EPSILON = 0.00005;
    return normalize(vec3(
        sceneSDF(vec3(p.x + EPSILON, p.y, p.z)).x - sceneSDF(vec3(p.x - EPSILON, p.y, p.z)).x,
        sceneSDF(vec3(p.x, p.y + EPSILON, p.z)).x - sceneSDF(vec3(p.x, p.y - EPSILON, p.z)).x,
        sceneSDF(vec3(p.x, p.y, p.z  + EPSILON)).x - sceneSDF(vec3(p.x, p.y, p.z - EPSILON)).x
    ));
}

vec3 rayDirection(float FOV) {
  float z = u_resolution.y / tan(radians(FOV) / 2.0);
  return normalize(vec3(TexCoords, z));
}

vec4 rayMarch(vec3 ro, vec3 rayDir) {
  float t = 0.001;
  for (int i = 0; i < 1000; i++) {
    vec3 p = ro + rayDir * t;
    vec2 res = sceneSDF(p);
    float d = res.x;
    float matId = res.y;
    if (d < 0.0001) return vec4(p, matId);
    t += d;
    if (t > 50.0) break;
  }
  return vec4(0.0, 0.0, 0.0, -1.0);
}

void main() {
  vec2 uv = TexCoords;
  uv.x *= u_resolution.x / u_resolution.y;

  vec3 rayOrigin = vec3(-0.2, 1.0, 2.0);
  vec3 rayDir = normalize(vec3(uv.x, uv.y, -1.0));
  rayDir = rotateX(0.5) * rayDir;

  vec3 light = vec3(3 * sin(u_time * 4), 3.0, 3 * cos(u_time * 4));

  vec3 c = vec3(1.0);
  vec3 p = rayOrigin;
  for (int i = 0; i < 1000; i++) {
    vec4 res = rayMarch(p, rayDir);
    p = res.xyz;
    if (res.w == -1) {
      c *= vec3(0.0, 1.0, 1.0);
      break;
    }
    vec3 n = calcNormal(p);
    rayDir = reflect(rayDir, n);
    c *= getMaterial(res.w).color;
  }

  FragColor = vec4(c, 0.01);
}
