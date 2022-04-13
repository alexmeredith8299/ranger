#include <dirent.h>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
  //Loop over directory to get file extensions (test example)
  std::string dirpath = "../train";
  std::string goal_dirpath = "../train_png_test";
  std::string goal_maskpath = "../train_mask_png_test";
  DIR *dirp;
  struct dirent* dent;
  dirp=opendir(dirpath.c_str());
  do {
      dent = readdir(dirp);
      if (dent)
      {
        std::string fname = dent->d_name;
        if(fname!=".." && fname!=".") {
          //std::cout<<"File name is: "<<fname<<"\n";
          std::string extension = fname.substr(fname.find_last_of(".") + 1);
          //std::cout<<"Extension is: "<<extension<<"\n";
          if(extension=="tif") {
            std::string prefix = fname.substr(0, fname.find_last_of("."));
            std::string postfix = prefix.substr(prefix.find_last_of("_")+1);
            if(postfix=="rgb") {
              std::cout<<"file exists with name: "<<fname<<"\n";
              std::string cmd = "magick "+dirpath+"/"+fname+" "+goal_dirpath+"/"+prefix+"."+"png";
              std::cout<<"cmd = "<<cmd<<"\n";
              system(cmd.c_str());
            } else if(postfix=="ref") {
              std::string base_fname = prefix.substr(0, prefix.find_last_of("_"));
              std::cout<<"mask exists with name: "<<fname<<"\n";
              std::string mask_cmd = "magick "+dirpath+"/"+fname+" "+goal_maskpath+"/"+base_fname+"_rgb.png";
              std::cout<<"cmd = "<<mask_cmd<<"\n";
              system(mask_cmd.c_str());
            }
          }
        }
      }
  } while (dent);
  closedir(dirp);
}
