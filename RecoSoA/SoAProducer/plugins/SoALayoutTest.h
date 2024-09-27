#ifndef SoA_Layout_Test_h
#define SoA_Layout_Test_h

#include <Eigen/Core>
#include <Eigen/Dense>

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"


GENERATE_SOA_LAYOUT(SoAHostLayoutTemplate,
                    /*SoAHostViewTemplate,*/
                    // predefined static scalars
                    // size_t size;
                    // size_t alignment;

                    // columns: one value per element
                    SOA_COLUMN(double, x),
                    SOA_COLUMN(double, y),
                    SOA_COLUMN(double, z),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, a),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, b),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, r),
                    // scalars: one value for the whole structure
                    SOA_SCALAR(const char*, description),
                    SOA_SCALAR(uint32_t, someNumber))

using SoAHostLayout = SoAHostLayoutTemplate<>;
using SoAHostView = SoAHostLayout::View;

#endif