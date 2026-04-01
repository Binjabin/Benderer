# Benderer Path Tracer - Complete Architecture Analysis

## Executive Summary

Benderer is a sophisticated Monte Carlo path tracer written in C++ that implements multiple importance sampling (MIS) for unified handling of surface materials, volumetric media, and environment lighting. The architecture is modular, separating concerns into integrators (rendering algorithms), samplers (direction/distance sampling), records (data flow), and scene components (geometry, materials, lights).

---

## 1. OVERALL ARCHITECTURE

### System Flow Diagram

```
main.cpp
  └─ scene::render()
     └─ camera::render()
        └─ For each pixel:
           └─ integrator::ray_color()  [MIS, Simple, RR variants]
              └─ path_trace() [recursive]
                 ├─ world::m_surfaces->surface_hit()
                 ├─ world::m_mediums->medium_hit()
                 ├─ Sampling (direct_light_sampler, medium_sampler)
                 └─ Material evaluation (BSDF, phase functions)
```

### Key Design Principles

1. **Modular Integrators**: Different rendering algorithms (simple path tracing, RR termination, MIS) implement the same `integrator` interface
2. **Separated Sampling**: Dedicated samplers for direct lighting and media handle complexity
3. **Data-Driven Path State**: `path_state` carries throughput and weighting information through recursion
4. **Media as First-Class**: Volumetric effects are fully integrated, not bolted on
5. **Explicit Light Tracking**: Tracks which bounces used direct sampling for MIS weighting

---

## 2. INTEGRATOR ARCHITECTURE

### Base Class: `integrator`

**Location**: [source/integrators/integrator.h](source/integrators/integrator.h)

```cpp
class integrator {
    virtual color ray_color(const ray& r, int depth, const world& world) const;
};
```

**Responsibility**: Abstract interface for rendering algorithms. Subclasses override `ray_color()` to implement recursive path tracing.

### Integrator Variants

#### A. `simple_path_tracer` (Baseline)
**Location**: [source/integrators/simple_path_tracer.h](source/integrators/is_path_tracer.h)

**Purpose**: Basic unbiased path tracer with no variance reduction

**Key Characteristics**:
- Direct recursion without termination heuristics
- No Russian roulette
- Samplesdirect BSDF to get next direction
- Minimal state tracking

**Control Flow**:
```
ray_color()
└─ path_trace(ray, world, path_state)
   ├─ Check throughput > epsilon, terminate if not
   ├─ Test surface intersection
   ├─ Test media intersection (if surface further)
   ├─ No hit → return skybox
   ├─ Medium hit → handle scatter/absorb, recurse
   ├─ Surface hit → apply transmittance
   │  ├─ Get surface emittance
   │  ├─ Test max depth termination
   │  ├─ Call scatter() for next direction
   │  └─ Recurse via get_indirect_result()
   └─ Return accumulated radiance
```

#### B. `simple_medium_path_tracer` (Media Support)
**Location**: [source/integrators/simple_medium_path_tracer.h](source/integrators/simple_medium_path_tracer.h)

**Purpose**: Extends baseline to handle volumetric media fully

**Additions Over Simple**:
- Separates media transmittance from scattering
- Implements distance sampling via `medium_sampler::sample_distance()`
- Tracks whether medium event is scatter or absorption
- Does NOT use MIS weighting (so NEE contributions not downweighted)

**Media Handling**:
- Only one medium event per ray segment (null collision tracking internally)
- Transmittance applied separately from BSDF contribution
- Phase function handles scattering direction in media

#### C. `rr_medium_path_tracer` (Russian Roulette)
**Location**: [source/integrators/rr_medium_path_tracer.h](source/integrators/rr_medium_path_tracer.h)

**Purpose**: Variance reduction via Russian roulette termination

**Configuration**:
```cpp
rr_medium_path_tracer(int max_depth, int rr_depth)
// rr_depth = depth to start Russian roulette
// In main.cpp: (info.max_depth() * 3) / 4
```

**RR Mechanism**:
```cpp
if (depth >= m_rr_start_depth) {
    double p = clamp(max_component(throughput));  // [0.05, 0.95]
    if (random() > p) return final_color;  // Terminate
    throughput /= p;  // Importance reweight
}
```

#### D. `mis_medium_path_tracer` (Multiple Importance Sampling)
**Location**: [source/integrators/mis_medium_path_tracer.h](source/integrators/mis_medium_path_tracer.h)

**Purpose**: Unbiased variance reduction via balanced MIS weighting

**Configuration**:
```cpp
mis_medium_path_tracer(int max_depth, int rr_depth, int direct_samples)
// direct_samples = #NEE samples per surface bounce
// In main.cpp: typically 5
```

**Key Differences from Simple**:
1. **Direct Lighting (NEE)**: Explicitly samples light sources via `direct_light_sampler`
2. **Weight Balancing**: Power heuristic `w = pdf_bsdf^2 / (pdf_bsdf^2 + pdf_light^2)`
3. **State Tracking**: `path_state.last_bounce_used_nee` flags if bounce sampled light
4. **Explicit Light Check**: Only applies NEE to non-delta, non-light surfaces

### MIS Implementation Deep Dive

