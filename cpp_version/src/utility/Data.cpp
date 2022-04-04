/*-------------------------------------------------------------------------------
 This file is part of ranger.

 Copyright (c) [2014-2018] [Marvin N. Wright]

 This software may be modified and distributed under the terms of the MIT license.

 Please note that the C++ core of ranger is distributed under MIT license and the
 R package "ranger" under GPL3 license.
 #-------------------------------------------------------------------------------*/

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <dirent.h>

#include "Data.h"
#include "utility.h"

// relevant STB headers
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

namespace ranger {

Data::Data() :
    num_rows(0), num_rows_rounded(0), num_cols(0), snp_data(0), num_cols_no_snp(0), externalData(true), index_data(0), max_num_unique_values(
        0), order_snps(false) {
}

size_t Data::getVariableID(const std::string& variable_name) const {
  auto it = std::find(variable_names.cbegin(), variable_names.cend(), variable_name);
  if (it == variable_names.cend()) {
    throw std::runtime_error("Variable " + variable_name + " not found.");
  }
  return (std::distance(variable_names.cbegin(), it));
}

// #nocov start (cannot be tested anymore because GenABEL not on CRAN)
void Data::addSnpData(unsigned char* snp_data, size_t num_cols_snp) {
  num_cols = num_cols_no_snp + num_cols_snp;
  num_rows_rounded = roundToNextMultiple(num_rows, 4);
  this->snp_data = snp_data;
}
// #nocov end

bool Data::batchDataLoader(std::string dirpath, std::string mask_dirpath, std::vector<std::string>& dependent_variable_names,
  size_t kernel_size) {
  std::cout<<"Batch data loading is on.\n";

  bool has_csv = false;
  bool has_other = false;
  bool has_img = false;

  //Loop over directory to get file extensions (test example)
  DIR *dirp;
  struct dirent* dent;
  dirp=opendir(dirpath.c_str());
  do {
      dent = readdir(dirp);
      if (dent)
      {
        std::string fname = dent->d_name;
        if(fname!=".." && fname!=".") {
          std::cout<<"File name is: "<<fname<<"\n";
          std::string extension = fname.substr(fname.find_last_of(".") + 1);
          std::cout<<"Extension is: "<<extension<<"\n";
          if(extension == "jpeg" || extension == "png"||extension=="jpg"||extension=="tif") {
            has_img = true;
          } else if (extension == "csv") {
            has_csv = true;
          } else {
            has_other = true;
          }
        }
      }
  } while (dent);
  closedir(dirp);
  if(has_other) {
    throw std::runtime_error("Some files in batch dataloader have extensions other than .csv, .jpeg, or .img");
  }
  if(has_img) {
    std::cout<<"Testing batch data loading for images....";
    //throw std::runtime_error("Batch data loading is currently not supported for images");
  } else {
    //Loop over directory
    DIR *dirp;
    struct dirent* dent;
    size_t total_cols = 0;
    size_t total_rows = 0;
    dirp=opendir(dirpath.c_str());
    do {
        dent = readdir(dirp);
        if (dent)
        {
          std::string fname = dent->d_name;
          if(fname!=".." && fname!=".") {
            std::cout<<"Getting rows and cols for file: "<<fname<<"\n";
            std::string extension = fname.substr(fname.find_last_of(".") + 1);
            if(extension == "csv") {
              //Load file
              std::ifstream input_file;
              input_file.open(dirpath+"/"+fname);
              //Error if cannot open
              if (!input_file.good()) {
                throw std::runtime_error("Could not open input file.");
              }
              // Check if comma, semicolon or whitespace seperated
              std::string header_line;
              getline(input_file, header_line);
              //Get cols
              size_t n_cols = 0;
              if (header_line.find(',') != std::string::npos) {
                n_cols = getNumColsForCsv(input_file, header_line, dependent_variable_names, ',', false);
              } else if (header_line.find(';') != std::string::npos) {
                n_cols = getNumColsForCsv(input_file, header_line, dependent_variable_names, ';', false);
              } else {
                n_cols = getNumColsForCsv(input_file, header_line, dependent_variable_names, ';', true);
              }
              //If cols mismatch -> error
              if (total_cols == 0) {
                total_cols = n_cols;
              }
              if(total_cols != n_cols) {
                throw std::runtime_error("Number of columns does not match across files");
              }
              //Get rows (# of lines excluding header)
              size_t line_count = 0;
              std::string line;
              while (getline(input_file, line)) {
                ++line_count;
              }
              total_rows += line_count;
              //Close file
              input_file.close();
              std::cout<<"File "<<fname<<"has "<<line_count<<" rows and "<<n_cols<<" cols\n";
            //TODO: fix extensions here.
            } else if(extension=="jpeg"||extension=="jpg"||extension=="png"||extension=="tif") {
              //Get rows, cols for img
              std::tuple<size_t, size_t, size_t> dims = getImgDims(dirpath+"/"+fname);
              size_t n_rows = std::get<0>(dims)*std::get<1>(dims);
              size_t n_cols = kernel_size*kernel_size*std::get<2>(dims);
              //If cols mismatch -> error
              if (total_cols == 0) {
                total_cols = n_cols;
              }
              if(total_cols != n_cols) {
                throw std::runtime_error("Number of columns does not match across files");
              }
              total_rows += n_rows;
            }
          }
        }
    } while (dent);
    closedir(dirp);
    std::cout<<"All files have "<<total_rows<<" rows and "<<total_cols<<" cols\n";
    //Reserve chunk of memory for data
    size_t num_dependent_variables = dependent_variable_names.size();
    num_cols = total_cols;
    num_rows = total_rows;
    reserveMemory(num_dependent_variables);
    size_t row = 0;
    //Loop over files and load data from each one
    dirp=opendir(dirpath.c_str());
    do {
        dent = readdir(dirp);
        if (dent)
        {
          std::string fname = dent->d_name;
          if(fname!=".." && fname!=".") {
            std::cout<<"Loading data for file: "<<fname<<"\n";
            std::string extension = fname.substr(fname.find_last_of(".") + 1);
            if(extension == "csv") {
              //Load file
              std::ifstream input_file;
              input_file.open(dirpath+"/"+fname);
              //Error if cannot open
              if (!input_file.good()) {
                throw std::runtime_error("Could not open input file.");
              }
              // Check if comma, semicolon or whitespace seperated
              std::string header_line;
              getline(input_file, header_line);
              //Get cols
              size_t n_cols = 0;
              bool result;
              if (header_line.find(',') != std::string::npos) {
                result = loadFromFileOther(input_file, header_line, dependent_variable_names, ',', row);
                row = num_rows;
                num_rows = total_rows;
              } else if (header_line.find(';') != std::string::npos) {
                result = loadFromFileOther(input_file, header_line, dependent_variable_names, ';', row);
                row = num_rows;
                num_rows = total_rows;
              } else {
                result = loadFromFileWhitespace(input_file, header_line, dependent_variable_names, row);
                row = num_rows;
                num_rows = total_rows;
              }
              input_file.close();
            }
          }
        }
    } while (dent);
    closedir(dirp);
    throw std::runtime_error("Batch data loading is currently not supported");
  }
  return false;
}

// #nocov start
bool Data::loadFromFile(std::string filename, std::string mask_filename, std::vector<std::string>& dependent_variable_names,
    bool batch_data, size_t kernel_size) {

  bool result;

  if(batch_data) {
    result = batchDataLoader(filename, mask_filename, dependent_variable_names, kernel_size);
    return result;
  } else {
    std::cout<<"Batch data loading is off.\n";
    // Open input file
    std::ifstream input_file;
    input_file.open(filename);
    if (!input_file.good()) {
      throw std::runtime_error("Could not open input file.");
    }

    //For jpegs and pngs
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    if(extension == "jpeg" || extension == "png") {
      //Close file (we will reopen with stb)
      input_file.close();
      //Load image size and reserve memory
      //std::tuple<size_t, size_t, size_t> dims = getImgDims(filename);
      //W x H
      //size_t rows = std::get<0>(dims)*std::get<1>(dims);
      //Kernel area x C
      //size_t cols = kernel_size*kernel_size*std::get<2>(dims);
      //num_rows = rows;
      //num_cols = cols;
      //Default is 1 independent var, so 1 image mask only.
      //reserveMemory(1);
      //Load from img
      //std::cout<<"Image with filename "<<filename<<" has dims WxHxC="<<std::get<0>(dims)<<"x"<<std::get<1>(dims)<<"x"<<std::get<2>(dims)<<"\n";
      result = loadFromImg(filename, mask_filename);//, kernel_size, 0,0,0);
      num_cols_no_snp = num_cols;
      std::cout<<"loaded img\n";
      return result;
    } else { //For csvs
      //Same code as ranger repo that I forked this from. So it should still work for csvs.
      // Count number of rows
      size_t line_count = 0;
      std::string line;
      while (getline(input_file, line)) {
        ++line_count;
      }
      num_rows = line_count - 1;
      input_file.close();
      input_file.open(filename);

      // Check if comma, semicolon or whitespace seperated
      std::string header_line;
      getline(input_file, header_line);

      size_t num_dependent_variables = dependent_variable_names.size();
      // Find out if comma, semicolon or whitespace seperated and call appropriate method
      if (header_line.find(',') != std::string::npos) {
        num_cols = getNumColsForCsv(input_file, header_line, dependent_variable_names, ',', false);
        reserveMemory(num_dependent_variables);
        result = loadFromFileOther(input_file, header_line, dependent_variable_names, ',', 0);
      } else if (header_line.find(';') != std::string::npos) {
        num_cols = getNumColsForCsv(input_file, header_line, dependent_variable_names, ';', false);
        reserveMemory(num_dependent_variables);
        result = loadFromFileOther(input_file, header_line, dependent_variable_names, ';', 0);
      } else {
        num_cols = getNumColsForCsv(input_file, header_line, dependent_variable_names, ';', true);
        reserveMemory(num_dependent_variables);
        result = loadFromFileWhitespace(input_file, header_line, dependent_variable_names, 0);
      }
      externalData = false;
      input_file.close();
      return result;
    }
  }
}

bool Data::loadFromFileWhitespace(std::ifstream& input_file, std::string header_line,
    std::vector<std::string>& dependent_variable_names, size_t row_start) {
  size_t num_dependent_variables = dependent_variable_names.size();
  std::vector<size_t> dependent_varIDs;
  dependent_varIDs.resize(num_dependent_variables);

  // Read header
  std::string header_token;
  std::stringstream header_line_stream(header_line);
  size_t col = 0;
  while (header_line_stream >> header_token) {
    bool is_dependent_var = false;
    for (size_t i = 0; i < dependent_variable_names.size(); ++i) {
      if (header_token == dependent_variable_names[i]) {
        dependent_varIDs[i] = col;
        is_dependent_var = true;
      }
    }
    if (!is_dependent_var) {
      variable_names.push_back(header_token);
    }
    ++col;
  }

  //num_cols = variable_names.size();
  //Not tested for whitespace-separated files
  //char separator = 'B';
  //num_cols = getNumColsForCsv(input_file, header_line, dependent_variable_names, separator, true);
  num_cols_no_snp = num_cols;

  // Read body
  //reserveMemory(num_dependent_variables);
  bool error = false;
  std::string line;
  size_t row = row_start;
  while (getline(input_file, line)) {
    double token;
    std::stringstream line_stream(line);
    size_t column = 0;
    while (readFromStream(line_stream, token)) {
      size_t column_x = column;
      bool is_dependent_var = false;
      for (size_t i = 0; i < dependent_varIDs.size(); ++i) {
        if (column == dependent_varIDs[i]) {
          set_y(i, row, token, error);
          is_dependent_var = true;
          break;
        } else if (column > dependent_varIDs[i]) {
          --column_x;
        }
      }
      if (!is_dependent_var) {
        set_x(column_x, row, token, error);
      }
      ++column;
    }
    if (column > (num_cols + num_dependent_variables)) {
      throw std::runtime_error(
          std::string("Could not open input file. Too many columns in row ") + std::to_string(row) + std::string("."));
    } else if (column < (num_cols + num_dependent_variables)) {
      throw std::runtime_error(
          std::string("Could not open input file. Too few columns in row ") + std::to_string(row)
              + std::string(". Are all values numeric?"));
    }
    ++row;
  }
  num_rows = row;
  return error;
}


/*bool Data::loadFromImg(std::string img_path, std::string mask_path, size_t kernel_size,
  size_t width1, size_t height1, size_t channels1) {

  int width, height, channels;

  //Load img with STB
  uint8_t *img = stbi_load(img_path.c_str(), &width, &height, &channels, 0);
  //int kernel_size = 3; //TODO: make this changeable in CLA. 3x3 kernel is default.
  std::cout<<"Kernel size= "<<kernel_size<<"\n";

  //Catch STB errors
  if (img == NULL)
  {
      printf("error loading image, reason: %s\n", stbi_failure_reason());
      exit(1);
  }
  //Free image
  stbi_image_free(img);

  //Reserve memory for one dependent variable and kernel_size kernel with 3 channels
  num_rows = width*height;
  num_cols = kernel_size*kernel_size*3;
  reserveMemory(1);

  //For testing: generate fake mask (i.e. sety to 1 for everything)
  if(mask_path=="") {
    //Set y for no mask (just give mask=1 for every row)
    size_t row = 0;
    size_t depvar_i = 0;
    bool error = false;
    for (int i = 0; i < width; i++)
    {
      for (int j = 0; j < height; j++)
      {
        set_y(depvar_i, row, 1, error);
        row += 1;
      }
    }
  } else { //Load mask
    int mask_width, mask_height, mask_channels;
    uint8_t *img_mask = stbi_load(mask_path.c_str(), &mask_width, &mask_height, &mask_channels, 0);
    std::cout<<"Img mask has filename="<<mask_path<<" and WxHxC="<<mask_width<<"x"<<mask_height<<"x"<<mask_channels<<"\n";

    //Catch STB errors
    if (img_mask == NULL)
    {
        printf("error loading image, reason: %s\n", stbi_failure_reason());
        exit(1);
    }

    //Error if mask size != training img size
    if(mask_width!=width || mask_height!=height) {
      throw std::runtime_error(
          std::string("Mask dimensions do not match image dimensions"));
    }

    //Set y for mask
    size_t row = 0;
    size_t depvar_i = 0;
    bool error = false;
    for (int i = 0; i < width; i++)
    {
      for (int j = 0; j < height; j++)
      {
        //Assume 1 channel in the mask
        int offset = (channels)*((width * j) + i);
        int mask_val = img[offset];
        set_y(depvar_i, row, mask_val, error);
        //std::cout<<"set y with row ="<<row<<" and val="<<mask_val<<"\n";
        row += 1;
      }
    }
    stbi_image_free(img_mask);
    std::cout<<"freed mask\n";
  }
  //Set x for img
  int width_load, height_load, channels_load;
  //Load img with STB
  img = stbi_load(img_path.c_str(), &width_load, &height_load, &channels_load, 0);
  //TODO: check if dims match?
  std::cout<<"WxHxC="<<width_load<<"x"<<height_load<<"x"<<channels_load<<"\n";
  size_t row = 0;
  bool error = false;
  int max_offset = std::floor(kernel_size/2);
  for(int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      size_t column_x = 0;
      //Loop over image kernel
      for(int k = i-max_offset; k <= i+max_offset; k++) {
        for(int l = j-max_offset; l <= j+max_offset; l++) {
          //Ensure we stay in bounds of the img
          int kcol = k;
          int lrow = l;
          if(k < 0) {
            kcol = 0;
          } else if (k >=width) {
            kcol = width-1;
          }
          if(l < 0) {
            lrow = 0;
          } else if (l >= height) {
            lrow = height-1;
          }
          //Assume 3 channels for R, G, B
          //std::cout<<"num_rows="<<num_rows<<", num_cols="<<num_cols<<"\n";
          int offset = (channels) * ((width * lrow) + kcol);
          int r_px = img[offset];
          set_x(column_x, row, r_px, error);
          column_x += 1;
          int g_px = img[offset + 1];
          set_x(column_x, row, g_px, error);
          column_x += 1;
          int b_px = img[offset + 2];
          set_x(column_x, row, b_px, error);
          column_x += 1;
          //std::cout<<"Set x with column_x="<<column_x<<", row="<<row<<", b_px="<<b_px<<"\n";
        }
      }
      row += 1;
    }
  }
  std::cout<<"about to free img\n";
  stbi_image_free(img);
  std::cout<<"freed img\n";
  return false;
}*/
bool Data::loadFromImg(std::string img_path, std::string mask_path) {
  int width, height, channels;
  std::cout<<"Img filename="<<img_path<<"\n";

  //Load img with STB
  uint8_t *img = stbi_load(img_path.c_str(), &width, &height, &channels, 0);
  int kernel_size = 3; //TODO: make this changeable in CLA. 3x3 kernel is default.

  //Catch STB errors
  if (img == NULL)
  {
      printf("error loading image, reason: %s\n", stbi_failure_reason());
      exit(1);
  }
  //Free image
  stbi_image_free(img);

  //Reserve memory for one dependent variable and kernel_size kernel with 3 channels
  num_rows = width*height;
  num_cols = kernel_size*kernel_size*3;
  reserveMemory(1);

  //For testing: generate fake mask (i.e. sety to 1 for everything)
  if(mask_path=="") {
  //if(true) {
    //Set y for no mask (just give mask=1 for every row)
    /*int mask_width, mask_height, mask_channels;
    uint8_t *img_mask = stbi_load(mask_path.c_str(), &mask_width, &mask_height, &mask_channels, 0);

    //Catch STB errors
    if (img_mask == NULL)
    {
        printf("error loading image, reason: %s\n", stbi_failure_reason());
        exit(1);
    }
    stbi_image_free(img_mask);*/
    size_t row = 0;
    size_t depvar_i = 0;
    bool error = false;
    for (int i = 0; i < width; i++)
    {
      for (int j = 0; j < height; j++)
      {
        if(j>200) {
          set_y(depvar_i, row, 0, error);
        } else {
          set_y(depvar_i, row, 1, error);
        }
        row += 1;
      }
    }
  } else { //Load mask
    int mask_width, mask_height, mask_channels;

    uint8_t *img_mask = stbi_load(mask_path.c_str(), &mask_width, &mask_height, &mask_channels, 0);
    std::cout<<"mask_channels="<<mask_channels<<"\n";
    //Catch STB errors
    if (img_mask == NULL)
    {
        printf("error loading image, reason: %s\n", stbi_failure_reason());
        exit(1);
    }

    //Error if mask size != training img size
    if(mask_width!=width || mask_height!=height) {
      throw std::runtime_error(
          std::string("Mask dimensions do not match image dimensions"));
    }

    //Set y for mask
    size_t row = 0;
    size_t depvar_i = 0;
    bool error = false;
    for (int i = 0; i < width; i++)
    {
      for (int j = 0; j < height; j++)
      {
        //set_y(depvar_i, row, 0, error);
        //Assume 1 channel in the mask
        int offset = (mask_channels)*((width * j) + i);
        if(offset > 20000) {
          offset = 65535;
          offset = 58367;
        } else {
          offset = 0;
        }
        //offset = 65535;
        std::cout<<"offset="<<offset<<"\n";
        //int offset = 0;//width*j+i;
        int mask_val = img[offset];
        //TODO: change to allow for multiple classes (this is setting cloud px/not pitch black px to 1)
        /*if(mask_val!=0 && mask_val!=255) {
          std::cout<<"maskval="<<mask_val<<"\n";
        }*/
        std::cout<<"mask_val="<<mask_val<<"\n";
        if(offset > 20000) {
          //mask_val = 1;
          std::cout<<"writing 1\n";
          set_y(depvar_i, row, 1, error);
        } else {
          std::cout<<"writing 0\n";
          set_y(depvar_i, row, 0, error);
        }
        //set_y(depvar_i, row, 0, error);
        row += 1;
      }
    }
    stbi_image_free(img_mask);
  }
  //Set x for img
  img = stbi_load(img_path.c_str(), &width, &height, &channels, 0);
  size_t row = 0;
  bool error = false;
  int max_offset = std::floor(kernel_size/2);
  for(int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      size_t column_x = 0;
      //Loop over image kernel
      for(int k = i-max_offset; k <= i+max_offset; k++) {
        for(int l = j-max_offset; l <= j+max_offset; l++) {
          //Ensure we stay in bounds of the img
          int kcol = k;
          int lrow = l;
          if(k < 0) {
            kcol = 0;
          } else if (k >=width) {
            kcol = width-1;
          }
          if(l < 0) {
            lrow = 0;
          } else if (l >= height) {
            lrow = height-1;
          }
          //Assume 3 channels for R, G, B
          int offset = (channels) * ((width * lrow) + kcol);
          int r_px = img[offset];
          set_x(column_x, row, r_px, error);
          column_x += 1;
          int g_px = img[offset + 1];
          set_x(column_x, row, g_px, error);
          column_x += 1;
          int b_px = img[offset + 2];
          set_x(column_x, row, b_px, error);
          column_x += 1;
        }
      }
      row += 1;
    }
  }
  stbi_image_free(img);

