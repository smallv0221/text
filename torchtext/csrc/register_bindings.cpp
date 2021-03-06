#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <regex.h>
#include <regex_tokenizer.h>         // @manual
#include <sentencepiece.h>           // @manual
#include <torch/csrc/jit/python/pybind_utils.h> // @manual
#include <torch/csrc/utils/pybind.h> // @manual
#include <torch/script.h>
#include <vectors.h> // @manual
#include <vocab.h>   // @manual

namespace torchtext {

namespace py = pybind11;

namespace {
Vocab build_vocab_from_text_file(const std::string &file_path,
                                 const std::string &unk_token,
                                 const int64_t min_freq,
                                 const int64_t num_cpus,
                                 py::object fn) {
  torch::jit::script::Module module(*torch::jit::as_module(fn));
  return _build_vocab_from_text_file(file_path, unk_token, min_freq, num_cpus, module);
}
} // namespace

// Registers our custom classes with pybind11.
PYBIND11_MODULE(_torchtext, m) {
  // Classes
  py::class_<Regex>(m, "Regex")
      .def(py::init<std::string>())
      .def("Sub", &Regex::Sub);

  py::class_<RegexTokenizer>(m, "RegexTokenizer")
      .def_readonly("patterns_", &RegexTokenizer::patterns_)
      .def_readonly("replacements_", &RegexTokenizer::replacements_)
      .def_readonly("to_lower_", &RegexTokenizer::to_lower_)
      .def(py::init<std::vector<std::string>, std::vector<std::string>, bool>())
      .def("forward", &RegexTokenizer::forward);

  py::class_<SentencePiece>(m, "SentencePiece")
      .def(py::init<std::string>())
      .def("_return_content",
           [](const SentencePiece &self) { return py::bytes(self.content_); })
      .def("Encode", &SentencePiece::Encode)
      .def("EncodeAsIds", &SentencePiece::EncodeAsIds)
      .def("DecodeIds", &SentencePiece::DecodeIds)
      .def("EncodeAsPieces", &SentencePiece::EncodeAsPieces)
      .def("DecodePieces", &SentencePiece::DecodePieces)
      .def("GetPieceSize", &SentencePiece::GetPieceSize)
      .def("unk_id", &SentencePiece::unk_id)
      .def("PieceToId", &SentencePiece::PieceToId)
      .def("IdToPiece", &SentencePiece::IdToPiece);

  py::class_<Vectors>(m, "Vectors")
      .def(py::init<std::vector<std::string>, std::vector<int64_t>,
                    torch::Tensor, torch::Tensor>())
      .def_readonly("vectors_", &Vectors::vectors_)
      .def_readonly("unk_tensor_", &Vectors::unk_tensor_)
      .def("get_stoi", &Vectors::get_stoi)
      .def("__getitem__", &Vectors::__getitem__)
      .def("lookup_vectors", &Vectors::lookup_vectors)
      .def("__setitem__", &Vectors::__setitem__)
      .def("__len__", &Vectors::__len__);

  py::class_<Vocab>(m, "Vocab")
      .def(py::init<std::vector<std::string>, std::string>())
      .def_readonly("itos_", &Vocab::itos_)
      .def_readonly("unk_token_", &Vocab::unk_token_)
      .def("__getitem__", &Vocab::__getitem__)
      .def("__len__", &Vocab::__len__)
      .def("insert_token", &Vocab::insert_token)
      .def("append_token", &Vocab::append_token)
      .def("lookup_token", &Vocab::lookup_token)
      .def("lookup_tokens", &Vocab::lookup_tokens)
      .def("lookup_indices", &Vocab::lookup_indices)
      .def("get_stoi", &Vocab::get_stoi)
      .def("get_itos", &Vocab::get_itos);

  // Functions
  m.def("_load_token_and_vectors_from_file",
        &_load_token_and_vectors_from_file);
  m.def("_load_vocab_from_file", &_load_vocab_from_file);
  m.def("_build_vocab_from_text_file", &build_vocab_from_text_file);
}

TORCH_LIBRARY_FRAGMENT(torchtext, m) {
  m.class_<Regex>("Regex")
    .def(torch::init<std::string>())
    .def("Sub", &Regex::Sub)
    .def_pickle(
        // __getstate__
        [](const c10::intrusive_ptr<Regex> &self) -> std::string {
          return self->re_str_;
        },
        // __setstate__
        [](std::string state) -> c10::intrusive_ptr<Regex> {
          return c10::make_intrusive<Regex>(std::move(state));
        });

  using RegexTokenizerState = std::tuple<std::vector<std::string>, std::vector<std::string>, bool>;
  m.class_<RegexTokenizer>("RegexTokenizer")
    .def(torch::init<std::vector<std::string>, std::vector<std::string>, bool>())
    .def("forward", &RegexTokenizer::forward)
    .def_pickle(
        // __getstate__
        [](const c10::intrusive_ptr<RegexTokenizer> &self) -> RegexTokenizerState {
          return std::make_tuple(
                 self->patterns_,
                 self->replacements_,
                 self->to_lower_);
        },
        // __setstate__
        [](RegexTokenizerState state) -> c10::intrusive_ptr<RegexTokenizer> {
          return c10::make_intrusive<RegexTokenizer>(
                 std::move(std::get<0>(state)),
                 std::move(std::get<1>(state)),
                 std::get<2>(state));
        });
  m.class_<SentencePiece>("SentencePiece")
    .def(torch::init<std::string>())
    .def("Encode", &SentencePiece::Encode)
    .def("EncodeAsIds", &SentencePiece::EncodeAsIds)
    .def("DecodeIds", &SentencePiece::DecodeIds)
    .def("EncodeAsPieces", &SentencePiece::EncodeAsPieces)
    .def("DecodePieces", &SentencePiece::DecodePieces)
    .def("GetPieceSize", &SentencePiece::GetPieceSize)
    .def("unk_id", &SentencePiece::unk_id)
    .def("PieceToId", &SentencePiece::PieceToId)
    .def("IdToPiece", &SentencePiece::IdToPiece)
    .def_pickle(
        // __getstate__
        [](const c10::intrusive_ptr<SentencePiece> &self) -> std::string {
          return self->content_;
        },
        // __setstate__
        [](std::string state) -> c10::intrusive_ptr<SentencePiece> {
          return c10::make_intrusive<SentencePiece>(std::move(state));
        });

  m.class_<Vectors>("Vectors")
    .def(torch::init<std::vector<std::string>, std::vector<std::int64_t>, torch::Tensor, torch::Tensor>())
    .def("__getitem__", &Vectors::__getitem__)
    .def("lookup_vectors", &Vectors::lookup_vectors)
    .def("__setitem__", &Vectors::__setitem__)
    .def("__len__", &Vectors::__len__)
    .def_pickle(
        // __getstate__
        [](const c10::intrusive_ptr<Vectors> &self) -> VectorsStates {
          return _set_vectors_states(self);
        },
        // __setstate__
        [](VectorsStates states) -> c10::intrusive_ptr<Vectors> {
          return _get_vectors_from_states(states);
        });

  m.class_<Vocab>("Vocab")
    .def(torch::init<StringList, std::string>())
    .def("__getitem__", &Vocab::__getitem__)
    .def("__len__", &Vocab::__len__)
    .def("insert_token", &Vocab::insert_token)
    .def("append_token", &Vocab::append_token)
    .def("lookup_token", &Vocab::lookup_token)
    .def("lookup_tokens", &Vocab::lookup_tokens)
    .def("lookup_indices", &Vocab::lookup_indices)
    .def("get_stoi", &Vocab::get_stoi)
    .def("get_itos", &Vocab::get_itos)
    .def_pickle(
        // __getstate__
        [](const c10::intrusive_ptr<Vocab> &self) -> VocabStates {
          return _set_vocab_states(self);
        },
        // __setstate__
        [](VocabStates states) -> c10::intrusive_ptr<Vocab> {
          return _get_vocab_from_states(states);
        });

  m.def("torchtext::generate_sp_model", &generate_sp_model);
  m.def("torchtext::load_sp_model", &load_sp_model);
  m.def("torchtext::load_sp_model_string", &load_sp_model_string);
}

} // namespace torchtext
