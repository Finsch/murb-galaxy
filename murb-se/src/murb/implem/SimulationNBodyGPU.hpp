#ifndef SIMULATION_N_BODY_GPU_HPP_
#define SIMULATION_N_BODY_GPU_HPP_

#include <string>

#include "core/SimulationNBodyInterface.hpp"

// Déclaration des fonctions CUDA
#ifdef __cplusplus
extern "C" {
#endif
    void launchResetAccelerationsKernel(float* ax, float* ay, float* az, int n,
                                        int blocksPerGrid, int threadsPerBlock);
    void launchComputeAccelerationsKernel(float* m, float* qx, float* qy, float* qz,
                                          float* ax, float* ay, float* az,
                                          float G, float softSquared, int n,
                                          int blocksPerGrid, int threadsPerBlock);
    void launchUpdatePositionsVelocitiesKernel(float* m, float* qx, float* qy, float* qz,
                                               float* vx, float* vy, float* vz,
                                               float* ax, float* ay, float* az,
                                               float dt, int n,
                                               int blocksPerGrid, int threadsPerBlock);
#ifdef __cplusplus
}
#endif

class SimulationNBodyGPU : public SimulationNBodyInterface {
  protected:
    float* d_m;
    float* d_qx, * d_qy, * d_qz;
    float* d_vx, * d_vy, * d_vz;
    float* d_ax, * d_ay, * d_az;
   
    float* h_ax, * h_ay, * h_az;
   
    int threadsPerBlock;
    int blocksPerGrid;

  public:
    SimulationNBodyGPU(const unsigned long nBodies, const std::string& scheme = "galaxy",
                       const float soft = 0.035f, const unsigned long randInit = 0);
    virtual ~SimulationNBodyGPU();
   
    virtual void computeOneIteration() override;

  protected:
    void initIteration();
    void computeBodiesAcceleration();
    void transferDataToGPU();
    void transferDataFromGPU();
   
    void allocateGPUMemory();
    void freeGPUMemory();
};

#endif /* SIMULATION_N_BODY_GPU_HPP_ */