  return 1;
}

std::tuple<size_t, size_t, size_t> Data::getImgDims(std::string img_path) {

  //Initialize W, H, C (C generally expected to be 3, but this code is generic to num channels)
  int width, height, channels;

  //Load img with STB
  uint8_t *img = stbi_load(img_path.c_str(), &width, &height, &channels, 0);

  //Catch STB errors
  if (img == NULL)
  {
      printf("error loading image, reason: %s\n", stbi_failure_reason());
      exit(1);
  }
  //Free image
  stbi_image_free(img);

  //Num_rows = # of px to classify (so width x height)
  //size_t n_rows = width*height;
  //Num_cols = # of features (so kernel area x channels)
  //size_t n_cols = kernel_size*kernel_size*channels;
  return std::make_tuple(width, height, channels);//n_rows, n_cols);
}

size_t Data::getNumColsForCsv(std::ifstream& input_file, std::string header_line,
    std::vector<std::string>& dependent_variable_names, char separator, bool whitespace) {
  // Read header
  std::string header_token;
  std::stringstream header_line_stream(header_line);
  std::vector<std::string> tmp_variable_names;
  size_t col = 0;
  if (whitespace) {
    //Get line (separated by whitespace)
    while (header_line_stream >> header_token) {
      bool is_dependent_var = false;
      for (size_t i = 0; i < dependent_variable_names.size(); ++i) {
        if (header_token == dependent_variable_names[i]) {
          //dependent_varIDs[i] = col;
          is_dependent_var = true;
        }
      }
      if (!is_dependent_var) {
        tmp_variable_names.push_back(header_token);
      }
      ++col;
    }
  } else {
    //Get line using separator
    while (getline(header_line_stream, header_token, separator)) {
      bool is_dependent_var = false;
      for (size_t i = 0; i < dependent_variable_names.size(); ++i) {
        if (header_token == dependent_variable_names[i]) {
          //dependent_varIDs[i] = col;
          is_dependent_var = true;
        }
      }
      if (!is_dependent_var) {
        tmp_variable_names.push_back(header_token);
      }
      ++col;
    }
  }
  //Figure out # of cols
  size_t n_cols = tmp_variable_names.size();
  return n_cols;
}

