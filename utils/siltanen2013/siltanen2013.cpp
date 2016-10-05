#include "run_methods.h"

#include "common/write_audio_file.h"

#include <iostream>

//  TODO test raytracer diffuse output to compenstate for fast img-src decay
//
//  TODO bidirectional mic outputs don't match

int main() {

    //  constants ------------------------------------------------------------//

    const geo::box box{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    constexpr auto sample_rate{16000.0};
    constexpr auto speed_of_sound{340.0};
    constexpr auto acoustic_impedance{400.0f};

    constexpr glm::vec3 source{2.09, 2.12, 2.12};
    constexpr glm::vec3 receiver{2.09, 3.08, 0.96};

    constexpr auto reflectance{0.95};
    const auto absorption{1 - pow(reflectance, 2)};

    const microphone mic{glm::vec3{0, 0, 1}, 1.0};

    //  simulations ----------------------------------------------------------//

    auto waveguide{run(box,
                       absorption,
                       source,
                       receiver,
                       mic,
                       speed_of_sound,
                       acoustic_impedance,
                       sample_rate,
                       run_waveguide)};

    auto exact_img_src{run(box,
                           absorption,
                           source,
                           receiver,
                           mic,
                           speed_of_sound,
                           acoustic_impedance,
                           sample_rate,
                           run_exact_img_src)};

    auto fast_img_src{run(box,
                          absorption,
                          source,
                          receiver,
                          mic,
                          speed_of_sound,
                          acoustic_impedance,
                          sample_rate,
                          run_fast_img_src)};

    //  postprocessing -------------------------------------------------------//

    auto waveguide_pressure{
            map_to_vector(waveguide, [](auto i) { return i.pressure; })};
    waveguide_filter(
            waveguide_pressure.begin(), waveguide_pressure.end(), sample_rate);

    auto waveguide_attenuated{waveguide::attenuate(
            mic, acoustic_impedance, waveguide.begin(), waveguide.end())};
    waveguide_filter(waveguide_attenuated.begin(),
                     waveguide_attenuated.end(),
                     sample_rate);

    const auto outputs = {&waveguide_pressure,
                          &waveguide_attenuated,
                          &exact_img_src,
                          &fast_img_src};

    //  Normalise all outputs against one another.
    const auto max_mags{
            map_to_vector(outputs, [](auto i) { return max_mag(*i); })};

    const auto mag{*std::max_element(max_mags.begin(), max_mags.end())};

    for (const auto& ptr : outputs) {
        for (auto& samp : *ptr) {
            samp /= mag;
        }
    }

    snd::write("waveguide_pressure.wav", {waveguide_pressure}, sample_rate, 16);
    snd::write("waveguide_attenuated.wav",
               {waveguide_attenuated},
               sample_rate,
               16);
    snd::write("exact_img_src.wav", {exact_img_src}, sample_rate, 16);
    snd::write("fast_img_src.wav", {fast_img_src}, sample_rate, 16);

    return EXIT_SUCCESS;
}