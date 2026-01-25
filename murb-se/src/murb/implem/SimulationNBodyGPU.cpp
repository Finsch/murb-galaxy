#include "SimulationNBodyGPU.hpp"

#include <cuda_runtime.h>
#include <cassert>
#include <cmath>
#include <iostream>

// Déclaration des fonctions wrapper pour les kernels
extern void launchResetAccelerationsKernel(float* ax, float* ay, float* az, int n,
                                          int blocksPerGrid, int threadsPerBlock);
extern void launchComputeAccelerationsKernel(float* m, float* qx, float* qy, float* qz,
                                            float* ax, float* ay, float* az,
                                            float G, float softSquared, int n,
                                            int blocksPerGrid, int threadsPerBlock);
extern void launchUpdatePositionsVelocitiesKernel(float* m, float* qx, float* qy, float* qz,
                                                 float* vx, float* vy, float* vz,
                                                 float* ax, float* ay, float* az,
                                                 float dt, int n,
                                                 int blocksPerGrid, int threadsPerBlock);

SimulationNBodyGPU::SimulationNBodyGPU(const unsigned long nBodies, const std::string& scheme,
                                       const float soft, const unsigned long randInit)
    : SimulationNBodyInterface(nBodies, scheme, soft, randInit),
      d_m(nullptr), d_qx(nullptr), d_qy(nullptr), d_qz(nullptr),
      d_vx(nullptr), d_vy(nullptr), d_vz(nullptr),
      d_ax(nullptr), d_ay(nullptr), d_az(nullptr),
      h_ax(nullptr), h_ay(nullptr), h_az(nullptr)
{
    this->flopsPerIte = 20.f * (float)this->bodies.getN() * (float)this->bodies.getN();
    
    threadsPerBlock = 256;
    blocksPerGrid = (this->bodies.getN() + threadsPerBlock - 1) / threadsPerBlock;
    
    allocateGPUMemory();
    transferDataToGPU();
    
    size_t n = this->bodies.getN() + this->bodies.getPadding();
    h_ax = new float[n];
    h_ay = new float[n];
    h_az = new float[n];
}

SimulationNBodyGPU::~SimulationNBodyGPU()
{
    freeGPUMemory();
    delete[] h_ax;
    delete[] h_ay;
    delete[] h_az;
}

void SimulationNBodyGPU::allocateGPUMemory()
{
    size_t n = this->bodies.getN() + this->bodies.getPadding();
    size_t size = n * sizeof(float);
    
    cudaError_t err;
    
    err = cudaMalloc(&d_m, size);
    if (err != cudaSuccess) std::cerr << "cudaMalloc d_m: " << cudaGetErrorString(err) << std::endl;
    
    err = cudaMalloc(&d_qx, size);
    if (err != cudaSuccess) std::cerr << "cudaMalloc d_qx: " << cudaGetErrorString(err) << std::endl;
    
    err = cudaMalloc(&d_qy, size);
    if (err != cudaSuccess) std::cerr << "cudaMalloc d_qy: " << cudaGetErrorString(err) << std::endl;
    
    err = cudaMalloc(&d_qz, size);
    if (err != cudaSuccess) std::cerr << "cudaMalloc d_qz: " << cudaGetErrorString(err) << std::endl;
    
    err = cudaMalloc(&d_vx, size);
    if (err != cudaSuccess) std::cerr << "cudaMalloc d_vx: " << cudaGetErrorString(err) << std::endl;
    
    err = cudaMalloc(&d_vy, size);
    if (err != cudaSuccess) std::cerr << "cudaMalloc d_vy: " << cudaGetErrorString(err) << std::endl;
    
    err = cudaMalloc(&d_vz, size);
    if (err != cudaSuccess) std::cerr << "cudaMalloc d_vz: " << cudaGetErrorString(err) << std::endl;
    
    err = cudaMalloc(&d_ax, size);
    if (err != cudaSuccess) std::cerr << "cudaMalloc d_ax: " << cudaGetErrorString(err) << std::endl;
    
    err = cudaMalloc(&d_ay, size);
    if (err != cudaSuccess) std::cerr << "cudaMalloc d_ay: " << cudaGetErrorString(err) << std::endl;
    
    err = cudaMalloc(&d_az, size);
    if (err != cudaSuccess) std::cerr << "cudaMalloc d_az: " << cudaGetErrorString(err) << std::endl;
    
    this->allocatedBytes += 9 * size;
}

