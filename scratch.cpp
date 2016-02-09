#define COURANT (1.0 / sqrt(3.0))

typedef enum {
    id_none = 0,
    id_nx = 1 << 1,
    id_px = 1 << 2,
    id_ny = 1 << 3,
    id_py = 1 << 4,
    id_nz = 1 << 5,
    id_pz = 1 << 6,
    id_reentrant = 1 << 7,
} BoundaryType;

typedef struct {
    int ports[PORTS];
    float3 position;
    bool inside;
    int bt;
    int boundary_index;
} Node;

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a##__VA_ARGS__

#define TEMPLATE_FILTER_MEMORY(order) \
    typedef struct { float array[order]; } CAT(FilterMemory, order);

TEMPLATE_FILTER_MEMORY(BIQUAD_ORDER);
TEMPLATE_FILTER_MEMORY(CANONICAL_FILTER_ORDER);

#define TEMPLATE_FILTER_COEFFICIENTS(order) \
    typedef struct {                        \
        float b[order + 1];                 \
        float a[order + 1];                 \
    } CAT(FilterCoefficients, order);

TEMPLATE_FILTER_COEFFICIENTS(BIQUAD_ORDER);
TEMPLATE_FILTER_COEFFICIENTS(CANONICAL_FILTER_ORDER);

typedef struct { FilterMemory2 array[BIQUAD_SECTIONS]; } BiquadMemoryArray;
typedef struct {
    FilterCoefficients2 array[BIQUAD_SECTIONS];
} BiquadCoefficientsArray;

//  we assume that a0 == 1.0f
//  that is, the FILTER COEFFICIENTS ARE NORMALISED
#define FILTER_STEP(order)                                             \
    float CAT(filter_step_, order)(                                    \
        float input,                                                   \
        global CAT(FilterMemory, order) * m,                           \
        const global CAT(FilterCoefficients, order) * c);              \
    float CAT(filter_step_, order)(                                    \
        float input,                                                   \
        global CAT(FilterMemory, order) * m,                           \
        const global CAT(FilterCoefficients, order) * c) {             \
        int i = 0;                                                     \
        float out = input * c->b[i] + m->array[i];                     \
        for (; i != order - 1; ++i) {                                  \
            m->array[i] =                                              \
                i * c->b[i + 1] - c->a[i + 1] * out + m->array[i + 1]; \
        }                                                              \
        m->array[i] = i * c->b[i + 1] - c->a[i + 1] * out;             \
        return out;                                                    \
    }

FILTER_STEP(BIQUAD_ORDER);
FILTER_STEP(CANONICAL_FILTER_ORDER);

float biquad_cascade(float input,
                     global BiquadMemoryArray * bm,
                     const global BiquadCoefficientsArray * bc);
float biquad_cascade(float input,
                     global BiquadMemoryArray * bm,
                     const global BiquadCoefficientsArray * bc) {
    for (int i = 0; i != BIQUAD_SECTIONS; ++i) {
        input = CAT(filter_step_, BIQUAD_ORDER)(
            input, bm->array + i, bc->array + i);
    }
    return input;
}

kernel void filter_test(
    const global float * input,
    global float * output,
    global BiquadMemoryArray * biquad_memory,
    const global BiquadCoefficientsArray * biquad_coefficients) {
    size_t index = get_global_id(0);
    output[index] = biquad_cascade(
        input[index], biquad_memory + index, biquad_coefficients + index);
}

kernel void filter_test_2(const global float * input,
                          global float * output,
                          global CAT(FilterMemory, CANONICAL_FILTER_ORDER) *
                              canonical_memory,
                          const global CAT(FilterCoefficients,
                                           CANONICAL_FILTER_ORDER) *
                              canonical_coefficients) {
    size_t index = get_global_id(0);
    output[index] = CAT(filter_step_, CANONICAL_FILTER_ORDER)(
        input[index], canonical_memory + index, canonical_coefficients + index);
}

typedef struct {
    CAT(FilterMemory, CANONICAL_FILTER_ORDER) filter_memory;
    int coefficient_index;
} BoundaryData;

typedef struct { BoundaryData array[1]; } BoundaryDataArray1;
typedef struct { BoundaryData array[2]; } BoundaryDataArray2;
typedef struct { BoundaryData array[3]; } BoundaryDataArray3;

typedef enum {
    id_port_nx = 0,
    id_port_px = 1,
    id_port_ny = 2,
    id_port_py = 3,
    id_port_nz = 4,
    id_port_pz = 5,
} PortDirection;

