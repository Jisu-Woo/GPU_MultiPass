# GPU_multiPass
- Using C++ and OpenGL
- Gaussian blur + Edge detection + Toon shading
<br/>

- pass 1 : 3D scene rendering
- pass 2: 1D Gaussian blur
- pass 3: 1D Gaussian blur
- pass 4 : edge detection
- pass 5 : toon shading
<br/>

- press '1' -> 3D scene rendering
- press '2' -> pass 1 + pass 5
- press '3' -> pass 1 + pass 4 + pass 5
- press 'g' -> add or delete gaussian blur(pass 2 + pass 3) (toggle button!)
<br/>

- using two FBO
- using two texture