bool Data::loadFromFileOther(std::ifstream& input_file, std::string header_line,
    std::vector<std::string>& dependent_variable_names, char seperator, size_t row_start) {
  size_t num_dependent_variables = dependent_variable_names.size();
  std::vector<size_t> dependent_varIDs;
  dependent_varIDs.resize(num_dependent_variables);
  //reserveMemory(num_dependent_variables);

  // Read header and fill in dependent vars
  std::string header_token;
  std::stringstream header_line_stream(header_line);
  size_t col = 0;
  while (getline(header_line_stream, header_token, seperator)) {
    bool is_dependent_var = false;
    for (size_t i = 0; i < dependent_variable_names.size(); ++i) {
      if (header_token == dependent_variable_names[i]) {
        dependent_varIDs[i] = col;
        is_dependent_var = true;
      }
      if (!is_dependent_var) {
        variable_names.push_back(header_token);
      }
    }
    ++col;
  }

  //num_cols = getNumColsForCsv(input_file, header_line, dependent_variable_names, seperator, false);//variable_names.size();
  //num_cols = variable_names.size();
  //std::cout<<"num_cols="<<num_cols<<'\n';
  num_cols_no_snp = num_cols;

  // Read body
  //reserveMemory(num_dependent_variables);
  bool error = false;
  std::string line;
  size_t row = row_start;
  while (getline(input_file, line)) {
    std::string token_string;
    double token;
    std::stringstream line_stream(line);
    size_t column = 0;
    while (getline(line_stream, token_string, seperator)) {
      std::stringstream token_stream(token_string);
      readFromStream(token_stream, token);

      size_t column_x = column;
      bool is_dependent_var = false;
      for (size_t i = 0; i < dependent_varIDs.size(); ++i) {
        if (column == dependent_varIDs[i]) {
          set_y(i, row, token, error);
          //std::cout<<"setting y with i="<<i<<", row="<<row<<", token="<<token<<"\n";
          is_dependent_var = true;
          break;
        } else if (column > dependent_varIDs[i]) {
          --column_x;
        }
      }
      if (!is_dependent_var) {
        set_x(column_x, row, token, error);
        //std::cout<<"setting x with column_x="<<column_x<<", row="<<row<<", token="<<token<<"\n";
      }
      ++column;
    }
    ++row;
  }
  num_rows = row;
  return error;
}
// #nocov end

