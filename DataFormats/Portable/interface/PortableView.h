#ifndef DataFormats_Portable_interface_PortableView_h
#define DataFormats_Portable_interface_PortableView_h

// #include <utility> 
// // #include <string>
// // #include <vector>
#include "DataFormats/Portable/interface/PortableCollection.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"

using Buffer = cms::alpakatools::host_buffer<std::byte[]>;

// Definizione della struct Argument
template <typename Collection>
    struct Argument {
        using Layout = typename Collection::Layout;
        using View = typename Layout::View;

        std::unique_ptr<Collection> collection;
        std::vector<std::string> columnNames;
        std::vector<View> pointers;

        Argument(std::unique_ptr<Collection> coll, std::vector<std::string> names)
            : collection(std::move(coll)), columnNames(std::move(names)) {
                // Layout lay = collection -> layout();
                // View view{lay};
                //initializePointers();
            }

        Argument(const Argument&) = delete;
        Argument& operator=(const Argument&) = delete;

        Argument(Argument&&) = default;
        Argument& operator=(Argument&&) = default;    

    };

template <typename... Collections>
class PortableView {
public:

    PortableView() = default;

    PortableView(Argument<Collections>&&... args) 
        : arguments_(std::make_tuple(std::forward<Argument<Collections>>(args)...)) {}

    ~PortableView() = default;

    template <std::size_t I>
    auto& get() {
        return std::get<I>(arguments_);
    }

    static constexpr std::size_t size() {
        return sizeof...(Collections);
    }

private:
    std::tuple<Argument<Collections>...> arguments_;
    int32_t elements_;

};

#endif // DataFormats_Portable_interface_PortableView_h