PortDirection get_inner_node_direction(BoundaryType boundary_type);
PortDirection get_inner_node_direction(BoundaryType boundary_type) {
    switch (boundary_type) {
        case id_nx:
            return id_port_px;
        case id_px:
            return id_port_nx;
        case id_ny:
            return id_port_py;
        case id_py:
            return id_port_ny;
        case id_nz:
            return id_port_pz;
        case id_pz:
            return id_port_nz;

        default:
            return -1;
    }
}

PortDirection opposite(PortDirection pd);
PortDirection opposite(PortDirection pd) {
    switch (pd) {
        case id_port_nx:
            return id_port_px;
        case id_port_px:
            return id_port_nx;
        case id_port_ny:
            return id_port_py;
        case id_port_py:
            return id_port_ny;
        case id_port_nz:
            return id_port_pz;
        case id_port_pz:
            return id_port_nz;

        default:
            return -1;
    }
}

PortDirection get_ghost_node_direction(BoundaryType boundary_type);
PortDirection get_ghost_node_direction(BoundaryType boundary_type) {
    return opposite(get_inner_node_direction(boundary_type));
}

typedef struct { PortDirection array[4]; } SurroundingPorts;

SurroundingPorts on_boundary_1d(PortDirection pd);
SurroundingPorts on_boundary_1d(PortDirection pd) {
    switch (pd) {
        case id_port_nx:
        case id_port_px:
            return (SurroundingPorts){
                {id_port_ny, id_port_py, id_port_nz, id_port_pz}};
        case id_port_ny:
        case id_port_py:
            return (SurroundingPorts){
                {id_port_nx, id_port_px, id_port_nz, id_port_pz}};
        case id_port_nz:
        case id_port_pz:
            return (SurroundingPorts){
                {id_port_nx, id_port_px, id_port_ny, id_port_py}};

        default:
            return (SurroundingPorts){{-1, -1, -1, -1}};
    }
}

float sum_surrounding(const global float * current,
                      const global Node * node,
                      PortDirection pd);
float sum_surrounding(const global float * current,
                      const global Node * node,
                      PortDirection pd) {
    float ret = 0;
    SurroundingPorts on_boundary = on_boundary_1d(pd);
    for (int i = 0; i != 4; ++i) {
        int index = node->ports[on_boundary.array[i]];
        if (index == -1) {
            //  TODO error!
        }
        ret += current[index];
    }
    return ret;
}

//  call with the index of the BOUNDARY node, and the relative direction of the
//  ghost point
//
//  we don't actually care about the pressure at the ghost point other than to
//  calculate the boundary filter input
void ghost_point_pressure_update(const global float * current,
                                 const global Node * boundary_node,
                                 float next_pressure,
                                 float prev_pressure,
                                 global BoundaryData * boundary_data,
                                 const global CAT(FilterCoefficients,
                                                  CANONICAL_FILTER_ORDER) *
                                     boundary,
                                 PortDirection inner_direction);
void ghost_point_pressure_update(const global float * current,
                                 const global Node * boundary_node,
                                 float next_pressure,
                                 float prev_pressure,
                                 global BoundaryData * boundary_data,
                                 const global CAT(FilterCoefficients,
                                                  CANONICAL_FILTER_ORDER) *
                                     boundary,
                                 PortDirection inner_direction) {
    int inner_index = boundary_node->ports[inner_direction];
    float inner_pressure = current[inner_index];

    float filt_state = boundary_data->filter_memory.array[0];

    float b0 = boundary->b[0];
    float a0 = boundary->a[0];

    float ret = inner_pressure +
                (a0 * (prev_pressure - next_pressure)) / (b0 * COURANT) +
                filt_state / b0;

    //  now we can update the filter at this boundary node
    float filter_input = inner_pressure - ret;
    CAT(filter_step_, CANONICAL_FILTER_ORDER)(filter_input, &boundary_data->filter_memory, boundary);
}

float boundary_1d(const global float * current,
                  const global float * previous,
                  const global Node * boundary_node,
                  global BoundaryDataArray1 * boundary_data_1,
                  const global CAT(FilterCoefficients, CANONICAL_FILTER_ORDER) *
                      boundary_coefficients,
                  BoundaryType bt);
