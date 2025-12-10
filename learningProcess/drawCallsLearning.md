# Draw Calls and CPU-GPU Communication

## What Causes CPU-GPU Bottlenecks?

The bottleneck happens when the **CPU can't prepare draw calls fast enough** for the GPU, or when there's too much **state change overhead**.

---

## 1. Too Many Draw Calls ⚠️ (BIGGEST CULPRIT)

**The problem:**
```cpp
// BAD - 10,000 draw calls
for (int i = 0; i < 10000; i++) {
    glBindTexture(...);
    glBindVertexArray(VAOs[i]);
    glDrawElements(6, ...);  // Draw one quad
}
```

Each `glDrawElements()` has overhead:
- CPU validation
- Driver overhead
- Command buffer submission
- GPU state validation

**In Unity terms:** 10,000 game objects with different materials = 10,000 draw calls = slow!

---

## 2. State Changes ⚠️⚠️ (SECOND BIGGEST)

**Cost hierarchy (from most to least expensive):**

```
🔴 Shader program change        (glUseProgram)         - VERY EXPENSIVE
🟠 Texture binding               (glBindTexture)        - EXPENSIVE
🟡 VAO change                    (glBindVertexArray)    - MODERATE
🟢 Uniform updates               (glUniform*)           - CHEAP
🟢 Draw call                     (glDrawElements)       - CHEAP (if no state changes)
```

**Why shader changes are expensive:**
- GPU pipeline flush
- Recompile/relink operations
- Cache invalidation

**Example of expensive rendering:**
```cpp
// TERRIBLE - Constantly switching shaders
for (each object) {
    glUseProgram(object.shader);      // 🔴 Expensive!
    glBindTexture(object.texture);    // 🟠 Expensive!
    glBindVertexArray(object.VAO);    // 🟡 Moderate
    glDrawElements(...);
}
```

**Optimized version:**
```cpp
// GOOD - Sort by shader, then texture
for (each shader) {
    glUseProgram(shader);              // 🔴 Once per shader
    for (each texture using this shader) {
        glBindTexture(texture);        // 🟠 Once per texture
        for (each object using this texture) {
            glBindVertexArray(VAO);    // 🟡 Once per object
            glDrawElements(...);       // Draw
        }
    }
}
```

---

## 3. Sending Vertices 🟢 (Usually NOT the bottleneck)

**Clarification:** You're **NOT sending vertices every frame**!

```cpp
// ONE TIME (during initialization)
glBufferData(GL_ARRAY_BUFFER, vertices.size(), vertices.data(), GL_STATIC_DRAW);
// ↑ Uploads to GPU memory, never sent again

// EVERY FRAME (just references the data already on GPU)
glDrawElements(...);  // No vertex data sent!
```

**Exceptions where vertices ARE sent every frame:**
- Dynamic geometry (particles, procedural meshes)
- Using `GL_DYNAMIC_DRAW` or `GL_STREAM_DRAW`
- Vertex skinning on CPU (bad practice)

---

## 4. Uniform Updates 🟢 (Small data, cheap)

```cpp
glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);  // Sends 64 bytes
```

Uniforms are **tiny** (4x4 matrix = 64 bytes). Not a bottleneck unless you're updating thousands per frame.

---

## The Render Thread (Single Threaded Pipeline)

The rendering pipeline is fundamentally **serial**:

```
CPU Thread (Main):
  Update game logic
  Prepare draw commands
       ↓
  [Command Buffer]
       ↓
GPU Thread (Driver):
  Process commands
  Submit to GPU
       ↓
GPU Hardware:
  Execute draw calls (parallel internally)
  Rasterize
  Fragment shading
```

**Why it's single-threaded:**
- OpenGL is a **state machine** - order matters!
- Can't execute draw call #5 before draw call #4 (z-ordering, blending, etc.)
- GPU executes commands **in order submitted**

**Modern solutions:**
- **Vulkan/DirectX 12**: Allow multi-threaded command buffer recording
- **Command buffers**: CPU prepares commands in advance, GPU processes later
- **Persistent mapped buffers**: Reduce CPU-GPU synchronization

---

## Unity's Specific Problem

**What causes "CPU sending too much to GPU" in Unity:**

### 1. Draw Call Batching Failures

Unity tries to batch objects with same material:

```
❌ 1000 cubes with different materials = 1000 draw calls
✅ 1000 cubes with SAME material = 1 draw call (batched)
```

**Batching breaks when:**
- Different materials/shaders
- Different textures
- Dynamic lighting on some objects
- Scale differences (static batching only)
- Using `renderer.material` (creates instance) instead of `renderer.sharedMaterial`

### 2. SetPass Calls (Shader Program Changes)

**Most expensive operation!** Unity profiler shows "SetPass calls" - this is shader switching.

```
1000 objects, 10 different shaders = 10 SetPass calls
Better than: 1000 objects, 1000 different shaders = 1000 SetPass calls
```

### 3. Per-Object Data

Even with same shader/texture, each object might need:
- Different transform matrix (MVP)
- Different color tint
- Different material properties

**Old way (expensive):**
```cpp
for (each object) {
    glUniform*(object.transform);  // Update uniform
    glDrawElements(...);           // Draw
}
```

**Modern way (GPU instancing):**
```cpp
glDrawElementsInstanced(1000);  // Draw 1000 instances in ONE call
// Transforms stored in instance buffer
```

---

## Concrete Example: Chessboard Implementation

**Current optimized implementation (2 draw calls):**
```cpp
// Draw white squares
whiteSquareTexture_->Bind(0);     // State change
glBindVertexArray(whiteVAO_);     // State change
glDrawElements(96);               // Draw 32 white squares (96 indices)

// Draw black squares
blackSquareTexture_->Bind(0);     // State change
glBindVertexArray(blackVAO_);     // State change
glDrawElements(96);               // Draw 32 black squares
```

**Cost breakdown:**
- Shader change: 0 (same shader both calls)
- Texture change: 2 ⚠️
- VAO change: 2 🟡
- Uniforms: 1 (MVP set once)
- Vertices sent: 0 (already on GPU)
- **Total: Very efficient!**

**Bad implementation (64 draw calls):**
```cpp
// One draw call per square
for (each square) {
    if (isWhite) glBindTexture(whiteTexture);
    else glBindTexture(blackTexture);
    glBindVertexArray(square.VAO);
    glDrawElements(6);  // 6 indices per square
}
```

**Cost:**
- Texture changes: 64 (alternates every square!) 😱
- VAO changes: 64 😱
- Draw calls: 64 😱
- **Much slower!**

---

## Performance Rules of Thumb

**From most to least impactful:**

