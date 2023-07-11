#version 130

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Light {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

out vec4 frag_col;

in vec3 frag_3Dpos;
in vec3 vs_normal;
in vec2 vs_tex_coord;

uniform Material material;
uniform Light light, other_light;
uniform vec3 view_pos, other_view_pos;

void main() {

  // Contribución de la primera luz (Original).
  // Ambient
  vec3 ambient = light.ambient * material.ambient;
  vec3 light_dir = normalize(light.position - frag_3Dpos);

  // Diffuse
  float diff = max(dot(vs_normal, light_dir), 0.0);
  vec3 diffuse = light.diffuse * diff * material.diffuse;

  // Specular
  vec3 view_dir = normalize(view_pos - frag_3Dpos);
  vec3 reflect_dir = reflect(-light_dir, vs_normal);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = light.specular * spec * material.specular;
  
  // Contribución de la segunda luz (Other).
  // Ambient
  vec3 other_ambient = other_light.ambient * material.ambient;
  vec3 other_light_dir = normalize(other_light.position - frag_3Dpos);

  // Diffuse
  float other_diff = max(dot(vs_normal, other_light_dir), 0.0);
  vec3 other_diffuse = other_light.diffuse * other_diff * material.diffuse;

  // Specular
  vec3 other_view_dir = normalize(other_view_pos - frag_3Dpos);
  vec3 other_reflect_dir = reflect(-other_light_dir, vs_normal);
  float other_spec = pow(max(dot(other_view_dir, other_reflect_dir), 0.0), material.shininess);
  vec3 other_specular = other_light.specular * other_spec * material.specular;

  vec3 result = ambient + diffuse + specular + other_ambient + other_diffuse + other_specular;
  frag_col = vec4(result, 1.0);
}
