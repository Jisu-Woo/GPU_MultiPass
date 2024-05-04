#version 150 core

out vec4 out_Color;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;

uniform vec3 diffColor;
uniform vec3 specColor;
uniform float shininess;

uniform sampler2D diffTex;
uniform sampler2D bumpTex;
uniform sampler2D Texture0;

//input parameter
uniform int Pass;
uniform int number;
uniform int gaussian;

//for gaussian blur
const float OFFSETS[5] = float[5](
	0.0,
	1.0,
	2.0,
	3.0,
	4.0
);

const float WEIGHTS[5] = float[5](
	0.0276, 
	0.0663, 
	0.1238, 
	0.1802, 
	0.2042
);


const vec3 lum = vec3(0.2126, 0.7152, 0.0722); 

in vec3 normal;
in vec3 worldPos;
in vec2 texCoord;



mat3 getTBN(vec3 N){
	vec3 Q1 = dFdx(worldPos), Q2 = dFdy(worldPos);
	vec2 st1 = dFdx(texCoord), st2 = dFdy(texCoord);
	float D = st1.s*st2.t-st2.s*st1.t;
	return mat3(normalize(( Q1*st2.t - Q2*st1.t)*D),
				normalize((-Q1*st2.s + Q2*st1.s)*D), N);
}

//3D scene rendering
vec4 pass1() {
	vec3 toLight = lightPosition - worldPos;
	vec3 L = normalize(toLight);
	vec3 N = normalize(normal);

	float Bu = texture(bumpTex, texCoord + vec2(0.00001, 0)).r - texture(bumpTex, texCoord - vec2(0.00001, 0)).r;
	float Bv = texture(bumpTex, texCoord + vec2(0, 0.00001)).r - texture(bumpTex, texCoord - vec2(0, 0.00001)).r;
	vec3 bumpVec = vec3(-Bu*30, -Bv*30, 1);
	mat3 tbn = getTBN(N);
	N = normalize(tbn * bumpVec);

	vec3 V = normalize(cameraPosition - worldPos);
	vec3 R = reflect(-L,N);
	vec3 Ii = vec3(200, 200, 200) / dot(toLight,toLight);
	vec3 dColor = texture(diffTex, texCoord).xyz;
	vec3 color =  Ii * (dColor * max(0,dot(N,L)) + specColor*pow(max(0,dot(R,V)),shininess) );
	
	return vec4(pow(color,vec3(1/2.2)), 1.0);
}


//1D gaussian blur 1 (세로 방향)
vec4 pass2() {
	ivec2 pix = ivec2(gl_FragCoord.xy); 
	vec4 sum = texelFetch(Texture0, pix, 0) * WEIGHTS[0]; 
	for( int i = 1; i < 5; i++ ) { 
		sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,OFFSETS[i])) * WEIGHTS[i]; 
		sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,-OFFSETS[i])) * WEIGHTS[i]; 
	} 
	return sum; 
}

//1D gaussian blur 2 (가로 방향)
vec4 pass3() {
	ivec2 pix = ivec2(gl_FragCoord.xy); 
	vec4 sum = texelFetch(Texture0, pix, 0) * WEIGHTS[0];
	for( int i = 1; i < 5; i++ ) { 
		sum += texelFetchOffset( Texture0, pix, 0, ivec2(OFFSETS[i],0)) * WEIGHTS[i]; 
		sum += texelFetchOffset( Texture0, pix, 0, ivec2(-OFFSETS[i],0)) * WEIGHTS[i]; 
	} 
	return sum;

}







// Approximates the brightness of a RGB value. 
float luminance( vec3 color ) { 
	return dot(lum, color);
}

