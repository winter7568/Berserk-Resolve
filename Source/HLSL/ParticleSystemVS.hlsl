#include "ParticleSystem.hlsli"

GS_IN main(VS_IN vin)
{
    GS_IN output = (GS_IN) 0;

    output.position = vin.position;
    output.size = vin.size;
    output.color = vin.color;
    output.param = vin.param;

    return output;
}
