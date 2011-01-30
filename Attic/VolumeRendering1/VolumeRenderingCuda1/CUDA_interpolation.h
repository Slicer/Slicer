#ifndef __CUDA_INTERPOLATION_H__
#define __CUDA_INTERPOLATION_H__

template <typename T> class CUDAkernel_Interpolate_NearestNaighbor
{
public:
    __device__ float operator()(const void* SourceData, const int3& VolumeSize, const float3& Pos)
    {
        return ((T*)SourceData)[(int)(__float2int_rn(Pos.z)*VolumeSize.x*VolumeSize.y + 
            __float2int_rn(Pos.y)*VolumeSize.x +
            __float2int_rn(Pos.x))];
    }
};



template <typename T> class CUDAkernel_Interpolate_Trilinear
{
public:
    __device__ float operator()(const void* SourceData, const int3& VolumeSize, const float3& Pos)
    {
        float posX = Pos.x - __float2int_rd(Pos.x);
        float posY = Pos.y - __float2int_rd(Pos.y);
        float posZ = Pos.z - __float2int_rd(Pos.z);

        int base = __float2int_rd((Pos.z)) * VolumeSize.x * VolumeSize.y + 
            __float2int_rd((Pos.y)) * VolumeSize.x +
            __float2int_rd((Pos.x));

        return interpolater(posX, posY, posZ,
            ((T*)SourceData)[base],
            ((T*)SourceData)[(int)(base + VolumeSize.x * VolumeSize.y)],
            ((T*)SourceData)[(int)(base + VolumeSize.x)],
            ((T*)SourceData)[(int)(base + VolumeSize.x * VolumeSize.y + VolumeSize.x)],
            ((T*)SourceData)[(int)(base + 1)],
            ((T*)SourceData)[(int)(base + VolumeSize.x * VolumeSize.y + 1)],
            ((T*)SourceData)[(int)(base + VolumeSize.x + 1)],
            ((T*)SourceData)[(int)(base + VolumeSize.x * VolumeSize.y + VolumeSize.x + 1)]);
    }
private:
__device__ T interpolater(float posX, float posY, float posZ,
                         T val1, T val2, T val3, T val4,
                         T val5, T val6, T val7, T val8)
{
    float revX = 1-posX;
    float revY = 1-posY;
    float revZ = 1-posZ;

    return ((T)
        (revX * (revY * (revZ * val1  +
        posZ * val2) +
        posY * (revZ * val3  +
        posZ * val4))+
        posX * (revY * (revZ * val5  +
        posZ * val6)   +
        posY * (revZ * val7 +
        posZ * val8)))
        );
}
};

#endif /* __CUDA_INTERPOLATION_H__ */
