#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include "SimulationNBodySIMD.hpp"
#include "mipp.h"


SimulationNBodySIMD::SimulationNBodySIMD(const unsigned long nBodies, const std::string &scheme, const float soft,
                                           const unsigned long randInit)
    : SimulationNBodyInterface(nBodies, scheme, soft, randInit)
{
    // METRIQUE DE PERFORMANCE
    this->flopsPerIte = 20.f * (float)this->getBodies().getN() * (float)this->getBodies().getN(); // Number of floating-point operations per iteration.
    // ALLOCATION MEMOIRE
    this->accelerations.resize(this->getBodies().getN());
}

void SimulationNBodySIMD::initIteration()
{
    for (unsigned long iBody = 0; iBody < this->getBodies().getN(); iBody++) {
        this->accelerations[iBody].ax = 0.f;
        this->accelerations[iBody].ay = 0.f;
        this->accelerations[iBody].az = 0.f;
    }
}


void SimulationNBodySIMD::computeBodiesAcceleration()
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

    for (unsigned long iBody = 0; iBody < n; iBody++) {
        const float qx_i = qx[iBody];
        const float qy_i = qy[iBody];
        const float qz_i = qz[iBody];
        
        // Accumulateurs VECTORIELS
        mipp::Reg<float> axi_vec(0.f);
        mipp::Reg<float> ayi_vec(0.f);
        mipp::Reg<float> azi_vec(0.f);
        
        // Boucle vectorielle
        unsigned long jBody = 0;
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
            // mipp::Reg<float> distSqrt = mipp::sqrt(distSq);
            // mipp::Reg<float> invDistCube = mipp::Reg<float>(1.0f) / (distSq * distSqrt);
            mipp::Reg<float> invDist = mipp::rsqrt(distSq);  // 1/sqrt(distSq) - plus rapide !
            mipp::Reg<float> invDistCube = invDist * invDist * invDist;  // (1/sqrt)^3 = 1/distSq^(3/2)
            
            mipp::Reg<float> ai = m_j * invDistCube * G_local;
            
            mipp::Reg<float> ax_contrib = ai * rijx;
            mipp::Reg<float> ay_contrib = ai * rijy;
            mipp::Reg<float> az_contrib = ai * rijz;
            
            // Accumulation vectorielle
            axi_vec += ax_contrib;
            ayi_vec += ay_contrib;
            azi_vec += az_contrib;
        }
        
        // Réduction horizontale UNE SEULE FOIS
        float axi = mipp::hadd(axi_vec);
        float ayi = mipp::hadd(ayi_vec);
        float azi = mipp::hadd(azi_vec);

        // Reste scalaire
        for (; jBody < n; jBody++) {
            const float rijx = qx[jBody] - qx_i;
            const float rijy = qy[jBody] - qy_i;
            const float rijz = qz[jBody] - qz_i;
            
            const float rijSquared = rijx*rijx + rijy*rijy + rijz*rijz;
            const float distSq = rijSquared + softSquared;
            // const float invDistCube = 1.0f / (distSq * std::sqrt(distSq));
            const float invDist = 1.0f / std::sqrt(distSq);
            const float invDistCube = invDist * invDist * invDist;

            const float ai = G_local * m[jBody] * invDistCube;
            
            axi += ai * rijx;
            ayi += ai * rijy;
            azi += ai * rijz;
        }
        
        this->accelerations[iBody].ax = axi;
        this->accelerations[iBody].ay = ayi;
        this->accelerations[iBody].az = azi;
    }
}


/* KEEP THIS : Its NBodyOptim without symmetry
void SimulationNBodySIMD::computeBodiesAcceleration()
{
    
    // OPTIMISATIONS SÉQUENTIELLES :
    // #1: softSquared en dehors des boucles
    // #2: Remplacer std::pow par des multiplications directes  
    // #3: Symétrie des forces - on calcule seulement pour j > i
    // #4: Récupérer les données en SoA pour une meilleure localité mémoire

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
        
        for (unsigned long jBody = 0; jBody < n; jBody++) {
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
            // const float aj = G_local * m_i * invDistCube;      // 2 flops 
            
            // Accumulation pour le corps i
            // add the acceleration value into the acceleration vector: ai 
            axi += ai * rijx; // 2 flops
            ayi += ai * rijy; // 2 flops
            azi += ai * rijz; // 2 flops

            // // Pour le corps j (utilisant la symétrie: f_ji = -f_ij)
            // this->accelerations[jBody].ax -= aj * rijx; // 2 flops
            // this->accelerations[jBody].ay -= aj * rijy; // 2 flops
            // this->accelerations[jBody].az -= aj * rijz; // 2 flops
        }
        
        // Stockage final des accumulations pour le corps i
        this->accelerations[iBody].ax = axi;
        this->accelerations[iBody].ay = ayi;
        this->accelerations[iBody].az = azi;
    }
}

*/



void SimulationNBodySIMD::computeOneIteration()
{
    this->initIteration();
    this->computeBodiesAcceleration();
    // time integration
    this->bodies.updatePositionsAndVelocities(this->accelerations, this->dt);
}