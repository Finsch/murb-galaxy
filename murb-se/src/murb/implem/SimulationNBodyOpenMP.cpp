#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <omp.h>  // newxd

#include "SimulationNBodyOpenMP.hpp"

SimulationNBodyOpenMP::SimulationNBodyOpenMP(const unsigned long nBodies, const std::string &scheme, 
                                             const float soft, const unsigned long randInit)
    : SimulationNBodyInterface(nBodies, scheme, soft, randInit)
{
    this->flopsPerIte = 20.f * (float)this->getBodies().getN() * (float)this->getBodies().getN(); 
    this->accelerations.resize(this->getBodies().getN());
}

void SimulationNBodyOpenMP::initIteration()
{
    const unsigned long n = this->getBodies().getN();
    
    for (unsigned long iBody = 0; iBody < n; iBody++) {
        this->accelerations[iBody].ax = 0.f;
        this->accelerations[iBody].ay = 0.f;
        this->accelerations[iBody].az = 0.f;
    }
}

void SimulationNBodyOpenMP::computeBodiesAcceleration()
{
    // Récupération des données en SoA 
    const auto& d = this->getBodies().getDataSoA();   

    const float* qx = d.qx.data();
    const float* qy = d.qy.data();
    const float* qz = d.qz.data();
    const float* m  = d.m.data();
    
    const unsigned long n = this->getBodies().getN();    
    const float softSquared = this->soft * this->soft;
    const float G_local = this->G;

    // ========== PARALLÉLISATION OPENMP ==========
    // Suggested cmdline(s):
    // export OMP_NUM_THREADS=8
    // export OMP_SCHEDULE="static,2"
    // ./bin/murb -n 1000 -i 1000 -v --nv --im cpu+omp
    
    #pragma omp parallel
    {
        // 1. Tableau local par thread (évite false sharing)
        std::vector<accAoS_t<float>> local_accelerations(n);
        for (unsigned long iBody = 0; iBody < n; iBody++) {
            local_accelerations[iBody].ax = 0.f;
            local_accelerations[iBody].ay = 0.f;
            local_accelerations[iBody].az = 0.f;
        }

        // 2. Parallélisation de la boucle externe SEULEMENT
        #pragma omp for schedule(runtime) 
        for (unsigned long iBody = 0; iBody < n; iBody++) {
            const float qx_i = qx[iBody];
            const float qy_i = qy[iBody];
            const float qz_i = qz[iBody];
            const float m_i  = m[iBody];
            
            // Accumulateurs locaux pour iBody (optimisation cache)
            float axi = 0.f, ayi = 0.f, azi = 0.f;
            
            // Boucle interne séquentielle (traitée par chaque thread)
            for (unsigned long jBody = iBody + 1; jBody < n; jBody++) {
                const float rijx = qx[jBody] - qx_i;
                const float rijy = qy[jBody] - qy_i;
                const float rijz = qz[jBody] - qz_i;

                const float rijSquared = rijx*rijx + rijy*rijy + rijz*rijz; 
                const float distSq = rijSquared + softSquared;
                const float invDistCube = 1.0f / (distSq * std::sqrt(distSq));

                const float ai = G_local * m[jBody] * invDistCube; 
                const float aj = G_local * m_i * invDistCube; 
                
                // Accumulation pour iBody (local au thread)
                axi += ai * rijx;
                ayi += ai * rijy;
                azi += ai * rijz;

                // Mise à jour de jBody dans le tableau local du thread
                // Pas de conflit car chaque thread a son propre tableau
                local_accelerations[jBody].ax -= aj * rijx;
                local_accelerations[jBody].ay -= aj * rijy;
                local_accelerations[jBody].az -= aj * rijz;
            }
            
            // Stockage final des accumulations pour le corps i
            local_accelerations[iBody].ax += axi;
            local_accelerations[iBody].ay += ayi;
            local_accelerations[iBody].az += azi;
        }

        // Réduction : Fusionner les tableaux locaux dans le tableau global
        // 3. Réduction avec section critique
        #pragma omp critical
        {
        for (unsigned long iBody = 0; iBody < n; iBody++) {
            this->accelerations[iBody].ax += local_accelerations[iBody].ax;
            this->accelerations[iBody].ay += local_accelerations[iBody].ay;
            this->accelerations[iBody].az += local_accelerations[iBody].az;
        }
        }
    }
}

void SimulationNBodyOpenMP::computeOneIteration()
{
    this->initIteration();
    this->computeBodiesAcceleration();
    // time integration
    this->bodies.updatePositionsAndVelocities(this->accelerations, this->dt);
}