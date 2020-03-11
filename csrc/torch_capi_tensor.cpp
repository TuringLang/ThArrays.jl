#include <map>
#include <sstream>
#include <iostream>

#include "torch_capi_tensor.h"

#include <c10/core/ScalarType.h>

std::map<int, torch::ScalarType> TYPE_MAP_REV = \
    {
     {0,  torch::kByte}, // _(uint8_t, Byte) /* 0 */
     {1,  torch::kChar}, // _(int8_t, Char) /* 1 */
     {2,  torch::kShort}, // _(int16_t, Short) /* 2 */
     {3,  torch::kInt}, // _(int, Int) /* 3 */
     {4,  torch::kLong}, // _(int64_t, Long) /* 4 */
     {5,  torch::kHalf}, // _(at::Half, Half) /* 5 */
     {6,  torch::kFloat}, // _(float, Float) /* 6 */
     {7,  torch::kDouble}, // _(double, Double) /* 7 */
     {8,  torch::kComplexHalf}, // _(at::ComplexHalf, ComplexHalf) /* 8 */
     {9,  torch::kComplexFloat}, // _(std::complex<float>, ComplexFloat) /* 9 */
     {10, torch::kComplexDouble}, // _(std::complex<double>, ComplexDouble) /* 10 */
     {11, torch::kBool}, // _(bool, Bool) /* 11 */
     // {12, torch::kQInt8}, // _(c10::qint8, QInt8) /* 12 */
     // {13, torch::kQUint8}, // _(c10::quint8, QUInt8) /* 13 */
     // {14, torch::kQInt32}, // _(c10::qint32, QInt32) /* 14 */
     // {15, torch::kBFloat16}, // _(at::BFloat16, BFloat16) /* 15 */
    };

// creation and repr
torch::Tensor* tensor_from_data(
    void *data, size_t datalen, int tid,
    int64_t *size_data, size_t dim,
    int grad) {
    c10::ArrayRef<int64_t> sizes(size_data, dim);

    uint8_t *buf = new uint8_t[datalen];
    memcpy(buf, data, datalen);

    torch::Tensor res = torch::from_blob(
        buf, sizes,
        [=](void *p) -> void { delete[] buf; },
        at::dtype(TYPE_MAP_REV.at(tid)).requires_grad(grad));
    return new torch::Tensor(res);
}

void tensor_destroy(torch::Tensor *tensor) {
    // std::cout << "DEBUG: Tensor " << tensor << " is destroyed!\n";
    if(tensor) delete tensor;
}

const char* tensor_to_string(torch::Tensor *tensor) {
    std::stringstream ss;
    ss << *tensor;
    std::string str = ss.str();
    char* res = static_cast<char*>(malloc(str.length() + 1));
    memcpy(res, str.c_str(), str.length());
    res[str.length()] = 0;
    return res;
}

// retrieve Tensor info
int8_t tensor_method_dtype(torch::Tensor *tensor) {
    c10:: ScalarType t = c10::typeMetaToScalarType(tensor->dtype());
    return static_cast<int8_t>(t);
}

int64_t tensor_method_ndimension(torch::Tensor *tensor) {
    return tensor->ndimension();
}

void tensor_method_sizes(torch::Tensor *tensor, int64_t *buf) {
    int64_t *p = buf;
    torch::IntArrayRef sizes = tensor->sizes();
    for (auto sz : sizes) {
        *p = sz;
        p++;
    }
}

void* tensor_method_data_ptr(torch::Tensor *tensor) {
    return tensor->data_ptr();
}

void tensor_method_data_copy(torch::Tensor *tensor, void *buf, size_t len) {
    memcpy(buf, tensor->data_ptr(), len);
}


// methods on Tensor
torch::Tensor* tensor_method_sum(torch::Tensor *t) {
    torch::Tensor sum = t->sum();
    return new torch::Tensor(sum);
}

torch::Tensor* tensor_method_grad(torch::Tensor *t) {
    torch::Tensor g = t->grad();
    return new torch::Tensor(g);
}

void tensor_method_backward(
    torch::Tensor *t, torch::Tensor *g,
    bool keep_graph=false, bool create_graph=false) {
    if (g) {
        t->backward(*g, keep_graph, create_graph);
    } else {
        t->backward({}, keep_graph, create_graph);
    }
}

// operators
torch::Tensor* tensor_op_add(torch::Tensor *a, torch::Tensor *b) {
    torch::Tensor sum = *a + *b;
    return new torch::Tensor(sum);
}