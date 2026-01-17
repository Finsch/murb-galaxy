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
    this->flopsPerIte = 20.f * (float)this->getBodies().getN() * (float)this->getBodies().getN();
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

/* => Nombre de FPS dans la version naïve : environ 82.1 FPS

Comment optimiser ici? 

- Symétrie des forces : F_ij = -F_ji, on peut se contenter de faire la moitié des opérations 
- Calcul du softening factor en dehors de la boucle
- Suppression des appels à pow

    => Avec ces améliorations, on a environ 185.7 FPS soit un gain de 103.6 FPS par rapport à la version naïve.
*/


void SimulationNBodyOptim::computeBodiesAcceleration()
{
    // Récupération de les positions, des vitesses, des rayons et des masses de tout les corps
    const std::vector<dataAoS_t<float>> &d = this->getBodies().getDataAoS(); 

    // compute e² (en dehors de la boucle)
    const float softSquared = std::pow(this->soft, 2); // 1 flops

    // flops = n(n-1)/2 * 30 (au lieu de n² * 20)
    for (unsigned long iBody = 0; iBody < this->getBodies().getN(); iBody++) {
        // On commence à iBody+1 pour éviter les paires redondantes et les auto-interactions
        for (unsigned long jBody = iBody + 1; jBody < this->getBodies().getN(); jBody++) {

            //Calcul de la distance entre le corps courant avec tout les autres corps
            const float rijx = d[jBody].qx - d[iBody].qx; // 1 flop
            const float rijy = d[jBody].qy - d[iBody].qy; // 1 flop
            const float rijz = d[jBody].qz - d[iBody].qz; // 1 flop

            // compute the || rij ||² distance between body i and body j
            const float rijSquared = rijx*rijx + rijy*rijy + rijz*rijz; // 3 flops (au lieu de 5)
            // compute the acceleration value between body i and body j: || ai || = G.mj / (|| rij ||² + e²)^{3/2}
            const float distSquared = rijSquared + softSquared;
            const float invDist = 1.0f / std::sqrt(distSquared);
            const float invDistCubed = invDist * invDist * invDist;
            const float ai = this->G * d[jBody].m * invDistCubed; // 6 flops (au lieu de 5)
            // acceleration value for body j due to body i (réciproque)
            const float aj = this->G * d[iBody].m * invDistCubed; // 2 flops (au lieu de 5)

            // add the acceleration value into the acceleration vector: ai += || ai ||.rij
            this->accelerations[iBody].ax += ai * rijx; // 2 flops
            this->accelerations[iBody].ay += ai * rijy; // 2 flops
            this->accelerations[iBody].az += ai * rijz; // 2 flops
            
            // Pour le corps j, la force est opposée (loi des actions réciproques)
            this->accelerations[jBody].ax -= aj * rijx; // 2 flops (signe opposé)
            this->accelerations[jBody].ay -= aj * rijy; // 2 flops (signe opposé)
            this->accelerations[jBody].az -= aj * rijz; // 2 flops (signe opposé)
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