float boundary_1d(const global float * current,
                  const global float * previous,
                  const global Node * boundary_node,
                  global BoundaryDataArray1 * boundary_data_1,
                  const global CAT(FilterCoefficients, CANONICAL_FILTER_ORDER) *
                      boundary_coefficients,
                  BoundaryType bt) {
    PortDirection inner_direction = get_inner_node_direction(bt);
    int inner_index = boundary_node->ports[inner_direction];
    float inner_pressure = current[inner_index];

    float summed_surrounding =
        sum_surrounding(current, boundary_node, inner_direction);

    size_t index = get_global_id(0);

    float COURANT_SQ = COURANT * COURANT;

    float current_surrounding_weighting =
        COURANT_SQ * (2 * inner_pressure + summed_surrounding);
    float current_boundary_weighting =
        2 * (1 - 3 * COURANT_SQ) * current[index];

    global BoundaryDataArray1 * bda =
        boundary_data_1 + boundary_node->boundary_index;
    global BoundaryData * boundary_data = &bda->array[0];
    const global CAT(FilterCoefficients, CANONICAL_FILTER_ORDER) * boundary =
        boundary_coefficients + boundary_data->coefficient_index;

    float filt_state = boundary_data->filter_memory.array[0];

    float b0 = boundary->b[0];
    float a0 = boundary->a[0];

    float filter_weighting = COURANT_SQ * filt_state / b0;

    float coeff_weighting = COURANT * a0 / b0;
    float prev_pressure = previous[index];
    float prev_weighting = (coeff_weighting - 1) * prev_pressure;

    float ret = (current_surrounding_weighting + current_boundary_weighting +
                 filter_weighting + prev_weighting) /
                (1 + coeff_weighting);

    ghost_point_pressure_update(current,
                                boundary_node,
                                ret,
                                prev_pressure,
                                boundary_data,
                                boundary,
                                inner_direction);

    return ret;
}

kernel void waveguide(const global float * current,
                      global float * previous,
                      const global Node * nodes,
                      global BoundaryDataArray1 * boundary_data_1,
                      global BoundaryDataArray2 * boundary_data_2,
                      global BoundaryDataArray3 * boundary_data_3,
                      const global CAT(FilterCoefficients,
                                       CANONICAL_FILTER_ORDER) *
                          boundary_coefficients,
                      const global float * transform_matrix,
                      global float3 * velocity_buffer,
                      float spatial_sampling_period,
                      float T,
                      ulong read,
                      global float * output) {
    size_t index = get_global_id(0);
    const global Node * node = nodes + index;

    float next_pressure = 0;

    //  find the next pressure at this node, assign it to next_pressure
    switch (popcount(node->bt)) {
        //  this is inside or outside, not a boundary
        case 0:
            if (node->inside) {
                for (int i = 0; i != PORTS; ++i) {
                    int port_index = node->ports[i];
                    if (port_index >= 0 && nodes[port_index].inside)
                        next_pressure += current[port_index];
                }

                next_pressure /= (PORTS / 2);
                next_pressure -= previous[index];
            }
            break;
        //  this is a 1d-boundary node
        case 1:
            break;
        //  this is an edge where two boundaries meet
        case 2:
            break;
        //  this is a corner where three boundaries meet
        case 3:
            break;
    }

    barrier(CLK_GLOBAL_MEM_FENCE);
    previous[index] = next_pressure;
    barrier(CLK_GLOBAL_MEM_FENCE);

    if (index == read) {
        *output = previous[index];

        //
        //  instantaneous intensity for mic modelling
        //

        float differences[PORTS] = {0};
        for (int i = 0; i != PORTS; ++i) {
            int port_index = node->ports[i];
            if (port_index >= 0 && nodes[port_index].inside)
                differences[i] = (previous[port_index] - previous[index]) /
                                 spatial_sampling_period;
        }

        //  the default for Eigen is column-major matrices
        //  so we'll assume that transform_matrix is column-major

        //  multiply differences by transformation matrix
        float3 multiplied = (float3)(0);
        for (int i = 0; i != PORTS; ++i) {
            multiplied += (float3)(transform_matrix[0 + i * 3],
                                   transform_matrix[1 + i * 3],
                                   transform_matrix[2 + i * 3]) *
                          differences[i];
        }

        //  muliply by -1/ambient_density
        float ambient_density = 1.225;
        multiplied /= -ambient_density;

        //  numerical integration
        //
        //  I thought integration meant just adding to the previous value like
        //  *velocity_buffer += multiplied;
        //  but apparently the integrator has the transfer function
        //  Hint(z) = Tz^-1 / (1 - z^-1)
        //  so hopefully this is right
        //
        //  Hint(z) = Y(z)/X(z) = Tz^-1/(1 - z^-1)
        //  y(n) = Tx(n - 1) + y(n - 1)

        *velocity_buffer += T * multiplied;
    }
}
