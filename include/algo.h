
#include <cinttypes>
namespace algo {

template <class T, class Pred>
void sort(T* begin, T* end, Pred pred) {
    /*
        原地堆排
        左子节点2*i,右子节点2*i+1
    */
    T* arr = begin - 1;
    uint32_t len = end - begin;
    auto makeheap = [&](uint32_t tail) {
        uint32_t i = tail;
        while (i != 1) {
            uint32_t parent = i / 2;
            if (!pred(arr[parent], arr[i])) {
                const auto& q = arr[i];
                arr[i] = arr[parent];
                arr[parent] = q;
                i /= 2;
            } else
                break;
        }
    };
    for (uint32_t i = 2; i <= len; i++) {
        makeheap(i);
    }
}
template <class T>
void sort(T* begin, T* end) {
    sort(begin, end, [](const T& a, const T& b) { return a < b; });
}

}  // namespace algo