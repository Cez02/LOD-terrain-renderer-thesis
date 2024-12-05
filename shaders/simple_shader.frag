#version 460

#extension GL_GOOGLE_include_directive : enable

layout(location=0) in Interpolants {
    vec3  wPos;
    vec3  wNormal;
    vec3 wColor;
    flat uint meshletID;
} IN;



layout(location=0,index=0) out vec4 out_Color;

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
    out_Color = vec4(IN.wColor, 1.0);
/*
    float ambientFactor = max( dot(normalize(IN.wNormal), normalize(vec3(1, 1, 1))), 0.0);

    out_Color = vec4(
        vec3(1.0, 0.0, 0.0) * ambientFactor,
        1.0
    );
    */
}