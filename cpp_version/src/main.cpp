/*-------------------------------------------------------------------------------
 This file is part of ranger.

 Copyright (c) [2014-2018] [Marvin N. Wright]

 This software may be modified and distributed under the terms of the MIT license.

 Please note that the C++ core of ranger is distributed under MIT license and the
 R package "ranger" under GPL3 license.
 #-------------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <memory>

#include "globals.h"
#include "ArgumentHandler.h"
#include "ForestClassification.h"
#include "ForestRegression.h"
#include "ForestSurvival.h"
#include "ForestProbability.h"
#include "utility.h"

using namespace ranger;

void run_ranger(const ArgumentHandler& arg_handler, std::ostream& verbose_out) {
  verbose_out << "Starting Ranger. Hello!" << std::endl;
  verbose_out <<"Treetype is "<<arg_handler.treetype<<std::endl;
  verbose_out<<TREE_CLASSIFICATION<<std::endl;
  verbose_out<<"Writetoimg is "<<arg_handler.writetoimg<<std::endl;
  verbose_out<<"memmode is "<<arg_handler.memmode<<std::endl;
  
  // Create forest object
  std::unique_ptr<Forest> forest { };
  switch (arg_handler.treetype) {
  case TREE_CLASSIFICATION:
    if (arg_handler.probability) {
      forest = make_unique_ranger<ForestProbability>();
    } else {
      forest = make_unique_ranger<ForestClassification>();
    }
    break;
  case TREE_REGRESSION:
    forest = make_unique_ranger<ForestRegression>();
    break;
  case TREE_SURVIVAL:
    forest = make_unique_ranger<ForestSurvival>();
    break;
  case TREE_PROBABILITY:
    forest = make_unique_ranger<ForestProbability>();
    break;
  }
  verbose_out<<"About to initialize forest" <<std::endl;
  // Call Ranger
  verbose_out <<"Initializing forest." <<std::endl;
  forest->initCpp(arg_handler.depvarname, arg_handler.memmode, arg_handler.file, arg_handler.mask, arg_handler.mtry,
      arg_handler.outprefix, arg_handler.ntree, &verbose_out, arg_handler.seed, arg_handler.nthreads,
      arg_handler.predict, arg_handler.impmeasure, arg_handler.targetpartitionsize, arg_handler.splitweights,
      arg_handler.alwayssplitvars, arg_handler.statusvarname, arg_handler.replace, arg_handler.catvars,
      arg_handler.savemem, arg_handler.splitrule, arg_handler.caseweights, arg_handler.predall, arg_handler.fraction,
      arg_handler.alpha, arg_handler.minprop, arg_handler.holdout, arg_handler.predictiontype,
      arg_handler.randomsplits, arg_handler.maxdepth, arg_handler.regcoef, arg_handler.usedepth, arg_handler.writetoimg,
      arg_handler.imgwidth, arg_handler.imgheight, arg_handler.batchtrain, arg_handler.kernelsize);
  verbose_out <<"Calling forest.run()"<<std::endl;
  forest->run(true, !arg_handler.skipoob);

  if (arg_handler.write) {
    forest->saveToFile();
  }
  forest->writeOutput();
  verbose_out << "Finished Ranger." << std::endl;
}

int main(int argc, char **argv) {

  try {
    // Handle command line arguments
    ArgumentHandler arg_handler(argc, argv);
    if (arg_handler.processArguments() != 0) {
      return 0;
    }
    arg_handler.checkArguments();

    if (arg_handler.verbose) {
      run_ranger(arg_handler, std::cout);
    } else {
      std::ofstream logfile { arg_handler.outprefix + ".log" };
      if (!logfile.good()) {
        throw std::runtime_error("Could not write to logfile.");
      }
      run_ranger(arg_handler, logfile);
    }
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << " Ranger will EXIT now." << std::endl;
    return -1;
  }

  return 0;
}
