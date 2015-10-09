#ifndef VQRO_CONTROL_CONTROLLER_H
#define VQRO_CONTROL_CONTROLLER_H

#include <memory>
#include "vqro/base/base.h"
#include "vqro/db/search_engine.h"


using vqro::db::SearchEngine;


namespace vqro {
namespace control {


class Controller {
 public:
  string state_dir;
  std::unique_ptr<SearchEngine> search_engine;

  Controller(string dir) : state_dir(dir) {
    search_engine.reset(new SearchEngine(dir));
  }

};


}  // namespace control
}  // namespace vqro

#endif  // VQRO_CONTROL_CONTROLLER_H