1. **Minimize draw calls** - Batch geometry with same material
2. **Sort by shader** - Minimize program changes
3. **Sort by texture** - Minimize texture binding
4. **Use instancing** - Draw many objects with one call
5. **Minimize state changes** - Keep same state when possible
6. **Only worry about vertex data if:**
   - Uploading every frame
   - Uploading many MB
   - Using CPU-side skinning

---

## What is a Draw Call?

A **draw call** is a command sent from the CPU to the GPU saying: *"Render geometry NOW with the current OpenGL state"*

The main draw call functions:
- `glDrawElements()` - Draw using index buffer
- `glDrawArrays()` - Draw directly from vertex buffer

### What Gets Sent in a Draw Call?

When you call `glDrawElements()`, you're **NOT sending the VAO/VBO/EBO** - they're already on the GPU!

You're sending:
1. **Which VAO to use** (already bound with `glBindVertexArray()`)
2. **How many vertices/indices to draw**
3. **What OpenGL state to use**:
   - Active shader program
   - Bound textures
   - Active texture units
   - Uniforms (MVP matrix, etc.)
   - Depth testing, blending, etc.

---

## Why Separate Draw Calls for Different Textures?

**GPU rendering pipeline:**

```
Vertex Shader (per vertex)
    ↓
Rasterization
    ↓
Fragment Shader (per pixel) ← Samples from ONE texture at a time
    ↓
Framebuffer
```

**The key limitation:** During a single draw call, the fragment shader can only sample from the **currently bound texture units**.

In a textured fragment shader:
```glsl
uniform sampler2D texture1;  // ONE texture
FragColor = texture(texture1, TexCoord);
```

This samples from **one** texture. You can't say "use texture A for these pixels, texture B for those pixels" in a single draw call.

### Why Can't We Draw Different Textures Together?

**Option 1: What doesn't work**
```
❌ Bind white + black textures
❌ Draw all squares in one call
Problem: GPU doesn't know which squares use which texture
```

**Option 2: Separate draw calls (works)**
```
✅ Bind white texture → Draw white squares (call 1)
✅ Bind black texture → Draw black squares (call 2)
```

**Option 3: Texture Atlas (advanced, works)**
```
✅ Combine both textures into ONE big texture
✅ Use different UV coordinates to sample different parts
✅ Draw all squares in one call

Example:
[White texture | Black texture] ← One big texture
UV (0,0 to 0.5,1) = white
UV (0.5,0 to 1,1) = black
```

---

## State Machine Concept

OpenGL is a **state machine**. Think of it like a big settings panel:
- Current shader: ✅ textured.frag
- Texture unit 0: 🔄 changes between calls
- Active VAO: 🔄 changes between calls
- Depth testing: ✅ enabled
- MVP matrix: ✅ set once

When you call `glDrawElements()`, it uses **whatever state is currently set**.

---

## UV Coordinates and Textures

UV coordinates are **independent of textures**.

- UV (0,0) to (1,1) **always** means "entire texture" regardless of which texture
- White square texture: UV (0,0)→(1,1) = entire white texture
- Black square texture: UV (0,0)→(1,1) = entire black texture

The **same UV coordinates** map to different images when different textures are bound.

---

## Summary

**What causes CPU→GPU bottleneck:**
- ❌ Too many draw calls (thousands)
- ❌ Excessive shader switching (SetPass calls)
- ❌ Excessive texture switching
- ⚠️ Many VAO switches (moderate cost)
- ✅ Vertex data (only if uploaded every frame)
- ✅ Uniforms (tiny data, cheap)

**The render thread IS linear** - GPU processes commands in order. Modern APIs (Vulkan/DX12) allow multi-threaded command recording, but OpenGL is fundamentally single-threaded.

**Optimized chessboard: 2 draw calls for 64 squares** - Very efficient!

---

## Shader Switching vs Shader Complexity

These are **two separate bottlenecks** affecting different parts of the pipeline:

**Shader switching** = CPU/Driver bottleneck
**Shader complexity** = GPU bottleneck

### Shader Switching (CPU Bottleneck)

**The problem with many shaders:**

```cpp
// TERRIBLE for CPU
for (each object) {
    glUseProgram(object.shader);  // 🔴 EXPENSIVE CPU operation
    glDrawElements(...);
}
```

**Why `glUseProgram()` is expensive:**
- GPU pipeline flush (discard work in flight)
- State validation
- Uniform location lookup reset
- Shader cache invalidation
- Driver overhead

**Cost is PER SWITCH, not per complexity:**
- Switching between 2 simple shaders: Expensive
- Switching between 2 complex shaders: Expensive (same cost!)
- **The complexity doesn't matter for switching cost**

### Shader Complexity (GPU Bottleneck)

**How shader complexity affects performance:**

```glsl
// Simple shader - runs FAST per pixel
void main() {
    FragColor = texture(tex, UV);  // 1 texture lookup
}

// Complex shader - runs SLOW per pixel
void main() {
    // 100 instructions per pixel!
    vec3 normal = calculateNormal();
    vec3 lighting = calculatePBRLighting();  // 10 texture lookups
    vec3 reflections = calculateReflections();
    vec3 fog = calculateVolumetricFog();
    FragColor = vec4(normal + lighting + reflections + fog, 1.0);
}
```

**GPU cost = Shader complexity × Number of pixels**

Example:
- 1920×1080 screen = 2,073,600 pixels
- Complex shader runs 2 million times per frame!
- If shader takes 2x longer → entire frame takes 2x longer

---

## One Complex vs Multiple Simple Shaders

### Scenario A: One Complex Shader (90% + 10%)

```cpp
// ONE shader for everything
glUseProgram(complexShader);
glDrawElements(allObjects);  // 1 draw call
```

**Cost:**
- CPU: 1 shader switch ✅ Minimal
- GPU: Complex shader runs on **100% of pixels** ❌ Wasteful

**Problem:** 90% of objects don't need complex features, but they pay the full cost anyway!

```glsl
// Complex shader with branches
if (usePBR) {
    // 100 instructions (only needed for 10%)
} else {
    // Simple path (needed for 90%)
}
```

**Branches in shaders are BAD:**
- GPU runs BOTH paths on some pixels (SIMD architecture)
- Still pays most of the cost even on simple objects

### Scenario B: Multiple Simple Shaders (90% simple, 10% complex)

```cpp
// Simple shader for 90% of objects
glUseProgram(simpleShader);
glDrawElements(simpleObjects);  // 1 draw call

// Complex shader for 10% of objects
glUseProgram(complexShader);
glDrawElements(complexObjects);  // 1 draw call
```

