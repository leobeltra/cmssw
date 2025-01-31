#include <iostream>

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"

// #define GENERATE_SHIT(CLASS, ...) \
//     struct CLASS { \
//         MAKE_COMMAS(_ITERATE_ON_ALL_METHODS(GENERATE_METHODS_IF_VALID, ~, __VA_ARGS__))    \
//     } \
                                     
// GENERATE_SHIT(shit, 
//               SOA_COLUMN(double, x),
//               SOA_METHODS(template <typename T1, typename T2, typename T3>
//                                          auto more_then_one_input(T1 x, T2 y) { return x + y;}
//                                          template <typename Q1, typename... Args> void print(Q1 first, Args... rest) {
//                                         std::cout << first << " ";
//                                             print(rest...);  // Ricorsione variadica
// }))   

// // MAKE_COMMAS(GENERATE_ALL_METHODS(_ITERATE_ON_ALL(GENERATE_METHODS_IF_VALID, ~,  SOA_METHODS(
// //                                                           template <typename T1, typename T2, typename T3>
// //                                                           auto more_then_one_input(T1 x, T2 y) { return x + y;}
// //                                                           template <typename Q1, typename... Args> void print(Q1 first, Args... rest) {
// //                                                             std::cout << first << " ";
// //                                                             print(rest...);  // Ricorsione variadica
// //                                                           }))))    

// MAKE_COMMAS(GENERATE_METHODS_IF_VALID(SOA_METHODS(
//             template <typename T1, typename T2, typename T3>
//             auto more_then_one_input(T1 x, T2 y) { return x + y;}
//             template <typename Q1, typename... Args> void print(Q1 first, Args... rest) {
//             std::cout << first << " ";
//             print(rest...);  // Ricorsione variadica
//             })))

// FIRST_ELEM_TUPLE_IN_SEQ(SOA_METHODS(
//             template <typename T1, typename T2, typename T3>
//             auto more_then_one_input(T1 x, T2 y) { return x + y;}
//             template <typename Q1, typename... Args> void print(Q1 first, Args... rest) {
//             std::cout << first << " ";
//             print(rest...);  // Ricorsione variadica
//             }))

// MAKE_COMMAS_IN_SEQ(SOA_METHODS(
//             template <typename T1, typename T2, typename T3>
//             auto more_then_one_input(T1 x, T2 y) { return x + y;}
//             template <typename Q1, typename... Args> void print(Q1 first, Args... rest) {
//             std::cout << first << " ";
//             print(rest...);  // Ricorsione variadica
//             }))

GENERATE_METHODS_IF_VALID(SOA_METHODS(
            template <typename T1, typename T2, typename T3>
            auto more_then_one_input(T1 x, T2 y) { return x + y;}
            template <typename Q1, typename... Args> void print(Q1 first, Args... rest) {
            std::cout << first << " ";
            print(rest...);  // Ricorsione variadica
            }))