//Edge Detection
vec4 pass4() {
	ivec2 pix = ivec2(gl_FragCoord.xy); 

	float s00;
	float s10;
	float s20;
	float s01;
	float s21;
	float s02;
	float s12;
	float s22;

	//Texture0 input
	s00 = luminance( texelFetchOffset(Texture0, pix, 0, ivec2(-1,1)).rgb); 
	s10 = luminance( texelFetchOffset(Texture0, pix, 0, ivec2(-1,0)).rgb); 
	s20 = luminance( texelFetchOffset(Texture0, pix, 0, ivec2(-1,-1)).rgb); 
	s01 = luminance( texelFetchOffset(Texture0, pix, 0, ivec2(0,1)).rgb); 
	s21 = luminance( texelFetchOffset(Texture0, pix, 0, ivec2(0,-1)).rgb); 
	s02 = luminance( texelFetchOffset(Texture0, pix, 0, ivec2(1,1)).rgb); 
	s12 = luminance( texelFetchOffset(Texture0, pix, 0, ivec2(1,0)).rgb); 
	s22 = luminance( texelFetchOffset(Texture0, pix, 0, ivec2(1,-1)).rgb); 
	


	float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22); 
	float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22); 
	float g = sx * sx + sy * sy; 

	float EdgeThreshold = 0.8f;

	//Edge Threshold
	if( g > EdgeThreshold ) return vec4(0.0, 0.0, 0.0, 1.0); 
	else return texelFetch(Texture0, pix, 0);

}






// RGB를 XYZ로 변환
vec3 RGBtoXYZ(vec3 rgb) {
    vec3 xyz;
    xyz.x = 0.4124564 * rgb.r + 0.3575761 * rgb.g + 0.1804375 * rgb.b;
    xyz.y = 0.2126729 * rgb.r + 0.7151522 * rgb.g + 0.0721750 * rgb.b;
    xyz.z = 0.0193339 * rgb.r + 0.1191920 * rgb.g + 0.9503041 * rgb.b;
    return xyz;
}

// XYZ를 xyY로 변환
vec3 XYZtoxyY(vec3 xyz) {
    float total = xyz.x + xyz.y + xyz.z;
    if (total == 0.0) {
        return vec3(0.0, 0.0, xyz.y);  // 피할 수 없는 경우 검정색으로
    }
    return vec3(xyz.x / total, xyz.y / total, xyz.y);
}

// xyY를 XYZ로 변환
vec3 xyYtoXYZ(vec3 xyY) {
    if (xyY.y == 0.0) {
        return vec3(0.0, 0.0, 0.0);
    }
    float Y = xyY.z;
    float X = (xyY.x * Y) / xyY.y;
    float Z = (Y / xyY.y) * (1.0 - xyY.x - xyY.y);
    return vec3(X, Y, Z);
}

// XYZ를 RGB로 변환
vec3 XYZtoRGB(vec3 xyz) {
    vec3 rgb;
    rgb.r =  3.2404542 * xyz.x - 1.5371385 * xyz.y - 0.4985314 * xyz.z;
    rgb.g = -0.9692660 * xyz.x + 1.8760108 * xyz.y + 0.0415560 * xyz.z;
    rgb.b =  0.0556434 * xyz.x - 0.2040259 * xyz.y + 1.0572252 * xyz.z;
    return rgb;
}

// Quantization 함수
float quantize(float y, int levels) {
    return floor(y * levels) / levels;
}


// Toon Shading
vec4 pass5() {
	vec3 rgb;

	rgb = texture(Texture0, texCoord).rgb;
	
	
    vec3 xyz = RGBtoXYZ(rgb);
    vec3 xyY = XYZtoxyY(xyz);

    // Y만을 Quantize
    xyY.z = quantize(xyY.z, 5);  // 밝기를 5단계로 quantize

    vec3 newXYZ = xyYtoXYZ(xyY);
    vec3 newRGB = XYZtoRGB(newXYZ);
    vec4 color = vec4(newRGB, 1.0);

	return color;

}




void main(void)
{
	if( Pass == 1 ) out_Color = pass1();
	else if( Pass == 2 ) out_Color = pass2();
	else if( Pass == 3 ) out_Color = pass3();
	else if( Pass == 4 ) out_Color = pass4();
	else if( Pass == 5 ) out_Color = pass5();
}

