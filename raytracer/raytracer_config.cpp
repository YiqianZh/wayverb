#include "raytracer_config.h"

namespace config {

int Raytracer::get_rays() const {
    return rays;
}

int Raytracer::get_impulses() const {
    return impulses;
}

float Raytracer::get_ray_hipass() const {
    return ray_hipass;
}

bool Raytracer::get_do_normalize() const {
    return do_normalize;
}

bool Raytracer::get_trim_predelay() const {
    return trim_predelay;
}

bool Raytracer::get_trim_tail() const {
    return trim_tail;
}

bool Raytracer::get_remove_direct() const {
    return remove_direct;
}

float Raytracer::get_volume_scale() const {
    return volume_scale;
}

int& Raytracer::get_rays() {
    return rays;
}
int& Raytracer::get_impulses() {
    return impulses;
}
float& Raytracer::get_ray_hipass() {
    return ray_hipass;
}
bool& Raytracer::get_do_normalize() {
    return do_normalize;
}
bool& Raytracer::get_trim_predelay() {
    return trim_predelay;
}
bool& Raytracer::get_trim_tail() {
    return trim_tail;
}
bool& Raytracer::get_remove_direct() {
    return remove_direct;
}
float& Raytracer::get_volume_scale() {
    return volume_scale;
}
}
