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
    // RÉCUPÉRATION DES DONNÉES : positions, vitesses, rayons et masses de tous les corps
    const std::vector<dataAoS_t<float>> &d = this->getBodies().getDataAoS();
    
    const unsigned long n = this->getBodies().getN(); // get number of bodies, to avoid multiple calls in the loops

    // OPTIMISATION #1: softSquared en dehors des boucles
    // OPTIMISATION #2: Remplacer std::pow par des multiplications directes
    // OPTIMISATION #3: Symétrie des forces - on calcule seulement pour j > i
        // on ne calcule pas fii (-1) et on utilise la symétrie fij = -fji (/2)
                // A -> (B,C,D); B -> (C,D); C -> (D)
                // i (1->n) et j (i+1->n)

    const float softSquared = this->soft * this->soft; // 1 flop

    for (unsigned long iBody = 0; iBody < n; iBody++) {
        // flops = n * 20
        for (unsigned long jBody = iBody + 1; jBody < n; jBody++) {
            const float rijx = d[jBody].qx - d[iBody].qx; // 1 flop
            const float rijy = d[jBody].qy - d[iBody].qy; // 1 flop
            const float rijz = d[jBody].qz - d[iBody].qz; // 1 flop

            // compute the || rij ||² distance between body i and body j
            const float rijSquared = rijx*rijx + rijy*rijy + rijz*rijz; // 3 mult + 2 add = 5 flops

            // compute the acceleration value between body i and body j: || ai || = G.mj / (|| rij ||² + e²)^{3/2}
            // const float ai = this->G * d[jBody].m / std::pow(rijSquared + softSquared, 3.f / 2.f); // 5 flops
            const float invDist = 1.0f / std::sqrt(rijSquared + softSquared); // 3 flops
            const float invDistCube = invDist * invDist * invDist; // 2 flops
            const float ai = this->G * d[jBody].m * invDistCube; // 2 flops 

            const float aj = this->G * d[iBody].m * invDistCube; // 2 flops 
            
            // add the acceleration value into the acceleration vector: ai += || ai ||.rij
            this->accelerations[iBody].ax += ai * rijx; // 2 flops
            this->accelerations[iBody].ay += ai * rijy; // 2 flops
            this->accelerations[iBody].az += ai * rijz; // 2 flops

            // Pour le corps j (utilisant la symétrie: f_ji = -f_ij)
            this->accelerations[jBody].ax -= aj * rijx; // 2 flops
            this->accelerations[jBody].ay -= aj * rijy; // 2 flops
            this->accelerations[jBody].az -= aj * rijz; // 2 flops
        }
    }
}

void SimulationNBodyOptim::computeOneIteration()
{
    this->initIteration();
    this->computeBodiesAcceleration();
    // time integration
    this->bodies.updatePositionsAndVelocities(this->accelerations, this->dt);
}