#### Direct Sampling (NEE - Next Event Estimation)

**Surface Direct Lighting** (lines ~290-335):

```cpp
color get_surface_direct_result(const ray& r, const surface_hit_rec& rec, const world& world) const {
    point3 p = rec.m_intersection.get_p();
    vec3 n = rec.get_normal();
    
    for (int i = 0; i < m_direct_samples; i++) {
        local_light_sample ls;
        if (!direct_light_sampler::sample_from_point(world, p, ls)) break;
        
        // Shadow test to light
        ray shadow_ray = ray(p, ls.m_direction);
        interval shadow_t = interval(epsilon, ls.m_distance - epsilon);
        
        if (!world.m_surfaces->surface_hit_check(shadow_ray, shadow_t)) {
            // Also check media transmittance
            color transmittance = medium_sampler::transmittance_homogeneous(...);
            
            // BSDF * cosine * light_radiance
            double cos_mat = dot(shadow_ray.direction(), n);
            color bsdf = mat->bsdf(isect, -r.direction(), shadow_ray.direction());
            // ... accumulate weighted by geometry and pdf
        }
    }
    return direct;
}
```

**Medium Direct Lighting** (in similar pattern for volumetric scattering)

#### MIS Weighting at Surface Hits

**When Surface is NOT Emissive Light**:
```cpp
const bool used_nee_here = (!rec.m_is_explicit_light && !rec.m_mat->is_delta());
color direct = colors::black;
if (used_nee_here) {
    direct = get_surface_direct_result(r, rec, world);
}
```

**When Surface IS Emissive Light**:
```cpp
color surface_emittance = rec.m_mat->emission(rec.m_intersection);
if (rec.m_is_explicit_light && p_state.last_bounce_used_nee) {
    // We sampled light directly, so downweight emittance by MIS
    double light_pdf_w = direct_light_sampler::pdf_w(world, r.origin(), r.direction());
    double bsdf_pdf = p_state.prev_bsdf_pdf;
    double w = power_heuristic(bsdf_pdf, m_direct_samples * light_pdf_w);
    surface_emittance *= w;
}
// power_heuristic = (pdf1^2) / (pdf1^2 + pdf2^2)
```

**Skybox Weighting**:
```cpp
if (p_state.last_bounce_used_nee) {
    double light_pdf_w = direct_light_sampler::pdf_w(world, r.origin(), r.direction());
    double w = power_heuristic(bsdf_pdf, m_direct_samples * light_pdf_w);
    col_from_sky *= w;
}
```

#### Indirect Sampling (BSDF Sampling)

```cpp
path_result get_indirect_result(const ray& r, const world& world, 
                                const path_state& p_state,
                                const surface_scatter_rec& srec,
                                const surface_hit_rec& rec, 
                                const bool used_nee) const {
    path_state child_state = p_state;
    child_state.depth++;
    child_state.last_bounce_used_nee = used_nee;  // Track current bounce
    
    if (srec.is_delta) {
        // Specular: no MIS needed, just follow direction
        child_state.overall_throughput = srec.bsdf * child_state.overall_throughput;
        // Note: don't update prev_bsdf_pdf for specular
    }
    else {
        // Diffuse/Glossy: compute throughput with cosine weighting
        double cos_theta = dot(scatter_dir, rec.get_normal());
        color throughput = srec.bsdf * cos_theta / srec.w_pdf;
        child_state.prev_bsdf_pdf = srec.w_pdf;  // Store for next loop's MIS
        child_state.overall_throughput *= throughput;
    }
    
    return path_trace(srec.s_ray, world, child_state);
}
```

#### Key MIS Properties

| Property | Implementation |
|----------|-----------------|
| **Unbiasedness** | Power heuristic with balanced weight sums to 1 |
| **Variance Reduction** | Much lower variance than pure BSDF sampling on lights |
| **Delta Handling** | Specular surfaces bypass MIS (correct: delta has infinite pdf) |
| **Skybox Integration** | Environment lights weighted same as surface lights |
| **Media Support** | MIS extends to medium direct & indirect |

#### Known Limitations (From TODOs)

```cpp
//TODO: This part isn't correct now. We aren't downweighting for MIS at the right place. 
//      Better than nothing though..
//TODO: Only truly fixed with bidirectional path tracing!
```
These comments indicate that perfect MIS for light emission is complex—next-event estimation ideally needs counterpart light path sampling (bidirectional).

---

## 3. PATH STATE & DATA FLOW

### `path_state` Structure

**Location**: [source/records/path_state.h](source/records/path_state.h)

```cpp
struct path_state {
    // Light transport weighting
    vec3 overall_throughput;           // Accumulated color throughput (V from above)
    double prev_bsdf_pdf;              // BSDF PDF from previous bounce (for MIS)
    
    // Control flow
    int depth;                         // Current bounce count
    
    // MIS tracking
    bool last_bounce_used_nee = false; // Did prev bounce use direct sampling?
    
    // Unused fields
    vec3 prev_vertex_position;         // For future use
    double rr_probability;             // Not actively used
    
    static path_state initial_path_state() {
        path_state p;
        p.depth = 0;
        p.overall_throughput = colors::white;  // RGB(1,1,1)
        p.prev_bsdf_pdf = 1.0;
        p.last_bounce_used_nee = false;
        return p;
    }
};
```

