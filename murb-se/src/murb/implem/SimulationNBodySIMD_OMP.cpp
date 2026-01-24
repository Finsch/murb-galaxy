#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include "SimulationNBodySIMD_OMP.hpp"
#include "mipp.h"


SimulationNBodySIMD_OMP::SimulationNBodySIMD_OMP(const unsigned long nBodies, const std::string &scheme, const float soft,
                                           const unsigned long randInit)
    : SimulationNBodyInterface(nBodies, scheme, soft, randInit)
{
    this->flopsPerIte = 20.f * (float)this->getBodies().getN() * (float)this->getBodies().getN();
    this->accelerations.resize(this->getBodies().getN());
}

void SimulationNBodySIMD_OMP::initIteration()
{
    for (unsigned long iBody = 0; iBody < this->getBodies().getN(); iBody++) {
        this->accelerations[iBody].ax = 0.f;
        this->accelerations[iBody].ay = 0.f;
        this->accelerations[iBody].az = 0.f;
    }
}

void SimulationNBodySIMD_OMP::computeBodiesAcceleration()
{
    const auto& d = this->getBodies().getDataSoA();   
    const float* qx = d.qx.data();
    const float* qy = d.qy.data();
    const float* qz = d.qz.data();
    const float* m  = d.m.data();
    
    const unsigned long n = this->getBodies().getN();
    const float softSquared = this->soft * this->soft;
    const float G_local = this->G;
    
    constexpr int SIMD_SIZE = mipp::N<float>();
    
    // ========== OPENMP + SIMD ==========
    // Suggested cmdline(s):
    // export OMP_NUM_THREADS=8
    // export OMP_SCHEDULE="dynamic,4"
    // ./bin/murb -n 5000 -i 100 -v --nv --im cpu+simd+omp
    
    #pragma omp parallel
    {
        // Pas besoin de tableau local ici car pas de symétrie !
        // Chaque iBody est indépendant        
        #pragma omp for schedule(runtime)
        for (unsigned long iBody = 0; iBody < n; iBody++) {
            const float qx_i = qx[iBody];
            const float qy_i = qy[iBody];
            const float qz_i = qz[iBody];
            
            // Accumulateurs VECTORIELS
            mipp::Reg<float> axi_vec(0.f);
            mipp::Reg<float> ayi_vec(0.f);
            mipp::Reg<float> azi_vec(0.f);
            
            // SIMD mask pour éviter self-interaction (i == j)
            mipp::Reg<unsigned int> mask_vec;
            
            // Boucle vectorielle principale
            unsigned long jBody = 0;
            
            // Boucle vectorielle principale
            for (; jBody + SIMD_SIZE <= n; jBody += SIMD_SIZE) {
                // Chargements vectoriels
                mipp::Reg<float> qx_j = mipp::loadu<float>(&qx[jBody]);
                mipp::Reg<float> qy_j = mipp::loadu<float>(&qy[jBody]);
                mipp::Reg<float> qz_j = mipp::loadu<float>(&qz[jBody]);
                mipp::Reg<float> m_j = mipp::loadu<float>(&m[jBody]);
                
                // Calculs vectoriels
                mipp::Reg<float> rijx = qx_j - qx_i;
                mipp::Reg<float> rijy = qy_j - qy_i;
                mipp::Reg<float> rijz = qz_j - qz_i;
                
                mipp::Reg<float> rijSquared = rijx * rijx + rijy * rijy + rijz * rijz;
                mipp::Reg<float> distSq = rijSquared + softSquared;
                mipp::Reg<float> invDist = mipp::rsqrt(distSq);  // 1/sqrt(distSq) - SIMD!
                mipp::Reg<float> invDistCube = invDist * invDist * invDist;
                
                mipp::Reg<float> ai = m_j * invDistCube * G_local;
                
                // Accumulation vectorielle
                axi_vec += ai * rijx;
                ayi_vec += ai * rijy;
                azi_vec += ai * rijz;
            }
            
            // Réduction horizontale
            float axi = mipp::hadd(axi_vec);
            float ayi = mipp::hadd(ayi_vec);
            float azi = mipp::hadd(azi_vec);
            
            // Reste scalaire final
            for (; jBody < n; jBody++) {
                const float rijx = qx[jBody] - qx_i;
                const float rijy = qy[jBody] - qy_i;
                const float rijz = qz[jBody] - qz_i;
                
                const float rijSquared = rijx*rijx + rijy*rijy + rijz*rijz;
                const float distSq = rijSquared + softSquared;
                const float invDist = 1.0f / std::sqrt(distSq);
                const float invDistCube = invDist * invDist * invDist;
                
                const float ai = G_local * m[jBody] * invDistCube;
                
                axi += ai * rijx;
                ayi += ai * rijy;
                azi += ai * rijz;
            }
            
            // Stockage direct (pas de conflit car chaque iBody est unique)
            this->accelerations[iBody].ax = axi;
            this->accelerations[iBody].ay = ayi;
            this->accelerations[iBody].az = azi;
        }
    }
}

void SimulationNBodySIMD_OMP::computeOneIteration()
{
    this->initIteration();
    this->computeBodiesAcceleration();
    // time integration
    this->bodies.updatePositionsAndVelocities(this->accelerations, this->dt);
}
