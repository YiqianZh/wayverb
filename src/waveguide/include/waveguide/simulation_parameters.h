#pragma once

#include <cstdlib>

namespace wayverb {
namespace waveguide {

struct single_band_parameters final {
    /// The actual cutoff of the waveguide mesh in Hz.
    double cutoff;

    /// The proportion of the 'valid' spectrum that should be used.
    /// Values between 0 and 1 are valid, but 0.6 or lower is recommended.
    double usable_portion;
};

struct multiple_band_constant_spacing_parameters final {
    /// The number of bands which should be simulated with the waveguide.
    /// Be careful with high numbers.
    /// The waveguide will be run once, at the required sampling rate, for each
    /// band, so i.e. 4 bands will take 4 times as long.
    size_t bands;
    
    /// The cutoff to use for all bands.
    double cutoff;

    /// As above.
    double usable_portion;
};

constexpr auto compute_cutoff_frequency(double sample_rate,
                                        double usable_portion) {
    return sample_rate * 0.25 * usable_portion;
}

constexpr auto compute_sampling_frequency(double cutoff,
                                          double usable_portion) {
    return cutoff / (0.25 * usable_portion);
}

template <typename T>
constexpr auto compute_sampling_frequency(const T& t) {
    return compute_sampling_frequency(t.cutoff, t.usable_portion);
}

}  // namespace waveguide
}  // namespace wayverb
