#include <pybind11/pybind11.h>
#include "top5_filter.hpp"
#include "vol_engine.hpp"
#include "book.hpp"

namespace py = pybind11;

PYBIND11_MODULE(dual_core, m){
    m.doc()="Deribit Feed-Strategy Dual Engine 1.4M/sec pybind";

    py::class_<Level>(m,"Level").def(py::init<>()).def_readwrite("price",&Level::price).def_readwrite("size",&Level::size);
    py::class_<OrderBook>(m,"OrderBook").def(py::init<>());
    py::class_<Top5Filter>(m,"Top5Filter")
        .def(py::init<>())
        .def("filtered_mid",&Top5Filter::filtered_mid)
        .def("raw_mid",&Top5Filter::raw_mid)
        .def("spread_bps",&Top5Filter::spread_bps)
        .def("edge_bps",&Top5Filter::edge_bps)
        .def("edge_usd_per_100btc",&Top5Filter::edge_usd_per_100btc)
        .def("is_liquid",&Top5Filter::is_liquid);

    py::class_<Trade>(m,"Trade").def(py::init<>()).def_readwrite("price",&Trade::price).def_readwrite("size",&Trade::size);
    py::class_<VolEngine>(m,"VolEngine")
        .def(py::init<>())
        .def("update",&VolEngine::update)
        .def_readonly("rv",&VolEngine::rv)
        .def_readonly("garch_vol",&VolEngine::garch_vol)
        .def("edge_bps",&VolEngine::edge_bps);
}
