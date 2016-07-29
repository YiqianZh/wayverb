#pragma once

#include "raytracer/cl_structs.h"
#include "raytracer/results.h"

#include "common/hrtf_utils.h"
#include "common/receiver_settings.h"
#include "common/stl_wrappers.h"

namespace raytracer {

double pressure_to_intensity(double pressure, double Z = 400);
double intensity_to_pressure(double intensity, double Z = 400);

/// Sum impulses ocurring at the same (sampled) time and return a vector in
/// which each subsequent item refers to the next sample of an impulse
/// response.
template <typename Impulse>  //  an Impulse has a .volume and a .time
aligned::vector<aligned::vector<float>> flatten_impulses(
        const aligned::vector<Impulse>& impulse, double samplerate) {
    const auto MAX_TIME_LIMIT = 20.0f;
    // Find the index of the final sample based on time and samplerate
    const auto maxtime = std::min(
            proc::max_element(impulse,
                              [](auto i, auto j) { return i.time < j.time; })
                    ->time,
            MAX_TIME_LIMIT);

    const auto MAX_SAMPLE = round(maxtime * samplerate) + 1;

    //  Create somewhere to store the results.
    aligned::vector<aligned::vector<float>> flattened(
            detail::components_v<VolumeType>,
            aligned::vector<float>(MAX_SAMPLE, 0));

    //  For each impulse, calculate its index, then add the impulse's volumes
    //  to the volumes already in the output array.
    for (const auto& i : impulse) {
        const auto SAMPLE = round(i.time * samplerate);
        if (SAMPLE < MAX_SAMPLE) {
            for (auto j = 0u; j != flattened.size(); ++j) {
                const auto intensity = i.volume.s[j];
                //const auto pressure = intensity_to_pressure(intensity);
                flattened[j][SAMPLE] += intensity;
            }
        }
    }

    //  impulses are intensity levels, now we need to convert to pressure
    proc::for_each(flattened, [](auto& i) {
        proc::for_each(i, [](auto& j) {
            j = intensity_to_pressure(j);
        });
    });

    return flattened;
}

/// Maps flattenImpulses over a vector of input vectors.
template <typename Impulse>
aligned::vector<aligned::vector<aligned::vector<float>>> flatten_impulses(
        const aligned::vector<aligned::vector<Impulse>>& impulse,
        double samplerate) {
    return map_to_vector(impulse, [samplerate](const auto& i) {
        return flatten_impulses(i, samplerate);
    });
}

/// Recursively check a collection of Impulses for the earliest non-zero time of
/// an impulse.
template <typename T>
inline auto find_predelay(const T& ret) {
    return std::accumulate(ret.begin() + 1,
                           ret.end(),
                           findPredelay(ret.front()),
                           [](auto a, const auto& b) {
                               auto pd = findPredelay(b);
                               if (a == 0) {
                                   return pd;
                               }
                               if (pd == 0) {
                                   return a;
                               }
                               return std::min(a, pd);
                           });
}

/// The base case of the findPredelay recursion.
template <typename T>
inline auto find_predelay_base(const T& t) {
    return t.time;
}

template <>
inline auto find_predelay(const AttenuatedImpulse& i) {
    return find_predelay_base(i);
}

template <>
inline auto find_predelay(const Impulse& i) {
    return find_predelay_base(i);
}

/// Recursively subtract a time value from the time fields of a collection of
/// Impulses.
template <typename T>
inline void fix_predelay(T& ret, float seconds) {
    for (auto& i : ret) {
        fixPredelay(i, seconds);
    }
}

template <typename T>
inline void fix_predelay_base(T& ret, float seconds) {
    ret.time = ret.time > seconds ? ret.time - seconds : 0;
}

/// The base case of the fixPredelay recursion.
template <>
inline void fix_predelay(AttenuatedImpulse& ret, float seconds) {
    fix_predelay_base(ret, seconds);
}

template <>
inline void fix_predelay(Impulse& ret, float seconds) {
    fix_predelay_base(ret, seconds);
}

/// Fixes predelay by finding and then removing predelay.
template <typename T>
inline void fix_predelay(T& ret) {
    auto predelay = findPredelay(ret);
    fixPredelay(ret, predelay);
}

template <typename Impulse>
aligned::vector<float> flatten_filter_and_mixdown(
        const aligned::vector<Impulse>& input, double output_sample_rate) {
    return multiband_filter_and_mixdown(
            flatten_impulses(input, output_sample_rate), output_sample_rate);
}

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::ReceiverSettings& receiver,
        const results& input,
        double output_sample_rate);

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::ReceiverSettings& receiver,
        const aligned::vector<Impulse>& input,
        double output_sample_rate);

}  // namespace raytracer
