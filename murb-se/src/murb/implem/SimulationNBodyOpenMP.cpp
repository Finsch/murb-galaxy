#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include "SimulationNBodyOptim.hpp"

SimulationNBodyOptim::SimulationNBodyOptim(const unsigned long nBodies, const std::string &scheme, const float soft,
                                           const unsigned long randInit)
    : SimulationNBodyInterface(nBodies, scheme, soft, randInit)
{
    // METRIQUE DE PERFORMANCE
    this->flopsPerIte = 20.f * (float)this->getBodies().getN() * (float)this->getBodies().getN(); // Number of floating-point operations per iteration.
    // ALLOCATION MEMOIRE
    this->accelerations.resize(this->getBodies().getN());
}

void SimulationNBodyOptim::initIteration()
{
    for (unsigned long iBody = 0; iBody < this->getBodies().getN(); iBody++) {
        this->accelerations[iBody].ax = 0.f;
        this->accelerations[iBody].ay = 0.f;
        this->accelerations[iBody].az = 0.f;
    }
}

void SimulationNBodyOptim::computeBodiesAcceleration()
{
    // RÉCUPÉRATION DES DONNÉES : positions, vitesses, rayons et masses en SoA
    const auto& d = this->getBodies().getDataSoA();   

    // Pointeurs vers les tableaux séparés
    const float* qx = d.qx.data();  // Tableau des positions x
    const float* qy = d.qy.data();  // Tableau des positions y
    const float* qz = d.qz.data();  // Tableau des positions z
    const float* m  = d.m.data();   // Tableau des masses
    
    const unsigned long n = this->getBodies().getN(); // get number of bodies, to avoid multiple calls in the loops

    const float softSquared = this->soft * this->soft; // 1 flop
    const float G_local = this->G; // Copie locale pour optimisation

    for (unsigned long iBody = 0; iBody < n; iBody++) {

        const float qx_i = qx[iBody];  // Chargement une seule fois
        const float qy_i = qy[iBody];
        const float qz_i = qz[iBody];
        const float m_i  = m[iBody];
        
        // Variables temporaires pour accumuler l'accélération de iBody
        float axi = 0.f, ayi = 0.f, azi = 0.f;
        
        for (unsigned long jBody = iBody + 1; jBody < n; jBody++) {
            // Calcul du vecteur distance
            const float rijx = qx[jBody] - qx_i; // 1 flop
            const float rijy = qy[jBody] - qy_i; // 1 flop
            const float rijz = qz[jBody] - qz_i; // 1 flop

            // Distance au carré
            // compute the || rij ||² distance between body i and body j
            const float rijSquared = rijx*rijx + rijy*rijy + rijz*rijz; // 3 mult + 2 add = 5 flops

            // Calcul de l'accélération
            // compute the acceleration value between body i and body j: || ai || = G.mj / (|| rij ||² + e²)^{3/2}
            const float distSq = rijSquared + softSquared; // 1 flop
            const float invDistCube = 1.0f / (distSq * std::sqrt(distSq)); // 3 flops

            // Accélérations scalaires
            const float ai = G_local * m[jBody] * invDistCube; // 2 flops 
            const float aj = G_local * m_i * invDistCube;      // 2 flops 
            
            // Accumulation pour le corps i
            // add the acceleration value into the acceleration vector: ai 
            axi += ai * rijx; // 2 flops
            ayi += ai * rijy; // 2 flops
            azi += ai * rijz; // 2 flops

            // Pour le corps j (utilisant la symétrie: f_ji = -f_ij)
            this->accelerations[jBody].ax -= aj * rijx; // 2 flops
            this->accelerations[jBody].ay -= aj * rijy; // 2 flops
            this->accelerations[jBody].az -= aj * rijz; // 2 flops
        }
        
        // Stockage final des accumulations pour le corps i
        this->accelerations[iBody].ax += axi;
        this->accelerations[iBody].ay += ayi;
        this->accelerations[iBody].az += azi;
    }
}

void SimulationNBodyOptim::computeOneIteration()
{
    this->initIteration();
    this->computeBodiesAcceleration();
    // time integration
    this->bodies.updatePositionsAndVelocities(this->accelerations, this->dt);
}