void SimulationNBodyGPU::freeGPUMemory()
{
    cudaFree(d_m);
    cudaFree(d_qx);
    cudaFree(d_qy);
    cudaFree(d_qz);
    cudaFree(d_vx);
    cudaFree(d_vy);
    cudaFree(d_vz);
    cudaFree(d_ax);
    cudaFree(d_ay);
    cudaFree(d_az);
}

void SimulationNBodyGPU::transferDataToGPU()
{
    size_t n = this->bodies.getN() + this->bodies.getPadding();
    size_t size = n * sizeof(float);
   
    auto& dataSoA = this->bodies.getDataSoA();
   
    // Cast les données constantes en non-constantes
    cudaMemcpy(d_m, dataSoA.m.data(), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_qx, dataSoA.qx.data(), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_qy, dataSoA.qy.data(), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_qz, dataSoA.qz.data(), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_vx, dataSoA.vx.data(), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_vy, dataSoA.vy.data(), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_vz, dataSoA.vz.data(), size, cudaMemcpyHostToDevice);
   
    cudaDeviceSynchronize();
}

void SimulationNBodyGPU::transferDataFromGPU()
{
    size_t n = this->bodies.getN() + this->bodies.getPadding();
    size_t size = n * sizeof(float);
   
    auto& dataSoA = this->bodies.getDataSoA();
   
    // Utiliser const_cast pour enlever la constness
    cudaMemcpy(const_cast<float*>(dataSoA.qx.data()), d_qx, size, cudaMemcpyDeviceToHost);
    cudaMemcpy(const_cast<float*>(dataSoA.qy.data()), d_qy, size, cudaMemcpyDeviceToHost);
    cudaMemcpy(const_cast<float*>(dataSoA.qz.data()), d_qz, size, cudaMemcpyDeviceToHost);
    cudaMemcpy(const_cast<float*>(dataSoA.vx.data()), d_vx, size, cudaMemcpyDeviceToHost);
    cudaMemcpy(const_cast<float*>(dataSoA.vy.data()), d_vy, size, cudaMemcpyDeviceToHost);
    cudaMemcpy(const_cast<float*>(dataSoA.vz.data()), d_vz, size, cudaMemcpyDeviceToHost);
   
    cudaDeviceSynchronize();
}




void SimulationNBodyGPU::initIteration()
{
    launchResetAccelerationsKernel(d_ax, d_ay, d_az, this->bodies.getN(),
                                  blocksPerGrid, threadsPerBlock);
    cudaDeviceSynchronize();
}

void SimulationNBodyGPU::computeBodiesAcceleration()
{
    const float softSquared = this->soft * this->soft;
    
    launchComputeAccelerationsKernel(d_m, d_qx, d_qy, d_qz, d_ax, d_ay, d_az,
                                    this->G, softSquared, this->bodies.getN(),
                                    blocksPerGrid, threadsPerBlock);
    cudaDeviceSynchronize();
}

void SimulationNBodyGPU::computeOneIteration()
{
    this->initIteration();
    this->computeBodiesAcceleration();
    
    launchUpdatePositionsVelocitiesKernel(d_m, d_qx, d_qy, d_qz, d_vx, d_vy, d_vz,
                                         d_ax, d_ay, d_az, this->dt, this->bodies.getN(),
                                         blocksPerGrid, threadsPerBlock);
    
    cudaDeviceSynchronize();
    transferDataFromGPU();
}
