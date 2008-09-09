#ifndef __CUDA_MATRIX_MATH_H__
#define __CUDA_MATRIX_MATH_H__

__device__ void MatMul(const float mat[4][4], float3* out, float inX, float inY, float inZ, float inW)
{
    out->x = mat[0][0] * inX + mat[0][1] * inY + mat[0][2] * inZ + mat[0][3] * inW;
    out->y = mat[1][0] * inX + mat[1][1] * inY + mat[1][2] * inZ + mat[1][3] * inW;
    out->z = mat[2][0] * inX + mat[2][1] * inY + mat[2][2] * inZ + mat[2][3] * inW;
}

__device__ void MatMul(const float mat[4][4], float3* out, float inX, float inY, float inZ)
{
    MatMul(mat, out, inX, inY, inZ, 1.0f);
}

__device__ void MatMul(const float mat[4][4], float3* out, const float3& in, float inW)
{
    MatMul(mat, out, in.x, in.y, in.z, inW);
}

__device__ void MatMul(const float mat[4][4], float3* out, const float3& in)
{
    MatMul(mat, out, in, 1.0f);
}

__device__ float3 MatMul(const float mat[4][4], const float3& in)
{
    float3 out;
    MatMul(mat, &out, in);
    return out;
}

__device__ float3 MatMul(const float mat[4][4], const float3& in, float inW)
{
    float3 out;
    MatMul(mat, &out, in, inW);
    return out;
}

__device__ float3 MatMul(const float mat[4][4], float inX, float inY, float inZ, float inW)
{
    float3 out;
    MatMul(mat, &out, inX, inY, inZ, inW);
    return out;
}

__device__ float3 MatMul(const float mat[4][4], float inX, float inY, float inZ)
{
    float3 out;
    MatMul(mat, &out, inX, inY, inZ);
    return out;
}
__device__ float VecLen(const float3& vec)
{
    return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

#endif /* __CUDA_MATRIX_MATH_H__ */
