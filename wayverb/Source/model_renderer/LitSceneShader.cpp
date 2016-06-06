#include "LitSceneShader.hpp"

LitSceneShader::LitSceneShader()
        : ShaderProgram(vertex_shader, fragment_shader) {
}

void LitSceneShader::set_model_matrix(const glm::mat4& mat) const {
    set_matrix("v_model", mat);
}
void LitSceneShader::set_view_matrix(const glm::mat4& mat) const {
    set_matrix("v_view", mat);
}
void LitSceneShader::set_projection_matrix(const glm::mat4& mat) const {
    set_matrix("v_projection", mat);
}

void LitSceneShader::set_matrix(const std::string& s,
                                const glm::mat4& mat) const {
    glUniformMatrix4fv(
        get_uniform_location(s), 1, GL_FALSE, glm::value_ptr(mat));
}

void LitSceneShader::set_colour(const glm::vec3& c) const {
    glUniform3f(get_uniform_location("f_solid_color"), c.r, c.g, c.b);
}

const std::string LitSceneShader::vertex_shader(R"(
#version 150
in vec3 v_position;
in vec4 v_color;
out vec4 f_color;
out vec3 f_modelview;
uniform mat4 v_model;
uniform mat4 v_view;
uniform mat4 v_projection;

void main() {
    vec4 modelview = v_view * v_model * vec4(v_position, 1.0);
    gl_Position = v_projection * modelview;
    f_color = v_color;
    f_modelview = modelview.xyz;
}
)");

const std::string LitSceneShader::fragment_shader(R"(
#version 150
in vec4 f_color;
in vec3 f_modelview;
out vec4 frag_color;
uniform vec3 f_solid_color;

const vec3 direction = normalize(vec3(0, -1, -1));
const vec3 ambient = vec3(0.05);

void main() {
    vec3 f_normal = normalize(cross(dFdx(f_modelview), dFdy(f_modelview)));
    float diffuse = max(0.0, dot(f_normal, -direction));
    frag_color = vec4(f_solid_color * diffuse + ambient, 1.0);
}
)");