**Cost:**
- CPU: 2 shader switches ⚠️ Some overhead
- GPU: Simple shader on 90%, complex on 10% ✅ Optimal

**Result:** Much faster overall! The GPU isn't wasting time on unnecessary calculations.

### The Math

Let's use real numbers:

**Scenario A: One complex shader**
```
CPU cost: 1 switch = 0.01ms
GPU cost: Complex shader on 2M pixels = 10ms
Total: 10.01ms (99 FPS)
```

**Scenario B: Two shaders (simple + complex)**
```
CPU cost: 2 switches = 0.02ms
GPU cost:
  - Simple shader on 1.8M pixels (90%) = 1.8ms
  - Complex shader on 0.2M pixels (10%) = 1ms
Total: 2.82ms (354 FPS) 🚀
```

**Scenario B is 3.5× faster!**

### When to Use One Shader vs Multiple

**Use ONE shader when:**
- ✅ All objects need the same features
- ✅ Very few objects (< 10 draw calls total)
- ✅ Shader is simple regardless
- ✅ Mobile/low-end hardware (driver overhead is worse)

**Use MULTIPLE shaders when:**
- ✅ Different objects need different features
- ✅ Significant complexity difference (simple vs PBR)
- ✅ Can batch objects with same shader
- ✅ Desktop/console hardware (can handle switches better)

### Practical Guidelines

**Minimize Shader Switches**
```cpp
// BAD: Random order
drawObject1(shader_A);
drawObject2(shader_B);
drawObject3(shader_A);  // Switch back!
drawObject4(shader_B);  // Switch again!

// GOOD: Sort by shader
drawObject1(shader_A);
drawObject3(shader_A);
drawObject2(shader_B);
drawObject4(shader_B);
```

**Target: 5-20 shader switches per frame**
- < 5: Probably over-batching, GPU might be idle
- 5-20: Sweet spot ✅
- 20-100: Okay for complex scenes
- > 100: Problem, need better batching

---

## Unity Material Instancing and Shader Variants

### What Unity Does Behind the Scenes

When you modify **any** per-object rendering property, Unity creates a **material instance**:

```csharp
// In your code (BAD - creates instance!)
renderer.material.color = Color.red;  // ❌ Creates unique material instance

// What Unity does internally:
Material instance = new Material(sharedMaterial);
instance.color = Color.red;
renderer.material = instance;  // Now this object has its own material
```

**Result:** That object can NO LONGER be batched with others!

### Real-World Example: 3 Identical Fences

**Scenario:** 3 identical fences, same material, but different settings:

```
Fence 1: Shadows ON,  Realtime lighting
Fence 2: Shadows OFF, Baked lighting
Fence 3: Shadows ON,  Realtime lighting
```

**What Unity Does:**

```cpp
// Unity's rendering order (simplified):

// Fence 1: Unique rendering path
glUseProgram(shader_variant_realtime_shadows);
glDrawElements(fence1);

// Fence 2: DIFFERENT rendering path
glUseProgram(shader_variant_baked_noshadows);  // ⚠️ Shader switch!
glDrawElements(fence2);

// Fence 3: Back to first variant
glUseProgram(shader_variant_realtime_shadows);  // ⚠️ Shader switch!
glDrawElements(fence3);

// Total: 3 draw calls, 3 shader switches for 3 identical meshes!
```

**Without the differences:**
```cpp
// If all 3 fences had identical settings:
glUseProgram(shader_variant_realtime_shadows);
glDrawElements(allThreeFences);  // ✅ 1 draw call (batched!)
```

### Unity's Shader Variant System

Unity doesn't create entirely new shaders, but it does create **shader variants** (which are effectively different compiled programs).

**How Shader Variants Work:**

```glsl
// Unity's generated variants:

#ifdef REALTIME_LIGHTING
    // Calculate dynamic lighting
#else
    // Use baked lightmap
#endif

#ifdef SHADOWS_ON
    // Calculate shadow sampling
#endif
```

Unity compiles **every combination:**
```
variant_1: REALTIME + SHADOWS_ON
variant_2: REALTIME + SHADOWS_OFF
variant_3: BAKED + SHADOWS_ON
variant_4: BAKED + SHADOWS_OFF
```

**Each variant = a different `glUseProgram()` call!**

---

## Common Unity Performance Killers

### 1. Accessing `.material` instead of `.sharedMaterial`

```csharp
// ❌ BAD - Creates instance EVERY TIME
void Update() {
    GetComponent<Renderer>().material.color = Color.red;
}

// ✅ GOOD - Modifies shared material (affects all objects)
void Start() {
    GetComponent<Renderer>().sharedMaterial.color = Color.red;
}

// ✅ BETTER - Create instance once, reuse
void Start() {
    mat = GetComponent<Renderer>().material;  // Instance created once
}
void Update() {
    mat.color = Color.red;  // Modify instance
}
```

### 2. Different Lighting Settings Per Object

```
❌ Object A: Cast Shadows ON
❌ Object B: Cast Shadows OFF
❌ Object C: Cast Shadows ON
Result: A and C can't batch together because B is in between!
```

### 3. Different Light Probe Settings

```
❌ Tree 1: Light Probes = Blend Probes
❌ Tree 2: Light Probes = Custom
❌ Tree 3: Light Probes = Blend Probes
Result: 3 draw calls for same tree!
```

### 4. GPU Instancing Disabled

Without GPU instancing:
```
1000 identical trees = 1000 draw calls (if any setting differs)
```

With GPU instancing:
```
1000 identical trees = 1 draw call (even with different transforms)
```

---

## Unity Profiler: What to Look For

### SetPass Calls

**Most important metric!** This shows shader variant switches.

```
SetPass calls: 500
Batches: 500
Saved by batching: 0

❌ No batching happening! Every object uses different shader variant
```

```
SetPass calls: 10
Batches: 500
Saved by batching: 490

✅ Great batching! Only 10 shader switches for 500 objects
```

### Dynamic Batching Failures

Unity console shows why batching failed:
```
"Objects have different materials"
    → Material instances created

"Objects affected by different lights"
    → Different lighting settings

"Shader disables batching"
    → Shader has multi-pass or complex features
```

---

## The Material Inspector Trap

When you change these in the Inspector:

### Mesh Renderer Settings (Per-Object)

```
✅ Cast Shadows: Off/On/Two Sided
✅ Receive Shadows: On/Off
✅ Light Probes: Off/Blend Probes/Custom
✅ Reflection Probes: Off/Blend/Simple
```

**Each different combination = different shader variant = breaks batching!**

### Material Override (WORST OFFENDER)