void Data::getAllValues(std::vector<double>& all_values, std::vector<size_t>& sampleIDs, size_t varID, size_t start,
    size_t end) const {

  // All values for varID (no duplicates) for given sampleIDs
  if (getUnpermutedVarID(varID) < num_cols_no_snp) {

    all_values.reserve(end - start);
    for (size_t pos = start; pos < end; ++pos) {
      all_values.push_back(get_x(sampleIDs[pos], varID));
    }
    std::sort(all_values.begin(), all_values.end());
    all_values.erase(std::unique(all_values.begin(), all_values.end()), all_values.end());
  } else {
    // If GWA data just use 0, 1, 2
    all_values = std::vector<double>( { 0, 1, 2 });
  }
}

void Data::getMinMaxValues(double& min, double&max, std::vector<size_t>& sampleIDs, size_t varID, size_t start,
    size_t end) const {
  if (sampleIDs.size() > 0) {
    min = get_x(sampleIDs[start], varID);
    max = min;
  }
  for (size_t pos = start; pos < end; ++pos) {
    double value = get_x(sampleIDs[pos], varID);
    if (value < min) {
      min = value;
    }
    if (value > max) {
      max = value;
    }
  }
}

void Data::sort() {

  // Reserve memory
  index_data.resize(num_cols_no_snp * num_rows);

  // For all columns, get unique values and save index for each observation
  for (size_t col = 0; col < num_cols_no_snp; ++col) {

    // Get all unique values
    std::vector<double> unique_values(num_rows);
    for (size_t row = 0; row < num_rows; ++row) {
      unique_values[row] = get_x(row, col);
    }
    std::sort(unique_values.begin(), unique_values.end());
    unique_values.erase(unique(unique_values.begin(), unique_values.end()), unique_values.end());

    // Get index of unique value
    for (size_t row = 0; row < num_rows; ++row) {
      size_t idx = std::lower_bound(unique_values.begin(), unique_values.end(), get_x(row, col))
          - unique_values.begin();
      index_data[col * num_rows + row] = idx;
    }

    // Save unique values
    unique_data_values.push_back(unique_values);
    if (unique_values.size() > max_num_unique_values) {
      max_num_unique_values = unique_values.size();
    }
  }
}