**Role in Path Tracing**:

The state flows through the recursion tree, capturing critical MIS information:

```
Camera Ray
    → path_trace(ray, world, state_0: depth=0, throughput=1, last_nee=false)
        ├─ Surface NEE: sample light directly
        ├─ Recurse: state_1: depth=1, prev_pdf=bsdf_pdf_0, last_nee=true
        │   → Light hit: apply MIS weight using prev_bsdf_pdf
        └─ OR Indirect BSDF: 
            └─ Recurse: state_1: depth=1, prev_pdf=bsdf_pdf_0, last_nee=false
                → Next bounce NEE: uses prev_bsdf_pdf for weight calculation
```

### `path_result` Structure

**Location**: [source/records/path_result.h](source/records/path_result.h)

```cpp
struct path_result {
    color radiance_from_path;          // Final contribution from this path segment
    bool terminated_on_light;          // Did path end explicitly on a light?
    
    static path_result color_path_result(color c);
    static path_result empty_path_result();  // Returns black
};
```

**Usage**: Carries final radiance + termination flag back up recursion

### Key Data Structures for Path Information

| Structure | Location | Purpose |
|-----------|----------|---------|
| `surface_hit_rec` | records/surface_hit_rec.h | Surface intersection: position, normal, material, light flag |
| `medium_hit_rec` | records/medium_hit_rec.h | Media intersection: scatter vs absorb, transmittance, σ_s |
| `surface_scatter_rec` | records/surface_scatter_rec.h | Scattered ray info: direction, BSDF, PDF, is_delta flag |
| `medium_scatter_rec` | records/medium_scatter_rec.h | Phase function result: direction, phase_pdf, w_pdf |
| `intersection` | records/intersection.h | Ray-surface hit: t, position, time, UV coords |
| `interaction` | records/interaction.h | Basic hit info: t, position, time (subset of intersection) |

---

## 4. SAMPLING SYSTEMS

### A. Direct Light Sampler

**Location**: [source/samplers/direct_light_sampler.h](source/samplers/direct_light_sampler.h)

**Purpose**: Importance sample the light sources (surface lights + environment) from a given point

#### Method 1: `sample_from_point()`

Used for surface NEE in path tracing:

```cpp
static bool sample_from_point(const world& world, const point3& from, 
                              local_light_sample& light_sample)
```

**Process**:
1. Query scene for available light sources:
   - `sky->get_flux_weight()` — environment light intensity weighting
   - `lights->get_flux_weight()` — surface light intensity weighting
2. Choose light class proportional to flux: `sample_seed < sky_weight ? env : surface`
3. Sample specific light within class by flux:
   - Environment: `sky->sample_light_over_flux(p)` → sampled direction
   - Surface: `lights->sample_light_over_flux(random, p)` → sampled surface point
4. Return `local_light_sample`:
   ```cpp
   struct local_light_sample {
       color m_radiance;          // Emitted light color
       vec3 m_direction;          // Direction to sampled point
       double m_pdf_w;            // Probability density (solid angle)
       double m_distance;         // Distance to light
       double m_geometry_term;    // Geometry factor (cos/dist²)
       bool m_is_env_light;       // Type flag
   };
   ```

#### Method 2: `pdf_w()`

Query probability density of a given light direction:

```cpp
static double pdf_w(const world& world, const point3& from, const vec3& direction)
```

Used for MIS weighting—evaluates the probability that `sample_from_point()` would have selected the given direction.

#### Method 3: `sample_from_scene()`

For bidirectional/light path tracing (less used in current system):

```cpp
static bool sample_from_scene(const world& world, light_ray_sample& light_sample)
```

Samples a light ray originating from light sources.

#### Light Sampling Strategy

**Flux-Weighted Importance Sampling**:
```cpp
const double sky_weight = sky ? sky->get_flux_weight() : 0.0;
const double lights_weight = lights ? lights->get_flux_weight() : 0.0;
const double flux_sum = sky_weight + lights_weight;

// Choose light class
if (sample_seed < sky_weight || lights_weight <= 0.0) {
    // Environment
    double p = sky_weight / flux_sum;  // Class PDF
    env_sample = sky->sample_light_over_flux(p);
} else {
    // Surface lights
    double p = lights_weight / flux_sum;
    surface_sample = lights->sample_light_over_flux(new_seed, p);
}
```

**Flux** = Total emitted power:
- For surfaces: `luminance(emission * surface_area)`
- For environment: cubemap integral

---

### B. Medium Sampler

**Location**: [source/samplers/medium_sampler.h](source/samplers/medium_sampler.h)

**Purpose**: Sample distance to a scattering/absorption event in media

#### Core Algorithm: `sample_distance()`

```cpp
static bool sample_distance(const ray& r, medium_intersections& intersections,
                            const interval& t, medium_hit_rec& rec)
```

**Input**:
- `r`: Ray through media
- `intersections`: Collection of overlapping media regions (with entry/exit intervals)
- `t`: Ray segment to sample within

**Output**:
- `rec`: Populated with event info (position, scatter vs absorb, transmittance)
- Returns: `true` if event occurred; `false` if ray passes through without event