```csharp
// In Inspector, you do this:
mesh_renderer.material.SetFloat("_Metallic", 0.5f);

// Unity creates:
Material newInstance = Instantiate(originalMaterial);
newInstance.SetFloat("_Metallic", 0.5f);
// ❌ Now this object is UNIQUE - cannot batch!
```

---

## The Hidden Cost: Shader Variant Explosion

Unity Standard Shader has **~60,000 variants**!

```
Features:
- Albedo map (yes/no) = 2 variants
- Normal map (yes/no) = 2 variants
- Metallic map (yes/no) = 2 variants
- Emission (yes/no) = 2 variants
- Shadows (on/off/two-sided) = 3 variants
- Lighting (realtime/baked/mixed) = 3 variants

Total variants = 2 × 2 × 2 × 2 × 3 × 3 = 144
```

And that's just 6 features! Standard shader has 20+.

**Build time:** Compiling 60,000 shader variants = minutes
**Runtime cost:** Switching between them constantly = slow

---

## How to Fix Unity Performance Issues

### Option 1: Make Settings Identical

```
All fences:
✅ Cast Shadows: ON
✅ Receive Shadows: ON
✅ Light Probes: Blend Probes
✅ Same material (no instances)
```

**Result:** 1 draw call for all fences ✅

### Option 2: Use GPU Instancing

Enable in material shader:
```
Material → Inspector → "Enable GPU Instancing" ✅
```

**Result:** 1 draw call even if transforms differ ✅

### Option 3: Use SRP Batcher (URP/HDRP)

```
Project Settings → Graphics → SRP Batcher ✅
```

Batches objects even with different material properties (as long as shader is compatible).

### Option 4: Use MaterialPropertyBlock for Per-Object Data

```csharp
// ✅ Doesn't create material instance!
MaterialPropertyBlock props = new MaterialPropertyBlock();
props.SetColor("_Color", Color.red);
renderer.SetPropertyBlock(props);

// Still batches with other objects!
```

### Option 5: Shader Feature Stripping

Remove unused variants in shader:
```csharp
#pragma multi_compile _ SHADOWS_ON
// Don't compile SHADOWS_OFF variant if never used
```

---

## Best Practices Summary

### 1. Standardize Settings

Make objects **as similar as possible:**
```
✅ All trees: Same shadow/lighting settings
✅ All rocks: Same shadow/lighting settings
✅ All buildings: Same shadow/lighting settings
```

### 2. Never Touch `.material` for Reading

```csharp
// ❌ Creates instance just to READ!
Color c = renderer.material.color;

// ✅ Read from shared material
Color c = renderer.sharedMaterial.color;
```

### 3. Enable GPU Instancing

Checkbox in material inspector:
```
Enable GPU Instancing ✅
```

### 4. Use SRP Batcher (Modern Unity)

Much better batching system for URP/HDRP.

---

## Final Summary

**Shader switching is expensive, but shader complexity is MORE expensive when applied to unnecessary pixels.**

**Best practice:**
1. Use the simplest shader for each object type
2. Sort draw calls by shader
3. Aim for 5-20 shader switches per frame
4. Don't use one "uber shader" for everything
5. Profile! GPU time >> CPU time means shaders are too complex

**Unity-specific:**
- ✅ Different per-object settings = different shader variants
- ✅ Different shader variants = separate draw calls
- ✅ Unity creates material instances when you modify properties
- ✅ SetPass calls in profiler = number of shader switches

**Your worst enemy:**
- Complex shader running on pixels that don't need it (90% waste)
- Material instances breaking batching
- Different lighting/shadow settings per object

**Not as bad as you think:**
- Shader switching (as long as < 100 per frame)
- GPU instancing solves most batching issues

---

## Unity Batching Systems Deep Dive

Unity has **four different batching systems**, each with different rules and trade-offs:

### 1. SRP Batcher (URP/HDRP Only)

**What it does:**
- Reduces CPU overhead of draw calls
- Does NOT reduce draw call count
- Keeps GPU data persistent between draw calls
- Objects still rendered separately, but with minimal CPU cost

**How it works:**
```cpp
// Without SRP Batcher:
for (each object) {
    SetShaderData();      // CPU work
    SetMaterialData();    // CPU work
    Draw();
}

// With SRP Batcher:
SetShaderDataOnce();      // CPU work once
for (each object) {
    UpdateObjectData();   // Minimal CPU work
    Draw();               // GPU-side only
}
```

**Requirements:**
- Shader must be SRP compatible
- Properties must be in CBUFFER blocks:

```hlsl
// ✅ SRP Compatible
CBUFFER_START(UnityPerMaterial)
    float4 _Color;
    float _Metallic;
CBUFFER_END

// ❌ NOT SRP Compatible
float4 _Color;  // Outside CBUFFER
float _Metallic;
```

**What BREAKS SRP Batcher:**

1. ❌ **Different shaders** (obviously)
2. ❌ **Material properties NOT in CBUFFER**
3. ❌ **Using MaterialPropertyBlock** (see below)
4. ❌ **Shader without SRP Batcher compatibility**
5. ❌ **Built-in render pipeline** (SRP Batcher only works in URP/HDRP)
6. ❌ **Particle systems** (not compatible)
7. ❌ **Different shader keywords** (SHADOWS_ON vs SHADOWS_OFF = different variant)
8. ❌ **Skinned meshes** (most of them, some newer Unity versions support it)
9. ❌ **Multi-pass shaders** (forward+ only supports single pass)

**What DOESN'T break SRP Batcher:**
- ✅ Different textures (as long as same shader)
- ✅ Different material property values (colors, floats, etc.)
- ✅ Different meshes
- ✅ Different transforms (position, rotation, scale)
- ✅ Different object properties (within same shader)

**Example:**
```
100 trees with:
- Same shader (TreeShader)
- Different colors via material properties
- Different positions

Result with SRP Batcher:
- 100 draw calls (one per tree)
- BUT: Minimal CPU overhead (GPU-side batching)
- Much faster than without SRP Batcher
```

---

### 2. GPU Instancing

**What it does:**
- Combines identical objects into ONE draw call
- GPU renders multiple instances in parallel
- Each instance can have different transform and limited per-instance data

**How it works:**
```cpp
// Without GPU Instancing:
for (1000 trees) {
    UpdateTransform(tree.position);
    Draw(tree);
}
// 1000 draw calls

// With GPU Instancing:
DrawInstanced(allTreeTransforms, 1000);
// 1 draw call!
```

**Requirements:**
- Material has "Enable GPU Instancing" checked
- Same mesh
- Same material
- Shader supports GPU instancing

