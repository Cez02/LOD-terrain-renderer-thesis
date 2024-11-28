#version 460

#extension GL_GOOGLE_include_directive : enable

layout(location=0) in Interpolants {
    vec3  wPos;
    vec3  wNormal;
    flat uint meshletID;
} IN;



layout(location=0,index=0) out vec4 out_Color;

void main() {

    float fact = float(IN.meshletID % 10);

    //out_Color = vec4( fact, 1.0f / fact, 0.1f , 1.0f);
    out_Color = vec4(IN.wNormal + vec3(1, 1, 1), 2.0f) * 0.5f;

/*
    float ambientFactor = max( dot(normalize(IN.wNormal), normalize(vec3(1, 1, 1))), 0.0);

    out_Color = vec4(
        vec3(1.0, 0.0, 0.0) * ambientFactor,
        1.0
    );
    */
}