#### Physical Basis: Null Collision Tracking

This uses **null collision tracking** (also called "decomposition sampling") to unify multiple overlapping media:

1. **Pre-process**: Pack overlapping media into `medium_slice` objects:
   ```cpp
   // Slice = [t_min, t_max] where set of active media is constant
   // Example: [0, 2] with Media A
   //          [2, 5] with Media A + B
   //          [5, 10] with Media B
   ```

2. **Per-slice handling**:
   ```cpp
   for each slice {
       // Compute majorant extinction: max(σ_t across all media)
       sigma_maj = max(σ_t_A, σ_t_B, ...);
       
       while slice_traversal {
           // Sample free-flight from majorant
           dt = -log(1 - random()) / sigma_maj;
           
           if (cursor_t + dt > slice_end) {
               // Transmit through rest of slice
               transmittance *= exp(-σ_t * (slice_end - cursor_t));
               break;
           }
           
           // We have a potential event at cursor_t + dt
           // Check if it's a real collision (null collision rejection)
           if (random() > σ_t / sigma_maj) {
               // Null collision - keep marching
               continue;
           }
           
           // Real event - choose which medium caused it
           i_chosen = sample_weighted(σ_t_1, σ_t_2, ...)
           
           // Choose scatter vs absorb in chosen medium
           p_scatter = σ_s / σ_t;
           if (random() < p_scatter) {
               rec.m_is_scatter = true;
               // Phase function will determine direction next
           } else {
               rec.m_is_scatter = false;  // Absorption
           }
           return true;  // Event found
       }
   }
   
   // No event in any slice
   return false;
   ```

**Produces**:
- `rec.m_transmittance`: RGB transmittance from ray origin to event
- `rec.m_transmittance_pdf_scalar`: Probability scalar used for weighting
- `rec.m_sigma_s`: Scattering cross-section at event point
- `rec.m_is_scatter`: Boolean event type

#### Transmittance Calculation: `transmittance_homogeneous()`

For shadow rays, compute color attenuation without event sampling:

```cpp
color transmittance = colors::white;
for each slice {
    double distance = slice.max - slice.min;
    transmittance *= exp(-sigma_t_total * distance);
}
return transmittance;
```

---

## 5. SCENE COMPONENTS

### Scene Hierarchy

```
scene
├─ camera
└─ world
   ├─ surfaces (hittable)
   │  ├─ geometric shapes
   │  └─ light sources (marked explicit_light)
   ├─ mediums (hittable)
   │  └─ volumetric regions
   ├─ lights (surface reference)
   │  └─ subset of surfaces that emit
   └─ sky (skybox)
      └─ environment light
```

### `world` Structure

**Location**: [source/scene/world.h](source/scene/world.h)

```cpp
class world {
public:
    shared_ptr<surface> m_surfaces;      // All geometry
    shared_ptr<medium> m_mediums;        // All volumes
    shared_ptr<surface> m_lights;        // Light references
    shared_ptr<skybox> m_sky;            // Environment
    double m_furthest_distance;          // Scene radius
    
    void accelerate() {
        m_surfaces = make_shared<surface_tree_node>(
            make_shared<surface_list>(m_surfaces)
        );
    }
};
```

**Acceleration**: Wraps surfaces in a BVH tree for fast ray tracing.

### `scene` Structure

**Location**: [source/scene/scene.h](source/scene/scene.h)

```cpp
class scene {
public:
    camera m_cam;
    world m_world;
    
    void finalize() {
        m_world.m_surfaces->compute_properties();  // BBox, flux calc, etc.
        m_world.m_lights->compute_properties();
        m_world.m_lights->set_explicit_light(true);  // Mark all lights
    }
    
    void render(filename, info, integrator, post, writers...);
};
```

**Initialization**: `finalize()` pre-computes light properties for sampling.

### Hit Interfaces

#### Surface Ray Testing

```cpp
class surface : public hittable {
    virtual bool surface_hit(const ray& r, interval ray_t, 
                            surface_hit_rec& rec) const = 0;
    
    virtual bool surface_hit_check(const ray& r, interval ray_t) const = 0;
    // hit_check is shadow ray optimization—occlusion test only
};
```

#### Medium Ray Testing

```cpp
class medium : public hittable {
    virtual bool medium_hit(const ray& r, interval ray_t, 
                           medium_intersections& recs) const = 0;
};
```

Returns **all** media regions a ray passes through (as intervals), not just the closest.

---

## 6. MATERIALS & SCATTERING

### Surface Materials

**Location**: [source/scene/material/surface_material.h](source/scene/material/surface_material.h)

```cpp
class surface_material {
public:
    // PDF-aware BSDF evaluation
    virtual color bsdf(const intersection& i, const vec3& in, 
                       const vec3& out) = 0;
    
    // Probability density of direction 'out' given incoming 'in'
    virtual double pdf(const intersection& i, const vec3& in, 
                       const vec3& out) = 0;
    
    // Generate random outgoing direction
    virtual bool scatter(const intersection& i, const vec3& in, 
                        surface_scatter_rec& srec) = 0;
    
    // Emitted light
    virtual color emission(const intersection& i) const = 0;
    
    // Type queries
    virtual bool is_delta() const = 0;
    virtual color get_radiance() const = 0;
};
```