```hlsl
// In shader:
#pragma multi_compile_instancing

UNITY_INSTANCING_BUFFER_START(Props)
    UNITY_DEFINE_INSTANCED_PROP(float4, _Color)
UNITY_INSTANCING_BUFFER_END(Props)
```

**What BREAKS GPU Instancing:**

1. ❌ **Different meshes** (each mesh = separate instance batch)
2. ❌ **Different materials** (even if same shader)
3. ❌ **Different shaders** (obviously)
4. ❌ **Lightmap differences** (different lightmap UVs)
5. ❌ **Light probe differences** (different probe settings)
6. ❌ **Reflection probe differences**
7. ❌ **Shadow casting differences** (one casts shadows, one doesn't)
8. ❌ **Mesh with > 511 vertices** on some platforms (mobile limit)
9. ❌ **Using MaterialPropertyBlock for non-instanced properties**
10. ❌ **GPU instancing disabled in material**
11. ❌ **Shader doesn't support instancing** (`#pragma multi_compile_instancing` missing)

**What DOESN'T break GPU Instancing:**
- ✅ Different transforms (position, rotation, scale) - this is the point!
- ✅ Different per-instance properties (color, metallic, etc. if in UNITY_INSTANCING_BUFFER)
- ✅ Different layer/tag (doesn't affect rendering)

**Hardware Limits:**
```
Desktop: 500-1000 instances per draw call
Console: 500-1000 instances per draw call
Mobile:  100-500 instances per draw call (varies by GPU)
```

**Break-even point:**
- Desktop: ~20-50 objects (GPU instancing starts being faster)
- Mobile: ~50-100 objects (higher due to vertex processing cost)

---

### 3. Dynamic Batching (Legacy, Mostly Deprecated)

**What it does:**
- Combines small meshes into one mesh at runtime
- CPU combines vertices on the fly
- Very limited use case

**What BREAKS Dynamic Batching:**

1. ❌ **Mesh > 300 vertices** (hard limit)
2. ❌ **Different materials**
3. ❌ **Different shaders**
4. ❌ **Scale differences** (mirrored objects can't batch)
5. ❌ **Multi-pass shaders**
6. ❌ **Lightmap differences**
7. ❌ **Shadow differences**
8. ❌ **Different blend shapes or normals**

**Why it's deprecated:**
- High CPU cost (combining meshes every frame)
- Only works for tiny meshes
- GPU Instancing is better in every way

**When to use:**
- Never, basically
- Maybe for very simple 2D sprites on low-end mobile

---

### 4. Static Batching

**What it does:**
- Combines static objects into one big mesh at build time
- Zero runtime CPU cost
- Increases memory usage (mesh data duplicated)

**Requirements:**
- GameObject marked as "Static"
- Same material

**What BREAKS Static Batching:**

1. ❌ **GameObject not marked Static**
2. ❌ **Different materials**
3. ❌ **Different shaders**
4. ❌ **Object moves at runtime**
5. ❌ **Lightmap differences** (different baked lighting)

**What DOESN'T break Static Batching:**
- ✅ Different meshes (they get combined)
- ✅ Different scales/rotations (baked into combined mesh)
- ✅ Different textures (as long as same material)

**Trade-offs:**
- ✅ Zero runtime CPU cost
- ✅ Fewer draw calls
- ❌ Increased memory (mesh data duplicated per material)
- ❌ Lost per-object culling (if one piece visible, entire batch drawn)
- ❌ Larger build size

---

## MaterialPropertyBlock: The Special Case

MaterialPropertyBlock is Unity's way to modify per-object properties WITHOUT creating material instances.

### How to Use MaterialPropertyBlock

```csharp
// You MUST explicitly create and use it:
MaterialPropertyBlock props = new MaterialPropertyBlock();
props.SetColor("_Color", Color.red);
props.SetFloat("_Metallic", 0.5f);
renderer.SetPropertyBlock(props);

// This is NOT accidental - you must write this code intentionally
```

### Can You Use MaterialPropertyBlock Accidentally?

**NO!** You cannot accidentally use MaterialPropertyBlock. It requires:
1. Explicitly creating a `MaterialPropertyBlock` object
2. Setting properties on it
3. Calling `renderer.SetPropertyBlock(props)`

**This is different from:**
```csharp
// This DOES create a material instance (accidental):
renderer.material.color = Color.red;  // ❌ Creates instance

// This uses shared material (no instance):
renderer.sharedMaterial.color = Color.red;  // ✅ Modifies shared material
```

### What MaterialPropertyBlock Breaks

**Breaks:**
- ❌ **SRP Batcher** (always breaks it)
- ❌ **Static Batching** (always breaks it)

**Doesn't Break:**
- ✅ **GPU Instancing** (if property is in UNITY_INSTANCING_BUFFER)
- ✅ **Dynamic Batching** (if all other conditions met)

### When to Use MaterialPropertyBlock

**Use it when:**
- You need per-object color/properties
- You want to use GPU Instancing
- You're already at many draw calls and SRP Batcher won't help

**Don't use it when:**
- You're trying to optimize with SRP Batcher
- You can use shared materials instead
- You're on low-end hardware (material instances might be better)

---

## Comparison Table: What Breaks What?

| Change to Object | SRP Batcher | GPU Instancing | Dynamic Batch | Static Batch |
|------------------|-------------|----------------|---------------|--------------|
| **Different mesh** | ✅ OK | ❌ BREAKS | ✅ OK (if < 300 verts) | ✅ OK |
| **Different material** | ❌ BREAKS | ❌ BREAKS | ❌ BREAKS | ❌ BREAKS |
| **Different shader** | ❌ BREAKS | ❌ BREAKS | ❌ BREAKS | ❌ BREAKS |
| **Different texture (same material)** | ✅ OK | ✅ OK | ✅ OK | ✅ OK |
| **Different transform** | ✅ OK | ✅ OK (the point!) | ✅ OK | ❌ BREAKS (must be static) |
| **Different scale** | ✅ OK | ✅ OK | ❌ BREAKS (mirrors) | ❌ BREAKS (baked at build) |
| **Material property change via CBUFFER** | ✅ OK | ✅ OK | ❌ BREAKS | ❌ BREAKS |
| **Material instance (renderer.material)** | ❌ BREAKS | ❌ BREAKS | ❌ BREAKS | ❌ BREAKS |
| **MaterialPropertyBlock** | ❌ BREAKS | ✅ OK (if instanced prop) | ✅ OK | ❌ BREAKS |
| **Different lightmap** | ✅ OK | ❌ BREAKS | ❌ BREAKS | ❌ BREAKS |
| **Different shadow settings** | ❌ BREAKS (variant) | ❌ BREAKS | ❌ BREAKS | ❌ BREAKS |
| **Different light probe** | ❌ BREAKS (variant) | ❌ BREAKS | ❌ BREAKS | ✅ OK |
| **Skinned mesh** | ❌ BREAKS (usually) | ❌ BREAKS | ❌ BREAKS | ❌ BREAKS |
| **Particle system** | ❌ BREAKS | ✅ OK | ✅ OK | ❌ BREAKS |

---

## Changing Material Properties While Maintaining Batching

### Scenario: You Want to Change Color Per Object

**Option 1: Shared Material (affects ALL objects)**
```csharp
renderer.sharedMaterial.color = Color.red;
// ✅ SRP Batcher: OK
// ✅ GPU Instancing: OK
// ✅ Dynamic Batching: OK
// ⚠️ Problem: Changes ALL objects with this material!
```

**Option 2: Material Instance (breaks batching)**
```csharp
renderer.material.color = Color.red;  // Creates instance
// ❌ SRP Batcher: BROKEN (different material)
// ❌ GPU Instancing: BROKEN (different material)
// ❌ Dynamic Batching: BROKEN (different material)
// ❌ Static Batching: BROKEN (different material)
// ✅ Benefit: Only affects this object
```

**Option 3: MaterialPropertyBlock (best for per-object properties)**
```csharp
MaterialPropertyBlock props = new MaterialPropertyBlock();
props.SetColor("_Color", Color.red);
renderer.SetPropertyBlock(props);
// ❌ SRP Batcher: BROKEN
// ✅ GPU Instancing: OK (if _Color is instanced property)
// ✅ Dynamic Batching: OK
// ❌ Static Batching: BROKEN
// ✅ Benefit: Per-object color, still instances
```

### Which Properties Can You Change?

**For SRP Batcher to work:**
- ✅ Any property in `CBUFFER_START(UnityPerMaterial)`
- ✅ Different values per material (not per object)
- ❌ Cannot use MaterialPropertyBlock

**For GPU Instancing to work:**
- ✅ Properties in `UNITY_INSTANCING_BUFFER_START(Props)`
- ✅ Can use MaterialPropertyBlock for instanced properties
- ✅ Different values per object

**Example shader supporting both:**
```hlsl
// SRP Batcher compatible
CBUFFER_START(UnityPerMaterial)
    float _Smoothness;  // Same for all objects using this material
CBUFFER_END

// GPU Instancing compatible
UNITY_INSTANCING_BUFFER_START(Props)
    UNITY_DEFINE_INSTANCED_PROP(float4, _Color)  // Different per object
UNITY_INSTANCING_BUFFER_END(Props)
```

---

## Vertex Shader Modifications and Batching

**Question:** Does vertex shader logic (like wind animation) break batching?

**Answer:** NO! Vertex shader modifications do NOT break batching, as long as implemented correctly.

### Why Vertex Shader Logic Doesn't Break Batching

When you bind a shader program with `glUseProgram()`, the GPU executes that vertex shader for every vertex. The calculations happen **on the GPU** - there's no state change, no CPU overhead.

**Example: Wind animation on grass**

```hlsl
// Vertex shader with wind
void vert(inout appdata_full v) {
    // Wind calculation runs on GPU for every vertex
    float windStrength = _Time.y * _WindSpeed;
    float3 windOffset = sin(v.vertex.x + windStrength) * float3(1, 0, 0);
    v.vertex.xyz += windOffset;
}
```

This is part of the **shader program itself** - not a state change!

### What Works With Each Batching System

**SRP Batcher + Vertex Shader Animation:**
✅ **Fully compatible**
- All grass blades use same shader
- Wind calculation happens in vertex shader
- Each blade can have different wind phase via material properties
- No state changes between draw calls

**GPU Instancing + Vertex Shader Animation:**
✅ **Fully compatible**
- Can pass per-instance wind data:

```hlsl
UNITY_INSTANCING_BUFFER_START(Props)
    UNITY_DEFINE_INSTANCED_PROP(float, _WindPhase)
UNITY_INSTANCING_BUFFER_END(Props)

void vert(inout appdata_full v) {
    float windPhase = UNITY_ACCESS_INSTANCED_PROP(Props, _WindPhase);
    float3 windOffset = sin(v.vertex.x + _Time.y + windPhase);
    v.vertex.xyz += windOffset;
}
```

Result: 1000 grass blades = 1 instanced draw call, each with different wind phase!

**Dynamic Batching + Vertex Shader Animation:**
⚠️ **Compatible but not recommended**
- Dynamic batching combines vertices on CPU
- Vertex shader animation happens after batching
- Works, but defeats the purpose (animating combined mesh)

**Static Batching + Vertex Shader Animation:**
✅ **Compatible**
- Combined mesh at build time
- Vertex shader animates the combined mesh
- Works great for grass fields

### What DOES Break Batching

**These vertex shader patterns BREAK batching:**

1. ❌ **Different shaders per object**
```csharp
// Grass 1 uses WindShader_v1
// Grass 2 uses WindShader_v2
// Result: Can't batch (different shader programs)
```

2. ❌ **Using MaterialPropertyBlock for non-instanced wind properties**
```csharp
// This breaks SRP Batcher:
MaterialPropertyBlock props = new MaterialPropertyBlock();
props.SetFloat("_WindSpeed", 2.0f);
renderer.SetPropertyBlock(props);
```

3. ❌ **Different shader keywords**
```csharp
// Material 1: WIND_SIMPLE enabled
// Material 2: WIND_COMPLEX enabled
// Result: Different shader variants = can't batch
```

### Best Practices for Animated Batching

**For grass/foliage wind:**

**Option A: Global wind (SRP Batcher friendly)**
```hlsl
// All grass uses same wind calculation
float3 windOffset = sin(v.vertex.xz * _WindFreq + _Time.y * _WindSpeed);
// No per-instance data needed
// Result: All grass batched with SRP Batcher
```

**Option B: Per-instance variation (GPU Instancing)**
```hlsl
// Each grass blade has unique phase
float phase = UNITY_ACCESS_INSTANCED_PROP(Props, _WindPhase);
float3 windOffset = sin(v.vertex.xz * _WindFreq + _Time.y + phase);
// Result: All grass in 1 instanced draw call
```

**Option C: Hybrid (best of both)**
```hlsl
// Global wind + local variation from vertex position
float localVariation = frac(v.vertex.x * 0.1 + v.vertex.z * 0.1);
float3 windOffset = sin(v.vertex.xz * _WindFreq + _Time.y + localVariation);
// No per-instance data needed
// Each blade automatically different based on position
// Result: Perfect for SRP Batcher!
```

### Summary: Vertex Shader and Batching

| Batching System | Vertex Shader Animation | Notes |
|-----------------|-------------------------|-------|
| **SRP Batcher** | ✅ Fully compatible | Shader logic doesn't count as state change |
| **GPU Instancing** | ✅ Fully compatible | Can pass per-instance animation data |
| **Dynamic Batching** | ⚠️ Works but weird | Animates combined mesh |
| **Static Batching** | ✅ Fully compatible | Animates pre-combined mesh |

**Key takeaway:** Vertex shader modifications are part of the shader program, not state changes. As long as all objects use the **same shader**, they can batch regardless of what calculations happen in the vertex shader.

---

## Depth Sorting vs Batching

### The Fundamental Conflict

Unity must choose between two optimizations:

**Option A: Batch by material (fewer draw calls)**
```
Tree 1 (far) + Tree 2 (near) → 1 draw call
Rock 1 (far) + Rock 2 (near) → 1 draw call
```

**Option B: Sort by depth (better GPU fill rate)**
```
Tree 2 (near)  → Draw first (early-z rejection helps later pixels)
Rock 2 (near)  → Draw
Tree 1 (far)   → Draw (many pixels rejected by early-z)
Rock 1 (far)   → Draw (many pixels rejected by early-z)
```

### Why Unity Sorts by Depth

**For opaque objects:** Unity renders **front-to-back** (near to far)

**Why?** GPU early-z rejection optimization:

```
1. Draw near tree (fills depth buffer)
2. Draw rock behind it (GPU tests depth first)
3. GPU: "This rock pixel is behind tree, skip pixel shader!"
4. Result: Expensive pixel shader not executed
```

This saves massive amounts of pixel shader work, especially with complex PBR shaders.

### Why This Breaks Batching

When Unity sorts by depth, objects of the same material are scattered:

```
Frame profiler shows:
Draw 1: Tree (near)    - TreeMaterial
Draw 2: Rock (near)    - RockMaterial
Draw 3: Bush (medium)  - BushMaterial
Draw 4: Tree (medium)  - TreeMaterial  ← Can't batch with Draw 1!
Draw 5: Tree (far)     - TreeMaterial  ← Can't batch with Draw 1 or 4!

Result: 5 draw calls for 3 materials
```

### The Solution: GPU Instancing

GPU Instancing can batch objects **regardless of depth order**:

```
Without GPU Instancing:
Draw 1: Tree (near)    - TreeMaterial
Draw 2: Tree (medium)  - TreeMaterial
Draw 3: Tree (far)     - TreeMaterial
Total: 3 separate draw calls

With GPU Instancing:
Draw 1: Instanced(Tree × 3) - TreeMaterial
Total: 1 instanced draw call
GPU still renders them in depth order internally!
```

**Key insight:** GPU Instancing handles depth sorting on the GPU side, not CPU side.

---

## Depth Priming (URP/HDRP)

Depth priming is an advanced optimization that pre-fills the depth buffer to maximize early-z rejection.

### What is Depth Priming?

Unity renders the scene in **two passes**:

**Pass 1: Depth Prepass**
```cpp
for (each opaque object) {
    UseSimpleDepthShader();  // Only writes depth, no color
    Draw(object);            // Fast (no lighting calculations)
}
```

**Pass 2: Color Pass**
```cpp
for (each opaque object) {
    UseFullShader();         // Full PBR, lighting, etc.
    Draw(object);            // GPU skips occluded pixels (perfect early-z)
}
```

### How It Helps

**Without depth priming:**
```
Draw Tree 1 (near)  - Pixel shader runs on 10,000 pixels
Draw Tree 2 (medium) - Pixel shader runs, 3,000 pixels occluded (wasted work!)
Draw Tree 3 (far)   - Pixel shader runs, 8,000 pixels occluded (wasted work!)

Total wasted work: 11,000 pixel shader invocations
```

**With depth priming:**
```
=== Depth Pass ===
Draw Tree 1 (near)  - Simple depth shader (cheap)
Draw Tree 2 (medium) - Simple depth shader (cheap)
Draw Tree 3 (far)   - Simple depth shader (cheap)

=== Color Pass ===
Draw Tree 1 (near)  - Full shader on 10,000 pixels
Draw Tree 2 (medium) - Full shader, GPU skips 3,000 occluded pixels automatically
Draw Tree 3 (far)   - Full shader, GPU skips 8,000 occluded pixels automatically

Total wasted work: 0 pixel shader invocations!
```

### Depth Priming Modes (Universal Renderer Data)

**Disabled:**
- No depth prepass
- Relies on front-to-back sorting for early-z
- Depth sorting becomes CRITICAL for performance
- Lower CPU overhead (one pass)
- Higher GPU waste (imperfect early-z rejection)

**Auto (Default):**
- Unity profiles and decides
- Enables when:
  - Complex pixel shaders detected
  - High overdraw scenarios
  - Many overlapping objects
- Disables when:
  - Simple shaders
  - Minimal overdraw
  - Mobile platform

**Forced:**
- Always does depth prepass
- Best for:
  - Complex PBR materials
  - Dense scenes (cities, forests)
  - Desktop/console platforms
- Worst for:
  - Simple shaders (wasted vertex processing)
  - Mobile (vertex processing is expensive)
  - Sparse scenes (no overdraw to save)

### How Depth Priming Affects Batching

**Without Depth Priming (Disabled):**
- Depth sorting is CRITICAL
- Unity prioritizes: depth order > material batching
- Result: More draw calls, but better GPU efficiency

```
Tree 1 (near)  - TreeMaterial
Rock 1 (near)  - RockMaterial
Tree 2 (far)   - TreeMaterial  ← Can't batch with Tree 1 (wrong order)
```

**With Depth Priming (Forced):**
- Depth sorting is LESS critical
- Unity can prioritize: material batching > depth order
- Result: Better batching, still perfect early-z

```
=== Depth Pass ===
Tree 1 (near)  - DepthOnly
Rock 1 (near)  - DepthOnly
Tree 2 (far)   - DepthOnly

=== Color Pass === (can now batch better!)
All Trees      - TreeMaterial (batched!)
All Rocks      - RockMaterial (batched!)
```

### The Ultimate Combination

**GPU Instancing + Depth Priming = Best Performance**

```
=== Depth Pass ===
Instanced Draw: 100 Trees - DepthOnly (1 draw call)
Instanced Draw: 50 Rocks  - DepthOnly (1 draw call)

=== Color Pass ===
Instanced Draw: 100 Trees - TreeMaterial (1 draw call)
Instanced Draw: 50 Rocks  - RockMaterial (1 draw call)

Total: 4 draw calls for 150 objects
Perfect early-z rejection
Minimal CPU overhead
```

### When to Use Each Mode

**Use Disabled when:**
- ✅ Simple shaders (unlit, mobile-optimized)
- ✅ Mobile platforms (vertex processing is bottleneck)
- ✅ Minimal overdraw (outdoor scenes, skyboxes)
- ✅ Draw call count is the bottleneck (let sorting help)

**Use Auto when:**
- ✅ You're not sure (let Unity decide)
- ✅ Mixed scene complexity
- ✅ Cross-platform project
- ✅ Don't want to micromanage

**Use Forced when:**
- ✅ Complex PBR materials (Standard shader, HDRP Lit)
- ✅ Dense scenes with high overdraw (cities, interiors, forests)
- ✅ Desktop/console only (can afford vertex processing)
- ✅ Profiler shows pixel shader overdraw as bottleneck
- ✅ Expensive post-processing effects

### Common Misconceptions

**❌ "Depth priming fixes all performance issues"**
- No! It trades vertex processing for pixel processing
- Only helps if pixel shaders are the bottleneck
- Can hurt performance on vertex-bound scenes

**❌ "Force depth priming as a band-aid for messy team practices"**
- No! Fix the root issues:
  - Expensive shaders → optimize shaders
  - Broken batching → fix material management
  - Shader variants → reduce variant explosion
- Depth priming masks problems, doesn't solve them

**✅ "Depth priming is a strategic optimization tool"**
- Yes! Use it when profiling shows it helps
- Measure before and after
- Different scenes may need different settings

### Performance Trade-offs

**Costs of Depth Priming:**
- ❌ Every vertex processed TWICE (depth pass + color pass)
- ❌ Additional draw calls in depth pass
- ❌ Memory bandwidth for depth buffer writes/reads
- ❌ On mobile: significant vertex processing cost

**Benefits of Depth Priming:**
- ✅ Perfect early-z rejection (no wasted pixel shaders)
- ✅ Allows better material batching (depth sorting less critical)
- ✅ Massive savings with complex pixel shaders
- ✅ On desktop: vertex processing is cheap

### Example Scenarios

**Scenario 1: Mobile Game with Simple Shaders**
```
Decision: Disabled
Reason:
- Mobile GPUs struggle with vertex processing
- Simple shaders = low pixel cost anyway
- Depth priming would waste more than it saves
```

**Scenario 2: Desktop Game with PBR Materials**
```
Decision: Forced
Reason:
- Complex pixel shaders (PBR, many texture samples)
- Desktop GPU handles vertex processing easily
- High overdraw in urban environments
- Pixel shader cost >> vertex shader cost
```

**Scenario 3: Cross-Platform Adventure Game**
```
Decision: Auto
Reason:
- Mixed scene types (outdoor + indoor)
- Some simple, some complex materials
- Let Unity profile and decide per scene
- Mobile and desktop targets
```

### Profiling: How to Decide

**Look for these signs depth priming will help:**

1. **Frame Debugger shows high overdraw**
   - Many objects drawn over same pixels
   - Window → Analysis → Frame Debugger → Overdraw mode

2. **Profiler shows pixel shader cost**
   - GPU time high
   - Fragment shader time >> Vertex shader time

3. **Complex shaders in scene**
   - PBR materials
   - Multiple texture samples
   - Complex lighting calculations

**Look for these signs depth priming will hurt:**

1. **Profiler shows vertex cost**
   - Vertex shader time >> Fragment shader time
   - Skinned mesh animation cost high

2. **Simple shaders**
   - Unlit materials
   - Mobile-optimized shaders
   - Single texture sample

3. **Mobile platform**
   - Vertex processing is expensive
   - Tile-based rendering already helps

### Best Practice: Don't Use as Band-Aid

**Wrong approach:**
```
Team is messy with:
- Creating expensive shaders
- Breaking batching with material variants
- Different settings per object

Solution: Force depth priming and forget about it ❌
```

**Why this is wrong:**
1. Masks the real performance problems
2. Mobile performance will tank
3. Team never learns optimization
4. Technical debt accumulates

**Right approach:**
```
1. Profile and identify ACTUAL bottleneck
2. Fix root causes:
   - Optimize expensive shaders
   - Standardize material usage
   - Use GPU instancing properly
   - Reduce shader variants
3. Use depth priming strategically where profiling shows benefit
4. Educate team on proper practices
```

---

## Summary: The Complete Optimization Strategy

**1. Choose the Right Batching System**

For most modern projects:
- ✅ Use **SRP Batcher** (URP/HDRP) as baseline
- ✅ Enable **GPU Instancing** on materials with many instances
- ❌ Avoid Dynamic Batching (deprecated)
- ✅ Use Static Batching for static background objects

**2. Make Shaders Compatible**

```hlsl
// SRP Batcher compatible
CBUFFER_START(UnityPerMaterial)
    float4 _Color;
CBUFFER_END

// GPU Instancing compatible
#pragma multi_compile_instancing
UNITY_INSTANCING_BUFFER_START(Props)
    UNITY_DEFINE_INSTANCED_PROP(float4, _Tint)
UNITY_INSTANCING_BUFFER_END(Props)
```

**3. Manage Materials Properly**

```csharp
// ❌ Never do this
renderer.material.color = Color.red;  // Creates instance

// ✅ For shared property changes
renderer.sharedMaterial.color = Color.red;

// ✅ For per-object properties (with GPU Instancing)
MaterialPropertyBlock props = new MaterialPropertyBlock();
props.SetColor("_Tint", Color.red);
renderer.SetPropertyBlock(props);
```

**4. Use Depth Priming Strategically**

- **Disabled:** Mobile, simple shaders, outdoor scenes
- **Auto:** Cross-platform, mixed complexity, default choice
- **Forced:** Desktop/console, complex PBR, dense urban scenes

**5. Profile, Don't Guess**

Use Unity Profiler to identify actual bottlenecks:
- **SetPass calls:** Shader switching (aim for < 50)
- **Batches:** Draw call count (aim for < 500)
- **GPU time:** Check if pixel or vertex bound
- **Overdraw:** Frame Debugger overdraw mode

**The Golden Rules:**
1. ✅ Keep shaders SRP compatible
2. ✅ Enable GPU Instancing for repeated objects
3. ✅ Never access `.material` unless necessary
4. ✅ Standardize object settings (shadows, light probes)
5. ✅ Use depth priming based on profiling, not guessing
6. ✅ Vertex shader animations don't break batching
7. ❌ Don't use depth priming as a band-aid for poor practices
