#ifndef Algebra_SoA_h
#define Algebra_SoA_h

#include <Eigen/Core>
#include <Eigen/Dense>

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"

GENERATE_SOA_LAYOUT(AlgebraSoATemplate,
                    SOA_COLUMN(double, eigenvalues),
                    SOA_COLUMN(double, eigenvector_1),
                    SOA_COLUMN(double, eigenvector_2),
                    SOA_COLUMN(double, eigenvector_3),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, candidateDirection))

using AlgebraSoA = AlgebraSoATemplate<>;
using AlgebraSoAView = AlgebraSoA::View;
using AlgebraSoAConstView = AlgebraSoA::ConstView;

#endif