**Key Design**:
- `scatter()` generates direction + calculates PDF in one call (more efficient)
- `bsdf()` evaluates color given direction (decoupled for MIS queries)
- `is_delta()` flags specular surfaces (Dirac delta BSDFs)

**Scatter Record** (`surface_scatter_rec`):

```cpp
struct surface_scatter_rec {
    ray s_ray;              // Outgoing ray
    color bsdf;             // BSDF value at this direction
    double w_pdf;           // PDF of direction sample
    bool is_delta;          // Specular flag
};
```

### Medium Materials

**Location**: [source/scene/material/medium_material.h](source/scene/material/medium_material.h)

```cpp
class medium_material {
public:
    // Cross-sections (properties, not probabilities)
    virtual color sigma_a(const point3& p) const;  // Absorption
    virtual color sigma_s(const point3& p) const;  // Scattering
    virtual color sigma_t(const point3& p) const;  // Total = a + s
    virtual color sigma_maj() const;               // Majorant (for null collision)
    
    // Phase function: direction distribution of scattered photons
    virtual void scatter(const vec3& in_dir, 
                        medium_scatter_rec& srec) const;
    virtual color phase(const interaction& i, const vec3& in, 
                        const vec3& out) const;
    virtual double phase_pdf(const interaction& i, const vec3& in, 
                            const vec3& out) const;
    
    // Emission (for emitted media, e.g., fire)
    virtual color emission(const point3& p) const;
};
```

**Cross-Section Units**: Power per unit distance per unit volume
- **σ_a**: Probability of absorption per unit distance
- **σ_s**: Probability of scattering per unit distance
- **σ_t = σ_a + σ_s**: Total extinction coefficient

**Phase Function**: Encodes anisotropic scattering (e.g., Henyey-Greenstein for smoke)

---

## 7. CAMERA & RENDERING LOOP

### Camera Rendering

**Location**: [source/scene/camera.h](source/scene/camera.h)

```cpp
void render(const string& filename, const world& world, image_info info,
            const integrator& itgr, post_process& post, 
            image_writer& preview, image_writer& result) {
    // For each pixel (i, j):
    for (int sample = 0; sample < spp; sample++) {
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                // Generate primary ray
                ray r = get_ray(i, j);
                
                // Trace through integrator
                color pixel_color = itgr.ray_color(r, max_depth, world);
                
                // Accumulate into buffer
                accum_buffer[pixel] += pixel_color;
                
                // Periodic preview update
                if (work_count++ % preview_freq == 0) {
                    update_preview();
                }
            }
        }
    }
    
    // Tone-map and save
    accum_to_display(display_buff, accum_buff, post, spp);
    result_writer.write(filename, display_buff);
}
```

**Sampling**: Unbiased Monte Carlo averaging—multiple paths per pixel reduce variance.

**Preview/Result Writers**: Abstract interface for image output formats (PNG, PPM, etc.).

---

## 8. CONTROL FLOW: DETAILED MIS PATH TRACE EXAMPLE

### Example: Ray hits diffuse surface with light in scene

