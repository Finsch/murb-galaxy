#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <math.h>

// Kernels CUDA
__global__ void resetAccelerationsKernel(float* ax, float* ay, float* az, int n)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
   
    if (i < n) {
        ax[i] = 0.0f;
        ay[i] = 0.0f;
        az[i] = 0.0f;
    }
}

__global__ void computeAccelerationsKernel(float* m, float* qx, float* qy, float* qz,
                                           float* ax, float* ay, float* az,
                                           float G, float softSquared, int n)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
   
    if (i >= n) return;
   
    float axi = 0.0f;
    float ayi = 0.0f;
    float azi = 0.0f;
   
    float qxi = qx[i];
    float qyi = qy[i];
    float qzi = qz[i];
   
    for (int j = 0; j < n; j++) {
        if (i == j) continue;
       
        float rx = qx[j] - qxi;
        float ry = qy[j] - qyi;
        float rz = qz[j] - qzi;
       
        float rSquared = rx * rx + ry * ry + rz * rz;
        float distSquared = rSquared + softSquared;
       
        float invDist = rsqrtf(distSquared);
        float invDistCubed = invDist * invDist * invDist;
       
        float a = G * m[j] * invDistCubed;
       
        axi += a * rx;
        ayi += a * ry;
        azi += a * rz;
    }
   
    ax[i] = axi;
    ay[i] = ayi;
    az[i] = azi;
}

__global__ void updatePositionsVelocitiesKernel(float* m, float* qx, float* qy, float* qz,
                                                float* vx, float* vy, float* vz,
                                                float* ax, float* ay, float* az,
                                                float dt, int n)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
   
    if (i >= n) return;
   
    float vx_new = vx[i] + ax[i] * dt;
    float vy_new = vy[i] + ay[i] * dt;
    float vz_new = vz[i] + az[i] * dt;
   
    float qx_new = qx[i] + (vx[i] + vx_new) * 0.5f * dt;
    float qy_new = qy[i] + (vy[i] + vy_new) * 0.5f * dt;
    float qz_new = qz[i] + (vz[i] + vz_new) * 0.5f * dt;
   
    qx[i] = qx_new;
    qy[i] = qy_new;
    qz[i] = qz_new;
    vx[i] = vx_new;
    vy[i] = vy_new;
    vz[i] = vz_new;
}

// Wrapper functions pour être appelées depuis le C++
// NOTE: Ces fonctions doivent être compilées avec NVCC
extern "C" {
    void launchResetAccelerationsKernel(float* ax, float* ay, float* az, int n,
                                       int blocksPerGrid, int threadsPerBlock)
    {
        resetAccelerationsKernel<<<blocksPerGrid, threadsPerBlock>>>(ax, ay, az, n);
    }
   
    void launchComputeAccelerationsKernel(float* m, float* qx, float* qy, float* qz,
                                         float* ax, float* ay, float* az,
                                         float G, float softSquared, int n,
                                         int blocksPerGrid, int threadsPerBlock)
    {
        computeAccelerationsKernel<<<blocksPerGrid, threadsPerBlock>>>(
            m, qx, qy, qz, ax, ay, az, G, softSquared, n);
    }
   
    void launchUpdatePositionsVelocitiesKernel(float* m, float* qx, float* qy, float* qz,
                                              float* vx, float* vy, float* vz,
                                              float* ax, float* ay, float* az,
                                              float dt, int n,
                                              int blocksPerGrid, int threadsPerBlock)
    {
        updatePositionsVelocitiesKernel<<<blocksPerGrid, threadsPerBlock>>>(
            m, qx, qy, qz, vx, vy, vz, ax, ay, az, dt, n);
    }
}
