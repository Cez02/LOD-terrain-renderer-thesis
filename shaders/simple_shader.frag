#version 460

#extension GL_GOOGLE_include_directive : enable

// Allows us to use float16_t for arithmetic purposes.
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require


#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

layout(location=0) in Interpolants {
    vec3  wPos;
    vec3  wNormal;
    vec3  wColor;
    float wHeight;
    flat uint meshletID;
} IN;



layout(location=0,index=0) out vec4 out_Color;

vec3 getColorFromHeight(float height){
    vec3 Color = vec3(0.5, 0.5, 0.5);

    if(height == 32768) Color = vec3(0.6, 0, 0.6);
    else if      (height <= 0  )   Color = vec3(0.,       0.,        1.); //blue
    else if (height < 500)   Color = vec3(0.,       height/500,    0.); //->green
    else if (height < 1000)  Color = vec3(height/500-1, 1.,        0.); //->yellow
    else if (height < 2000)  Color = vec3(1.,       2.-height/1000,0.); //->red
    else                      Color = vec3(1.,       height/2000-1 ,height/2000-1);  //->white

    return Color;
}

void main() {

    float fact = float(IN.meshletID % 3);

    //out_Color = vec4( fact, 1.0f / fact, 0.1f , 1.0f);
    /*
    if(fact == 0){
        out_Color = vec4(0.7, 0.2, 0.0, 1.0);
    }
    else if(fact == 1){
        out_Color = vec4(0.0, 0.2, 0.7, 1.0);
    }
    else{
        out_Color = vec4(0.0, 0.7, 0.0, 1.0);
    }
    */

    //out_Color = vec4(IN.wNormal + vec3(1, 1, 1), 2.0f) * 0.5f;
    out_Color = vec4(getColorFromHeight(IN.wHeight), 1.0);
    //out_Color = vec4(1.0, 1.0, 1.0, 1.0);
/*
    float ambientFactor = max( dot(normalize(IN.wNormal), normalize(vec3(1, 1, 1))), 0.0);

    out_Color = vec4(
        vec3(1.0, 0.0, 0.0) * ambientFactor,
        1.0
    );
    */
}