```
camera.render() per pixel
├─ ray_color(primary_ray, max_depth=8, world)
│
├─ path_trace(primary_ray, world, state₀)
│  ├─ depth=0, throughput=(1,1,1), last_nee=false
│  │
│  ├─ surface_hit_check() → Found: wall at t=2.0
│  ├─ medium_hit() → No media
│  │
│  ├─ GET SURFACE PROPERTIES
│  │  ├─ Is emissive? No (m_is_explicit_light=false)
│  │  ├─ Is delta? No (diffuse material)
│  │  └─ Not a light, so used_nee_here = true
│  │
│  ├─ DIRECT LIGHTING (NEE)
│  │  └─ get_surface_direct_result()
│  │     ├─ For i=1 to m_direct_samples (5):
│  │     │  ├─ direct_light_sampler::sample_from_point()
│  │     │  │  ├─ Flux-weight sky vs surfaces
│  │     │  │  └─ Choose light, return direction & pdf
│  │     │  │
│  │     │  ├─ Cast shadow ray to light
│  │     │  ├─ Check surface occlusion → Clear
│  │     │  ├─ Check media transmittance → transmittance=(0.9,0.9,0.9)
│  │     │  │
│  │     │  ├─ BSDF evaluation:
│  │     │  │  ├─ cos_term = dot(normal, to_light) = 0.8
│  │     │  │  ├─ bsdf = mat->bsdf(isect, -primary_ray, to_light)
│  │     │  │  │           = (0.2, 0.2, 0.2) for gray diffuse
│  │     │  │  └─ light_radiance = (5, 5, 5)
│  │     │  │
│  │     │  └─ Accumulate: direct += bsdf * cos * light * transmittance
│  │     │                         = (0.2)(0.8)(5)(0.9) = 0.72 per channel
│  │     │
│  │     └─ Return direct ≈ (0.72, 0.72, 0.72) × 5 samples / 5 = ~0.72
│  │
│  ├─ CHECK TERMINATION
│  │  ├─ depth + 1 = 1 < 8 ✓
│  │  └─ No Russian roulette yet (MIS doesn't use it)
│  │
│  ├─ INDIRECT SAMPLING (BSDF)
│  │  ├─ mat->scatter(isect, -primary_ray, srec)
│  │  │  ├─ is_delta = false (diffuse)
│  │  │  ├─ Sample direction from cosine distribution
│  │  │  ├─ srec.w_pdf = cos_theta / π for Lambertian
│  │  │  └─ srec.s_ray = ray from wall into scene
│  │  │
│  │  └─ get_indirect_result()
│  │     ├─ cos_theta = dot(new_direction, normal) = 0.6
│  │     ├─ throughput = bsdf(0.2) * cos(0.6) / pdf(≈0.2)
│  │     │             ≈ (0.2 * 0.6 / 0.2) = (0.6)
│  │     │
│  │     ├─ Create child state:
│  │     │  ├─ depth = 1
│  │     │  ├─ prev_bsdf_pdf = 0.2 (for next bounce MIS)
│  │     │  ├─ overall_throughput *= (0.6)  → (0.6, 0.6, 0.6)
│  │     │  └─ last_bounce_used_nee = true (we did NEE this bounce)
│  │     │
│  │     └─ path_trace(new_ray, world, state₁)
│  │        ├─ Recursively traces...
│  │        ├─ Eventually hits ceiling light at depth=2
│  │        ├─ Ceiling is explicit light + last_bounce_used_nee=true
│  │        │  → Apply MIS weight to emittance:
│  │        │  ├─ light_pdf_w = direct_light_sampler::pdf_w(...)
│  │        │  ├─ bsdf_pdf = state₁.prev_bsdf_pdf = 0.2
│  │        │  ├─ w = power_heuristic(0.2, 5 * light_pdf_w)
│  │        │  └─ ceiling_emittance *= w  (downweight if light was easy to hit)
│  │        │
│  │        └─ Return result
│  │
│  ├─ ACCUMULATE
│  │  ├─ surface_emittance = 0 (wall doesn't emit)
│  │  ├─ transmittance from media = (1, 1, 1)
│  │  ├─ total = (surface_emittance + direct) * transmittance
│  │  │         + indirect_result * throughput * transmittance
│  │  │         ≈ (0, 0, 0) + (0.72, 0.72, 0.72) * 1.0
│  │  │           + indirect_contrib * (0.6, 0.6, 0.6) * 1.0
│  │  └─ Return to camera
│  │
│  └─ Return final pixel color
│
└─ Accumulate into image buffer
   (repeat for multiple samples per pixel)
```

---

## 9. KEY ALGORITHMS & EQUATIONS

### 1. **Path Throughput**

$$\text{throughput} = \prod_{i=0}^{n} \frac{\text{BSDF}_i \cdot \cos_i}{\text{pdf}_i}$$

Where each BSDF evaluation is importance-sampled with solid angle PDF.

### 2. **Power Heuristic (MIS Weight)**

$$w(x, y) = \frac{x^2}{x^2 + y^2}$$

Where:
- $x = \text{pdf from BSDF sampling}$
- $y = \text{pdf from light sampling}$

For multiple light samples $m$:
$$w = \frac{x^2}{x^2 + (m \cdot y)^2}$$

### 3. **Media Transmittance**

$$T(t_0 \to t_1) = \exp\left(-\int_{t_0}^{t_1} \sigma_t(t) \, dt\right)$$

For homogeneous media:
$$T = \exp(-\sigma_t \cdot d)$$

### 4. **Null Collision Sampling**

$$p(\text{scatter at } t) = \frac{\sigma_t(t)}{\sigma_{\text{maj}}} \exp\left(-\int_0^t \sigma_{\text{maj}} \, ds\right)$$

(Majorant ensures valid PDF; null collisions handle variation)

### 5. **Russian Roulette Termination**

$$\text{survival probability} = \text{clamp}(\max(\text{throughput}), 0.05, 0.95)$$

If $r > p$: terminate with $\text{contribution} = 0$

Else: continue with $\text{throughput} /= p$ (unbiased scaling)

---

## 10. DESIGN PATTERNS

### 1. **Policy-Based Integrator Selection**

```cpp
// main.cpp
auto itgr = mis_medium_path_tracer(max_depth, rr_depth, direct_samples);
// Could swap to: simple_path_tracer, rr_medium_path_tracer, simple_medium_path_tracer
```

Enables algorithm swapping without code duplication.

### 2. **Hit Record Containers**

Separate structures for different hit types:
- `intersection` - ray-surface
- `surface_hit_rec` - surface with material
- `medium_hit_rec` - volumetric event
- `medium_intersections` - all overlapping media

Allows type-safe passing through recursion.

### 3. **Scatter Record Pattern**

`scatter()` combines three operations:
1. Generate direction
2. Evaluate PDF
3. Flag delta/non-delta

Prevents redundant calculations.

### 4. **Flux-Weighted Importance Sampling**

Lights sampled proportional to emission power, not surface area. Variance reduction without needing explicit light sampling PDFs in materials.

### 5. **State Threading**

Passing `path_state` by reference through recursion (not copying):
```cpp
void path_trace(..., path_state& p_state) {
    // ... modify p_state ...
    path_result child = path_trace(..., child_state);
}
```

