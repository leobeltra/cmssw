#ifndef DataFormats_Portable_interface_PortableView_h
#define DataFormats_Portable_interface_PortableView_h

#include "DataFormats/Portable/interface/PortableCollection.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"

template <typename... Colls> 
class PortableView {
public:

    PortableView() = default;

    PortableView(Colls&... colls) : colls_(colls...) {}

    // default destructor
    ~PortableView() = default;

    // Access to different views
    template <std::size_t N>
    auto& getView() {
        return std::get<N>(colls_);
    }

    // template <std::size_t N, typename Column_Name>
    // auto getColumn() {
    //     auto& view = getView<N>();
    //     return view.getCol(Column_Name);
    // }

private:
    std::tuple<Colls&...> colls_;
};

#endif // DataFormats_Portable_interface_PortableView_h