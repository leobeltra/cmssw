#ifndef SoA_Layout_Test_h
#define SoA_Layout_Test_h

#include <Eigen/Core>
#include <Eigen/Dense>

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"
#include <iostream>

GENERATE_SOA_LAYOUT(SoAHostPositionLayoutTemplate,
                    /*SoAHostViewTemplate,*/
                    // predefined static scalars
                    // size_t size;
                    // size_t alignment;

                    // columns: one value per element
                    SOA_COLUMN(double, x),
                    SOA_COLUMN(double, y),
                    SOA_COLUMN(double, z),
                    // SOA_EIGEN_COLUMN(Eigen::Vector3d, a),
                    // SOA_EIGEN_COLUMN(Eigen::Vector3d, b),
                    // SOA_EIGEN_COLUMN(Eigen::Vector3d, r),
                    // scalars: one value for the whole structure
                    SOA_SCALAR(const char*, description),
                    SOA_SCALAR(uint32_t, someNumber))

using SoAHostPositionLayout = SoAHostPositionLayoutTemplate<>;
using SoAHostPositionView = SoAHostPositionLayout::View;

GENERATE_SOA_LAYOUT(SoAHostVelocityLayoutTemplate,
                    SOA_COLUMN(double, v_x),
                    SOA_COLUMN(double, v_y),
                    SOA_COLUMN(double, v_z),
                    SOA_SCALAR(const char*, description2),
                    SOA_SCALAR(uint32_t, someNumber2))
                    
using SoAHostVelocityLayout = SoAHostVelocityLayoutTemplate<>;    
using SoAHostVelocityView = SoAHostVelocityLayout::View;                


inline void printSoAView(SoAHostPositionView view){
    std::cout << "SoAHostDeviceView:" << std::endl;
    std::cout << "description: " << view.description() << std::endl;
    std::cout << "someNumber: " << view.someNumber() << std::endl;
    for (auto i = 0; i < view.metadata().size(); ++i) {
        std::cout << "Element " << i << ": ";
        std::cout << "x = " << view.x()[i] << ", ";
        std::cout << "y = " << view.y()[i] << ", ";
        std::cout << "z = " << view.z()[i] << std::endl;
        // std::cout << "a = " << view[i].a().transpose() << ", ";
        // std::cout << "b = " << view[i].b().transpose() << ", ";
        // std::cout << "r = " << view[i].r().transpose() << std::endl;
    }
};

#endif