Enables efficient backtick propagation of BSDF PDFs for MIS.

---

## 11. CRITICAL FLOW: FROM RAY SHOT TO FINAL COLOR

```
Main Loop (camera.cpp)
│
├─ For each pixel (i, j):
│  ├─ For each sample s:
│  │  │
│  │  ├─ Generate primary ray via lens
│  │  │  ├─ Sample pixel position (stratified optional)
│  │  │  ├─ Sample lens position (depth of field)
│  │  │  └─ Create ray from camera through pixel
│  │  │
│  │  ├─ integrator->ray_color(ray, max_depth, world)
│  │  │  │
│  │  │  └─ mis_medium_path_tracer::ray_color()
│  │  │     ├─ Initialize path_state (depth=0, throughput=white)
│  │  │     └─ path_trace(ray, world, state)  [RECURSIVE]
│  │  │
│  │  ├─ path_trace(ray, world, state) [depth k]
│  │  │  │
│  │  │  ├─ Early termination?
│  │  │  │  ├─ throughput ≈ 0? return black
│  │  │  │  └─ depth ≥ max? return emittance only
│  │  │  │
│  │  │  ├─ Scene intersection tests
│  │  │  │  ├─ world.m_surfaces->surface_hit(ray, [ε, ∞], rec_surf)
│  │  │  │  ├─ world.m_mediums->medium_hit(ray, [ε, t_surf], recs_med)
│  │  │  │  └─ Resolve which event is closest
│  │  │  │
│  │  │  ├─ Handle hits:
│  │  │  │  │
│  │  │  │  ├─ No hit:
│  │  │  │  │  └─ return skybox color (with media transmittance)
│  │  │  │  │
│  │  │  │  ├─ Medium hit (before surface):
│  │  │  │  │  ├─ Sample event via medium_sampler::sample_distance()
│  │  │  │  │  ├─ If absorption: return emittance
│  │  │  │  │  ├─ If scatter:
│  │  │  │  │  │  ├─ Direct: get_medium_direct_result()
│  │  │  │  │  │  ├─ Phase function scatter
│  │  │  │  │  │  ├─ Update throughput with σ_s/σ_t
│  │  │  │  │  │  └─ Recurse with depth++ and new state
│  │  │  │  │  │
│  │  │  │  │  └─ Return direct + indirect*throughput
│  │  │  │  │
│  │  │  │  └─ Surface hit:
│  │  │  │     ├─ Apply media transmittance from ray segment
│  │  │  │     ├─ Query emittance
│  │  │  │     ├─ If not delta & not light: NEE sampling
│  │  │  │     │  └─ direct_light_sampler::sample_from_point()
│  │  │  │     │     × m_direct_samples times
│  │  │  │     │     Shadow test + media transmittance
│  │  │  │     │     Accumulate weighted by BSDF*geometry/pdf
│  │  │  │     │
│  │  │  │     ├─ Scatter via material->scatter()
│  │  │  │     ├─ If delta:
│  │  │  │     │  └─ Bypass MIS, just follow direction
│  │  │  │     │
│  │  │  │     ├─ Else (non-delta):
│  │  │  │     │  ├─ Compute throughput = BSDF*cos(θ)/pdf
│  │  │  │     │  ├─ Store prev_bsdf_pdf in child_state
│  │  │  │     │  └─ Mark child_state.last_bounceused_nee = true
│  │  │  │     │
│  │  │  │     └─ Recurse: path_trace(scattered_ray, world, child_state)
│  │  │  │
│  │  │  ├─ Accumulate contributions
│  │  │  │  └─ emittance + direct + (indirect × throughput)
│  │  │  │     × media_transmittance
│  │  │  │
│  │  │  └─ Return path_result
│  │  │
│  │  ├─ Store pixel color in accumulation buffer
│  │  └─ Periodic preview updates
│  │
│  └─ Repeat samples
│
├─ Post-processing (tone mapping, gamma correction)
└─ Write final image
```

---

## 12. VARIANCE REDUCTION TECHNIQUES

### Implemented

| Technique | Method | Status |
|-----------|--------|--------|
| **Next Event Estimation (NEE/Direct Sampling)** | Explicit light sampling on non-specular surfaces | ✓ Active in MIS |
| **Multiple Importance Sampling (MIS)** | Power heuristic weighting for direct vs BSDF | ✓ Active in MIS |
| **Russian Roulette** | Probabilistic termination after depth N | ✓ Optional (RR variant) |
| **Null Collision Tracking** | Majorant sampling for overlapping media | ✓ In medium_sampler |
| **Stratified Sampling** | - | (Camera supports but not detailed) |
| **Transmittance Caching** | - | (TODO potentially) |

### Not Implemented

- **Bidirectional Path Tracing**: Required for perfect light transport MIS
- **Photon Mapping**: Alternative density estimation
- **Metropolis Sampling**: Markov chain for correlated paths
- **Quasi-Monte Carlo**: Low-discrepancy sequences

---

## 13. KNOWN LIMITATIONS & TODOs

### From Code Comments

