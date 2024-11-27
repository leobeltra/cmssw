#include <catch.hpp>
// #include <string>

#include "DataFormats/Portable/interface/PortableCollection.h"
#include "DataFormats/Portable/interface/PortableHostCollection.h"
#include "DataFormats/Portable/interface/PortableView.h"
#include "DataFormats/SoATemplate/interface/SoACommon.h"
#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"

namespace {
  GENERATE_SOA_LAYOUT(TestLayout1, SOA_COLUMN(double, x), SOA_COLUMN(double, y), SOA_COLUMN(int32_t, id))
  GENERATE_SOA_LAYOUT(TestLayout2, SOA_COLUMN(double, v_x), SOA_COLUMN(double, v_y))

  GENERATE_SOA_LAYOUT(SoA, SOA_COLUMN(double, p_x), SOA_COLUMN(double, p_y))

  using TestSoA1 = TestLayout1<>;
  using TestSoA2 = TestLayout2<>;
  using TestSoA1View = TestSoA1::View;
  using TestSoA2View = TestSoA2::View;

//   using TestSoA = SoA<>;
//   using TestSoAView = TestSoA::View;

  using TestSoA1Collection = PortableHostCollection<TestSoA1>;
  using TestSoA2Collection = PortableHostCollection<TestSoA2>;
  //using TestSoACollection = PortableHostCollection<TestSoA>;

  using TestSoAView = PortableView<TestSoA1Collection, TestSoA2Collection>;

  constexpr auto s_tag = "[PortableView]";
}  // namespace

void printSoAView(TestSoA1View view) {
    std::cout << "TestSoA1View:" << std::endl;
    for (auto i = 0; i < view.metadata().size(); ++i) {
        std::cout << "Element " << i << ": ";
        std::cout << "x = " << view.x()[i] << ", ";
        std::cout << "y = " << view.y()[i] << ", ";
    }
}

// This test is currently mostly about the code compiling
TEST_CASE("Use of PortableView<Colls...> on host code", s_tag) {
    std::size_t elems = 16;
    auto SoACollection_0 = std::make_unique<TestSoA1Collection>(elems, cms::alpakatools::host());
    auto SoACollection_1 = std::make_unique<TestSoA2Collection>(elems, cms::alpakatools::host());
    // auto SoACOllection = std::make_unique<TestSoACollection>(elems, cms::alpakatools::host());

    auto& view_0 = SoACollection_0 -> view();
   //  auto& view = SoACollection -> view();

    for (int i = 0; i < view_0.metadata().size(); i++) {
      view_0.x()[i] = static_cast<double>(i);
      view_0.y()[i] = static_cast<double>(i) * 2.0;
    }

    std::vector<std::string> colNames1 = {"x", "y"};
    std::vector<std::string> colNames2 = {"v_x"};

    Argument<TestSoA1Collection> arg1(std::move(SoACollection_0), colNames1);
    Argument<TestSoA2Collection> arg2(std::move(SoACollection_1), colNames2);

    // // myView owns pointers to the columns corresponding to the input name
    // TestSoAView myView(*SoACollection_0, colNames1);
    TestSoAView myView(std::move(arg1), std::move(arg2));

        // Accesso agli elementi della tupla
    auto& firstArg = myView.get<0>();
    auto& secondArg = myView.get<1>();

    // Stampa per verificare
    std::cout << "First Argument column names: ";
    for (const auto& name : firstArg.columnNames) {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    std::cout << "Second Argument column names: ";
    for (const auto& name : secondArg.columnNames) {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    printSoAView(firstArg.collection -> view());

    // TestSoAView myView(*SoACollection_0, *SoACollection_1);

        // Test di accesso ai dati tramite getView
    // Otteniamo la vista su TestSoA1 (primo layout)
    // auto& view1 = myView.getView<0>();  // Otteniamo la prima collezione
    // auto& view2 = myView.getView<1>();  // Otteniamo la seconda collezione

    // auto& view_of_view = view1->view();
    // // Adding other columns
    // myView.addColumns(TestSoA1, {"id"}, TestSoA2, {"v_y"});
}