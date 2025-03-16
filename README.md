# GPU_multiPass

## Description
Gaussian blur + Edge detection + Toon shading
<br/>

- pass 1 : 3D scene rendering
- pass 2: 1D Gaussian blur
- pass 3: 1D Gaussian blur
- pass 4 : edge detection
- pass 5 : toon shading
<br/>
- using two FBO
- using two texture


## Instructions
- press '1' -> 3D scene rendering
- press '2' -> pass 1 + pass 5
- press '3' -> pass 1 + pass 4 + pass 5
- press 'g' -> add or delete gaussian blur(pass 2 + pass 3) (toggle button!)
<br/>

## Results

#### - press '1'
![GPU_기본](https://github.com/user-attachments/assets/98e3ccad-d4a4-4dd1-bd51-6e81edbb2708)

#### - press '1' + 'g'
![GPU_1_g](https://github.com/user-attachments/assets/cb51b6ed-82fc-45dc-9660-e981d7f932f3)

#### - press '2'
![GPU_2](https://github.com/user-attachments/assets/78024fbf-992b-4617-9c82-533171f0a60b)

#### - press '2' + 'g'
![GPU_2_g](https://github.com/user-attachments/assets/445dfbff-4e25-41ab-9006-d2c182107041)

#### - press '3'
![GPU_3](https://github.com/user-attachments/assets/62ad0b5c-b81e-4b4b-a487-a937a830a989)

#### - press '3' + 'g'
![GPU_3_g](https://github.com/user-attachments/assets/9fdab673-afd4-4fa4-8490-7173e10575ce)