1. **Emittance MIS Weighting**:
   ```cpp
   //TODO: This part isn't correct now. We aren't downweighting for MIS at the right place. 
   //      Better than nothing though..
   //TODO: Only truly fixed with bidirectional path tracing!
   ```
   *Issue*: When hitting a light via BSDF, accounting for both BSDF and light sampling paths is incomplete.

2. **Medium Swell Timing** (in rr & simple_medium):
   ```cpp
   //TODO: This weighting here is for MVP purposes
   ```
   Medium throughput weighting may not perfectly implement MIS.

3. **Uninitialized Values**:
   Many structures use `uninit` and `uninit_vec` macros rather than explicit initialization. While intentional (catch use-before-set bugs), it indicates development-stage code.

### Design Considerations

- **Code Size**: `mis_medium_path_tracer.h` is quite large (300+ lines) due to method inlining
- **Potential Refactoring**: The integrators have significant code duplication; could benefit from base class extraction
- **Media Handling**: Overlapping media supported, but performance on complex volumetric scenes untested

---

## 14. INTEGRATION SUMMARY TABLE

| Component | Purpose | Key Classes/Structs | Integration Points |
|-----------|---------|-------------------|-------------------|
| **Integrators** | Rendering algorithms | `mis_medium_path_tracer`, `simple_path_tracer`, etc. | Camera calls `ray_color()` |
| **Path State** | Path information | `path_state`, `path_result` | Threaded through recursion |
| **Records** | Hit data | `surface_hit_rec`, `medium_hit_rec`, scatter recs | Populated by scene, used by integrator |
| **Samplers** | Direction/distance selection | `direct_light_sampler`, `medium_sampler` | Called by integrator for NEE & media |
| **Scene** | Geometry & materials | `world`, `surface`, `medium`, `skybox` | Queried for intersections & sampling |
| **Materials** | Scattering properties | `surface_material`, `medium_material` | Called to evaluate BSDF & phase functions |
| **Camera** | Image formation | `camera`, loops over pixels | Calls integrator per sample per pixel |

---

## 15. DETAILED MIS WEIGHT EQUATION

When a light surface is hit and the previous bounce used NEE:

$$w(\text{light hit}) = \frac{(\text{pdf}_{\text{BSDF}})^2}{(\text{pdf}_{\text{BSDF}})^2 + (m_{\text{samples}} \cdot \text{pdf}_{\text{light}})^2}$$

Example scenario:
- BSDF PDF of hitting light via diffuse = 0.15
- Direct light sampler PDF = 0.25
- Direct samples = 5

$$w = \frac{0.15^2}{0.15^2 + (5 \cdot 0.25)^2} = \frac{0.0225}{0.0225 + 1.5625} \approx 0.014$$

The weight is very small because the light was much easier to hit via direct sampling. This prevents biaasing toward happy accidents.

---

## 16. COMPLETE FILE DEPENDENCY GRAPH

```
main.cpp
├─ benderer.h (common includes)
├─ image/image_info_library.h
├─ scene/scene_library.h
├─ scene/scene.h
│  ├─ scene/camera.h
│  │  └─ integrators/integrator.h
│  │     └─ [all integrator implementations]
│  └─ scene/world.h
│     ├─ scene/hittables/surfaces/surface_list.h
│     ├─ scene/hittables/surfaces/surface_tree.h
│     ├─ scene/hittables/mediums/medium.h
│     └─ scene/skyboxes/skybox.h
├─ integrators/mis_medium_path_tracer.h
│  ├─ records/path_state.h
│  ├─ records/path_result.h
│  ├─ records/surface_hit_rec.h
│  ├─ records/medium_hit_rec.h
│  ├─ samplers/direct_light_sampler.h
│  └─ samplers/medium_sampler.h
└─ [image writers, post-processing, etc.]
```

---

## 17. SUMMARY: MIS PATH TRACER EXECUTION

### Initialization Phase
1. Scene created with geometry, materials, lights
2. `scene::finalize()` computes light properties (flux, bounding boxes)
3. World accelerated with BVH
4. MIS integrator configured with direct_samples=5, max_depth, rr_depth

### Per-Pixel Rendering
1. Camera generates stratified/jittered primary rays
2. For each sample:
   - Ray traced through `mis_medium_path_tracer::ray_color()`
   - Recursive `path_trace()` computes path contribution
   - Direct lighting (NEE) sampled on non-specular, non-light surfaces
   - BSDF-sampled indirect directions follow
   - Media events sampled via null collision tracking
   - Light hits weighted by MIS power heuristic
   - Results accumulated into framebuffer

### Post-Processing
1. Accumulation buffer averaged by sample count
2. Tone mapping applied
3. Image written to disk

---

## CONCLUSION

The Benderer path tracer is a well-architected, production-quality rendererexhibiting:

- **Modular design** enabling algorithm swapping
- **Complete physics** (surface + volume + environment)
- **MIS variance reduction** for unbiased, low-noise images
- **Sophisticated sampling** (flux-weighted lights, null collision media)
- **Clean state threading** for efficient path information propagation

Its main complexity arises from the unification of surface and volumetric transport, which is handled elegantly through null collision sampling and separated direct/indirect paths. The MIS implementation is well-executed but acknowledges (via TODOs) that perfect light transport would require bidirectional techniques.
