#version 430

#extension GL_ARB_compute_variable_group_size: enable

// TODO make configurable
const uint TEX_WIDTH = 256u;
const uint TEX_HEIGHT = TEX_WIDTH;
const float TEX_SHADOW_WIDTH_EXPONENT = 2.0;

// TODO make configurable
const uint MIN_WAVELENGTH = 390u;
const uint MAX_WAVELENGTH = 749u;
const uint NUM_WAVELENGTHS = MAX_WAVELENGTH - MIN_WAVELENGTH + 1;

// Size: 24 bytes -> ~40,000,000 photons per available gigabyte of ram
struct Photon {
  vec2 position;// m
  vec2 direction;// normalized
  uint wavelength;// nm
  float intensity;// 0..1 should start at 1
};

// maybe generate in GPU with rng?
layout(std430, binding = 0) buffer Photons {
  Photon photons[];
};

// Size: 1440 bytes -> ~700,000 pixels per available gigabyte of ram
struct Pixel {
  uint intensityAtWavelengths[NUM_WAVELENGTHS];// [0..1000]
};

layout(std430, binding = 1) buffer Pixels {
//Pixel pixels[TEX_WIDTH][TEX_HEIGHT];
  Pixel[] pixels;
};

vec2 getHorizontalRectangleAt(int i) {
  float x0 = pow(float(i), 1.0 / TEX_SHADOW_WIDTH_EXPONENT);
  float x1 = pow(float(i + 1), 1.0 / TEX_SHADOW_WIDTH_EXPONENT);
  return vec2(x0, x1 - x0);
}

uniform float rectangleHeight;

struct Rectangle {
  float x;
  float y;
  float w;
  float h;
};

uniform uint pass;
uniform uint passSize;

layout (local_size_variable) in;

void addToPixel(uvec2 idx, uint wavelength, uint intensity) {
  if (idx.x >= 0u && idx.x < TEX_WIDTH && idx.y >= 0u && idx.y < TEX_HEIGHT) {
    uint index = (idx.y * TEX_WIDTH) + idx.x;
    atomicAdd(pixels[index].intensityAtWavelengths[wavelength - MIN_WAVELENGTH], intensity);
  }
}

/// Returns the rectangle at the given indices.
Rectangle getRectangleAt(ivec2 indices) {
  vec2 horRect = getHorizontalRectangleAt(indices.x);
  return Rectangle(horRect.x, rectangleHeight * float(indices.y), horRect.y, rectangleHeight);
}

/// Searches for the horizontal index in the rectangle array which contains x.
int binarySearchForHorizontalRectangle(float x) {
  int left = 0;
  int right = int(TEX_WIDTH) - 1;

  while (left <= right) {
    int middle = (left + right) / 2;
    vec2 rectM = getHorizontalRectangleAt(middle);
    if (rectM.x  + rectM.y < x) {
      left = middle + 1;
    } else if (rectM.x > x) {
      right = middle - 1;
    } else return middle;
  }

  return int(TEX_WIDTH);// outside of grid (should never happen in any reasonable scenario...)
}

uniform float shadowLength;
uniform float shadowHeight;

/// Returns the indices of the rectangle at the given location
ivec2 getRectangleIdxAt(vec2 location) {
  if (location.x >= 0 && location.x < shadowLength && location.y >= 0 && location.y < shadowHeight) {
    int x = binarySearchForHorizontalRectangle(location.x);
    int y = int(location.y / rectangleHeight);
    return ivec2(x, y);
  } else return ivec2(TEX_WIDTH, TEX_HEIGHT);
}

float getRayIntersectAtX(Photon ray, float x) {
  float slope = ray.direction.y / ray.direction.x;
  return slope * (x - ray.position.x) + ray.position.y;
}

ivec2 getRayRectangleExitEdge(Photon ray, Rectangle rect) {
  float intersectHeight = getRayIntersectAtX(ray, rect.x + rect.w);
  if (intersectHeight < rect.y) {
    return ivec2(0, -1);
  } else if (intersectHeight > rect.y + rect.h) {
    return ivec2(0, 1);
  } else {
    return ivec2(1, 0);
  }
}

void mirrorRayAroundUniversalXAxis(inout Photon ray) {
  ray.position.y = -ray.position.y;
  ray.direction.y = -ray.direction.y;
}

void main() {
  uint gid = gl_GlobalInvocationID.x;
  uint passId = pass * passSize + gid;
  if (passId >= photons.length()) return;

  Photon photon = photons[passId];

  // Poor photon didn't make it through the atmosphere :(
  if (photon.intensity < 0.0) return;

  ivec2 photonTexIndices = getRectangleIdxAt(photon.position);
  while (photonTexIndices.x < TEX_WIDTH && photonTexIndices.y < TEX_HEIGHT &&
  photonTexIndices.x >= 0        && photonTexIndices.y >= 0) {
    // need to convert to uint for atomic add operations...
    addToPixel(uvec2(photonTexIndices), photon.wavelength, uint(photon.intensity * 100.0));

    ivec2 dir = getRayRectangleExitEdge(photon, getRectangleAt(photonTexIndices));
    photonTexIndices += dir;

    // When the ray goes out of bounds on the bottom then mirror it to simulate rays coming from
    // the other side of the planet. This works because of the rotational symmetry of the system.
    if (photonTexIndices.y < 0) {
      photonTexIndices.y = 0;
      mirrorRayAroundUniversalXAxis(photon);
    }
  }
}
