/*
 * CollidableParticleBase.h
 *
 *  Created on: 10 Mar 2014
 *      Author: ttitsche
 */

#pragma once

#include <eigen3/Eigen/Core>
#include "geometryConcrete/collision/collidables/CollidableBase.h"

namespace NuTo
{

//! @brief ... base class for all particles
//! TODO: Idea: implement an interface that somehow every particle
//! knows how to interact with every wall without explicit coding
class CollidableParticleBase: public NuTo::CollidableBase
{
public:

	//! @brief ... constructor, every particle needs a position, a velocity and an identifier
	//! @param rPosition ... position
	//! @param rVelocity ... velocity
	//! @param rIndex ... index, multiple particles with same index are allowed
	CollidableParticleBase(
			Eigen::VectorXd rPosition,
			Eigen::VectorXd rVelocity,
			const int rIndex);

	//! @brief ... getter for the particle volume
	virtual const double GetVolume() const = 0;

	//! @brief ... getter for the kinetic energy of the particle
	virtual const double GetKineticEnergy() const = 0;

protected:

	//! @brief particle position
	Eigen::Vector3d mPosition;

	//! @brief particle velocity
	Eigen::Vector3d mVelocity;

};

} /* namespace NuTo */