// TODO: Implement ordering for multiclass and survival
// #nocov start (cannot be tested anymore because GenABEL not on CRAN)
void Data::orderSnpLevels(bool corrected_importance) {
  // Stop if now SNP data
  if (snp_data == 0) {
    return;
  }

  size_t num_snps;
  if (corrected_importance) {
    num_snps = 2 * (num_cols - num_cols_no_snp);
  } else {
    num_snps = num_cols - num_cols_no_snp;
  }

  // Reserve space
  snp_order.resize(num_snps, std::vector<size_t>(3));

  // For each SNP
  for (size_t i = 0; i < num_snps; ++i) {
    size_t col = i;
    if (i >= (num_cols - num_cols_no_snp)) {
      // Get unpermuted SNP ID
      col = i - num_cols + num_cols_no_snp;
    }

    // Order by mean response
    std::vector<double> means(3, 0);
    std::vector<double> counts(3, 0);
    for (size_t row = 0; row < num_rows; ++row) {
      size_t row_permuted = row;
      if (i >= (num_cols - num_cols_no_snp)) {
        row_permuted = getPermutedSampleID(row);
      }
      size_t idx = col * num_rows_rounded + row_permuted;
      size_t value = (((snp_data[idx / 4] & mask[idx % 4]) >> offset[idx % 4]) - 1);

      // TODO: Better way to treat missing values?
      if (value > 2) {
        value = 0;
      }

      means[value] += get_y(row, 0);
      ++counts[value];
    }

    for (size_t value = 0; value < 3; ++value) {
      means[value] /= counts[value];
    }

    // Save order
    snp_order[i] = order(means, false);
  }

  order_snps = true;
}
// #nocov end

} // namespace ranger
